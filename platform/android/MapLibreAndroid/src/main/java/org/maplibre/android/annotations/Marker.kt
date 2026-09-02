package org.maplibre.android.annotations

import androidx.annotation.Keep
import org.maplibre.android.R
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView

/**
 * Marker is an annotation that shows an icon image at a geographical location. The default marker
 * uses a provided icon. This icon can be customized using [IconFactory] to generate an
 * [Icon] using a provided image. Markers are added to the map by first giving a
 * [LatLng] and using [MapLibreMap.addMarker]. The marker icon will be
 * centered at this position so it is common to add padding to the icon image before usage.
 *
 * Markers are designed to be interactive. They receive click events by default, and are often used
 * with event listeners to bring up info windows. An [InfoWindow] is displayed by default when
 * either a title or snippet is provided.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
open class Marker internal constructor() : Annotation() {
    /**
     * The position of the marker.
     */
    @field:Keep
    var position: LatLng? = null
        set(value) {
            field = value
            getMapLibreMap()?.updateMarker(this)
        }

    /**
     * The snippet of the marker. If `null`, the snippet is cleared.
     */
    var snippet: String? = null
        set(value) {
            field = value
            refreshInfoWindowContent()
        }

    /**
     * The title of the marker. If `null`, the title is cleared.
     */
    var title: String? = null
        set(value) {
            field = value
            refreshInfoWindowContent()
        }

    /**
     * The [Icon] currently used for the marker. If no Icon was set for the marker, the
     * default icon will be returned.
     */
    var icon: Icon? = null
        set(value) {
            field = value
            iconId = value?.id
            getMapLibreMap()?.updateMarker(this)
        }

    // Redundantly stored for JNI access
    @field:Keep
    private var iconId: String? = null

    /**
     * The [InfoWindow] the marker is using. If the marker hasn't had an info window
     * defined, this will be `null`.
     */
    var infoWindow: InfoWindow? = null
        private set

    /**
     * Do not use this property, used internally by the SDK.
     *
     * `true` if the infoWindow is shown.
     */
    var isInfoWindowShown: Boolean = false
        private set

    private var topOffsetPixels = 0
    private var rightOffsetPixels = 0

    /**
     * Creates a instance of [Marker] using the builder of Marker.
     *
     * @param baseMarkerOptions The builder used to construct the Marker.
     */
    constructor(baseMarkerOptions: BaseMarkerOptions<*, *>?) : this(
        baseMarkerOptions!!.markerPosition,
        baseMarkerOptions.markerIcon,
        baseMarkerOptions.markerTitle,
        baseMarkerOptions.markerSnippet,
    )

    internal constructor(position: LatLng?, icon: Icon?, title: String?, snippet: String?) : this() {
        this.position = position
        this.title = title
        this.snippet = snippet
        this.icon = icon
    }

    /**
     * Do not use this method, used internally by the SDK.
     */
    fun hideInfoWindow() {
        infoWindow?.close()
        isInfoWindowShown = false
    }

    /**
     * Update only for default Marker's InfoWindow content for Title and Snippet
     */
    private fun refreshInfoWindowContent() {
        val mapView = this.mapView
        val maplibreMap = this.maplibreMap
        if (isInfoWindowShown && mapView != null && maplibreMap != null && maplibreMap.infoWindowAdapter == null) {
            val infoWindow = getOrCreateInfoWindow(mapView)!!
            if (mapView.context != null) {
                infoWindow.adaptDefaultMarker(this, maplibreMap, mapView)
            }
            getMapLibreMap()?.updateMarker(this)
            infoWindow.onContentUpdate()
        }
    }

    /**
     * Do not use this method, used internally by the SDK. Use [MapLibreMap.selectMarker]
     * if you want to programmatically display the markers info window.
     *
     * @param maplibreMap The hosting MapLibreMap.
     * @param mapView     The hosting map view.
     * @return The info window that was shown.
     */
    open fun showInfoWindow(
        maplibreMap: MapLibreMap,
        mapView: MapView,
    ): InfoWindow? {
        setMapLibreMap(maplibreMap)
        setMapView(mapView)
        val infoWindowAdapter = getMapLibreMap()!!.infoWindowAdapter
        if (infoWindowAdapter != null) {
            // end developer is using a custom InfoWindowAdapter
            val content = infoWindowAdapter.getInfoWindow(this)
            if (content != null) {
                val customInfoWindow = InfoWindow(content, maplibreMap)
                infoWindow = customInfoWindow
                return showInfoWindow(customInfoWindow, mapView)
            }
        }

        val infoWindow = getOrCreateInfoWindow(mapView)!!
        if (mapView.context != null) {
            infoWindow.adaptDefaultMarker(this, maplibreMap, mapView)
        }
        return showInfoWindow(infoWindow, mapView)
    }

    private fun showInfoWindow(
        iw: InfoWindow,
        mapView: MapView,
    ): InfoWindow {
        iw.open(mapView, this, position!!, rightOffsetPixels, topOffsetPixels)
        isInfoWindowShown = true
        return iw
    }

    private fun getOrCreateInfoWindow(mapView: MapView): InfoWindow? {
        if (infoWindow == null && mapView.context != null) {
            infoWindow = InfoWindow(mapView, R.layout.maplibre_infowindow_content, getMapLibreMap())
        }
        return infoWindow
    }

    /**
     * Do not use this method, used internally by the SDK.
     *
     * @param topOffsetPixels the top offset pixels.
     */
    fun setTopOffsetPixels(topOffsetPixels: Int) {
        this.topOffsetPixels = topOffsetPixels
    }

    /**
     * Do not use this method, used internally by the SDK.
     *
     * @param rightOffsetPixels the right offset pixels.
     */
    fun setRightOffsetPixels(rightOffsetPixels: Int) {
        this.rightOffsetPixels = rightOffsetPixels
    }

    /**
     * Returns a String with the marker position.
     *
     * @return A String with the marker position.
     */
    override fun toString(): String = "Marker [position[$position]]"
}
