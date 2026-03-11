# Tile fixtures for integration tests

This directory contains tile fixtures used by the render integration tests in
`metrics/integration/render-tests/`. Tiles are referenced via `local://tiles/`
URLs in test style JSON files and served from `metrics/cache-style.db`.

Attribution is required for tiles derived from external data sources. If you add
new tile files here, please document them in the table below.

---

## Attribution

| File pattern | Format | Description | Source / License |
|---|---|---|---|
| `{z}-{x}-{y}.ocean.webp` | WebP raster-DEM (Mapbox terrain-RGB encoding) | Ocean bathymetry DEM tiles, z=0–1 | Derived from [GEBCO](https://www.gebco.net/) gridded bathymetric data. Public domain. |
| `{z}-{x}-{y}.osm_basemap.pbf` | Vector (PBF, raw/uncompressed) | Land, water, and coastline polygons, z=0–1 | © [OpenStreetMap contributors](https://www.openstreetmap.org/copyright), [ODbL](https://opendatacommons.org/licenses/odbl/). Polygons from [osmdata.openstreetmap.de](https://osmdata.openstreetmap.de/). |
| `{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding) | Terrain DEM tiles | Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). Added in [369af13](https://github.com/maplibre/maplibre-native/commit/369af13ead1b). |
| `{z}-{x}-{y}.satellite.png` | PNG raster | Satellite imagery tiles | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `{z}-{x}-{y}.cross-fade.png` | PNG raster | Raster tiles used for cross-fade transition tests | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `{z}-{x}-{y}.terrarium.png` | PNG raster-DEM (Terrarium encoding) | DEM tiles in Terrarium encoding | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `{z}-{x}-{y}.contour.png` | PNG raster | Contour overlay tiles | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `{z}-{x}-{y}.mvt` / `{z}-{x}-{y}.mlt` | Vector (MVT / MLT) | Vector tiles used for symbol/layer tests | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `counties-{z}-{x}-{y}.mvt` / `.mlt` | Vector (MVT / MLT) | US county boundary vector tiles | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `{z}-{x}-{y}.tms.mvt` / `.mlt` | Vector (MVT / MLT) | TMS-scheme vector tiles | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `alpha.png` | PNG | Single alpha-channel test image | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |
| `mapbox.mapbox-streets-v7` | Directory | Mapbox Streets v7 tiles | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) ([29428dc](https://github.com/maplibre/maplibre-native/commit/29428dce4200) for MLT) — origin unknown |
| `mapbox.satellite` | Directory | Mapbox Satellite tiles | Added in [eadb369](https://github.com/maplibre/maplibre-native/commit/eadb3697e93c) — origin unknown |

---

## Notes on stored tile format

- **PBF vector tiles** must be stored **uncompressed (raw PBF)**. Tile servers
  typically return gzip-encoded responses; the renderer cannot handle a gzip
  wrapper in the tile bytes, so tiles should be decompressed before storing.
- `metrics/cache-style.db` is a SQLite database committed directly to the
  repository. Individual tile entries can be inserted into it using the
  `mbgl-cache` tool (see `bin/cache.cpp`).
