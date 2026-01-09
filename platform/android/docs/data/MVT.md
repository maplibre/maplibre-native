# Vector Tiles

{{ activity_source_note("VectorTileActivity.kt") }}

You can specify where to load MVTs (which sometimes have `.pbf` extension) by creating a `TileSet` object with template parameters (for example `{z}` which will be replaced with the zoom level).

MapLibre has [a repo](https://github.com/maplibre/demotiles/tree/gh-pages/tiles-omt) with some example vector tiles with the OpenMapTiles schema around Innsbruck, Austria. In the example we load these MVTs and create a line layer for the road network.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/sources/VectorTileActivity.kt:addTileSet"
```

<figure markdown="span">
  ![Screenshot of road overlay from vector tile source]({{ s3_url("vectortilesource.png") }}){ width="400" }
  {{ openmaptiles_caption() }}
</figure>
