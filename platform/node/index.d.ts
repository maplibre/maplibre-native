declare module '@maplibre/maplibre-gl-native' {
  const enum ResourceKind {
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

  const enum MapMode {
    /**
     * Render all tiles in map view
     */
    Static = 'static',

    /**
     * Render and request only a single tile
     */
    Tile = 'tile',
  }

  type MapOptions = {
    /**
     * Will be used during a `Map.render` call to request all necessary map resources (tiles, fonts...)
     */
    request?: (
      request: { url: string; kind: ResourceKind },
      callback: (error?: Error, response?: RequestResponse) => void,
    ) => void;

    /**
     * Pixel ratio at which to render images
     *
     * @default 1
     */
    ratio?: number;

    /**
     * Mode in which map view will be rendered
     *
     * @default MapMode.Static
     */
    mode?: MapMode;
  };

  /**
   * Defines the map view to render and the resulting image
   */
  type RenderOptions = {
    /**
     * Zoom level
     *
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
    constructor(mapOptions?: MapOptions);

    /**
     * Load a style into a map
     */
    load: (style: any) => void;

    /**
     * Render a specific map view to an image with previously loaded map styles with render options.
     */
    render(renderOptions: RenderOptions, callback: (...args: [error: Error, buffer: undefined] | [error: undefined, buffer: Uint8Array]) => void): void;

    /**
     * Render a specific map view to an image with previously loaded map styles without render options.
     */
    render(callback: (...args: [error: Error, buffer: undefined] | [error: undefined, buffer: Uint8Array]) => void): void;

    /**
     * Call to permanently dispose the internal map resources, instance can't be used for further render calls
     */
    release: () => void;

    /**
     * Add source to map's style
     */
    addSource: (sourceId: string, source: object) => void;

    /**
     * Remove source from map's style
     */
    removeSource: (sourceId: string) => void;

    /**
     * Add layer to map's style
     */
    addLayer: (layer: object, beforeId?: string) => void;

    /**
     * Remove layer from map's style
     */
    removeLayer: (layerId: string) => void;

    /**
     * Add image to map's style
     */
    addImage: (imageId: string, image: any) => void;

    /**
     * Remove image from map's style
     */
    removeImage: (imageId: string) => void;

    /**
     * Set the extent of the zoom for a specified layer
     */
    setLayerZoomRange: (layerId: string, minZoom: number, maxZoom: number) => void;

    /**
     * Set the value for a layer's property
     */
    setLayoutProperty: (layerId: string, name: string, value: string) => void;

    /**
     * Set filter for specified style layer
     */
    setFilter: (layerId: string, filter: [] | null | undefined) => void;

    /**
     * Set size of the tile
     */
    setSize: (size: [number, number]) => void;

    /**
     * Set the center of the map
     */
    setCenter: (center: [number, number]) => void;

    /**
     * Set zoom of the map
     */
    setZoom: (zoom: number) => void;

    /**
     * Set bearing (rotation) of map
     */
    setBearing: (bearing: number) => void;

    /**
     * Set pitch (tilt angle) of map
     */
    setPitch: (pitch: number) => void;

    /**
     * Set light value of map
     */
    setLight: (light: any) => void;

    /**
     * Set axonometric view of map
     */
    setAxonometric: (state: boolean) => void;

    /**
     * Set X skew of map
     */
    setXSkew: (x: number) => void;

    /**
     * Set Y skew of map
     */
    setYSkew: (y: number) => void;
  }
}
