# PMTiles

Working with PMTiles

Starting MapLibre iOS 6.10.0, [PMTiles](https://docs.protomaps.com/pmtiles/) archives are supported as tile sources. Prefix any tile source URL with `pmtiles://` to read from a PMTiles archive:

- `pmtiles://https://` — stream tiles from a remote file
- `pmtiles://file://` — read a file from the device filesystem, including the app bundle

The `pmtiles://` prefix works with any [tile source type](https://maplibre.org/maplibre-style-spec/sources/) (e.g. `vector`, `raster`, `raster-dem`, etc.), from a style JSON or dynamically.

> Note: PMTiles sources do not support offline pack downloads or caching.

## Loading a style that uses PMTiles sources

Load a style JSON that references `pmtiles://` sources. The example below loads a satellite imagery basemap of the Central Alps and sets the initial camera position:

<!-- include-example(PMTilesStyleURL) -->

## Adding a PMTiles source directly

Add `vector` sources from PMTiles archives to an already-loaded style. The example overlays two places datasets — [Overture Maps](https://overturemaps.org/) (yellow) and [Foursquare OS Places](https://opensource.foursquare.com/os-places/) (blue) — both using `MLNCircleStyleLayer`. The `sourceLayerIdentifier` names the layer within the archive to render:

<!-- include-example(PMTilesAddSource) -->

The same pattern works for `raster` archives using `MLNRasterTileSource` and `MLNRasterStyleLayer`:

<!-- include-example(PMTilesRasterSource) -->

> Note: `raster-dem` sources added programmatically cannot specify `encoding` via the current iOS API. Define `raster-dem` sources with terrarium encoding in the style JSON instead, where the `encoding` field is fully supported.

## Loading a local PMTiles file

Bundle a `.pmtiles` file in your app target and reference it using its bundle URL. iOS app bundle files are accessible via `file://`, so no special handling is needed:

<!-- include-example(PMTilesLocalFile) -->
