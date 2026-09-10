package org.maplibre.android.maps

import android.graphics.RectF
import androidx.collection.LongSparseArray
import org.maplibre.android.annotations.Annotation

internal class ShapeAnnotationContainer(
    private val nativeMapView: NativeMap,
    private val annotations: LongSparseArray<Annotation>,
) : ShapeAnnotations {
    override fun obtainAllIn(rectF: RectF): List<Annotation> {
        val rect = nativeMapView.getDensityDependantRectangle(rectF)
        val annotationIds = nativeMapView.queryShapeAnnotations(rect)
        return getAnnotationsFromIds(annotationIds)
    }

    private fun getAnnotationsFromIds(annotationIds: LongArray): List<Annotation> {
        val shapeAnnotations = mutableListOf<Annotation>()
        for (annotationId in annotationIds) {
            annotations.get(annotationId)?.let { shapeAnnotations.add(it) }
        }
        return shapeAnnotations
    }
}
