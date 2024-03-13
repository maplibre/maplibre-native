package com.mapbox.mapboxsdk.location;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import com.mapbox.mapboxsdk.maps.MapboxMap;

class MapboxCameraAnimatorAdapter extends MapboxFloatAnimator {
  MapboxCameraAnimatorAdapter(@NonNull @Size(min = 2) Float[] values,
                              AnimationsValueChangeListener updateListener,
                              @Nullable MapboxMap.CancelableCallback cancelableCallback) {
    super(values, updateListener, Integer.MAX_VALUE);
    addListener(new MapboxAnimatorListener(cancelableCallback));
  }
}