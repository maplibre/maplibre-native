# PMTiles

{{ activity_source_note("PMTilesActivity.kt") }}

Starting MapLibre Android 11.7.0, [PMTiles](https://docs.protomaps.com/pmtiles/) archives are supported as tile sources. Prefix any tile source URL with `pmtiles://` to read from a PMTiles archive:

- `pmtiles://https://` — stream tiles from a remote file
- `pmtiles://file://` — read a file from device storage (use `getExternalFilesDir` or `filesDir` for the path)

The `pmtiles://` prefix works with any [tile source type](https://maplibre.org/maplibre-style-spec/sources/) (e.g. `vector`, `raster`, `raster-dem`, etc.), from a style JSON or dynamically.

Unlike MapLibre GL JS, which resolves relative URLs against the style's base URL, MapLibre Native requires the URL inside `pmtiles://` to be fully specified (e.g. `pmtiles://https://example.com/tiles.pmtiles`, not `pmtiles://tiles.pmtiles`).

> Note: PMTiles sources do not support offline pack downloads or caching.

## Loading a style that uses PMTiles sources

Load a style JSON that references `pmtiles://` sources. The example below loads a satellite imagery basemap of the Central Alps and sets the initial camera position:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/sources/PMTilesActivity.kt:loadStyle"
```

## Adding a PMTiles source directly

Add `vector` sources from PMTiles archives to an already-loaded style. The example overlays two places datasets — [Overture Maps](https://overturemaps.org/) (yellow) and [Foursquare OS Places](https://opensource.foursquare.com/os-places/) (blue) — both using `CircleLayer`. The `setSourceLayer` call names the layer within the archive to render:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/sources/PMTilesActivity.kt:addSource"
```

The same pattern works for `raster` archives using `RasterSource` and `RasterLayer`.

> Note: `raster-dem` sources added programmatically cannot specify `encoding` via the current Android API. Define `raster-dem` sources with terrarium encoding in the style JSON instead, where the `encoding` field is fully supported.

## Loading a local PMTiles file

For files on device storage, use `file://` with an absolute path. The example below uses `getExternalFilesDir(null)` to locate a file downloaded to the app's external files directory:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/sources/PMTilesActivity.kt:loadFromFile"
```

> Note: `pmtiles://asset://` (files in `src/main/assets/`) is not currently supported. `AssetManagerFileSource` does not implement byte-range reads, which PMTiles requires to read its header and metadata. Use `file://` from device storage instead. See the [open issue](https://github.com/maplibre/maplibre-native/issues/4360) for status.
