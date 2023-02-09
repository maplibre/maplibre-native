declare module '@maplibre/maplibre-gl-native' {
  enum ResourceKind {
    Unknown = 0,
    Style = 1,
    Source = 2,
    Tile = 3,
    Glyphs = 4,
    SpriteImage = 5,
    SpriteJSON = 6,
  }

  /**
   * Response expected by a request call during render
   */
  type RequestResponse = {
    data: Uint8Array;
    modified?: Date;
    expires?: Date;
    etag?: string;
  };

  type MapOptions = {
    /**
     * Will be used during a `Map.render` call to request all necessary map resources (tiles, fonts...)
     */
    request: (
      request: { url: string; kind: ResourceKind },
      callback: (error?: Error, response?: RequestResponse) => void,
    ) => void;

    /**
     * Pixel ratio at which to render images
     *
     * @default 1
     */
    ratio?: number;
  };

  /**
   * Defines how to render the map view and the resulting image
   */
  type RenderOptions = {
    /**
     * @default 0
     */
    zoom?: number;

    /**
     * Width of image in pixel
     *
     * @default 512
     */
    width?: number;

    /**
     * Height of image in pixel
     *
     * @default 512
     */
    height?: number;

    /**
     * Coordinates [longitude, latitude]
     *
     * @default [0, 0]
     */
    center?: [number, number];

    /**
     * Bearing of map view in degrees, counter-clockwise from north
     *
     * @default 0
     */
    bearing?: number;

    /**
     * Pitch of map view in degrees, arcing towards the horizon
     *
     * @default 0
     */
    pitch?: number;

    /**
     * @default []
     */
    classes?: string[];
  };

  /**
   * A `Map` instance is used to render images from map views
   */
  class Map {
    constructor(mapOptions: MapOptions);

    load: (style: any) => void;

    /**
     * Render a specific map view to an image with previously loaded map styles
     */
    render: (
      renderOptions: RenderOptions,
      callback: (...args: [error: Error, buffer: undefined] | [error: undefined, buffer: Uint8Array]) => void,
    ) => void;

    /**
     * Call to permanently dispose the internal map resources, instance can't be used for further render calls
     */
    release: () => void;
  }
}
