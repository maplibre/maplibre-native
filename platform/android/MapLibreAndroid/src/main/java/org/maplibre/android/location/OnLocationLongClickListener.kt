package org.maplibre.android.location

/**
 * The Location Component exposes an API for listening to when the user long clicks on the location
 * layer icon visible on the map. when this event occurs, the [onLocationComponentLongClick] method
 * gets invoked.
 */
fun interface OnLocationLongClickListener {
    /**
     * Called whenever user long clicks on the location layer drawn on the map.
     */
    fun onLocationComponentLongClick()
}
