import type { CameraOptions, SurfaceState } from 'libmaplibre_native_ohos.so';
import { parseAttributions, type AttributionItem } from './Attribution';

export interface MapStatus {
  text: string;
  detail: string;
  attributions: AttributionItem[];
}

function formatNumber(value: number, digits: number): string {
  if (!Number.isFinite(value)) {
    return 'n/a';
  }
  return value.toFixed(digits);
}

function cameraSummary(camera: CameraOptions): string {
  const latitude = camera.latitude ?? camera.center?.latitude;
  const longitude = camera.longitude ?? camera.center?.longitude;
  let summary = '';
  if (latitude !== undefined && longitude !== undefined) {
    summary = summary + ' cam=' + formatNumber(latitude, 4) + ',' + formatNumber(longitude, 4);
  }
  if (camera.zoom !== undefined) {
    summary = summary + ' z=' + formatNumber(camera.zoom, 2);
  }
  return summary;
}

function statusText(label: string, state: SurfaceState, pixelRatio: number): string {
  return label + ': ' + state.width.toString() + 'x' + state.height.toString() +
    ' pr=' + formatNumber(pixelRatio, 2) +
    ' ' + (state.backend ?? 'unknown') +
    ' fps=' + formatNumber(state.renderedFrameRate, 1) +
    ' tick=' + formatNumber(state.frameCallbackRate, 1) +
    ' render=' + (state.needsRender ? 'pending' : 'idle');
}

function statusDetail(state: SurfaceState, camera: CameraOptions): string {
  if (state.lastSurfaceError !== undefined) {
    return 'surface error: ' + state.lastSurfaceError;
  }
  if (state.lastMapLoadError !== undefined) {
    return 'map error: ' + state.lastMapLoadError;
  }
  if (state.lastRenderError !== undefined) {
    return 'render error: ' + state.lastRenderError;
  }
  if (state.lastGlyphsError !== undefined) {
    return 'glyph error: ' + state.lastGlyphsError;
  }
  if (state.lastSpritesError !== undefined) {
    return 'sprite error: ' + state.lastSpritesError;
  }
  if (state.lastStyleImageMissing !== undefined) {
    return 'missing image: ' + state.lastStyleImageMissing;
  }

  return 'window=' + (state.hasWindow ? 'yes' : 'no') +
    ' map=' + (state.hasMap ? 'yes' : 'no') +
    ' visible=' + (state.surfaceVisible ? 'yes' : 'no') +
    ' style=' + (state.styleLoaded ? 'loaded' : 'pending') +
    ' loaded=' + (state.mapLoaded ? 'yes' : 'no') +
    ' full=' + (state.fullyLoaded ? 'yes' : 'no') +
    cameraSummary(camera);
}

export function createMapStatus(label: string,
  state: SurfaceState,
  camera: CameraOptions,
  pixelRatio: number,
  attributions: string[]): MapStatus {
  return {
    text: statusText(label, state, pixelRatio),
    detail: statusDetail(state, camera),
    attributions: parseAttributions(attributions)
  };
}
