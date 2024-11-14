# Zoom Methods

!!! note

    You can find the full source code of this example in [`ManualZoomActivity.kt`](https://github.com/maplibre/maplibre-native/blob/main/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/ManualZoomActivity.kt) of the MapLibreAndroidTestApp.

This example shows different methods of zooming in.

<figure markdown="span">
  <video controls width="250" poster="https://maplibre-native.s3.eu-central-1.amazonaws.com/android-documentation-resources/zoom_methods_thumbnail.jpg">
    <source src="https://maplibre-native.s3.eu-central-1.amazonaws.com/android-documentation-resources/zoom_methods.mp4" />
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

