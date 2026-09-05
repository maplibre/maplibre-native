package org.maplibre.android.maps

import androidx.collection.LongSparseArray
import org.maplibre.android.annotations.Annotation

/**
 * Encapsulates [Annotation]'s functionality..
 */
internal class AnnotationContainer(
    private val nativeMap: NativeMap?,
    private val annotations: LongSparseArray<Annotation>,
) : Annotations {
    override fun obtainBy(id: Long): Annotation? = annotations.get(id)

    override fun obtainAll(): List<Annotation> {
        val result = mutableListOf<Annotation>()
        for (i in 0 until annotations.size()) {
            annotations.get(annotations.keyAt(i))?.let { result.add(it) }
        }
        return result
    }

    override fun removeBy(id: Long) {
        nativeMap?.removeAnnotation(id)
        annotations.remove(id)
    }

    override fun removeBy(annotation: Annotation) {
        removeBy(annotation.id)
    }

    override fun removeBy(annotationList: List<Annotation>) {
        val ids = LongArray(annotationList.size) { annotationList[it].id }

        removeNativeAnnotations(ids)

        for (id in ids) {
            annotations.remove(id)
        }
    }

    override fun removeAll() {
        val ids = LongArray(annotations.size()) { annotations.keyAt(it) }

        removeNativeAnnotations(ids)

        annotations.clear()
    }

    private fun removeNativeAnnotations(ids: LongArray) {
        nativeMap?.removeAnnotations(ids)
    }
}
