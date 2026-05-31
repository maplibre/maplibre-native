export interface ResourceOptions {
  apiKey?: string;
  cachePath?: string;
  assetPath?: string;
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
  renderedFrameRate: number;
  frameCallbackRate: number;
  backend?: string;
  surfaceVisible: boolean;
  lastSurfaceError?: string;
  lastMapLoadError?: string;
  lastRenderError?: string;
  lastStyleImageMissing?: string;
  lastGlyphsError?: string;
  lastSpritesError?: string;
}

export interface XComponentContext {
  destroy: () => void;
  getCameraOptions: () => CameraOptions;
  getPixelRatio: () => number;
  getStyleAttributions: () => string[];
  getSurfaceState: () => SurfaceState;
  jumpTo: (options: CameraOptions) => void;
  reduceMemoryUse: () => void;
  renderFrame: () => void;
  setBounds: (options: BoundOptions) => void;
  setClientOptions: (name: string, version?: string) => void;
  setFrameRateRange: (range: FrameRateRange) => void;
  setPixelRatio: (pixelRatio: number) => void;
  setRenderingEnabled: (enabled: boolean) => void;
  setResourceOptions: (options: ResourceOptions) => void;
  setStyleUrl: (url: string) => void;
  setTileCacheEnabled: (enabled: boolean) => void;
}
