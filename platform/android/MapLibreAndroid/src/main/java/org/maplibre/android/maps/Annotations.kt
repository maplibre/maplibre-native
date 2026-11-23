package org.maplibre.android.maps

import org.maplibre.android.annotations.Annotation

/**
 * Interface that defines convenient methods for working with a [Annotation]'s collection.
 */
internal interface Annotations {
    fun obtainBy(id: Long): Annotation?

    fun obtainAll(): List<Annotation>

    fun removeBy(id: Long)

    fun removeBy(annotation: Annotation)

    fun removeBy(annotationList: List<Annotation>)

    fun removeAll()
}
