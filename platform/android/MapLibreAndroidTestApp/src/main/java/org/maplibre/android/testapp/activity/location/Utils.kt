package org.maplibre.android.testapp.activity.location

import android.location.Location
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber
import java.util.*

/**
 * Useful utilities used throughout the test app.
 */
object Utils {
    private val STYLES = arrayOf(
        TestStyles.getPredefinedStyleWithFallback("Streets"),
        TestStyles.getPredefinedStyleWithFallback("Outdoor"),
        TestStyles.getPredefinedStyleWithFallback("Bright"),
        TestStyles.getPredefinedStyleWithFallback("Pastel"),
        TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid")
    )
    private var index = 0

    /**
     * Utility to cycle through map styles. Useful to test if runtime styling source and layers transfer over to new
     * style.
     *
     * @return a string ID representing the map style
     */
    fun nextStyle(): String {
        index++
        if (index == STYLES.size) {
            index = 0
        }
        return STYLES[index]
    }

    /**
     * Utility for getting a random coordinate inside a provided bounds and creates a [Location] from it.
     *
     * @param bounds bounds of the generated location
     * @return a [Location] object using the random coordinate
     */
    fun getRandomLocation(bounds: LatLngBounds): Location {
        val random = Random()
        val randomLat = bounds.latitudeSouth + (bounds.latitudeNorth - bounds.latitudeSouth) * random.nextDouble()
        val randomLon = bounds.longitudeWest + (bounds.longitudeEast - bounds.longitudeWest) * random.nextDouble()
        val location = Location("random-loc")
        location.longitude = randomLon
        location.latitude = randomLat
        Timber.d("getRandomLatLng: %s", location.toString())
        return location
    }
}
