package org.maplibre.android.maps;


import androidx.annotation.NonNull;

import org.maplibre.android.annotations.Polyline;
import org.maplibre.android.annotations.PolylineOptions;

import java.util.List;

/**
 * Interface that defines convenient methods for working with a {@link Polyline}'s collection.
 */
interface Polylines {
  Polyline addBy(@NonNull PolylineOptions polylineOptions, @NonNull MapLibreMap maplibreMap);

  List<Polyline> addBy(@NonNull List<PolylineOptions> polylineOptionsList, @NonNull MapLibreMap maplibreMap);

  void update(Polyline polyline);

  List<Polyline> obtainAll();
}
