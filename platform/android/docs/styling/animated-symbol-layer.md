# Animated SymbolLayer

{{ activity_source_note("AnimatedSymbolLayerActivity.kt") }}

<figure markdown="span">
  <video controls width="250" poster="{{ s3_url("animated_symbol_layer_thumbnail.jpg") }}" >
    <source src="{{ s3_url("animated_symbol_layer.mp4") }}" />
  </video>
  {{ openmaptiles_caption() }}
</figure>


Notice that there are (red) cars randomly moving around, and a (yellow) taxi that is always heading to the passenger (indicated by the M symbol), which upon arrival hops to a different location again. We will focus on the passanger and the taxi, because the cars randomly moving around follow a similar pattern.

In a real application you would of course retrieve the locations from some sort of external API, but for the purposes of this example a random latitude longtitude pair within bounds of the currently visible screen will do.

```kotlin title="Getter method to get a random location on the screen"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedSymbolLayerActivity.kt:latLngInBounds"
```

```kotlin title="Adding a passenger at a random location (on screen)"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedSymbolLayerActivity.kt:addPassenger"
```

Adding the taxi on screen is done very similarly.

```kotlin title="Adding the taxi with bearing"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedSymbolLayerActivity.kt:addTaxi"
```

For animating the taxi we use a [`ValueAnimator`](https://developer.android.com/reference/android/animation/ValueAnimator).

```kotlin title="Animate the taxi driving towards the passenger"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedSymbolLayerActivity.kt:animateTaxi"
```
