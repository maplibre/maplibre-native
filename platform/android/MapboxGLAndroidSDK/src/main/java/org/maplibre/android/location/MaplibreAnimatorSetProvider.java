package org.maplibre.android.location;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.view.animation.Interpolator;

import androidx.annotation.NonNull;

import java.util.List;

class MaplibreAnimatorSetProvider {
  private static MaplibreAnimatorSetProvider instance;

  private MaplibreAnimatorSetProvider() {
    // private constructor
  }

  static MaplibreAnimatorSetProvider getInstance() {
    if (instance == null) {
      instance = new MaplibreAnimatorSetProvider();
    }
    return instance;
  }

  void startAnimation(@NonNull List<Animator> animators, @NonNull Interpolator interpolator,
                      long duration) {
    AnimatorSet locationAnimatorSet = new AnimatorSet();
    locationAnimatorSet.playTogether(animators);
    locationAnimatorSet.setInterpolator(interpolator);
    locationAnimatorSet.setDuration(duration);
    locationAnimatorSet.start();
  }
}
