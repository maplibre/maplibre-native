package org.maplibre.android.location;

import android.animation.FloatEvaluator;
import android.animation.TypeEvaluator;

import androidx.annotation.NonNull;
import androidx.annotation.Size;

class MapLibreFloatAnimator extends MapLibreAnimator<Float> {
  MapLibreFloatAnimator(@NonNull @Size(min = 2) Float[] values,
                        @NonNull AnimationsValueChangeListener updateListener, int maxAnimationFps) {
    super(values, updateListener, maxAnimationFps);
  }

  @NonNull
  @Override
  TypeEvaluator provideEvaluator() {
    return new FloatEvaluator();
  }
}
