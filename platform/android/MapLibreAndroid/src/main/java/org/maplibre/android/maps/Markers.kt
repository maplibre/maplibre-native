package org.maplibre.android.maps;

import android.graphics.RectF;

import androidx.annotation.NonNull;

import org.maplibre.android.annotations.BaseMarkerOptions;
import org.maplibre.android.annotations.Marker;

import java.util.List;

/**
 * Interface that defines convenient methods for working with a {@link Marker}'s collection.
 */
interface Markers {
  Marker addBy(@NonNull BaseMarkerOptions markerOptions, @NonNull MapLibreMap maplibreMap);

  List<Marker> addBy(@NonNull List<? extends BaseMarkerOptions> markerOptionsList, @NonNull MapLibreMap maplibreMap);

  void update(@NonNull Marker updatedMarker, @NonNull MapLibreMap maplibreMap);

  List<Marker> obtainAll();

  @NonNull
  List<Marker> obtainAllIn(@NonNull RectF rectangle);

  void reload();
}
