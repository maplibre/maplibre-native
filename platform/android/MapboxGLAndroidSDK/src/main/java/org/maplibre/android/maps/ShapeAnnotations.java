package org.maplibre.android.maps;

import android.graphics.RectF;

import org.maplibre.android.annotations.Annotation;

import java.util.List;

interface ShapeAnnotations {

  List<Annotation> obtainAllIn(RectF rectF);

}
