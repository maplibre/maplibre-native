package org.maplibre.android.maps

import android.graphics.PointF
import androidx.annotation.FloatRange
import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.geometry.ProjectedMeters
import org.maplibre.android.geometry.VisibleRegion
import kotlin.math.abs
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.ln
import kotlin.math.sin

/**
 * A projection is used to translate between on screen location and geographic coordinates on
 * the surface of the Earth. Screen location is in screen pixels (not display pixels)
 * with respect to the top left corner of the map (and not necessarily of the whole screen).
 */
class Projection internal constructor(
    private val nativeMapView: NativeMap,
    private val mapView: MapView,
) {
    internal var contentPadding: IntArray
        get() {
            val padding = nativeMapView.contentPadding ?: return intArrayOf(0, 0, 0, 0)
            return intArrayOf(padding[0].toInt(), padding[1].toInt(), padding[2].toInt(), padding[3].toInt())
        }
        set(value) {
            val output = DoubleArray(value.size) { value[it].toDouble() }
            nativeMapView.contentPadding = output
        }

    /**
     * @deprecated unused
     */
    @Deprecated("unused")
    fun invalidateContentPadding() = Unit

    /**
     * Returns the spherical Mercator projected meters for a LatLng.
     */
    fun getProjectedMetersForLatLng(latLng: LatLng): ProjectedMeters = nativeMapView.projectedMetersForLatLng(latLng)

    /**
     * Returns the LatLng for a spherical Mercator projected meters.
     */
    fun getLatLngForProjectedMeters(projectedMeters: ProjectedMeters): LatLng = nativeMapView.latLngForProjectedMeters(projectedMeters)

    /**
     * Returns the distance spanned by one pixel at the specified latitude and current zoom level.
     *
     * The distance between pixels decreases as the latitude approaches the poles.
     * This relationship parallels the relationship between longitudinal coordinates at different latitudes.
     *
     * @param latitude The latitude for which to return the value.
     * @return The distance measured in meters.
     */
    fun getMetersPerPixelAtLatitude(
        @FloatRange(from = -90.0, to = 90.0) latitude: Double,
    ): Double = nativeMapView.getMetersPerPixelAtLatitude(latitude)

    /**
     * Returns the geographic location that corresponds to a screen location.
     * The screen location is specified in screen pixels (not display pixels) relative to the
     * top left of the map (not the top left of the whole screen).
     *
     * @param point A Point on the screen in screen pixels.
     * @return The LatLng corresponding to the point on the screen, or null if the ray through
     * the given screen point does not intersect the ground plane.
     */
    fun fromScreenLocation(point: PointF): LatLng = nativeMapView.latLngForPixel(point)

    /**
     * Returns the geographic locations that corresponds to screen locations.
     * The screen locations are specified in screen pixels (not display pixels) relative to the
     * top left of the map (not the top left of the whole screen).
     *
     * @param input  an array of input values representing screen coordinates
     * @param output an array of output values representing geographic locations
     */
    fun fromScreenLocations(
        input: DoubleArray,
        output: DoubleArray,
    ) {
        nativeMapView.latLngsForPixels(input, output)
    }

    /**
     * Gets a projection of the viewing frustum for converting between screen coordinates and
     * geo-latitude/longitude coordinates.
     *
     * This method ignores the content padding.
     *
     * @return The projection of the viewing frustum in its current state.
     */
    val visibleRegion: VisibleRegion
        get() = getVisibleRegion(true)

    /**
     * Gets a projection of the viewing frustum for converting between screen coordinates and
     * geo-latitude/longitude coordinates.
     *
     * @param ignorePadding True if the padding should be ignored,
     *                      false if the returned region should be reduced by the padding.
     * @return The projection of the viewing frustum in its current state.
     */
    fun getVisibleRegion(ignorePadding: Boolean): VisibleRegion {
        val left: Float
        val right: Float
        val top: Float
        val bottom: Float

        if (ignorePadding) {
            left = 0f
            right = mapView.width.toFloat()
            top = 0f
            bottom = mapView.height.toFloat()
        } else {
            val contentPadding = this.contentPadding
            left = contentPadding[0].toFloat()
            right = (mapView.width - contentPadding[2]).toFloat()
            top = contentPadding[1].toFloat()
            bottom = (mapView.height - contentPadding[3]).toFloat()
        }

        val center = fromScreenLocation(PointF(left + (right - left) / 2, top + (bottom - top) / 2))

        val topLeft = fromScreenLocation(PointF(left, top))
        val topRight = fromScreenLocation(PointF(right, top))
        val bottomRight = fromScreenLocation(PointF(right, bottom))
        val bottomLeft = fromScreenLocation(PointF(left, bottom))

        val latLngs = listOf(topRight, bottomRight, bottomLeft, topLeft)

        var maxEastLonSpan = 0.0
        var maxWestLonSpan = 0.0

        var east = 0.0
        var west = 0.0
        var north = GeometryConstants.MIN_LATITUDE
        var south = GeometryConstants.MAX_LATITUDE

        for (latLng in latLngs) {
            val bearing = bearing(center, latLng)

            if (bearing >= 0) {
                val span = getLongitudeSpan(latLng.longitude, center.longitude)
                if (span > maxEastLonSpan) {
                    maxEastLonSpan = span
                    east = latLng.longitude
                }
            } else {
                val span = getLongitudeSpan(center.longitude, latLng.longitude)
                if (span > maxWestLonSpan) {
                    maxWestLonSpan = span
                    west = latLng.longitude
                }
            }

            if (north < latLng.latitude) {
                north = latLng.latitude
            }
            if (south > latLng.latitude) {
                south = latLng.latitude
            }
        }

        if (east < west) {
            return VisibleRegion(
                topLeft,
                topRight,
                bottomLeft,
                bottomRight,
                LatLngBounds.from(north, east + GeometryConstants.LONGITUDE_SPAN, south, west),
            )
        }
        return VisibleRegion(topLeft, topRight, bottomLeft, bottomRight, LatLngBounds.from(north, east, south, west))
    }

    /**
     * Gets a projection of the viewing frustum for converting between screen coordinates and
     * geo-latitude/longitude coordinate bounds.
     *
     * This method ignores the content padding.
     *
     * @param bounds an array of 4 output values representing bounds(in the order of latNorth,
     *               lonEast, latSouth, lonWest).
     */
    fun getVisibleCoordinateBounds(bounds: DoubleArray) {
        nativeMapView.getVisibleCoordinateBounds(bounds)
    }

    /**
     * Returns a screen location that corresponds to a geographical coordinate (LatLng).
     * The screen location is in screen pixels (not display pixels) relative to the top left
     * of the map (not of the whole screen).
     *
     * @param location A LatLng on the map to convert to a screen location.
     * @return A Point representing the screen location in screen pixels.
     */
    fun toScreenLocation(location: LatLng): PointF = nativeMapView.pixelForLatLng(location)

    /**
     * Returns a screen locations that corresponds to a geographical coordinates.
     * The screen locations are in screen pixels (not display pixels) relative to the top left
     * of the map (not of the whole screen).
     *
     * @param input  an array of input values representing geographic locations
     * @param output an array of output values representing screen coordinates
     */
    fun toScreenLocations(
        input: DoubleArray,
        output: DoubleArray,
    ) {
        nativeMapView.pixelsForLatLngs(input, output)
    }

    internal fun getHeight(): Float = mapView.height.toFloat()

    internal fun getWidth(): Float = mapView.width.toFloat()

    /**
     * Calculates a zoom level based on minimum scale and current scale from MapView
     *
     * @param minScale The minimum scale to calculate the zoom level.
     * @return zoom level that fits the MapView.
     */
    fun calculateZoom(minScale: Float): Double = nativeMapView.zoom + ln(minScale.toDouble()) / ln(2.0)

    internal companion object {
        /**
         * Takes two [org.maplibre.geojson.Point]s and finds the geographic bearing between them.
         *
         * @param latLng1 the first point used for calculating the bearing
         * @param latLng2 the second point used for calculating the bearing
         * @return bearing in decimal degrees
         * @see <a href="http://turfjs.org/docs/#bearing">Turf Bearing documentation</a>
         */
        fun bearing(
            latLng1: LatLng,
            latLng2: LatLng,
        ): Double {
            val lon1 = degreesToRadians(latLng1.longitude)
            val lon2 = degreesToRadians(latLng2.longitude)
            val lat1 = degreesToRadians(latLng1.latitude)
            val lat2 = degreesToRadians(latLng2.latitude)

            val value1 = sin(lon2 - lon1) * cos(lat2)
            val value2 = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1)

            return radiansToDegrees(atan2(value1, value2))
        }

        /**
         * Converts an angle in degrees to radians.
         *
         * @param degrees angle between 0 and 360 degrees
         * @return angle in radians
         */
        fun degreesToRadians(degrees: Double): Double {
            val radians = degrees % 360
            return radians * Math.PI / 180
        }

        /**
         * Converts an angle in radians to degrees.
         *
         * @param radians angle in radians
         * @return degrees between 0 and 360 degrees
         */
        fun radiansToDegrees(radians: Double): Double {
            val degrees = radians % (2 * Math.PI)
            return degrees * 180 / Math.PI
        }

        /**
         * Get the absolute distance, in degrees, between the west and
         * east boundaries of this LatLngBounds
         *
         * @return Span distance
         */
        fun getLongitudeSpan(
            east: Double,
            west: Double,
        ): Double {
            val longSpan = abs(east - west)
            if (east > west) {
                return longSpan
            }

            // shortest span contains antimeridian
            return GeometryConstants.LONGITUDE_SPAN - longSpan
        }
    }
}
