# LatLngBounds API


!!! note

    You can find the full source code of this example in [`LatLngBoundsActivity.kt`](https://github.com/maplibre/maplibre-native/blob/main/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/LatLngBoundsActivity.kt) of the MapLibreAndroidTestApp.

This example demonstrates setting the camera to some bounds defined by some features. It sets these bounds when the map is initialized and when the [bottom sheet](https://m2.material.io/components/sheets-bottom) is opened or closed.

<figure markdown="span">
  <video controls width="250" poster="https://maplibre-native.s3.eu-central-1.amazonaws.com/android-documentation-resources/lat_lng_bounds_thumbnail.jpg">
    <source src="https://maplibre-native.s3.eu-central-1.amazonaws.com/android-documentation-resources/lat_lng_bounds.mp4" />
  </video>
</figure>


Here you can see how the feature collection is loaded and how `MapLibreMap.getCameraForLatLngBounds` is used to set the bounds during map initialization:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/LatLngBoundsActivity.kt:featureCollection"
```

The `createBounds` function uses the `LatLngBounds` API to include all points within the bounds:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/LatLngBoundsActivity.kt:createBounds"
```