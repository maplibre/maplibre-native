package org.maplibre.android.location;

import android.animation.ValueAnimator;
import android.view.animation.Interpolator;

import androidx.annotation.Nullable;

import org.maplibre.android.maps.MaplibreMap;
import org.maplibre.android.geometry.LatLng;

final class MapboxAnimatorProvider {

  private static MapboxAnimatorProvider INSTANCE;

  private MapboxAnimatorProvider() {
    // private constructor
  }

  public static MapboxAnimatorProvider getInstance() {
    if (INSTANCE == null) {
      INSTANCE = new MapboxAnimatorProvider();
    }
    return INSTANCE;
  }

  MapboxLatLngAnimator latLngAnimator(LatLng[] values, MapboxAnimator.AnimationsValueChangeListener updateListener,
                                      int maxAnimationFps) {
    return new MapboxLatLngAnimator(values, updateListener, maxAnimationFps);
  }

  MapboxFloatAnimator floatAnimator(Float[] values, MapboxAnimator.AnimationsValueChangeListener updateListener,
                                    int maxAnimationFps) {
    return new MapboxFloatAnimator(values, updateListener, maxAnimationFps);
  }

  MapboxCameraAnimatorAdapter cameraAnimator(Float[] values,
                                             MapboxAnimator.AnimationsValueChangeListener updateListener,
                                             @Nullable MaplibreMap.CancelableCallback cancelableCallback) {
    return new MapboxCameraAnimatorAdapter(values, updateListener, cancelableCallback);
  }

  /**
   * This animator is for the LocationComponent pulsing circle.
   *
   * @param updateListener the listener that is found in the {@link LocationAnimatorCoordinator}'s
   *                       listener array.
   * @param maxAnimationFps the max frames per second of the pulsing animation
   * @param pulseSingleDuration the number of milliseconds it takes for the animator to create
   *                            a single pulse.
   * @param pulseMaxRadius the max radius when the circle is finished with a single pulse.
   * @param pulseInterpolator the type of Android-system interpolator to use for
   *                                       the pulsing animation (linear, accelerate, bounce, etc.)
   * @return a built {@link PulsingLocationCircleAnimator} object.
   */
  PulsingLocationCircleAnimator pulsingCircleAnimator(MapboxAnimator.AnimationsValueChangeListener updateListener,
                                                      int maxAnimationFps,
                                                      float pulseSingleDuration,
                                                      float pulseMaxRadius,
                                                      Interpolator pulseInterpolator) {
    PulsingLocationCircleAnimator pulsingLocationCircleAnimator =
        new PulsingLocationCircleAnimator(updateListener, maxAnimationFps, pulseMaxRadius);
    pulsingLocationCircleAnimator.setDuration((long) pulseSingleDuration);
    pulsingLocationCircleAnimator.setRepeatMode(ValueAnimator.RESTART);
    pulsingLocationCircleAnimator.setRepeatCount(ValueAnimator.INFINITE);
    pulsingLocationCircleAnimator.setInterpolator(pulseInterpolator);
    return pulsingLocationCircleAnimator;
  }
}
