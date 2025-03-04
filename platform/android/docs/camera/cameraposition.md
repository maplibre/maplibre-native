# CameraPosition Capabilities

{{ activity_source_note("CameraPositionActivity.kt") }}

This example showcases how to listen to camera change events.

<figure markdown="span">
  <video controls width="250" poster="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/cameraposition_thumbnail.jpg">
    <source src="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/cameraposition.mp4" />
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
