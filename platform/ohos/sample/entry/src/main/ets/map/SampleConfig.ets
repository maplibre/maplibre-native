import type { BoundOptions, CameraOptions } from 'libmaplibre_native_ohos.so';

export const LOG_DOMAIN: number = 0x4d4c4e;
export const LOG_TAG: string = 'MapLibreSample';

export const CLIENT_NAME: string = 'MapLibreOhosSample';
export const CLIENT_VERSION: string = '0.1.0';

export interface StyleOption {
  name: string;
  url: string;
}

export const STYLE_OPTIONS: StyleOption[] = [
  {
    name: 'Demo',
    url: 'https://demotiles.maplibre.org/style.json'
  },
  {
    name: 'Bright',
    url: 'https://tiles.openfreemap.org/styles/bright'
  },
  {
    name: 'Liberty',
    url: 'https://tiles.openfreemap.org/styles/liberty'
  }
];

export const DEFAULT_STYLE_INDEX: number = 1;
export const DEFAULT_STYLE: StyleOption = STYLE_OPTIONS[DEFAULT_STYLE_INDEX];

export const DEFAULT_CAMERA: CameraOptions = {
  longitude: 106.5750,
  latitude: 29.5570,
  zoom: 12.0,
  bearing: 0,
  pitch: 0
};

export const DEFAULT_BOUNDS: BoundOptions = {
  bounds: { west: -180, south: -85, east: 180, north: 85 },
  minZoom: 0,
  maxZoom: 22,
  minPitch: 0,
  maxPitch: 60
};
