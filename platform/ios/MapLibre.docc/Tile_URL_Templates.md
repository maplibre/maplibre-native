# Tile URL Templates

Using URL Templates when defining tile sources

``MLNTileSource`` objects, specifically ``MLNRasterTileSource`` and
``MLNVectorTileSource`` objects, can be created using an initializer that accepts
an array of tile URL templates. Tile URL templates are strings that specify the
URLs of the vector tiles or raster tile images to load. A template resembles an
absolute URL, but with any number of placeholder strings that the source
evaluates based on the tile it needs to load. For example:

* `http://www.example.com/tiles/{z}/{x}/{y}.pbf` could be
   evaluated as `http://www.example.com/tiles/14/6/9.pbf`.
* `http://www.example.com/tiles/{z}/{x}/{y}{ratio}.png` could be
   evaluated as `http://www.example.com/tiles/14/6/9@2x.png`.

Tile URL templates are also used to define tilesets in TileJSON manifests or
[`raster`](https://maplibre.org/maplibre-style-spec/sources/#raster)
and
[`vector`](https://maplibre.org/maplibre-style-spec/sources/#vector-tiles)
sources in style JSON files. See the
[TileJSON specification](https://github.com/mapbox/tilejson-spec/tree/master/2.2.0)
for information about tile URL templates in the context of a TileJSON or style
JSON file.

Tile sources support the following placeholder strings in tile URL templates,
all of which are optional:

| Placeholder string | Description |
| --- | --- |
| `{x}` | The index of the tile along the map’s x axis according to Spherical Mercator projection. If the value is 0, the tile’s left edge corresponds to the 180th meridian west. If the value is 2^z−1, the tile’s right edge corresponds to the 180th meridian east. |
| `{y}` | The index of the tile along the map’s y axis according to Spherical Mercator projection. If the value is 0, the tile’s tile edge corresponds to arctan(sinh(π)), or approximately 85.0511 degrees north. If the value is 2^z−1, the tile’s bottom edge corresponds to −arctan(sinh(π)), or approximately 85.0511 degrees south. The y axis is inverted if the options parameter contains ``MLNTileSourceOptionTileCoordinateSystem`` with a value of ``MLNTileCoordinateSystem/MLNTileCoordinateSystemTMS``. |
| `{z}` | The tile’s zoom level. At zoom level 0, each tile covers the entire world map; at zoom level 1, it covers ¼ of the world; at zoom level 2, 1⁄16 of the world, and so on. For tiles loaded by a ``MLNRasterTileSource`` object, whether the tile zoom level matches the map’s current zoom level depends on the value of the source’s tile size as specified in the ``MLNTileSourceOptionTileSize`` key of the options parameter. |
| `{bbox-epsg-3857}` | The tile’s bounding box, expressed as a comma-separated list of the tile’s western, southern, eastern, and northern extents according to Spherical Mercator (EPSG:3857) projection. The bounding box is typically used with map services conforming to the <a href="http://www.opengeospatial.org/standards/wms">Web Map Service</a> protocol. |
| `{quadkey}` | A quadkey indicating both the tile’s location and its zoom level. The quadkey is typically used with <a href="https://msdn.microsoft.com/en-us/library/bb259689.aspx">Bing Maps</a>.  |
| `{ratio}` | A suffix indicating the resolution of the tile image. The suffix is the empty string for standard resolution displays and `@2x` for Retina displays, including displays for which `UIScreen.scale` is 3.  |
| `{prefix}` | Two hexadecimal digits chosen such that each visible tile has a different prefix. The prefix is typically used for domain sharding. |

For more information about the `{x}`, `{y}`, and `{z}` placeholder strings,
consult the
[OpenStreetMap Wiki](https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames).
