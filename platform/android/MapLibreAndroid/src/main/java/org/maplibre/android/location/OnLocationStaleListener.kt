package org.maplibre.android.location

/**
 * Listener that can be added as a callback when the last location update
 * is considered stale.
 *
 * The time from the last location update that determines if a location update
 * is stale or not is provided by [LocationComponentOptions.staleStateTimeout].
 */
fun interface OnLocationStaleListener {
    /**
     * Called when the stale state changes.
     * @param isStale true if location is stale, false otherwise
     */
    fun onStaleStateChange(isStale: Boolean)
}
