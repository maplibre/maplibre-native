## Attribution

| File pattern | Format | Description | Source / License |
|---|---|---|---|
| `{z}-{x}-{y}.ocean.webp` | WebP raster-DEM (Mapbox terrain-RGB encoding) | Ocean bathymetry DEM tiles, z=0–1 | Derived from [GEBCO](https://www.gebco.net/) gridded bathymetric data. Public domain. |
| `{z}-{x}-{y}.osm_basemap.pbf` | Vector (PBF, raw/uncompressed) | Land, water, and coastline polygons, z=0–1 | © [OpenStreetMap contributors](https://www.openstreetmap.org/copyright), [ODbL](https://opendatacommons.org/licenses/odbl/). Polygons from [osmdata.openstreetmap.de](https://osmdata.openstreetmap.de/). |
| `jaxa/{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding), 512px | Terrain DEM tiles, z0-z9 | Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). Every tile here was verified pixel-identical against the JAXA terrain-RGB source. Some were originally added (in `tiles/`) in [369af13](https://github.com/maplibre/maplibre-native/commit/369af13ead1b); the rest are the z0-z9 set the skirts tests need. |
| `{z}-{x}-{y}.terrain.png` (`12-758-*`, `12-759-*`) | PNG raster-DEM (Mapbox terrain-RGB encoding), 256px | Terrain DEM tiles over the Grand Canyon, z12 | **Provenance unverified.** Not from the JAXA source - that serves 512px tiles for these coordinates and the pixels do not match, even subsampled. Same region as the AWS terrarium tiles in `terrain/`, but terrain-RGB encoded. Do not re-fetch from JAXA: the resolution differs, and `tileSize` changes the tile cover. |
| `terrain/{z}-{x}-{y}.terrain.png` | PNG raster-DEM (**mixed**: terrarium and terrain-RGB) | Terrain DEM tiles for two unrelated regions | Terrarium tiles over the Grand Canyon from [AWS Terrain Tiles](https://registry.opendata.aws/terrain-tiles/) (verified byte-identical to `terrarium/{z}/{x}/{y}.png`); terrain-RGB tiles over the Dead Sea. **No longer used by the skirts tests** (migrated to `jaxa/`). |
| `terrain-shading/{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding) | Terrain DEM tiles over the Alps (N47 E011) | Matches the [maplibre/demotiles](https://demotiles.maplibre.org/terrain-tiles/tiles.json) `jaxa_terrainrgb_N047E011` set. Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). |

### Notes for adding DEM fixtures

- **One `raster-dem` source applies a single unpack vector to everything it
  loads**, so all tiles reachable by one source must share an encoding. Mixing
  terrarium and terrain-RGB produces garbage elevations *silently* - a terrarium
  tile read as terrain-RGB decodes to ~870,000 m. Verify by decoding both ways
  and keeping whichever yields a plausible elevation.
- Tests read tiles from the SQLite `metrics/cache-style.db`, **not** from these
  files; the files are provenance/source-of-record. Adding a tile here has no
  effect until it is also inserted into that DB (match the existing `tiles` table
  row format and use `insert or replace`). Consequently the on-disk folder name
  need not match the style URL, and for the moved `jaxa/` tiles it does not.
- `jaxa/` is uniform in *encoding* but not in *tile size*: the four
  `12-758-*`/`12-759-*` tiles are 256px, the rest 512px. Style `tileSize` drives
  the cover zoom, so check it when adding tiles.
