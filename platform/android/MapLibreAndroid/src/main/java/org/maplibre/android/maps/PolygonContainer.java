package org.maplibre.android.maps;


import androidx.annotation.NonNull;
import androidx.collection.LongSparseArray;

import org.maplibre.android.annotations.Annotation;
import org.maplibre.android.annotations.Polygon;
import org.maplibre.android.annotations.PolygonOptions;

import java.util.ArrayList;
import java.util.List;

/**
 * Encapsulates {@link Polygon}'s functionality.
 */
class PolygonContainer implements Polygons {

  private final NativeMap nativeMap;
  private final LongSparseArray<Annotation> annotations;

  PolygonContainer(NativeMap nativeMap, LongSparseArray<Annotation> annotations) {
    this.nativeMap = nativeMap;
    this.annotations = annotations;
  }

  @Override
  public Polygon addBy(@NonNull PolygonOptions polygonOptions, @NonNull MapLibreMap maplibreMap) {
    Polygon polygon = polygonOptions.getPolygon();
    if (!polygon.getPoints().isEmpty()) {
      long id = nativeMap != null ? nativeMap.addPolygon(polygon) : 0;
      polygon.setId(id);
      polygon.setMapLibreMap(maplibreMap);
      annotations.put(id, polygon);
    }
    return polygon;
  }

  @NonNull
  @Override
  public List<Polygon> addBy(@NonNull List<PolygonOptions> polygonOptionsList, @NonNull MapLibreMap maplibreMap) {
    int count = polygonOptionsList.size();

    Polygon polygon;
    List<Polygon> polygons = new ArrayList<>(count);
    if (nativeMap != null && count > 0) {
      for (PolygonOptions polygonOptions : polygonOptionsList) {
        polygon = polygonOptions.getPolygon();
        if (!polygon.getPoints().isEmpty()) {
          polygons.add(polygon);
        }
      }

      long[] ids = nativeMap.addPolygons(polygons);
      for (int i = 0; i < ids.length; i++) {
        polygon = polygons.get(i);
        polygon.setMapLibreMap(maplibreMap);
        polygon.setId(ids[i]);
        annotations.put(ids[i], polygon);
      }
    }
    return polygons;
  }

  @Override
  public void update(@NonNull Polygon polygon) {
    nativeMap.updatePolygon(polygon);
    annotations.setValueAt(annotations.indexOfKey(polygon.getId()), polygon);
  }

  @NonNull
  @Override
  public List<Polygon> obtainAll() {
    List<Polygon> polygons = new ArrayList<>();
    Annotation annotation;
    for (int i = 0; i < annotations.size(); i++) {
      annotation = annotations.get(annotations.keyAt(i));
      if (annotation instanceof Polygon) {
        polygons.add((Polygon) annotation);
      }
    }
    return polygons;
  }
}
