package org.maplibre.android.maps

import org.maplibre.android.annotations.Polygon
import org.maplibre.android.annotations.PolygonOptions

/**
 * Interface that defines convenient methods for working with a [Polygon]'s collection.
 */
internal interface Polygons {
    fun addBy(
        polygonOptions: PolygonOptions,
        maplibreMap: MapLibreMap,
    ): Polygon

    fun addBy(
        polygonOptionsList: List<PolygonOptions>,
        maplibreMap: MapLibreMap,
    ): List<Polygon>

    fun update(polygon: Polygon)

    fun obtainAll(): List<Polygon>
}
