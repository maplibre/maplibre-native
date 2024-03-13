package com.mapbox.mapboxsdk.location;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;

import androidx.annotation.Nullable;

import com.mapbox.mapboxsdk.maps.MapboxMap;

class MapboxAnimatorListener extends AnimatorListenerAdapter {

  @Nullable
  private final MapboxMap.CancelableCallback cancelableCallback;

  MapboxAnimatorListener(@Nullable MapboxMap.CancelableCallback cancelableCallback) {
    this.cancelableCallback = cancelableCallback;
  }

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