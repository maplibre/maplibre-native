package org.maplibre.android.location;

import android.animation.ValueAnimator;
import android.view.animation.Interpolator;

import androidx.annotation.Nullable;

import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.geometry.LatLng;

final class MapLibreAnimatorProvider {

  private static MapLibreAnimatorProvider INSTANCE;

  private MapLibreAnimatorProvider() {
    // private constructor
  }

  public static MapLibreAnimatorProvider getInstance() {
    if (INSTANCE == null) {
      INSTANCE = new MapLibreAnimatorProvider();
    }
    return INSTANCE;
  }

  MapLibreLatLngAnimator latLngAnimator(LatLng[] values, MapLibreAnimator.AnimationsValueChangeListener updateListener,
                                        int maxAnimationFps) {
    return new MapLibreLatLngAnimator(values, updateListener, maxAnimationFps);
  }

  MapLibreFloatAnimator floatAnimator(Float[] values, MapLibreAnimator.AnimationsValueChangeListener updateListener,
                                      int maxAnimationFps) {
    return new MapLibreFloatAnimator(values, updateListener, maxAnimationFps);
  }

  MapLibreCameraAnimatorAdapter cameraAnimator(Float[] values,
                                               MapLibreAnimator.AnimationsValueChangeListener updateListener,
                                               @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    return new MapLibreCameraAnimatorAdapter(values, updateListener, cancelableCallback);
  }

  MapLibrePaddingAnimator paddingAnimator(double[][] values,
                                        MapLibreAnimator.AnimationsValueChangeListener<double[]> updateListener,
                                        @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    return new MapLibrePaddingAnimator(values, updateListener, cancelableCallback);
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
  PulsingLocationCircleAnimator pulsingCircleAnimator(MapLibreAnimator.AnimationsValueChangeListener updateListener,
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
