package org.maplibre.android.constants

/**
 * Contains constants used throughout the sdk classes.
 */
object GeometryConstants {
    /**
     * The [equatorial radius](http://en.wikipedia.org/wiki/Earth_radius#Equatorial_radius)
     * value in meters
     */
    const val RADIUS_EARTH_METERS: Int = 6378137

    /**
     * This constant represents the lowest longitude value available to represent a wrapped geolocation.
     */
    const val MIN_WRAP_LONGITUDE: Double = -180.0

    /**
     * This constant represents the highest longitude value available to represent a wrapped geolocation.
     */
    const val MAX_WRAP_LONGITUDE: Double = 180.0

    /**
     * This constant represents the lowest longitude value available to represent a geolocation.
     */
    const val MIN_LONGITUDE: Double = -Double.MAX_VALUE

    /**
     * This constant represents the highest longitude value available to represent a geolocation.
     */
    const val MAX_LONGITUDE: Double = Double.MAX_VALUE

    /**
     * This constant represents the lowest latitude value available to represent a geolocation.
     */
    const val MIN_LATITUDE: Double = -90.0

    /**
     * This constant represents the latitude span when representing a geolocation.
     */
    const val LATITUDE_SPAN: Double = 180.0

    /**
     * This constant represents the longitude span when representing a geolocation.
     */
    const val LONGITUDE_SPAN: Double = 360.0

    /**
     * This constant represents the highest latitude value available to represent a geolocation.
     */
    const val MAX_LATITUDE: Double = 90.0

    /**
     * Maximum latitude value in Mercator projection.
     */
    const val MAX_MERCATOR_LATITUDE: Double = 85.05112877980659

    /**
     * Minimum latitude value in Mercator projection.
     */
    const val MIN_MERCATOR_LATITUDE: Double = -85.05112877980659
}
