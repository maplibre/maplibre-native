# Animation Types

{{ activity_source_note("CameraAnimationTypeActivity.kt") }}

This example showcases the different animation types.

- **Move**: available via the `MapLibreMap.moveCamera` method.
- **Ease**: available via the `MapLibreMap.easeCamera` method.
- **Animate**: available via the `MapLibreMap.animateCamera` method.

### Move

The `MapLibreMap.moveCamera` method jumps to the camera position provided.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraAnimationTypeActivity.kt:moveCamera"
```

<figure markdown="span">
  <video controls width="250" poster="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/move_animation_thumbnail.jpg">
    <source src="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/move_animation.mp4" />
  </video>
</figure>

### Ease

The `MapLibreMap.moveCamera` eases to the camera position provided (with constant ground speed).

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraAnimationTypeActivity.kt:easeCamera"
```

<figure markdown="span">
  <video preload="none" controls width="250" poster="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/ease_animation_thumbnail.jpg">
    <source src="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/ease_animation.mp4" />
  </video>
</figure>


### Animate

The `MapLibreMap.animateCamera` uses a powered flight animation move to the camera position provided[^1].

[^1]: The implementation is based on  Van Wijk, Jarke J.; Nuij, Wim A. A. “Smooth and efficient zooming and panning.” INFOVIS ’03. pp. 15–22. [https://www.win.tue.nl/~vanwijk/zoompan.pdf#page=5](https://www.win.tue.nl/~vanwijk/zoompan.pdf#page=5)

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraAnimationTypeActivity.kt:animateCamera"
```

<figure markdown="span">
  <video preload="none" controls width="250" poster="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/animate_animation_thumbnail.jpg">
    <source src="https://dwxvn1oqw6mkc.cloudfront.net/android-documentation-resources/animate_animation.mp4" />
  </video>
</figure>

## Animation Callbacks

In the previous section a `CancellableCallback` was passed to the last two animation methods. This callback shows a toast message when the animation is cancelled or when it is finished.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/camera/CameraAnimationTypeActivity.kt:callback"
```
