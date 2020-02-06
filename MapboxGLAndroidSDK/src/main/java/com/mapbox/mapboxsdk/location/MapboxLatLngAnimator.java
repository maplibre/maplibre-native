package com.mapbox.mapboxsdk.location;

import android.animation.TypeEvaluator;

import androidx.annotation.NonNull;

import com.mapbox.mapboxsdk.geometry.LatLng;

class MapboxLatLngAnimator extends MapboxAnimator<LatLng> {

  MapboxLatLngAnimator(@NonNull LatLng[] values, @NonNull AnimationsValueChangeListener updateListener,
                       int maxAnimationFps) {
    super(values, updateListener, maxAnimationFps);
  }

  @NonNull
  @Override
  TypeEvaluator provideEvaluator() {
    return new LatLngEvaluator();
  }
}
