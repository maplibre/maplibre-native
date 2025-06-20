# Building Layer

{{ activity_source_note("BuildingFillExtrusionActivity.kt") }}

In this example will show how to add a [Fill Extrusion](https://maplibre.org/maplibre-style-spec/layers/#fill-extrusion) layer to a style.
<figure markdown="span">
  <video controls width="400" poster="{{ s3_url("building_layer_thumbnail.jpg") }}" >
    <source src="{{ s3_url("building_layer.mp4") }}" />
  </video>
  {{ openmaptiles_caption() }}
</figure>

We use the [OpenFreeMap Bright](https://openfreemap.org/quick_start/) style which, unlike OpenFreeMap Libery, does not have a fill extrusion layer by default. However, if you inspect this style with [Maputnik](https://maplibre.org/maputnik) you will find that the multipolygons in the  `building` layer (of the `openfreemap` source) each have `render_min_height` and `render_height` properties.

```kotlin title="Setting up the fill extrusion layer"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/BuildingFillExtrusionActivity.kt:setupBuildings"
```

```kotlin title="Changing the light direction"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/BuildingFillExtrusionActivity.kt:lightPosition"
```

```kotlin title="Changing the light color"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/BuildingFillExtrusionActivity.kt:lightColor"
```
