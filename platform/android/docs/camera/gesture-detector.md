# Gesture Detector

The gesture detector of MapLibre Android is encapsulated in the [`maplibre-gestures-android`](https://github.com/maplibre/maplibre-gestures-android) package.

#### Gesture Listeners

You can add listeners for move, rotate, scale and shove gestures. For example, adding a move gesture listener with `MapLibreMap.addOnRotateListener`:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/GestureDetectorActivity.kt:addOnMoveListener"
```

Refer to the full example below for examples of listeners for the other gesture types.

#### Settings

You can access an `UISettings` object via `MapLibreMap.uiSettings`. Available settings include:

- **Toggle Quick Zoom**. You can double tap on the map to use quick zoom. You can toggle this behavior on and off (`UiSettings.isQuickZoomGesturesEnabled`).
- **Toggle Velocity Animations**. By default flicking causes the map to continue panning (while decelerating). You can turn this off with `UiSettings.isScaleVelocityAnimationEnabled`.
- **Toggle Rotate Enabled**. Use `uiSettings.isRotateGesturesEnabled`.
- **Toggle Zoom Enabled**. Use `uiSettings.isZoomGesturesEnabled`.

## Full Example Activity

```kotlin title="GestureDetectorActivity.kt"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/GestureDetectorActivity.kt"
```
