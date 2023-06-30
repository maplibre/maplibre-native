package org.maplibre.android.geometry

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.FloatRange
import androidx.annotation.Keep
import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.exceptions.InvalidLatLngBoundsException
import org.maplibre.android.utils.isInfinite
import org.maplibre.android.utils.isNaN

/**
 * A geographical area representing a latitude/longitude aligned rectangle.
 *
 *
 * This class does not wrap values to the world bounds.
 *
 */

@Keep
class LatLngBounds
/**
 * Construct a new LatLngBounds based on its corners, given in NESW
 * order.
 *
 *
 * @since 7.0.0 LatLngBounds cannot be wrapped any more, i.e longitudeWest has to be
 * less or equal to longitudeEast.
 *
 * For example, to represent bounds spanning 20 degrees crossing antimeridian with
 * the NE point as (10, -170) and the SW point as (-10, 170),
 * use (10, -190) and (-10, -170), or (10, -170) and (-10, -150).
 *
 * @param latitudeNorth Northern Latitude
 * @param longitudeEast Eastern Longitude
 * @param latitudeSouth Southern Latitude
 * @param longitudeWest Western Longitude
 */ @Keep internal constructor(
    /**
     * Get the north latitude value of this bounds.
     *
     * @return double latitude value for north
     */
    @field:Keep
    @JvmField
    val latitudeNorth: Double,
    /**
     * Get the east longitude value of this bounds.
     *
     * @return double longitude value for east
     */
    @field:Keep
    @JvmField
    val longitudeEast: Double,
    /**
     * Get the south latitude value of this bounds.
     *
     * @return double latitude value for south
     */
    @field:Keep
    @JvmField
    val latitudeSouth: Double,
    /**
     * Get the west longitude value of this bounds.
     *
     * @return double longitude value for west
     */
    @field:Keep
    @JvmField
    val longitudeWest: Double
) : Parcelable {

    /**
     * Calculates the centerpoint of this LatLngBounds by simple interpolation and returns
     * it as a point. This is a non-geodesic calculation which is not the geographic center.
     *
     * @return LatLng center of this LatLngBounds
     */
    val center: LatLng
        get() {
            val latCenter = (latitudeNorth + latitudeSouth) / 2.0
            val longCenter = (longitudeEast + longitudeWest) / 2.0
            return LatLng(latCenter, longCenter)
        }

    /**
     * Get the north latitude value of this bounds.
     *
     * @return double latitude value for north
     */
    fun getLatNorth(): Double {
        return latitudeNorth
    }

    /**
     * Get the south latitude value of this bounds.
     *
     * @return double latitude value for south
     */
    fun getLatSouth(): Double {
        return latitudeSouth
    }

    /**
     * Get the east longitude value of this bounds.
     *
     * @return double longitude value for east
     */
    fun getLonEast(): Double {
        return longitudeEast
    }

    /**
     * Get the west longitude value of this bounds.
     *
     * @return double longitude value for west
     */
    fun getLonWest(): Double {
        return longitudeWest
    }

    /**
     * Get the latitude-longitude pair of the south west corner of this bounds.
     *
     * @return LatLng of the south west corner
     */
    val southWest: LatLng
        get() = LatLng(latitudeSouth, longitudeWest)

    /**
     * Get the latitude-longitude pair if the north east corner of this bounds.
     *
     * @return LatLng of the north east corner
     */
    val northEast: LatLng
        get() = LatLng(latitudeNorth, longitudeEast)

    /**
     * Get the latitude-longitude pair of the south east corner of this bounds.
     *
     * @return LatLng of the south east corner
     */
    val southEast: LatLng
        get() = LatLng(latitudeSouth, longitudeEast)

    /**
     * Get the latitude-longitude pair of the north west corner of this bounds.
     *
     * @return LatLng of the north west corner
     */
    val northWest: LatLng
        get() = LatLng(latitudeNorth, longitudeWest)

    /**
     * Get the area spanned by this LatLngBounds
     *
     * @return LatLngSpan area
     */
    val span: LatLngSpan
        get() = LatLngSpan(latitudeSpan, longitudeSpan)

    /**
     * Get the absolute distance, in degrees, between the north and
     * south boundaries of this LatLngBounds
     *
     * @return Span distance
     */
    val latitudeSpan: Double
        get() = Math.abs(latitudeNorth - latitudeSouth)

    /**
     * Get the absolute distance, in degrees, between the west and
     * east boundaries of this LatLngBounds
     *
     * @return Span distance
     */
    val longitudeSpan: Double
        get() = Math.abs(longitudeEast - longitudeWest)

    /**
     * Validate if LatLngBounds is empty, determined if absolute distance is
     *
     * @return boolean indicating if span is empty
     */
    val isEmptySpan: Boolean
        get() = longitudeSpan == 0.0 || latitudeSpan == 0.0

    /**
     * Returns a string representaton of the object.
     *
     * @return the string representation
     */
    override fun toString(): String {
        return ("N:" + latitudeNorth + "; E:" + longitudeEast + "; S:" + latitudeSouth + "; W:" + longitudeWest)
    }

    /**
     * Return an array of LatLng objects resembling this bounds.
     *
     * @return an array of 2 LatLng objects.
     */
    fun toLatLngs(): Array<LatLng> {
        return arrayOf(northEast, southWest)
    }

    /**
     * Constructs a LatLngBounds from current bounds with an additional latitude-longitude pair.
     *
     * @param latLng the latitude lognitude pair to include in the bounds.
     * @return the newly constructed bounds
     */
    fun include(latLng: LatLng): LatLngBounds {
        return Builder().include(northEast).include(southWest).include(latLng).build()
    }

    /**
     * Determines whether this LatLngBounds matches another one via LatLng.
     *
     * @param other another object
     * @return a boolean indicating whether the LatLngBounds are equal
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other is LatLngBounds) {
            return latitudeNorth == other.latitudeNorth && latitudeSouth == other.latitudeSouth && longitudeEast == other.longitudeEast && longitudeWest == other.longitudeWest
        }
        return false
    }

    private fun containsLatitude(latitude: Double): Boolean {
        return latitude <= latitudeNorth && latitude >= latitudeSouth
    }

    private fun containsLongitude(longitude: Double): Boolean {
        return longitude <= longitudeEast && longitude >= longitudeWest
    }

    /**
     * Determines whether this LatLngBounds contains a point.
     *
     * @param latLng the point which may be contained
     * @return true, if the point is contained within the bounds
     */
    operator fun contains(latLng: LatLng): Boolean {
        return (containsLatitude(latLng.latitude) && containsLongitude(latLng.longitude))
    }

    /**
     * Determines whether this LatLngBounds contains another bounds.
     *
     * @param other the bounds which may be contained
     * @return true, if the bounds is contained within the bounds
     */
    operator fun contains(other: LatLngBounds): Boolean {
        return (contains(other.northEast) && contains(other.southWest))
    }

    /**
     * Returns a new LatLngBounds that stretches to contain both this and another LatLngBounds.
     *
     * @param bounds LatLngBounds to add
     * @return LatLngBounds
     */
    fun union(bounds: LatLngBounds): LatLngBounds {
        return unionNoParamCheck(bounds.latitudeNorth, bounds.longitudeEast, bounds.latitudeSouth, bounds.longitudeWest)
    }

    /**
     * Returns a new LatLngBounds that stretches to contain both this and another LatLngBounds.
     *
     *
     *
     * This values of northLat and southLat should be in the range of [-90, 90],
     * see [GeometryConstants.MIN_LATITUDE] and [GeometryConstants.MAX_LATITUDE],
     * otherwise IllegalArgumentException will be thrown.
     * northLat should be greater or equal southLat, otherwise  IllegalArgumentException will be thrown.
     *
     *
     *
     * eastLon should be greater or equal westLon, otherwise  IllegalArgumentException will be thrown.
     *
     * @param northLat Northern Latitude corner point
     * @param eastLon  Eastern Longitude corner point
     * @param southLat Southern Latitude corner point
     * @param westLon  Western Longitude corner point
     * @return LatLngBounds
     */
    fun union(northLat: Double, eastLon: Double, southLat: Double, westLon: Double): LatLngBounds {
        checkParams(northLat, eastLon, southLat, westLon)
        return unionNoParamCheck(northLat, eastLon, southLat, westLon)
    }

    private fun unionNoParamCheck(northLat: Double, eastLon: Double, southLat: Double, westLon: Double): LatLngBounds {
        return LatLngBounds(
            if (latitudeNorth < northLat) northLat else latitudeNorth,
            if (longitudeEast < eastLon) eastLon else longitudeEast,
            if (latitudeSouth > southLat) southLat else latitudeSouth,
            if (longitudeWest > westLon) westLon else longitudeWest
        )
    }

    /**
     * Returns a new LatLngBounds that is the intersection of this with another LatLngBounds,
     *
     * @param box LatLngBounds to intersect with
     * @return LatLngBounds
     */
    fun intersect(box: LatLngBounds): LatLngBounds? {
        return intersectNoParamCheck(box.latitudeNorth, box.longitudeEast, box.latitudeSouth, box.longitudeWest)
    }

    /**
     * Returns a new LatLngBounds that is the intersection of this with another box.
     *
     *
     *
     * This values of northLat and southLat should be in the range of [-90, 90],
     * see [GeometryConstants.MIN_LATITUDE] and [GeometryConstants.MAX_LATITUDE],
     * otherwise IllegalArgumentException will be thrown.
     * northLat should be greater or equal southLat, otherwise  IllegalArgumentException will be thrown.
     *
     *
     *
     * eastLon should be greater or equal westLon, otherwise  IllegalArgumentException will be thrown.
     *
     * @param northLat Northern Latitude corner point
     * @param eastLon  Eastern Longitude corner point
     * @param southLat Southern Latitude corner point
     * @param westLon  Western Longitude corner point
     * @return LatLngBounds
     */
    fun intersect(northLat: Double, eastLon: Double, southLat: Double, westLon: Double): LatLngBounds {
        checkParams(northLat, eastLon, southLat, westLon)
        return intersectNoParamCheck(northLat, eastLon, southLat, westLon)!!
    }

    private fun intersectNoParamCheck(northLat: Double, eastLon: Double, southLat: Double, westLon: Double): LatLngBounds? {
        val minLonWest = Math.max(longitudeWest, westLon)
        val maxLonEast = Math.min(longitudeEast, eastLon)
        if (maxLonEast >= minLonWest) {
            val minLatSouth = Math.max(latitudeSouth, southLat)
            val maxLatNorth = Math.min(latitudeNorth, northLat)
            if (maxLatNorth >= minLatSouth) {
                return LatLngBounds(maxLatNorth, maxLonEast, minLatSouth, minLonWest)
            }
        }
        return null
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return the hash code
     */
    override fun hashCode(): Int {
        return (latitudeNorth + 90 + (latitudeSouth + 90) * 1000 + (longitudeEast + 180) * 1000000 + (longitudeWest + 180) * 1000000000).toInt()
    }

    /**
     * Describe the kinds of special objects contained in this Parcelable instance's marshaled representation.
     *
     * @return a bitmask indicating the set of special object types marshaled by this Parcelable object instance.
     */
    override fun describeContents(): Int {
        return 0
    }

    /**
     * Flatten this object in to a Parcel.
     *
     * @param out   The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written
     */
    override fun writeToParcel(out: Parcel, flags: Int) {
        out.writeDouble(latitudeNorth)
        out.writeDouble(longitudeEast)
        out.writeDouble(latitudeSouth)
        out.writeDouble(longitudeWest)
    }

    /**
     * Builder for composing LatLngBounds objects.
     */
    class Builder {
        private val latLngList: MutableList<LatLng> = ArrayList()

        /**
         * Builds a new LatLngBounds.
         *
         *
         * Throws an [InvalidLatLngBoundsException] when no LatLngBounds can be created.
         *
         *
         * @return the build LatLngBounds
         */
        fun build(): LatLngBounds {
            if (latLngList.size < 2) {
                throw InvalidLatLngBoundsException(latLngList.size)
            }
            return fromLatLngs(latLngList)
        }

        /**
         * Adds a LatLng object to the LatLngBounds.Builder.
         *
         * @param latLngs the List of LatLng objects to be added
         * @return this
         */
        fun includes(latLngs: List<LatLng>): Builder {
            latLngList.addAll(latLngs)
            return this
        }

        /**
         * Adds a LatLng object to the LatLngBounds.Builder.
         *
         * @param latLng the LatLng to be added
         * @return this
         */
        fun include(latLng: LatLng): Builder {
            latLngList.add(latLng)
            return this
        }
    }

    companion object {
        /**
         * Returns the world bounds.
         *
         * @return the bounds representing the world
         */
        @JvmStatic
        fun world(): LatLngBounds {
            return from(GeometryConstants.MAX_LATITUDE, GeometryConstants.MAX_WRAP_LONGITUDE, GeometryConstants.MIN_LATITUDE, GeometryConstants.MIN_WRAP_LONGITUDE)
        }

        /**
         * Constructs a LatLngBounds that contains all of a list of LatLng
         * objects. Empty lists will yield invalid LatLngBounds.
         *
         * @param latLngs List of LatLng objects
         * @return LatLngBounds
         */
        fun fromLatLngs(latLngs: List<LatLng>): LatLngBounds {
            var minLat = GeometryConstants.MAX_LATITUDE
            var minLon = GeometryConstants.MAX_LONGITUDE
            var maxLat = GeometryConstants.MIN_LATITUDE
            var maxLon = GeometryConstants.MIN_LONGITUDE
            for (gp in latLngs) {
                val latitude = gp.latitude
                val longitude = gp.longitude
                minLat = Math.min(minLat, latitude)
                minLon = Math.min(minLon, longitude)
                maxLat = Math.max(maxLat, latitude)
                maxLon = Math.max(maxLon, longitude)
            }
            return LatLngBounds(maxLat, maxLon, minLat, minLon)
        }

        /**
         * Constructs a LatLngBounds from doubles representing a LatLng pair.
         *
         *
         * This values of latNorth and latSouth should be in the range of [-90, 90],
         * see [GeometryConstants.MIN_LATITUDE] and [GeometryConstants.MAX_LATITUDE],
         * otherwise IllegalArgumentException will be thrown.
         * latNorth should be greater or equal latSouth, otherwise  IllegalArgumentException will be thrown.
         *
         *
         * This method doesn't recalculate most east or most west boundaries.
         * Note @since 7.0.0  lonEast and lonWest will NOT be wrapped to be in the range of [-180, 180],
         * see [GeometryConstants.MIN_LONGITUDE] and [GeometryConstants.MAX_LONGITUDE]
         * lonEast should be greater or equal lonWest, otherwise  IllegalArgumentException will be thrown.
         *
         */
        @JvmStatic
        fun from(
            @FloatRange(from = GeometryConstants.MIN_LATITUDE, to = GeometryConstants.MAX_LATITUDE) latNorth: Double,
            lonEast: Double,
            @FloatRange(from = GeometryConstants.MIN_LATITUDE, to = GeometryConstants.MAX_LATITUDE) latSouth: Double,
            lonWest: Double
        ): LatLngBounds {
            checkParams(latNorth, lonEast, latSouth, lonWest)
            return LatLngBounds(latNorth, lonEast, latSouth, lonWest)
        }

        private fun checkParams(
            @FloatRange(from = GeometryConstants.MIN_LATITUDE, to = GeometryConstants.MAX_LATITUDE) latNorth: Double,
            lonEast: Double,
            @FloatRange(from = GeometryConstants.MIN_LATITUDE, to = GeometryConstants.MAX_LATITUDE) latSouth: Double,
            lonWest: Double
        ) {
            require(!(Double.isNaN(latNorth) || Double.isNaN(latSouth))) { "latitude must not be NaN" }
            require(!(Double.isNaN(lonEast) || Double.isNaN(lonWest))) { "longitude must not be NaN" }
            require(!(Double.isInfinite(lonEast) || Double.isInfinite(lonWest))) { "longitude must not be infinite" }
            require(
                !(latNorth > GeometryConstants.MAX_LATITUDE || latNorth < GeometryConstants.MIN_LATITUDE || latSouth > GeometryConstants.MAX_LATITUDE || latSouth < GeometryConstants.MIN_LATITUDE)
            ) { "latitude must be between -90 and 90" }
            require(latNorth >= latSouth) { "latNorth cannot be less than latSouth" }
            require(lonEast >= lonWest) { "lonEast cannot be less than lonWest" }
        }

        private fun lat_(z: Int, y: Int): Double {
            val n = Math.PI - 2.0 * Math.PI * y / Math.pow(2.0, z.toDouble())
            return Math.toDegrees(Math.atan(0.5 * (Math.exp(n) - Math.exp(-n))))
        }

        private fun lon_(z: Int, x: Int): Double {
            return x / Math.pow(2.0, z.toDouble()) * 360.0 - GeometryConstants.MAX_WRAP_LONGITUDE
        }

        /**
         * Constructs a LatLngBounds from a Tile identifier.
         *
         *
         * Returned bounds will have latitude in the range of Mercator projection.
         *
         * @param z Tile zoom level.
         * @param x Tile X coordinate.
         * @param y Tile Y coordinate.
         * @see GeometryConstants.MIN_MERCATOR_LATITUDE
         *
         * @see GeometryConstants.MAX_MERCATOR_LATITUDE
         */
        @JvmStatic
        fun from(z: Int, x: Int, y: Int): LatLngBounds {
            return LatLngBounds(lat_(z, y), lon_(z, x + 1), lat_(z, y + 1), lon_(z, x))
        }

        /**
         * Inner class responsible for recreating Parcels into objects.
         */
        @JvmField
        val CREATOR: Parcelable.Creator<LatLngBounds?> = object : Parcelable.Creator<LatLngBounds?> {
            override fun createFromParcel(parcel: Parcel): LatLngBounds {
                return readFromParcel(parcel)
            }

            override fun newArray(size: Int): Array<LatLngBounds?> {
                return arrayOfNulls(size)
            }
        }

        private fun readFromParcel(parcel: Parcel): LatLngBounds {
            val northLat = parcel.readDouble()
            val eastLon = parcel.readDouble()
            val southLat = parcel.readDouble()
            val westLon = parcel.readDouble()
            return LatLngBounds(northLat, eastLon, southLat, westLon)
        }
    }
}
