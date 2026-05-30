export interface NativeBinding {
  readonly __mapLibreNativeBinding: 'NativeBinding';
}

export interface ResourceOptions {
  apiKey?: string;
  cachePath?: string;
  assetPath?: string;
}

export interface ClientOptions {
  name: string;
  version?: string;
}

export interface LatLngBounds {
  west: number;
  south: number;
  east: number;
  north: number;
}

export interface BoundOptions {
  bounds?: LatLngBounds;
  minZoom?: number;
  maxZoom?: number;
  minPitch?: number;
  maxPitch?: number;
}

export interface CameraBoundsOptions {
  bounds: LatLngBounds;
  padding?: CameraPadding;
  bearing?: number;
  pitch?: number;
}

export interface Coordinate {
  longitude: number;
  latitude: number;
}

export interface CameraPadding {
  top?: number;
  left?: number;
  bottom?: number;
  right?: number;
}

export interface ScreenPoint {
  x: number;
  y: number;
}

export interface Vector3 {
  x: number;
  y: number;
  z: number;
}

export interface Vector4 {
  x: number;
  y: number;
  z: number;
  w: number;
}

export interface CameraOptions {
  longitude?: number;
  latitude?: number;
  center?: Coordinate;
  centerAltitude?: number;
  padding?: CameraPadding;
  anchor?: ScreenPoint;
  zoom?: number;
  bearing?: number;
  pitch?: number;
  roll?: number;
  fov?: number;
}

export interface FreeCameraOptions {
  position?: Vector3;
  orientation?: Vector4;
}

export interface AnimationOptions {
  duration?: number;
  velocity?: number;
  minZoom?: number;
}

export interface FrameRateRange {
  min: number;
  max: number;
  expected: number;
}

export interface SurfaceState {
  width: number;
  height: number;
  hasWindow: boolean;
  hasSurface: boolean;
  hasMap: boolean;
  needsRender: boolean;
  styleLoaded: boolean;
  mapLoaded: boolean;
  fullyLoaded: boolean;
  idle: boolean;
  lastFrameNeededRepaint: boolean;
  lastFrameComplete: boolean;
  renderedFrameCount: number;
  coreFrameCount: number;
  mapLoadErrorCount: number;
  renderErrorCount: number;
  sourceChangedCount: number;
  styleImageMissingCount: number;
  glyphsRequestedCount: number;
  glyphsLoadedCount: number;
  glyphsErrorCount: number;
  tileActionCount: number;
  spritesRequestedCount: number;
  spritesLoadedCount: number;
  spritesErrorCount: number;
  glesContextClientVersion: number;
  eglConfigDiagnostic?: string;
  framebufferDiagnostic?: string;
  frameCallbackCount: number;
  touchEventCount: number;
  gestureHandledCount: number;
  surfaceVisible: boolean;
  surfaceCreatedCount: number;
  surfaceChangedCount: number;
  surfaceDestroyedCount: number;
  surfaceShownCount: number;
  surfaceHiddenCount: number;
  surfaceErrorCount: number;
  lastSurfaceError?: string;
  lastMapLoadError?: string;
  lastRenderError?: string;
  lastStyleImageMissing?: string;
  lastGlyphsError?: string;
  lastSpritesError?: string;
}

export interface DebugOptions {
  readonly NoDebug: number;
  readonly TileBorders: number;
  readonly ParseStatus: number;
  readonly Timestamps: number;
  readonly Collision: number;
  readonly Overdraw: number;
  readonly StencilClip: number;
  readonly DepthBuffer: number;
}

export interface XComponentContext {
  cameraForBounds: (options: CameraBoundsOptions) => CameraOptions;
  destroy: () => void;
  easeTo: (options: CameraOptions, animation?: AnimationOptions) => void;
  fitBounds: (options: CameraBoundsOptions) => void;
  flyTo: (options: CameraOptions, animation?: AnimationOptions) => void;
  getBounds: () => BoundOptions;
  getCameraOptions: () => CameraOptions;
  getClientOptions: () => ClientOptions;
  getDebugOptions: () => number;
  getFreeCameraOptions: () => FreeCameraOptions;
  getFrameRateRange: () => FrameRateRange | undefined;
  getPixelRatio: () => number;
  getRenderingEnabled: () => boolean;
  getResourceOptions: () => ResourceOptions;
  getStyleJson: () => string | undefined;
  getStyleUrl: () => string | undefined;
  getSurfaceState: () => SurfaceState;
  logSurfaceState: (label?: string) => void;
  getTileCacheEnabled: () => boolean;
  jumpTo: {
    (options: CameraOptions): void;
    (longitude: number, latitude: number, zoom?: number, bearing?: number, pitch?: number): void;
  };
  reduceMemoryUse: () => void;
  renderFrame: () => void;
  runLoopOnce: () => void;
  setBounds: (options: BoundOptions) => void;
  setCameraOptions: (options: CameraOptions) => void;
  setClientOptions: (name: string, version?: string) => void;
  setDebugOptions: (debugOptions: number) => void;
  setFrameRateRange: (range: FrameRateRange) => void;
  setFreeCameraOptions: (options: FreeCameraOptions) => void;
  setPixelRatio: (pixelRatio: number) => void;
  setRenderingEnabled: (enabled: boolean) => void;
  setResourceOptions: (options: ResourceOptions) => void;
  setTileCacheEnabled: (enabled: boolean) => void;
  setStyleUrl: (url: string) => void;
  setStyleJson: (json: string) => void;
}

export const DebugOptions: DebugOptions;
export const registerXComponentNode: (node: object) => NativeBinding;
export function cameraForBounds(binding: NativeBinding, options: CameraBoundsOptions): CameraOptions;
export function destroy(binding: NativeBinding): void;
export function easeTo(binding: NativeBinding, options: CameraOptions, animation?: AnimationOptions): void;
export function fitBounds(binding: NativeBinding, options: CameraBoundsOptions): void;
export function flyTo(binding: NativeBinding, options: CameraOptions, animation?: AnimationOptions): void;
export function getBounds(binding: NativeBinding): BoundOptions;
export function getCameraOptions(binding: NativeBinding): CameraOptions;
export function getClientOptions(binding: NativeBinding): ClientOptions;
export function getDebugOptions(binding: NativeBinding): number;
export function getFreeCameraOptions(binding: NativeBinding): FreeCameraOptions;
export function getFrameRateRange(binding: NativeBinding): FrameRateRange | undefined;
export function getPixelRatio(binding: NativeBinding): number;
export function getRenderingEnabled(binding: NativeBinding): boolean;
export function getResourceOptions(binding: NativeBinding): ResourceOptions;
export function getStyleJson(binding: NativeBinding): string | undefined;
export function getStyleUrl(binding: NativeBinding): string | undefined;
export function getSurfaceState(binding: NativeBinding): SurfaceState;
export function logSurfaceState(binding: NativeBinding, label?: string): void;
export function getTileCacheEnabled(binding: NativeBinding): boolean;
export function jumpTo(
  binding: NativeBinding,
  longitude: number,
  latitude: number,
  zoom?: number,
  bearing?: number,
  pitch?: number,
): void;
export function jumpTo(binding: NativeBinding, options: CameraOptions): void;
export function renderFrame(binding: NativeBinding): void;
export function reduceMemoryUse(binding: NativeBinding): void;
export const runLoopOnce: () => void;
export function setBounds(binding: NativeBinding, options: BoundOptions): void;
export function setCameraOptions(binding: NativeBinding, options: CameraOptions): void;
export function setClientOptions(binding: NativeBinding, name: string, version?: string): void;
export function setDebugOptions(binding: NativeBinding, debugOptions: number): void;
export function setFrameRateRange(binding: NativeBinding, range: FrameRateRange): void;
export function setFreeCameraOptions(binding: NativeBinding, options: FreeCameraOptions): void;
export function setPixelRatio(binding: NativeBinding, pixelRatio: number): void;
export function setRenderingEnabled(binding: NativeBinding, enabled: boolean): void;
export function setResourceOptions(binding: NativeBinding, options: ResourceOptions): void;
export function setTileCacheEnabled(binding: NativeBinding, enabled: boolean): void;
export function setStyleUrl(binding: NativeBinding, url: string): void;
export function setStyleJson(binding: NativeBinding, json: string): void;

export interface MapLibreNativeModule {
  DebugOptions: DebugOptions;
  cameraForBounds: typeof cameraForBounds;
  destroy: typeof destroy;
  easeTo: typeof easeTo;
  fitBounds: typeof fitBounds;
  flyTo: typeof flyTo;
  getBounds: typeof getBounds;
  getCameraOptions: typeof getCameraOptions;
  getClientOptions: typeof getClientOptions;
  getDebugOptions: typeof getDebugOptions;
  getFreeCameraOptions: typeof getFreeCameraOptions;
  getFrameRateRange: typeof getFrameRateRange;
  getPixelRatio: typeof getPixelRatio;
  getRenderingEnabled: typeof getRenderingEnabled;
  getResourceOptions: typeof getResourceOptions;
  getStyleJson: typeof getStyleJson;
  getStyleUrl: typeof getStyleUrl;
  getSurfaceState: typeof getSurfaceState;
  logSurfaceState: typeof logSurfaceState;
  getTileCacheEnabled: typeof getTileCacheEnabled;
  jumpTo: typeof jumpTo;
  registerXComponentNode: typeof registerXComponentNode;
  reduceMemoryUse: typeof reduceMemoryUse;
  renderFrame: typeof renderFrame;
  runLoopOnce: typeof runLoopOnce;
  setBounds: typeof setBounds;
  setCameraOptions: typeof setCameraOptions;
  setClientOptions: typeof setClientOptions;
  setDebugOptions: typeof setDebugOptions;
  setFrameRateRange: typeof setFrameRateRange;
  setFreeCameraOptions: typeof setFreeCameraOptions;
  setPixelRatio: typeof setPixelRatio;
  setRenderingEnabled: typeof setRenderingEnabled;
  setResourceOptions: typeof setResourceOptions;
  setTileCacheEnabled: typeof setTileCacheEnabled;
  setStyleUrl: typeof setStyleUrl;
  setStyleJson: typeof setStyleJson;
}

declare const maplibreNative: MapLibreNativeModule;

export default maplibreNative;
