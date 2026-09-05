package org.maplibre.android.location.engine

import android.content.Intent
import android.location.Location
import android.location.LocationManager

/**
 * A wrapper class representing location result from the location engine.
 *
 * TODO: Override default equals(), hashCode() and toString()
 *
 * @since 1.0.0
 */
class LocationEngineResult private constructor(
    locations: List<Location>,
) {
    /**
     * Locations computed, ordered from oldest to newest.
     *
     * @since 1.0.0
     */
    val locations: List<Location> = locations.toList()

    /**
     * Most recent location available in this result, or null.
     *
     * @since 1.0.0
     */
    val lastLocation: Location?
        get() = locations.firstOrNull()

    companion object {
        /**
         * Creates [LocationEngineResult] instance for location.
         *
         * @param location default location added to the result.
         * @return instance of the new location result.
         * @since 1.0.0
         */
        @JvmStatic
        fun create(location: Location?): LocationEngineResult = LocationEngineResult(listOfNotNull(location))

        /**
         * Creates [LocationEngineResult] instance for given list of locations.
         *
         * @param locations list of locations.
         * @return instance of the new location result.
         * @since 1.0.0
         */
        @JvmStatic
        fun create(locations: List<Location?>?): LocationEngineResult = LocationEngineResult(locations?.filterNotNull() ?: emptyList())

        /**
         * Extracts location result from intent object
         *
         * @param intent valid intent object
         * @return location result.
         * @since 1.1.0
         */
        @JvmStatic
        fun extractResult(intent: Intent?): LocationEngineResult? = extractAndroidResult(intent)

        private fun extractAndroidResult(intent: Intent?): LocationEngineResult? =
            if (!hasResult(intent)) {
                null
            } else {
                create(intent!!.extras!!.getParcelable<Location>(LocationManager.KEY_LOCATION_CHANGED))
            }

        private fun hasResult(intent: Intent?): Boolean = intent != null && intent.hasExtra(LocationManager.KEY_LOCATION_CHANGED)
    }
}
