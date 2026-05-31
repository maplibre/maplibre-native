import { common } from '@kit.AbilityKit';
import { display } from '@kit.ArkUI';
import { hilog } from '@kit.PerformanceAnalysisKit';
import maplibreNative from 'libmaplibre_native_ohos.so';
import type { CameraOptions, ResourceOptions, SurfaceState, XComponentContext } from 'libmaplibre_native_ohos.so';

const LOG_DOMAIN: number = 0x4d4c4e;
const LOG_TAG: string = 'MapLibreSample';
const DEMO_STYLE_URL: string = 'https://demotiles.maplibre.org/style.json';
const BRIGHT_STYLE_URL: string = 'https://tiles.openfreemap.org/styles/bright';
const LIBERTY_STYLE_URL: string = 'https://tiles.openfreemap.org/styles/liberty';
const DEFAULT_STYLE_NAME: string = 'Bright';
const DEFAULT_STYLE_URL: string = BRIGHT_STYLE_URL;
const DEFAULT_CAMERA: CameraOptions = {
  longitude: 106.5750,
  latitude: 29.5570,
  zoom: 12.0,
  bearing: 0,
  pitch: 0
};

interface AttributionItem {
  key: string;
  label: string;
  url?: string;
}

@Entry
@Component
struct Index {
  @State private statusText: string = 'Waiting for XComponent';
  @State private statusDetail: string = 'No native binding';
  @State private attributionItems: AttributionItem[] = [];

  private map?: XComponentContext;
  private statusTimer: number = -1;

  aboutToDisappear(): void {
    this.stopStatusUpdates();
    if (this.map) {
      this.map.setRenderingEnabled(false);
      this.map.reduceMemoryUse();
      this.map.destroy();
      this.map = undefined;
    }
  }

  onPageShow(): void {
    if (this.map) {
      this.map.setRenderingEnabled(true);
      this.map.renderFrame();
      this.updateSurfaceState('Visible');
    }
    this.startStatusUpdates();
  }

  onPageHide(): void {
    this.stopStatusUpdates();
    if (!this.map) {
      return;
    }

    this.map.setRenderingEnabled(false);
    this.map.reduceMemoryUse();
    this.updateSurfaceState('Hidden');
  }

  private startStatusUpdates(): void {
    this.stopStatusUpdates();
    this.statusTimer = setInterval(() => {
      if (this.map) {
        this.map.renderFrame();
        this.updateSurfaceState('Live');
      }
    }, 500);
  }

  private stopStatusUpdates(): void {
    if (this.statusTimer >= 0) {
      clearInterval(this.statusTimer);
      this.statusTimer = -1;
    }
  }

  private updateSurfaceState(label: string): void {
    if (!this.map) {
      this.statusText = 'No native binding';
      this.statusDetail = 'Waiting for XComponent';
      this.attributionItems = [];
      return;
    }

    const state: SurfaceState = this.map.getSurfaceState();
    const camera: CameraOptions = this.map.getCameraOptions();
    const pixelRatio = this.map.getPixelRatio();

    this.statusText = label + ': ' + state.width.toString() + 'x' + state.height.toString() +
      ' pr=' + this.formatNumber(pixelRatio, 2) +
      ' ' + (state.backend ?? 'unknown') +
      ' fps=' + this.formatNumber(state.renderedFrameRate, 1) +
      ' tick=' + this.formatNumber(state.frameCallbackRate, 1) +
      ' render=' + (state.needsRender ? 'pending' : 'idle');

    if (state.lastSurfaceError !== undefined) {
      this.statusDetail = 'surface error: ' + state.lastSurfaceError;
    } else if (state.lastMapLoadError !== undefined) {
      this.statusDetail = 'map error: ' + state.lastMapLoadError;
    } else if (state.lastRenderError !== undefined) {
      this.statusDetail = 'render error: ' + state.lastRenderError;
    } else if (state.lastGlyphsError !== undefined) {
      this.statusDetail = 'glyph error: ' + state.lastGlyphsError;
    } else if (state.lastSpritesError !== undefined) {
      this.statusDetail = 'sprite error: ' + state.lastSpritesError;
    } else if (state.lastStyleImageMissing !== undefined) {
      this.statusDetail = 'missing image: ' + state.lastStyleImageMissing;
    } else {
      this.statusDetail = 'window=' + (state.hasWindow ? 'yes' : 'no') +
        ' map=' + (state.hasMap ? 'yes' : 'no') +
        ' visible=' + (state.surfaceVisible ? 'yes' : 'no') +
        ' style=' + (state.styleLoaded ? 'loaded' : 'pending') +
        ' loaded=' + (state.mapLoaded ? 'yes' : 'no') +
        ' full=' + (state.fullyLoaded ? 'yes' : 'no') +
        this.cameraSummary(camera);
    }

    this.attributionItems = this.parseAttributions(this.map.getStyleAttributions());
  }

  private cameraSummary(camera: CameraOptions): string {
    const latitude = camera.latitude ?? camera.center?.latitude;
    const longitude = camera.longitude ?? camera.center?.longitude;
    let summary = '';
    if (latitude !== undefined && longitude !== undefined) {
      summary = summary + ' cam=' + this.formatNumber(latitude, 4) + ',' + this.formatNumber(longitude, 4);
    }
    if (camera.zoom !== undefined) {
      summary = summary + ' z=' + this.formatNumber(camera.zoom, 2);
    }
    return summary;
  }

  private formatNumber(value: number, digits: number): string {
    if (!Number.isFinite(value)) {
      return 'n/a';
    }
    return value.toFixed(digits);
  }

  private decodeHtmlEntities(value: string): string {
    return value
      .replace(/&copy;/g, '(c)')
      .replace(/&amp;/g, '&')
      .replace(/&lt;/g, '<')
      .replace(/&gt;/g, '>')
      .replace(/&quot;/g, '"')
      .replace(/&#39;/g, "'")
      .replace(/\s+/g, ' ')
      .trim();
  }

  private stripAttributionHtml(value: string): string {
    return this.decodeHtmlEntities(value.replace(/<[^>]*>/g, ''));
  }

  private appendAttributionItem(items: AttributionItem[], label: string, url?: string): void {
    const trimmed = label.replace(/\s+/g, ' ').trim();
    if (trimmed.length === 0) {
      return;
    }

    items.push({
      key: items.length.toString() + ':' + trimmed + ':' + (url ?? ''),
      label: trimmed,
      url: url
    });
  }

  private appendAttributionHtml(items: AttributionItem[], value: string): void {
    const anchorPattern: RegExp = /<a[^>]*href=["']([^"']+)["'][^>]*>(.*?)<\/a>/g;
    let cursor = 0;
    let match: RegExpExecArray | null = anchorPattern.exec(value);
    while (match !== null) {
      if (match.index > cursor) {
        this.appendAttributionItem(items, this.stripAttributionHtml(value.slice(cursor, match.index)));
      }

      this.appendAttributionItem(items, this.stripAttributionHtml(match[2]), this.decodeHtmlEntities(match[1]));
      cursor = match.index + match[0].length;
      match = anchorPattern.exec(value);
    }

    if (cursor < value.length) {
      this.appendAttributionItem(items, this.stripAttributionHtml(value.slice(cursor)));
    }
  }

  private parseAttributions(values: string[]): AttributionItem[] {
    const items: AttributionItem[] = [];
    const seen: string[] = [];
    values.forEach((value: string) => {
      const trimmed = value.trim();
      if (trimmed.length === 0 || seen.indexOf(trimmed) >= 0) {
        return;
      }

      if (items.length > 0) {
        this.appendAttributionItem(items, '|');
      }
      seen.push(trimmed);
      this.appendAttributionHtml(items, trimmed);
    });
    return items;
  }

  private openAttributionUrl(url: string): void {
    const hostContext = this.getUIContext().getHostContext() as common.UIAbilityContext | undefined;
    if (!hostContext) {
      return;
    }

    hostContext.startAbility({
      action: 'ohos.want.action.viewData',
      uri: url
    }).catch((error: Error) => {
      hilog.warn(LOG_DOMAIN, LOG_TAG, 'Could not open attribution URL: %{public}s', JSON.stringify(error));
    });
  }

  private useRemoteStyle(name: string, url: string): void {
    if (!this.map) {
      return;
    }

    this.statusText = name + ': loading...';
    this.map.setStyleUrl(url);
    this.map.renderFrame();
    this.updateSurfaceState(name);
  }

  private resourceOptions(): ResourceOptions {
    const hostContext = this.getUIContext().getHostContext() as common.Context | undefined;
    if (!hostContext) {
      return {};
    }

    return { cachePath: hostContext.cacheDir + '/maplibre-cache.db' };
  }

  private displayPixelRatio(): number {
    try {
      const defaultDisplay = display.getDefaultDisplaySync();
      const pixelRatio = defaultDisplay.densityPixels;
      if (Number.isFinite(pixelRatio) && pixelRatio > 0) {
        return pixelRatio;
      }
    } catch (error) {
      hilog.warn(LOG_DOMAIN, LOG_TAG, 'Could not read display density: %{public}s', JSON.stringify(error));
    }

    return 1;
  }

  private displayRefreshRate(): number {
    try {
      const defaultDisplay = display.getDefaultDisplaySync();
      const refreshRate = Math.round(defaultDisplay.refreshRate);
      if (Number.isFinite(refreshRate) && refreshRate > 0) {
        return refreshRate;
      }
    } catch (error) {
      hilog.warn(LOG_DOMAIN, LOG_TAG, 'Could not read display refresh rate: %{public}s', JSON.stringify(error));
    }

    return 60;
  }

  private configureMap(map: XComponentContext): void {
    this.map = map;
    map.setClientOptions('MapLibreOhosSample', '0.1.0');
    map.setResourceOptions(this.resourceOptions());
    map.setTileCacheEnabled(true);
    const refreshRate = this.displayRefreshRate();
    map.setFrameRateRange({ min: Math.min(30, refreshRate), max: refreshRate, expected: refreshRate });
    map.setPixelRatio(this.displayPixelRatio());
    map.setRenderingEnabled(true);
    map.setBounds({
      bounds: { west: -180, south: -85, east: 180, north: 85 },
      minZoom: 0,
      maxZoom: 22,
      minPitch: 0,
      maxPitch: 60
    });
    map.setDebugOptions(0);
    map.jumpTo(DEFAULT_CAMERA);
    map.renderFrame();
    this.useRemoteStyle(DEFAULT_STYLE_NAME, DEFAULT_STYLE_URL);
  }

  build() {
    Column() {
      Stack({ alignContent: Alignment.TopStart }) {
        XComponent({
          id: 'maplibre_map',
          type: XComponentType.SURFACE,
          libraryname: 'maplibre_native_ohos'
        })
          .onLoad((context?: object) => {
            if (!context) {
              this.statusText = 'XComponent onLoad returned no context';
              return;
            }

            this.configureMap(context as XComponentContext);
          })
          .onDestroy(() => {
            if (this.map) {
              this.map.destroy();
              this.map = undefined;
            }
            this.statusText = 'XComponent destroyed';
            this.statusDetail = 'No native binding';
            this.attributionItems = [];
          })
          .width('100%')
          .height('100%')

        Text(this.statusText)
          .fontSize(10)
          .fontColor('#111827')
          .maxLines(1)
          .textShadow({ radius: 2, color: '#ffffff', offsetX: 0, offsetY: 0 })
          .margin({ left: 8, top: 8 })
          .hitTestBehavior(HitTestMode.Transparent)

        Text(this.statusDetail)
          .fontSize(10)
          .fontColor('#111827')
          .maxLines(1)
          .textShadow({ radius: 2, color: '#ffffff', offsetX: 0, offsetY: 0 })
          .margin({ left: 8, top: 24, right: 8 })
          .hitTestBehavior(HitTestMode.Transparent)

        Stack({ alignContent: Alignment.BottomEnd }) {
          Row() {
            ForEach(this.attributionItems, (item: AttributionItem) => {
              Text(item.label)
                .fontSize(10)
                .fontColor(item.url === undefined ? '#111827' : '#0645ad')
                .decoration({
                  type: item.url === undefined ? TextDecorationType.None : TextDecorationType.Underline
                })
                .textShadow({ radius: 2, color: '#ffffff', offsetX: 0, offsetY: 0 })
                .margin({ right: 2 })
                .onClick(() => {
                  if (item.url !== undefined) {
                    this.openAttributionUrl(item.url);
                  }
                })
            }, (item: AttributionItem) => item.key)
          }
          .justifyContent(FlexAlign.End)
          .width('100%')
          .padding({ left: 8, right: 8, bottom: 8 })
          .hitTestBehavior(HitTestMode.Transparent)
        }
        .width('100%')
        .height('100%')
        .hitTestBehavior(HitTestMode.Transparent)
      }
      .width('100%')
      .layoutWeight(1)

      Row() {
        Button('Demo')
          .onClick(() => {
            this.useRemoteStyle('Demo', DEMO_STYLE_URL);
          })
          .height(44)

        Button('Bright')
          .onClick(() => {
            this.useRemoteStyle('Bright', BRIGHT_STYLE_URL);
          })
          .height(44)

        Button('Liberty')
          .onClick(() => {
            this.useRemoteStyle('Liberty', LIBERTY_STYLE_URL);
          })
          .height(44)
      }
      .width('100%')
      .padding({ left: 12, right: 12, top: 8, bottom: 8 })
      .justifyContent(FlexAlign.SpaceEvenly)
      .backgroundColor('#f8fafc')
    }
    .width('100%')
    .height('100%')
  }
}
