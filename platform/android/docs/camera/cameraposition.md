# CameraPosition Capabilities

!!! note

    You can find the full source code of this example in [`CameraPositionActivity.kt`](https://github.com/maplibre/maplibre-native/blob/main/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraPositionActivity.kt) of the MapLibreAndroidTestApp.

This example showcases how to listen to camera change events.

<figure markdown="span">
  <video controls width="250" poster="https://maplibre-native.s3.eu-central-1.amazonaws.com/android-documentation-resources/cameraposition_thumbnail.jpg">
    <source src="https://maplibre-native.s3.eu-central-1.amazonaws.com/android-documentation-resources/cameraposition.mp4" />
  </video>
</figure>

The camera animation is kicked off with this code: 

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraPositionActivity.kt:cameraPosition"
```

Notice how the color of the button in the bottom right changes color. Depending on the state of the camera.

We can listen for changes to the state of the camera by registering a `OnCameraMoveListener`, `OnCameraIdleListener`, `OnCameraMoveCanceledListener` or `OnCameraMoveStartedListener` with the `MapLibreMap`. For example, the `OnCameraMoveListener` is defined with:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraPositionActivity.kt:moveListener"
```

And registered with:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraPositionActivity.kt:addOnCameraMoveListener"
```

Refer to the full example to learn the methods to register the other types of camera change events.