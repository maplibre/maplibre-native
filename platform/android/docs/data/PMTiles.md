# PMTiles

{{ activity_source_note("PMTilesActivity.kt") }}

Starting MapLibre Android 11.7.0, using [PMTiles](https://docs.protomaps.com/pmtiles/) as a data source is supported. You can prefix your vector tile source with `pmtiles://` to load a PMTiles file. The rest of the URL uses `https://` to load a remote PMTiles file, `asset://` to load an asset, or `file://` to load a local PMTiles file.

> Note: PMTiles sources currently do not support caching or offline pack downloads.

## Loading a style that uses PMTiles sources

The simplest approach is to load a style JSON that already references PMTiles sources. Pass the style URL to `setStyle`:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/sources/PMTilesActivity.kt:loadStyle"
```

For a working example style that combines a Protomaps basemap with Foursquare's open POI dataset, see the [wipfli/foursquare-os-places-pmtiles](https://github.com/wipfli/foursquare-os-places-pmtiles) repository.

## Adding a PMTiles source programmatically

Use `VectorSource` with a `pmtiles://` URI to add a PMTiles archive as a vector source to an existing style:

```kotlin
mapView.getMapAsync { map ->
    map.setStyle(Style.Builder().fromUri("https://example.com/base-style.json")) { style ->
        val source = VectorSource("my-pmtiles", "pmtiles://https://example.com/tiles.pmtiles")
        style.addSource(source)
        // Add layers referencing "my-pmtiles"
    }
}
```

PMTiles can be hosted on a simple static file server or CDN instead of a specialized tile server.

<figure markdown="span">
  ![Screenshot of PMTiles based style using Protomaps basemap with Foursquare POIs]({{ s3_url("pmtiles-demo.png") }}){ width="300" }
</figure>
