package org.maplibre.android.maps

import org.maplibre.android.annotations.Polyline
import org.maplibre.android.annotations.PolylineOptions

/**
 * Interface that defines convenient methods for working with a [Polyline]'s collection.
 */
internal interface Polylines {
    fun addBy(
        polylineOptions: PolylineOptions,
        maplibreMap: MapLibreMap,
    ): Polyline

    fun addBy(
        polylineOptionsList: List<PolylineOptions>,
        maplibreMap: MapLibreMap,
    ): List<Polyline>

    fun update(polyline: Polyline)

    fun obtainAll(): List<Polyline>
}
