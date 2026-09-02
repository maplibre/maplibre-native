package org.maplibre.android.maps

import androidx.collection.LongSparseArray
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.annotations.Polyline
import org.maplibre.android.annotations.PolylineOptions

/**
 * Encapsulates [Polyline]'s functionality.
 */
internal class PolylineContainer(
    private val nativeMap: NativeMap?,
    private val annotations: LongSparseArray<Annotation>,
) : Polylines {
    override fun addBy(
        polylineOptions: PolylineOptions,
        maplibreMap: MapLibreMap,
    ): Polyline {
        val polyline = polylineOptions.polyline
        val id = nativeMap?.addPolyline(polyline) ?: 0
        polyline.setMapLibreMap(maplibreMap)
        polyline.id = id
        annotations.put(id, polyline)
        return polyline
    }

    override fun addBy(
        polylineOptionsList: List<PolylineOptions>,
        maplibreMap: MapLibreMap,
    ): List<Polyline> {
        val count = polylineOptionsList.size
        val polylines = ArrayList<Polyline>(count)
        if (nativeMap != null && count > 0) {
            for (options in polylineOptionsList) {
                val polyline = options.polyline
                if (polyline.points.isNotEmpty()) {
                    polylines.add(polyline)
                }
            }

            val ids = nativeMap.addPolylines(polylines)
            for (i in ids.indices) {
                val polylineCreated = polylines[i]
                polylineCreated.setMapLibreMap(maplibreMap)
                polylineCreated.id = ids[i]
                annotations.put(ids[i], polylineCreated)
            }
        }
        return polylines
    }

    override fun update(polyline: Polyline) {
        nativeMap!!.updatePolyline(polyline)
        annotations.setValueAt(annotations.indexOfKey(polyline.id), polyline)
    }

    override fun obtainAll(): List<Polyline> {
        val polylines = mutableListOf<Polyline>()
        for (i in 0 until annotations.size()) {
            val annotation = annotations.get(annotations.keyAt(i))
            if (annotation is Polyline) {
                polylines.add(annotation)
            }
        }
        return polylines
    }
}
