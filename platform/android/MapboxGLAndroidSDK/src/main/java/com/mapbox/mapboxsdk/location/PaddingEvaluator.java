package com.mapbox.mapboxsdk.location;

import android.animation.TypeEvaluator;

import androidx.annotation.NonNull;
import androidx.annotation.Size;

class PaddingEvaluator implements TypeEvaluator<double[]> {
  private final double[] padding = new double[4];

  @NonNull
  @Override
  public double[] evaluate(float fraction, @NonNull @Size(min = 4) double[] startValue,
                           @NonNull @Size(min = 4) double[] endValue) {
    padding[0] = startValue[0] + (endValue[0] - startValue[0]) * fraction;
    padding[1] = startValue[1] + (endValue[1] - startValue[1]) * fraction;
    padding[2] = startValue[2] + (endValue[2] - startValue[2]) * fraction;
    padding[3] = startValue[3] + (endValue[3] - startValue[3]) * fraction;
    return padding;
  }
}