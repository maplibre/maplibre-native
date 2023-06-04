package org.maplibre.android.location;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import org.maplibre.android.maps.MaplibreMap;

class MapboxCameraAnimatorAdapter extends MapboxFloatAnimator {
  @Nullable
  private final MaplibreMap.CancelableCallback cancelableCallback;

  MapboxCameraAnimatorAdapter(@NonNull @Size(min = 2) Float[] values,
                              AnimationsValueChangeListener updateListener,
                              @Nullable MaplibreMap.CancelableCallback cancelableCallback) {
    super(values, updateListener, Integer.MAX_VALUE);
    this.cancelableCallback = cancelableCallback;
    addListener(new MapboxAnimatorListener());
  }

  private final class MapboxAnimatorListener extends AnimatorListenerAdapter {
    @Override
    public void onAnimationCancel(Animator animation) {
      if (cancelableCallback != null) {
        cancelableCallback.onCancel();
      }
    }

    @Override
    public void onAnimationEnd(Animator animation) {
      if (cancelableCallback != null) {
        cancelableCallback.onFinish();
      }
    }
  }
}
