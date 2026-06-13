import { common } from '@kit.AbilityKit';
import { display, NodeContent } from '@kit.ArkUI';
import { hilog } from '@kit.PerformanceAnalysisKit';
import maplibreNative from 'libmaplibre_native_ohos.so';
import type { CameraOptions, ResourceOptions, SurfaceState, XComponentContext } from 'libmaplibre_native_ohos.so';
import type { AttributionItem } from '../map/Attribution';
import { createMapStatus } from '../map/MapStatus';
import {
  CLIENT_NAME,
  CLIENT_VERSION,
  DEFAULT_BOUNDS,
  DEFAULT_CAMERA,
  DEFAULT_STYLE,
  LOG_DOMAIN,
  LOG_TAG,
  STYLE_OPTIONS,
  type StyleOption
} from '../map/SampleConfig';

@Entry
@Component
struct Index {
  @State private statusText: string = 'Waiting for XComponent';
  @State private statusDetail: string = 'No native binding';
  @State private attributionItems: AttributionItem[] = [];

  private map?: XComponentContext;
  private statusTimer: number = -1;
  private readonly mapNodeContent: NodeContent = new NodeContent();

  aboutToDisappear(): void {
    this.stopStatusUpdates();
    this.destroyMap();
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
      this.clearStatus();
      return;
    }

    const state: SurfaceState = this.map.getSurfaceState();
    const camera: CameraOptions = this.map.getCameraOptions();
    const pixelRatio = this.map.getPixelRatio();
    const status = createMapStatus(label, state, camera, pixelRatio, this.map.getStyleAttributions());
    this.statusText = status.text;
    this.statusDetail = status.detail;
    this.attributionItems = status.attributions;
  }

  private clearStatus(): void {
    this.statusText = 'No native binding';
    this.statusDetail = 'Waiting for XComponent';
    this.attributionItems = [];
  }

  private destroyMap(): void {
    if (!this.map) {
      return;
    }

    this.map.setRenderingEnabled(false);
    this.map.reduceMemoryUse();
    this.map.destroy();
    this.map = undefined;
  }

  private describeError(error: Error): string {
    if (error.message) {
      return error.message;
    }

    const serialized = JSON.stringify(error);
    if (serialized && serialized !== '{}') {
      return serialized;
    }

    return String(error);
  }

  private detachMap(): void {
    this.destroyMap();
    this.statusText = 'XComponent detached';
    this.statusDetail = 'No native binding';
    this.attributionItems = [];
  }

  private attachMap(): void {
    this.destroyMap();

    try {
      this.configureMap(maplibreNative.createMap(this.mapNodeContent));
    } catch (error) {
      const detail = this.describeError(error as Error);
      this.statusText = 'Could not create native map';
      this.statusDetail = detail;
      this.attributionItems = [];
      hilog.error(LOG_DOMAIN, LOG_TAG, 'Could not create native map: %{public}s', detail);
    }
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

  private useStyle(style: StyleOption): void {
    if (!this.map) {
      return;
    }

    this.statusText = style.name + ': loading...';
    this.map.setStyleUrl(style.url);
    this.map.renderFrame();
    this.updateSurfaceState(style.name);
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
    map.setClientOptions(CLIENT_NAME, CLIENT_VERSION);
    map.setResourceOptions(this.resourceOptions());
    map.setTileCacheEnabled(true);
    const refreshRate = this.displayRefreshRate();
    map.setFrameRateRange({ min: Math.min(30, refreshRate), max: refreshRate, expected: refreshRate });
    map.setPixelRatio(this.displayPixelRatio());
    map.setRenderingEnabled(true);
    map.setBounds(DEFAULT_BOUNDS);
    map.jumpTo(DEFAULT_CAMERA);
    map.renderFrame();
    this.useStyle(DEFAULT_STYLE);
  }

  build() {
    Column() {
      Stack({ alignContent: Alignment.TopStart }) {
        Stack() {
          ContentSlot(this.mapNodeContent)
        }
          .onAttach(() => {
            this.attachMap();
          })
          .onDetach(() => {
            this.detachMap();
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
        ForEach(STYLE_OPTIONS, (style: StyleOption) => {
          Button(style.name)
            .onClick(() => {
              this.useStyle(style);
            })
            .height(44)
        }, (style: StyleOption) => style.name)
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
