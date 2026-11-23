package org.maplibre.android.maps

import android.graphics.RectF
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker

/**
 * Interface that defines convenient methods for working with a [Marker]'s collection.
 */
internal interface Markers {
    fun addBy(
        markerOptions: BaseMarkerOptions<*, *>,
        maplibreMap: MapLibreMap,
    ): Marker

    fun addBy(
        markerOptionsList: List<BaseMarkerOptions<*, *>>,
        maplibreMap: MapLibreMap,
    ): List<Marker>

    fun update(
        updatedMarker: Marker,
        maplibreMap: MapLibreMap,
    )

    fun obtainAll(): List<Marker>

    fun obtainAllIn(rectangle: RectF): List<Marker>

    fun reload()
}
