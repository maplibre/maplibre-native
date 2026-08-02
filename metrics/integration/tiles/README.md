## Attribution

| File pattern | Format | Description | Source / License |
|---|---|---|---|
| `{z}-{x}-{y}.ocean.webp` | WebP raster-DEM (Mapbox terrain-RGB encoding) | Ocean bathymetry DEM tiles, z=0–1 | Derived from [GEBCO](https://www.gebco.net/) gridded bathymetric data. Public domain. |
| `{z}-{x}-{y}.osm_basemap.pbf` | Vector (PBF, raw/uncompressed) | Land, water, and coastline polygons, z=0–1 | © [OpenStreetMap contributors](https://www.openstreetmap.org/copyright), [ODbL](https://opendatacommons.org/licenses/odbl/). Polygons from [osmdata.openstreetmap.de](https://osmdata.openstreetmap.de/). |
| `jaxa/{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding), 512px | Terrain DEM tiles, z0-z9 | Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). Some added in [369af13](https://github.com/maplibre/maplibre-native/commit/369af13ead1b). |
| `{z}-{x}-{y}.terrain.png` (`12-758-*`, `12-759-*`) | PNG raster-DEM (Mapbox terrain-RGB encoding), 256px | Terrain DEM tiles over the Grand Canyon, z12 | Source unknown. |
| `terrain/{z}-{x}-{y}.terrain.png` | PNG raster-DEM (**mixed**: terrarium and terrain-RGB) | Terrain DEM tiles for two unrelated regions | Terrarium tiles over the Grand Canyon from [AWS Terrain Tiles](https://registry.opendata.aws/terrain-tiles/); terrain-RGB tiles over the Dead Sea, source unknown. |
| `terrain-shading/{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding) | Terrain DEM tiles over the Alps (N47 E011) | Matches the [maplibre/demotiles](https://demotiles.maplibre.org/terrain-tiles/tiles.json) `jaxa_terrainrgb_N047E011` set. Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). |
