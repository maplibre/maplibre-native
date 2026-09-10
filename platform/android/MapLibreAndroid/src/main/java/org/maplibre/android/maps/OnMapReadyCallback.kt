package org.maplibre.android.maps

/**
 * Interface definition for a callback to be invoked when the map is ready to be used.
 *
 * Once an instance of this interface is set on a [MapFragment] or [MapView] object,
 * the onMapReady(MapLibreMap) method is triggered when the map is ready to be used and provides an instance of
 * [MapLibreMap].
 */
fun interface OnMapReadyCallback {
    /**
     * Called when the map is ready to be used.
     *
     * @param maplibreMap An instance of MapLibreMap associated with the [MapFragment] or
     *                    [MapView] that defines the callback.
     */
    fun onMapReady(maplibreMap: MapLibreMap)
}
