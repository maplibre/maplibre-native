package org.maplibre.android.maps;


import androidx.annotation.NonNull;

import org.maplibre.android.annotations.Polygon;
import org.maplibre.android.annotations.PolygonOptions;

import java.util.List;

/**
 * Interface that defines convenient methods for working with a {@link Polygon}'s collection.
 */
interface Polygons {
  Polygon addBy(@NonNull PolygonOptions polygonOptions, @NonNull MapboxMap mapboxMap);

  List<Polygon> addBy(@NonNull List<PolygonOptions> polygonOptionsList, @NonNull MapboxMap mapboxMap);

  void update(Polygon polygon);

  List<Polygon> obtainAll();
}
