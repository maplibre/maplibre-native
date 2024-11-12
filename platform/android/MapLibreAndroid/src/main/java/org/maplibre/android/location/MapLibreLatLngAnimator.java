package org.maplibre.android.location;

import android.animation.TypeEvaluator;

import androidx.annotation.NonNull;

import org.maplibre.android.geometry.LatLng;

class MapLibreLatLngAnimator extends MapLibreAnimator<LatLng> {

  MapLibreLatLngAnimator(@NonNull LatLng[] values, @NonNull AnimationsValueChangeListener updateListener,
                         int maxAnimationFps) {
    super(values, updateListener, maxAnimationFps);
  }

  @NonNull
  @Override
  TypeEvaluator provideEvaluator() {
    return new LatLngEvaluator();
  }
}
