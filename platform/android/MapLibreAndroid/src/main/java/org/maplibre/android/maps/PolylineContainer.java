package org.maplibre.android.maps;


import androidx.annotation.NonNull;
import androidx.collection.LongSparseArray;

import org.maplibre.android.annotations.Annotation;
import org.maplibre.android.annotations.Polyline;
import org.maplibre.android.annotations.PolylineOptions;

import java.util.ArrayList;
import java.util.List;

/**
 * Encapsulates {@link Polyline}'s functionality.
 */
class PolylineContainer implements Polylines {

  private final NativeMap nativeMap;
  private final LongSparseArray<Annotation> annotations;

  PolylineContainer(NativeMap nativeMap, LongSparseArray<Annotation> annotations) {
    this.nativeMap = nativeMap;
    this.annotations = annotations;
  }

  @Override
  public Polyline addBy(@NonNull PolylineOptions polylineOptions, @NonNull MapLibreMap maplibreMap) {
    Polyline polyline = polylineOptions.getPolyline();
    long id = nativeMap != null ? nativeMap.addPolyline(polyline) : 0;
    polyline.setMapLibreMap(maplibreMap);
    polyline.setId(id);
    annotations.put(id, polyline);
    return polyline;
  }

  @NonNull
  @Override
  public List<Polyline> addBy(@NonNull List<PolylineOptions> polylineOptionsList, @NonNull MapLibreMap maplibreMap) {
    int count = polylineOptionsList.size();
    Polyline polyline;
    List<Polyline> polylines = new ArrayList<>(count);
    if (nativeMap != null && count > 0) {
      for (PolylineOptions options : polylineOptionsList) {
        polyline = options.getPolyline();
        if (!polyline.getPoints().isEmpty()) {
          polylines.add(polyline);
        }
      }

      long[] ids = nativeMap.addPolylines(polylines);
      for (int i = 0; i < ids.length; i++) {
        Polyline polylineCreated = polylines.get(i);
        polylineCreated.setMapLibreMap(maplibreMap);
        polylineCreated.setId(ids[i]);
        annotations.put(ids[i], polylineCreated);
      }
    }
    return polylines;
  }

  @Override
  public void update(@NonNull Polyline polyline) {
    nativeMap.updatePolyline(polyline);
    annotations.setValueAt(annotations.indexOfKey(polyline.getId()), polyline);
  }

  @NonNull
  @Override
  public List<Polyline> obtainAll() {
    List<Polyline> polylines = new ArrayList<>();
    Annotation annotation;
    for (int i = 0; i < annotations.size(); i++) {
      annotation = annotations.get(annotations.keyAt(i));
      if (annotation instanceof Polyline) {
        polylines.add((Polyline) annotation);
      }
    }
    return polylines;
  }
}
