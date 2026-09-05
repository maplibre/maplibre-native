package org.maplibre.android.maps

import android.graphics.RectF
import org.maplibre.android.annotations.Annotation

internal interface ShapeAnnotations {
    fun obtainAllIn(rectF: RectF): List<Annotation>
}
