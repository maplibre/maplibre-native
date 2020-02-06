package com.mapbox.mapboxsdk.location;

import androidx.annotation.Nullable;

import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapboxMap;

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
                                             @Nullable MapboxMap.CancelableCallback cancelableCallback) {
    return new MapboxCameraAnimatorAdapter(values, updateListener, cancelableCallback);
  }
}
