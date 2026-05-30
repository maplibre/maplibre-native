import { common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import maplibreNative from 'libmaplibre_native_ohos.so';
import type { CameraOptions, ResourceOptions, SurfaceState, XComponentContext } from 'libmaplibre_native_ohos.so';

const LOG_DOMAIN: number = 0x4d4c4e;
const LOG_TAG: string = 'MapLibreSample';
const DIAG_STYLE_JSON: string =
  '{"version":8,"sources":{},"layers":[{"id":"bg","type":"background","paint":{"background-color":"#ff0000"}}]}';
const LOCAL_VECTOR_STYLE_JSON: string =
  '{"version":8,"center":[0,0],"zoom":1,"pitch":0,"bearing":0,"sources":{"test":{"type":"geojson","data":{"type":"Feature","geometry":{"type":"Polygon","coordinates":[[[-30,-20],[30,-20],[30,20],[-30,20],[-30,-20]]]}}}},"layers":[{"id":"bg","type":"background","paint":{"background-color":"#004488"}},{"id":"fill","type":"fill","source":"test","paint":{"fill-color":"#ffdd00"}}]}';

const EMPTY_STYLE_JSON: string = '{"version":8,"sources":{},"layers":[]}';
const REMOTE_STYLE_URL: string = 'https://demotiles.maplibre.org/style.json';

@Entry
@Component
struct Index {
  @State private statusText: string = 'Waiting for XComponent';
  @State private statusDetail: string = 'No native binding';
  @State private statusResources: string = 'No resource callbacks';

  private map?: XComponentContext;
  private statusLogTimer: number = -1;

  aboutToDisappear(): void {
    this.stopStatusLogging();
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
      this.map.runLoopOnce();
      this.updateSurfaceState('Visible');
    }
    this.startStatusLogging();
  }

  onPageHide(): void {
    this.stopStatusLogging();
    if (!this.map) {
      return;
    }

    this.map.setRenderingEnabled(false);
    this.map.reduceMemoryUse();
    this.updateSurfaceState('Hidden');
  }

  private startStatusLogging(): void {
    this.stopStatusLogging();
    this.statusLogTimer = setInterval(() => {
      this.logSurfaceState('tick');
    }, 3000);
  }

  private stopStatusLogging(): void {
    if (this.statusLogTimer >= 0) {
      clearInterval(this.statusLogTimer);
      this.statusLogTimer = -1;
    }
  }

  private formatSurfaceStateLine(state: SurfaceState, camera: CameraOptions, label: string): string {
    let line: string = label + ': ' + state.width.toString() + 'x' + state.height.toString() +
      ' window=' + (state.hasWindow ? 'yes' : 'no') +
      ' map=' + (state.hasMap ? 'yes' : 'no') +
      ' render=' + (state.needsRender ? 'pending' : 'idle') +
      ' frames=' + state.renderedFrameCount.toString() + '/' + state.coreFrameCount.toString() +
      ' gl=' + state.glesContextClientVersion.toString() +
      ' cb=' + state.frameCallbackCount.toString() +
      '/' + state.touchEventCount.toString() +
      '/' + state.gestureHandledCount.toString() +
      ' surface=' + state.surfaceCreatedCount.toString() +
      '/' + state.surfaceChangedCount.toString() +
      '/' + state.surfaceDestroyedCount.toString() +
      ' visible=' + (state.surfaceVisible ? 'yes' : 'no') +
      ' show=' + state.surfaceShownCount.toString() +
      '/' + state.surfaceHiddenCount.toString() +
      ' style=' + (state.styleLoaded ? 'loaded' : 'pending') +
      ' loaded=' + (state.mapLoaded ? 'yes' : 'no') +
      ' idle=' + (state.idle ? 'yes' : 'no') +
      ' full=' + (state.fullyLoaded ? 'yes' : 'no') +
      ' repaint=' + (state.lastFrameNeededRepaint ? 'yes' : 'no') +
      ' complete=' + (state.lastFrameComplete ? 'yes' : 'no') +
      ' glyphs=' + state.glyphsRequestedCount.toString() +
      '/' + state.glyphsLoadedCount.toString() +
      '/' + state.glyphsErrorCount.toString() +
      ' sprites=' + state.spritesRequestedCount.toString() +
      '/' + state.spritesLoadedCount.toString() +
      '/' + state.spritesErrorCount.toString() +
      ' tiles=' + state.tileActionCount.toString() +
      ' src=' + state.sourceChangedCount.toString() +
      ' missing=' + state.styleImageMissingCount.toString();
    if (camera.latitude !== undefined && camera.longitude !== undefined) {
      line = line + ' cam=' + camera.latitude.toString() + ',' + camera.longitude.toString();
    } else if (camera.center !== undefined) {
      line = line + ' cam=' + camera.center.latitude.toString() + ',' + camera.center.longitude.toString();
    }
    if (camera.zoom !== undefined) {
      line = line + ' z=' + camera.zoom.toString();
    }
    if (camera.pitch !== undefined) {
      line = line + ' p=' + camera.pitch.toString();
    }
    if (camera.bearing !== undefined) {
      line = line + ' b=' + camera.bearing.toString();
    }
    if (state.lastSurfaceError !== undefined) {
      line = line + ' surfaceErr=' + state.lastSurfaceError;
    }
    if (state.lastMapLoadError !== undefined) {
      line = line + ' mapErr=' + state.lastMapLoadError;
    }
    if (state.lastRenderError !== undefined) {
      line = line + ' renderErr=' + state.lastRenderError;
    }
    if (state.lastSpritesError !== undefined) {
      line = line + ' spriteErr=' + state.lastSpritesError;
    }
    if (state.lastGlyphsError !== undefined) {
      line = line + ' glyphErr=' + state.lastGlyphsError;
    }
    return line;
  }

  private logSurfaceState(label: string): void {
    if (!this.map) {
      hilog.info(LOG_DOMAIN, LOG_TAG, '%{public}s: no map binding', label);
      return;
    }

    const state: SurfaceState = this.map.getSurfaceState();
    const camera: CameraOptions = this.map.getCameraOptions();
    const line: string = this.formatSurfaceStateLine(state, camera, label);
    hilog.info(LOG_DOMAIN, LOG_TAG, '%{public}s', line);
    this.map.logSurfaceState(label);
  }

  private updateSurfaceState(label: string): void {
    if (!this.map) {
      this.statusText = label + ': no binding';
      this.statusDetail = 'No native binding';
      this.statusResources = 'No resource callbacks';
      return;
    }

    const state: SurfaceState = this.map.getSurfaceState();
    this.statusText = label + ': ' + state.width.toString() + 'x' + state.height.toString() +
      ' window=' + (state.hasWindow ? 'yes' : 'no') +
      ' map=' + (state.hasMap ? 'yes' : 'no') +
      ' render=' + (state.needsRender ? 'pending' : 'idle');

    let detail: string = 'frames=' + state.renderedFrameCount.toString() +
      '/' + state.coreFrameCount.toString() +
      ' gl=' + state.glesContextClientVersion.toString() +
      ' cb=' + state.frameCallbackCount.toString() +
      '/' + state.touchEventCount.toString() +
      '/' + state.gestureHandledCount.toString() +
      ' surface=' + state.surfaceCreatedCount.toString() +
      '/' + state.surfaceChangedCount.toString() +
      '/' + state.surfaceDestroyedCount.toString() +
      ' visible=' + (state.surfaceVisible ? 'yes' : 'no') +
      ' show=' + state.surfaceShownCount.toString() +
      '/' + state.surfaceHiddenCount.toString() +
      ' style=' + (state.styleLoaded ? 'loaded' : 'pending') +
      ' loaded=' + (state.mapLoaded ? 'yes' : 'no') +
      ' idle=' + (state.idle ? 'yes' : 'no');
    if (state.lastSurfaceError !== undefined) {
      detail = 'surface error: ' + state.lastSurfaceError;
    } else if (state.lastMapLoadError !== undefined) {
      detail = 'map error: ' + state.lastMapLoadError;
    } else if (state.lastRenderError !== undefined) {
      detail = 'render error: ' + state.lastRenderError;
    }
    this.statusDetail = detail;
    let resources: string = 'glyphs=' + state.glyphsRequestedCount.toString() +
      '/' + state.glyphsLoadedCount.toString() +
      '/' + state.glyphsErrorCount.toString() +
      ' sprites=' + state.spritesRequestedCount.toString() +
      '/' + state.spritesLoadedCount.toString() +
      '/' + state.spritesErrorCount.toString() +
      ' tiles=' + state.tileActionCount.toString() +
      ' src=' + state.sourceChangedCount.toString() +
      ' missing=' + state.styleImageMissingCount.toString();
    if (state.lastGlyphsError !== undefined) {
      resources = 'glyph error: ' + state.lastGlyphsError;
    } else if (state.lastSpritesError !== undefined) {
      resources = 'sprite error: ' + state.lastSpritesError;
    } else if (state.lastStyleImageMissing !== undefined) {
      resources = 'missing image: ' + state.lastStyleImageMissing;
    }
    this.statusResources = resources;
  }

  private useDiagStyle(): void {
    if (!this.map) {
      return;
    }

    this.map.setStyleJson(DIAG_STYLE_JSON);
    this.map.jumpTo({ center: { longitude: 0, latitude: 0 }, zoom: 1, bearing: 0, pitch: 0 });
    this.map.renderFrame();
    this.updateSurfaceState('Diag red');
  }

  private useEmptyStyle(): void {
    if (!this.map) {
      return;
    }

    this.map.setStyleJson(EMPTY_STYLE_JSON);
    this.map.renderFrame();
    this.updateSurfaceState('Empty style');
  }

  private useLocalVectorStyle(): void {
    if (!this.map) {
      return;
    }

    this.map.setStyleJson(LOCAL_VECTOR_STYLE_JSON);
    this.map.jumpTo({ center: { longitude: 0, latitude: 0 }, zoom: 1, bearing: 0, pitch: 0 });
    this.map.renderFrame();
    this.updateSurfaceState('Local vector');
  }

  private useRemoteStyle(): void {
    if (!this.map) {
      return;
    }

    this.statusText = 'Remote: loading…';
    this.map.setStyleUrl(REMOTE_STYLE_URL);
    setTimeout(() => {
      if (this.map) {
        this.map.runLoopOnce();
        this.map.renderFrame();
        this.updateSurfaceState('Remote style');
      }
    }, 500);
  }

  private resourceOptions(): ResourceOptions {
    const hostContext = this.getUIContext().getHostContext() as common.Context | undefined;
    if (!hostContext) {
      return {};
    }

    return { cachePath: hostContext.cacheDir + '/maplibre-cache.db' };
  }

  private configureMap(map: XComponentContext): void {
    this.map = map;
    map.setClientOptions('MapLibreOhosSample', '0.1.0');
    map.setResourceOptions(this.resourceOptions());
    map.setTileCacheEnabled(true);
    map.setFrameRateRange({ min: 30, max: 120, expected: 60 });
    map.setPixelRatio(1);
    map.setRenderingEnabled(true);
    map.setStyleJson(EMPTY_STYLE_JSON);
    map.setBounds({
      bounds: { west: -180, south: -85, east: 180, north: 85 },
      minZoom: 0,
      maxZoom: 22,
      minPitch: 0,
      maxPitch: 60
    });
    // Debug overlays (ParseStatus tile labels) are useful on desktop but render poorly
    // on the current MatePad GLES path; leave them off for first device validation.
    map.setDebugOptions(0);
    map.getBounds();
    map.getCameraOptions();
    map.getClientOptions();
    map.setFreeCameraOptions(map.getFreeCameraOptions());
    map.getDebugOptions();
    map.getFrameRateRange();
    map.getPixelRatio();
    map.getRenderingEnabled();
    map.getResourceOptions();
    map.getStyleJson();
    map.getStyleUrl();
    const surfaceState: SurfaceState = map.getSurfaceState();
    const observedCoreCallbacks: boolean =
      surfaceState.styleLoaded || surfaceState.mapLoaded || surfaceState.coreFrameCount > 0;
    const observedResourceCallbacks: boolean =
      surfaceState.glyphsRequestedCount > 0 ||
        surfaceState.spritesRequestedCount > 0 ||
        surfaceState.tileActionCount > 0 ||
        surfaceState.styleImageMissingCount > 0;
    const observedErrors: boolean =
        surfaceState.lastSurfaceError !== undefined ||
        surfaceState.lastMapLoadError !== undefined ||
        surfaceState.lastRenderError !== undefined ||
        surfaceState.lastGlyphsError !== undefined ||
        surfaceState.lastSpritesError !== undefined;
    if (surfaceState.needsRender &&
      (surfaceState.renderedFrameCount === 0 ||
        observedCoreCallbacks ||
        observedResourceCallbacks ||
        observedErrors)) {
      map.runLoopOnce();
    }
    map.getTileCacheEnabled();
    this.updateSurfaceState('Configured');
  }

  build() {
    Column() {
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
          this.statusResources = 'No resource callbacks';
        })
        .width('100%')
        .layoutWeight(1)

      Column() {
        Text(this.statusText)
          .fontSize(13)
          .fontWeight(FontWeight.Medium)
          .fontColor('#111827')
          .maxLines(2)
          .width('100%')

        Text(this.statusDetail)
          .fontSize(12)
          .fontColor('#475569')
          .maxLines(2)
          .width('100%')
          .margin({ top: 2 })

        Text(this.statusResources)
          .fontSize(11)
          .fontColor('#64748b')
          .maxLines(2)
          .width('100%')
          .margin({ top: 2 })
      }
      .width('100%')
      .padding({ left: 12, right: 12, top: 8, bottom: 6 })
      .backgroundColor('#f8fafc')

      Row() {
        Button('Render')
          .onClick(() => {
            if (!this.map) {
              return;
            }

            const state: SurfaceState = this.map.getSurfaceState();
            if (!state.hasMap) {
              this.statusText = 'Render blocked: no map surface';
              this.statusDetail = state.lastSurfaceError !== undefined ?
                'surface error: ' + state.lastSurfaceError :
                'Check EGL/native window setup in logs';
              return;
            }

            this.map.fitBounds({
              bounds: { west: -10, south: -10, east: 10, north: 10 },
              padding: { top: 24, left: 24, bottom: 24, right: 24 }
            });
            this.map.renderFrame();
            this.updateSurfaceState('Rendered');
          })
          .height(44)

        Button('Remote')
          .onClick(() => {
            this.useRemoteStyle();
          })
          .height(44)

        Button('Empty')
          .onClick(() => {
            this.useEmptyStyle();
          })
          .height(44)

        Button('Diag')
          .onClick(() => {
            this.useDiagStyle();
          })
          .height(44)

        Button('Local')
          .onClick(() => {
            this.useLocalVectorStyle();
          })
          .height(44)

        Button('Memory')
          .onClick(() => {
            if (this.map) {
              this.map.reduceMemoryUse();
              this.updateSurfaceState('Memory reduced');
            }
          })
          .height(44)
      }
      .width('100%')
      .padding({ left: 12, right: 12, top: 8, bottom: 8 })
      .justifyContent(FlexAlign.SpaceEvenly)
    }
    .width('100%')
    .height('100%')
  }
}
