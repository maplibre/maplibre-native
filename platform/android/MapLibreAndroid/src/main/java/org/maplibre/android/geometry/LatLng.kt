package org.maplibre.android.geometry

import android.location.Location
import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.FloatRange
import androidx.annotation.Keep
import org.maplibre.geojson.Point
import org.maplibre.android.constants.GeometryConstants
import org.maplibre.turf.TurfConstants
import org.maplibre.turf.TurfMeasurement
import kotlin.math.abs

/**
 * A geographical location which contains a single latitude, longitude pair, with
 * optional altitude.
 *
 *
 * Latitude and longitude are expressed as decimal degrees
 * in the WGS84 datum. By default, altitude is 0.0, or sea level.
 *
 *
 *
 * MapLibre GL displays maps in the Mercator Projection and projects geographical
 * data automatically, so all data enters in the WGS84 datum.
 *
 */
class LatLng : Parcelable {

    /**
     * The latitude, in degrees.
     *
     * This value is in the range of [-90, 90], see [GeometryConstants.MIN_LATITUDE] and
     * [GeometryConstants.MAX_LATITUDE]
     *
     * @see GeometryConstants.MIN_LATITUDE
     *
     * @see GeometryConstants.MAX_LATITUDE
     */
    @Keep
    var latitude = 0.0
        set(
            @FloatRange(from = GeometryConstants.MIN_LATITUDE, to = GeometryConstants.MAX_LATITUDE) latitude
        ) {
            require(!latitude.isNaN()) { "latitude must not be NaN" }
            require(abs(latitude) <= GeometryConstants.MAX_LATITUDE) { "latitude must be between -90 and 90" }
            field = latitude
        }

    /**
     * The longitude, in degrees.
     *
     * This value is in the range of [-180, 180], see [GeometryConstants.MIN_LONGITUDE] and
     * [GeometryConstants.MAX_LONGITUDE]
     *
     * @see GeometryConstants.MIN_LONGITUDE
     * @see GeometryConstants.MAX_LONGITUDE
     */
    @Keep
    var longitude = 0.0
        /**
         * Set the longitude, in degrees.
         *
         *
         * This value is in the range of [-180, 180], see [GeometryConstants.MIN_LONGITUDE] and
         * [GeometryConstants.MAX_LONGITUDE]
         *
         *
         * @param longitude the longitude value in degrees
         * @see GeometryConstants.MIN_LONGITUDE
         *
         * @see GeometryConstants.MAX_LONGITUDE
         */
        set(@FloatRange(from = GeometryConstants.MIN_LONGITUDE, to = GeometryConstants.MAX_LONGITUDE) longitude) {
            require(!longitude.isNaN()) { "longitude must not be NaN" }
            require(!longitude.isInfinite()) { "longitude must not be infinite" }
            field = longitude
        }

    /**
     * The altitude, in meters.
     */
    var altitude = 0.0

    /**
     * Construct a new latitude, longitude point at (0, 0)
     */
    constructor() {
        latitude = 0.0
        longitude = 0.0
    }

    /**
     * Construct a new latitude, longitude point given double arguments
     *
     * @param latitude  Latitude in degrees
     * @param longitude Longitude in degrees
     */
    @Keep
    constructor(latitude: Double, longitude: Double) {
        this.latitude = latitude
        this.longitude = longitude
    }

    /**
     * Construct a new latitude, longitude, altitude point given double arguments
     *
     * @param latitude  Latitude in degrees
     * @param longitude Longitude in degress
     * @param altitude  Altitude in meters
     */
    constructor(latitude: Double, longitude: Double, altitude: Double) {
        this.latitude = latitude
        this.longitude = longitude
        this.altitude = altitude
    }

    /**
     * Construct a new latitude, longitude, altitude point given location argument
     *
     * @param location Android Location
     */
    constructor(location: Location) : this(location.latitude, location.longitude, location.altitude) {}

    /**
     * Construct a new latitude, longitude, altitude point given another latitude, longitude, altitude point.
     *
     * @param latLng LatLng to be cloned.
     */
    constructor(latLng: LatLng) {
        latitude = latLng.latitude
        longitude = latLng.longitude
        altitude = latLng.altitude
    }

    /**
     * Constructs a new latitude, longitude, altitude tuple given a parcel.
     *
     * @param in the parcel containing the latitude, longitude, altitude values
     */
    constructor(`in`: Parcel) {
        latitude = (`in`.readDouble())
        longitude = (`in`.readDouble())
        altitude = `in`.readDouble()
    }

    /**
     * Return a new LatLng object with a wrapped Longitude.  This allows original data object
     * to remain unchanged.
     *
     * @return new LatLng object with wrapped Longitude
     */
    fun wrap(): LatLng {
        return LatLng(
            latitude,
            wrap(longitude, GeometryConstants.MIN_WRAP_LONGITUDE, GeometryConstants.MAX_WRAP_LONGITUDE)
        )
    }

    /**
     * Indicates whether some other object is "equal to" this one.
     *
     * @param other The object to compare
     * @return True if equal, false if not
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }
        val latLng = other as LatLng
        return java.lang.Double.compare(latLng.altitude, altitude) == 0 && java.lang.Double.compare(latLng.latitude, latitude) == 0 && java.lang.Double.compare(latLng.longitude, longitude) == 0
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return the hash code value
     */
    override fun hashCode(): Int {
        var result: Int
        var temp: Long
        temp = java.lang.Double.doubleToLongBits(latitude)
        result = (temp xor (temp ushr 32)).toInt()
        temp = java.lang.Double.doubleToLongBits(longitude)
        result = 31 * result + (temp xor (temp ushr 32)).toInt()
        temp = java.lang.Double.doubleToLongBits(altitude)
        result = 31 * result + (temp xor (temp ushr 32)).toInt()
        return result
    }

    /**
     * Returns a string representation of the object.
     *
     * @return the string representation
     */
    override fun toString(): String {
        return "LatLng [latitude=$latitude, longitude=$longitude, altitude=$altitude]"
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
        out.writeDouble(latitude)
        out.writeDouble(longitude)
        out.writeDouble(altitude)
    }

    /**
     * Calculate distance between two points
     *
     * @param other Other LatLng to compare to
     * @return distance in meters
     */
    fun distanceTo(other: LatLng): Double {
        return TurfMeasurement.distance(Point.fromLngLat(longitude, latitude), Point.fromLngLat(other.longitude, other.latitude), TurfConstants.UNIT_METRES)
    }

    companion object {
        /**
         * Inner class responsible for recreating Parcels into objects.
         */
        @JvmField
        val CREATOR: Parcelable.Creator<LatLng> = object : Parcelable.Creator<LatLng> {
            override fun createFromParcel(parcel: Parcel): LatLng {
                return LatLng(parcel)
            }

            override fun newArray(size: Int): Array<LatLng?> {
                return arrayOfNulls(size)
            }
        }

        /**
         * Constrains value to the given range (including min & max) via modular arithmetic.
         *
         *
         * Same formula as used in Core GL (wrap.hpp)
         * std::fmod((std::fmod((value - min), d) + d), d) + min;
         *
         *
         *
         * Multiples of max value will be wrapped to max.
         *
         *
         * @param value Value to wrap
         * @param min   Minimum value
         * @param max   Maximum value
         * @return Wrapped value
         */
        fun wrap(value: Double, min: Double, max: Double): Double {
            val delta = max - min
            val firstMod = (value - min) % delta
            val secondMod = (firstMod + delta) % delta
            return if (value >= max && secondMod == 0.0) {
                max
            } else {
                secondMod + min
            }
        }
    }
}
