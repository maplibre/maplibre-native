# Max/Min Zoom

{{ activity_source_note("MaxMinZoomActivity.kt") }}

This example shows how to configure a maximum and a minimum zoom level.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/MaxMinZoomActivity.kt:zoomPreference"
```

## Bonus: Add Click Listener

As a bonus, this example also shows how you can define a click listener to the map.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/MaxMinZoomActivity.kt:addOnMapClickListener"
```

You can remove a click listener again with `MapLibreMap.removeOnMapClickListener`. To use this API you need to assign the click listener to a variable, since you need to pass the listener to that method.

<figure markdown="span">
  <video controls width="250" poster="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/max_min_zoom_thumbnail.jpg">
    <source src="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/max_min_zoom.mp4" />
  </video>
  {{ openmaptiles_caption }}
</figure>
