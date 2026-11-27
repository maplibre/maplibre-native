# Custom Projection

{{ activity_source_note("ArcticProjectionActivity.kt") }}

A map can draw tiles that were cut in another coordinate reference system, such as NASA GIBS's polar stereographic imagery of the Arctic in EPSG:3413, without warping them into Web Mercator. This is the MapLibre Native equivalent of MapLibre GL JS's `addProjection`.

!!! warning

    This API is experimental and may change in a future release.

A `CoordinateReferenceSystem` has a name, the tile grid the tiles were cut in, and a projection that converts longitudes and latitudes to positions in the system and back. The map calls the projection with many points at a time, from any thread, so it reads and writes packed arrays.

```kotlin title="An EPSG:3413 projection"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/projection/ArcticProjection.kt:coordinateReferenceSystem"
```

Register the projection before you load a style that names it. A style that names a projection nobody registered draws in Web Mercator.

```kotlin title="Registering the projection"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/projection/ArcticProjectionActivity.kt:addProjection"
```

The style selects it by name with its `projection.type`, and a source's `{bbox}` token takes each tile's box in the projection, as a WMS request needs.

```json title="From arctic.json"
"projection": {"type": "EPSG:3413"},
"sources": {
  "blue-marble": {
    "type": "raster",
    "tiles": ["https://gibs.earthdata.nasa.gov/wms/epsg3413/best/wms.cgi?SERVICE=WMS&REQUEST=GetMap&VERSION=1.3.0&LAYERS=BlueMarble_ShadedRelief_Bathymetry&STYLES=&FORMAT=image/jpeg&CRS=EPSG:3413&BBOX={bbox}&WIDTH=512&HEIGHT=512"],
    "tileSize": 512
  }
}
```

```kotlin title="Loading the style"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/projection/ArcticProjectionActivity.kt:setStyle"
```

`Style.removeProjection` removes it again. A map already drawn in it keeps it until its projection changes.
