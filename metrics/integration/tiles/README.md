## Attribution

| File pattern | Format | Description | Source / License |
|---|---|---|---|
| `{z}-{x}-{y}.ocean.webp` | WebP raster-DEM (Mapbox terrain-RGB encoding) | Ocean bathymetry DEM tiles, z=0–1 | Derived from [GEBCO](https://www.gebco.net/) gridded bathymetric data. Public domain. |
| `{z}-{x}-{y}.osm_basemap.pbf` | Vector (PBF, raw/uncompressed) | Land, water, and coastline polygons, z=0–1 | © [OpenStreetMap contributors](https://www.openstreetmap.org/copyright), [ODbL](https://opendatacommons.org/licenses/odbl/). Polygons from [osmdata.openstreetmap.de](https://osmdata.openstreetmap.de/). |
| `{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding) | Terrain DEM tiles | Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). Added in [369af13](https://github.com/maplibre/maplibre-native/commit/369af13ead1b). |
| `3-{x}-{y}.national-grid.png` | PNG raster | Synthetic tiles for `projection/national-grid/crosshairs`: white 256 px tiles at zoom 3 of Ordnance Survey's National Grid tile grid, x 1–3 and y 4–5, with red crosshairs centered on known eastings and northings | Drawn for the test. |
