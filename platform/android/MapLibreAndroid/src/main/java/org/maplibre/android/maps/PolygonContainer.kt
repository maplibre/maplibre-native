package org.maplibre.android.maps

import androidx.collection.LongSparseArray
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.annotations.Polygon
import org.maplibre.android.annotations.PolygonOptions

/**
 * Encapsulates [Polygon]'s functionality.
 */
internal class PolygonContainer(
    private val nativeMap: NativeMap?,
    private val annotations: LongSparseArray<Annotation>,
) : Polygons {
    override fun addBy(
        polygonOptions: PolygonOptions,
        maplibreMap: MapLibreMap,
    ): Polygon {
        val polygon = polygonOptions.polygon
        val id = nativeMap?.addPolygon(polygon) ?: 0
        polygon.id = id
        polygon.setMapLibreMap(maplibreMap)
        annotations.put(id, polygon)
        return polygon
    }

    override fun addBy(
        polygonOptionsList: List<PolygonOptions>,
        maplibreMap: MapLibreMap,
    ): List<Polygon> {
        val count = polygonOptionsList.size

        val polygons = ArrayList<Polygon>(count)
        if (nativeMap != null && count > 0) {
            for (polygonOptions in polygonOptionsList) {
                val polygon = polygonOptions.polygon
                if (polygon.points.isNotEmpty()) {
                    polygons.add(polygon)
                }
            }

            val ids = nativeMap.addPolygons(polygons)
            for (i in ids.indices) {
                val polygon = polygons[i]
                polygon.setMapLibreMap(maplibreMap)
                polygon.id = ids[i]
                annotations.put(ids[i], polygon)
            }
        }
        return polygons
    }

    override fun update(polygon: Polygon) {
        nativeMap!!.updatePolygon(polygon)
        annotations.setValueAt(annotations.indexOfKey(polygon.id), polygon)
    }

    override fun obtainAll(): List<Polygon> {
        val polygons = mutableListOf<Polygon>()
        for (i in 0 until annotations.size()) {
            val annotation = annotations.get(annotations.keyAt(i))
            if (annotation is Polygon) {
                polygons.add(annotation)
            }
        }
        return polygons
    }
}
