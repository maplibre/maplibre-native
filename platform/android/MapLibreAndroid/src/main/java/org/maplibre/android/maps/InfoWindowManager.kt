package org.maplibre.android.maps

import org.maplibre.android.annotations.InfoWindow
import org.maplibre.android.annotations.Marker

/**
 * Responsible for managing InfoWindows shown on the Map.
 *
 * Maintains a [List] of opened [InfoWindow] and tracks configurations as
 * allowConcurrentMultipleInfoWindows which allows to have multiple [InfoWindow] open at the
 * same time. Responsible for managing listeners as
 * [MapLibreMap.OnInfoWindowClickListener] and
 * [MapLibreMap.OnInfoWindowLongClickListener].
 */
internal class InfoWindowManager {
    private val infoWindows: MutableList<InfoWindow> = ArrayList()

    private var infoWindowAdapter: MapLibreMap.InfoWindowAdapter? = null
    private var allowConcurrentMultipleInfoWindows = false

    private var onInfoWindowClickListener: MapLibreMap.OnInfoWindowClickListener? = null
    private var onInfoWindowLongClickListener: MapLibreMap.OnInfoWindowLongClickListener? = null
    private var onInfoWindowCloseListener: MapLibreMap.OnInfoWindowCloseListener? = null

    fun update() {
        for (infoWindow in infoWindows) {
            infoWindow.update()
        }
    }

    fun setInfoWindowAdapter(infoWindowAdapter: MapLibreMap.InfoWindowAdapter?) {
        this.infoWindowAdapter = infoWindowAdapter
    }

    fun getInfoWindowAdapter(): MapLibreMap.InfoWindowAdapter? = infoWindowAdapter

    fun setAllowConcurrentMultipleOpenInfoWindows(allow: Boolean) {
        allowConcurrentMultipleInfoWindows = allow
    }

    fun isAllowConcurrentMultipleOpenInfoWindows(): Boolean = allowConcurrentMultipleInfoWindows

    fun isInfoWindowValidForMarker(marker: Marker?): Boolean =
        marker != null && (!marker.title.isNullOrEmpty() || !marker.snippet.isNullOrEmpty())

    fun setOnInfoWindowClickListener(listener: MapLibreMap.OnInfoWindowClickListener?) {
        onInfoWindowClickListener = listener
    }

    fun getOnInfoWindowClickListener(): MapLibreMap.OnInfoWindowClickListener? = onInfoWindowClickListener

    fun setOnInfoWindowLongClickListener(listener: MapLibreMap.OnInfoWindowLongClickListener?) {
        onInfoWindowLongClickListener = listener
    }

    fun getOnInfoWindowLongClickListener(): MapLibreMap.OnInfoWindowLongClickListener? = onInfoWindowLongClickListener

    fun setOnInfoWindowCloseListener(listener: MapLibreMap.OnInfoWindowCloseListener?) {
        onInfoWindowCloseListener = listener
    }

    fun getOnInfoWindowCloseListener(): MapLibreMap.OnInfoWindowCloseListener? = onInfoWindowCloseListener

    fun add(infoWindow: InfoWindow) {
        infoWindows.add(infoWindow)
    }
}
