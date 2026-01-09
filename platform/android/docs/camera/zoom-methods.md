# Zoom Methods

{{ activity_source_note("ManualZoomActivity.kt") }}

This example shows different methods of zooming in.

<figure markdown="span">
  <video controls width="250" poster="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/zoom_methods_thumbnail.jpg">
    <source src="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/zoom_methods.mp4" />
  </video>
</figure>

Each method uses `MapLibreMap.animateCamera`, but with a different `CameraUpdateFactory`.

#### Zooming In

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/ManualZoomActivity.kt:zoomIn"
```

#### Zooming Out

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/ManualZoomActivity.kt:zoomOut"
```

#### Zoom By Some Amount of Zoom Levels

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/ManualZoomActivity.kt:zoomBy"
```

#### Zoom to a Zoom Level

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/ManualZoomActivity.kt:zoomTo"
```

#### Zoom to a Point

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/ManualZoomActivity.kt:zoomToPoint"
```
