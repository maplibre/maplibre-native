## Attribution

| File pattern | Format | Description | Source / License |
|---|---|---|---|
| `{z}-{x}-{y}.ocean.webp` | WebP raster-DEM (Mapbox terrain-RGB encoding) | Ocean bathymetry DEM tiles, z=0–1 | Derived from [GEBCO](https://www.gebco.net/) gridded bathymetric data. Public domain. |
| `{z}-{x}-{y}.osm_basemap.pbf` | Vector (PBF, raw/uncompressed) | Land, water, and coastline polygons, z=0–1 | © [OpenStreetMap contributors](https://www.openstreetmap.org/copyright), [ODbL](https://opendatacommons.org/licenses/odbl/). Polygons from [osmdata.openstreetmap.de](https://osmdata.openstreetmap.de/). |
| `{z}-{x}-{y}.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding) | Terrain DEM tiles | Credit: [AW3D30 (JAXA)](https://www.eorc.jaxa.jp/ALOS/en/dataset/aw3d30/aw3d30_e.htm). Added in [369af13](https://github.com/maplibre/maplibre-native/commit/369af13ead1b). |
| `5-5-12.terrain.png` | PNG raster-DEM (Mapbox terrain-RGB encoding) | Terrain DEM tile for the globe hillshade render test | From the MapLibre GL JS test assets (`test/integration/assets/tiles/terrain/`, added in [maplibre-gl-js#1707](https://github.com/maplibre/maplibre-gl-js/pull/1707)), BSD-3-Clause. |
| `checkerboard.png` | PNG raster | One checkerboard tile served at every z/x/y for the globe `raster-warped` render test | From the MapLibre GL JS test assets (`test/integration/assets/tiles/checkerboard.png`), BSD-3-Clause. |
| `raster-zoom/{z}.png` | PNG raster | One flat-colour tile per zoom, served at every x/y, for the globe `antimeridian-lod` render test | From the MapLibre GL JS test assets (`test/integration/assets/tiles/raster-zoom/`), BSD-3-Clause. |
| `../image/projective-grid.png` | PNG image | Labelled grid for the globe `image-non-parallelogram` render test | From the MapLibre GL JS test assets (`test/integration/assets/image/projective-grid.png`), BSD-3-Clause. |
| `antimeridian/0-0-0.mvt` | Vector (PBF) | Zoom 0 tile whose buffer duplicates a polygon across the antimeridian, for the globe `antimeridian-overdraw/vector-tiles-z0` render test | From the MapLibre GL JS test assets (`test/integration/assets/tiles/`), [BSD-3-Clause](https://github.com/maplibre/maplibre-gl-js/blob/main/LICENSE.txt). |
| `checkerboard.mvt` | Vector (PBF) | Checkerboard polygons served for every tile, for the globe `fill-seams/checkerboard` render test | From the MapLibre GL JS test assets (`test/integration/assets/tiles/`), [BSD-3-Clause](https://github.com/maplibre/maplibre-gl-js/blob/main/LICENSE.txt). |
| `ocean.mvt` | Vector (PBF) | A full-tile polygon served for every tile, for the globe `fill-seams/ocean` render test | From the MapLibre GL JS test assets (`test/integration/assets/tiles/`), [BSD-3-Clause](https://github.com/maplibre/maplibre-gl-js/blob/main/LICENSE.txt). |
