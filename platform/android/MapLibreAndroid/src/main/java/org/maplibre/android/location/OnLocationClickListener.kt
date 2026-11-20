package org.maplibre.android.location

/**
 * The Location Component exposes an API for listening to when the user clicks on the location
 * layer icon visible on the map. When this event occurs, the [onLocationComponentClick] method
 * gets invoked.
 */
fun interface OnLocationClickListener {
    /**
     * Called whenever user clicks on the location layer drawn on the map.
     */
    fun onLocationComponentClick()
}
