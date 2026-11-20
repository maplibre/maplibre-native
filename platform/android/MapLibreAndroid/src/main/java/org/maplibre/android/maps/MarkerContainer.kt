package org.maplibre.android.maps

import android.graphics.RectF
import androidx.collection.LongSparseArray
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker

/**
 * Encapsulates [Marker]'s functionality.
 */
internal class MarkerContainer(
    private val nativeMapView: NativeMap?,
    private val annotations: LongSparseArray<Annotation>,
    private val iconManager: IconManager,
) : Markers {
    override fun addBy(
        markerOptions: BaseMarkerOptions<*, *>,
        maplibreMap: MapLibreMap,
    ): Marker {
        val marker = prepareMarker(markerOptions)
        val id = nativeMapView?.addMarker(marker) ?: 0
        marker.setMapLibreMap(maplibreMap)
        marker.id = id
        annotations.put(id, marker)
        return marker
    }

    override fun addBy(
        markerOptionsList: List<BaseMarkerOptions<*, *>>,
        maplibreMap: MapLibreMap,
    ): List<Marker> {
        val count = markerOptionsList.size
        val markers = ArrayList<Marker>(count)
        if (nativeMapView != null && count > 0) {
            for (markerOptions in markerOptionsList) {
                markers.add(prepareMarker(markerOptions))
            }

            if (markers.isNotEmpty()) {
                val ids = nativeMapView.addMarkers(markers)
                for (i in ids.indices) {
                    val createdMarker = markers[i]
                    createdMarker.setMapLibreMap(maplibreMap)
                    createdMarker.id = ids[i]
                    annotations.put(ids[i], createdMarker)
                }
            }
        }
        return markers
    }

    override fun update(
        updatedMarker: Marker,
        maplibreMap: MapLibreMap,
    ) {
        ensureIconLoaded(updatedMarker, maplibreMap)
        nativeMapView!!.updateMarker(updatedMarker)
        annotations.setValueAt(annotations.indexOfKey(updatedMarker.id), updatedMarker)
    }

    override fun obtainAll(): List<Marker> {
        val markers = mutableListOf<Marker>()
        for (i in 0 until annotations.size()) {
            val annotation = annotations.get(annotations.keyAt(i))
            if (annotation is Marker) {
                markers.add(annotation)
            }
        }
        return markers
    }

    override fun obtainAllIn(rectangle: RectF): List<Marker> {
        val rect = nativeMapView!!.getDensityDependantRectangle(rectangle)
        val ids = nativeMapView.queryPointAnnotations(rect)
        val idsList = ids.toList()

        val markers = ArrayList<Marker>(ids.size)
        for (annotation in obtainAnnotations()) {
            if (annotation is Marker && idsList.contains(annotation.id)) {
                markers.add(annotation)
            }
        }

        return ArrayList(markers)
    }

    override fun reload() {
        iconManager.reloadIcons()
        val count = annotations.size()
        for (i in 0 until count) {
            val annotation = annotations.get(i.toLong())
            if (annotation is Marker) {
                nativeMapView!!.removeAnnotation(annotation.id)
                annotation.id = nativeMapView.addMarker(annotation)
            }
        }
    }

    private fun prepareMarker(markerOptions: BaseMarkerOptions<*, *>): Marker {
        val marker = markerOptions.marker!!
        val icon = iconManager.loadIconForMarker(marker)
        marker.setTopOffsetPixels(iconManager.getTopOffsetPixelsForIcon(icon))
        return marker
    }

    private fun ensureIconLoaded(
        marker: Marker,
        maplibreMap: MapLibreMap,
    ) {
        iconManager.ensureIconLoaded(marker, maplibreMap)
    }

    private fun obtainAnnotations(): List<Annotation> {
        val result = mutableListOf<Annotation>()
        for (i in 0 until annotations.size()) {
            annotations.get(annotations.keyAt(i))?.let { result.add(it) }
        }
        return result
    }
}
