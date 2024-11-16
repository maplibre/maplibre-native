# Animated Image Source

{{ activity_source_note("AnimatedImageSourceActivity.kt") }}

This is the MapLibre Native equivalent of [this MapLibre GL JS example](https://maplibre.org/maplibre-gl-js/docs/examples/animate-images/).

<figure markdown="span">
  <video controls width="400" poster="{{ s3_url("animated_image_source_thumbnail.jpg") }}" >
    <source src="{{ s3_url("animated_image_source.mp4") }}" />
  </video>
</figure>


```kotlin title="Setting up the fill extrusion layer"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedImageSourceActivity.kt:onMapReady"
```