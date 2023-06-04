package org.maplibre.android.location;

import android.animation.ValueAnimator;
import android.view.animation.Interpolator;

import androidx.annotation.Nullable;

import org.maplibre.android.maps.MaplibreMap;
import org.maplibre.android.geometry.LatLng;

final class MaplibreAnimatorProvider {

  private static MaplibreAnimatorProvider INSTANCE;

  private MaplibreAnimatorProvider() {
    // private constructor
  }

  public static MaplibreAnimatorProvider getInstance() {
    if (INSTANCE == null) {
      INSTANCE = new MaplibreAnimatorProvider();
    }
    return INSTANCE;
  }

  MaplibreLatLngAnimator latLngAnimator(LatLng[] values, MaplibreAnimator.AnimationsValueChangeListener updateListener,
                                        int maxAnimationFps) {
    return new MaplibreLatLngAnimator(values, updateListener, maxAnimationFps);
  }

  MaplibreFloatAnimator floatAnimator(Float[] values, MaplibreAnimator.AnimationsValueChangeListener updateListener,
                                      int maxAnimationFps) {
    return new MaplibreFloatAnimator(values, updateListener, maxAnimationFps);
  }

  MaplibreCameraAnimatorAdapter cameraAnimator(Float[] values,
                                               MaplibreAnimator.AnimationsValueChangeListener updateListener,
                                               @Nullable MaplibreMap.CancelableCallback cancelableCallback) {
    return new MaplibreCameraAnimatorAdapter(values, updateListener, cancelableCallback);
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
  PulsingLocationCircleAnimator pulsingCircleAnimator(MaplibreAnimator.AnimationsValueChangeListener updateListener,
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
