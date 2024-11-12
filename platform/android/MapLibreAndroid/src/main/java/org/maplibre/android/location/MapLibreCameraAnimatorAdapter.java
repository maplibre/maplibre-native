package org.maplibre.android.location;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import org.maplibre.android.maps.MapLibreMap;

class MapLibreCameraAnimatorAdapter extends MapLibreFloatAnimator {

  MapLibreCameraAnimatorAdapter(@NonNull @Size(min = 2) Float[] values,
                                AnimationsValueChangeListener updateListener,
                                @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    super(values, updateListener, Integer.MAX_VALUE);
    addListener(new MapLibreAnimatorListener(cancelableCallback));
  }
}
