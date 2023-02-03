declare module '@maplibre/maplibre-gl-native' {
  export enum Resource {
    Unknown = 0,
    Style = 1,
    Source = 2,
    Tile = 3,
    Glyphs = 4,
    SpriteImage = 5,
    SpriteJSON = 6,
  }

  export type RequestResponse = {
    data: ArrayBuffer;
    modified?: Date;
    expires?: Date;
    etag?: string;
  };

  export type MapOptions = {
    request: (
      request: { url: string; kind: number },
      callback: (error?: Error, response?: RequestResponse) => void,
    ) => void;
    ration?: number;
  };

  export type RenderOptions = {
    zoom?: number; // defaults to 0
    width?: number; // px, defaults to 512
    height?: number; // px, defaults to 512
    center?: [number, number]; // coordinates, defaults to [0,0]
    bearing?: number; // degrees, counter-clockwise from north, defaults to 0
    pitch?: number; // degrees, arcing towards the horizon, defaults to 0
    classes?: string[]; // array of strings
  };

  export class Map {
    constructor(mapOptions: MapOptions);

    load: (style: any) => void;

    render: (
      renderOptions: RenderOptions,
      callback: (...args: [error: Error, buffer: undefined] | [error: undefined, buffer: Uint8Array]) => void,
    ) => void;

    release: () => void;
  }
}
