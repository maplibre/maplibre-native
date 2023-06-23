package org.maplibre.android.geometry

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.Keep

/**
 * ProjectedMeters is a projection of longitude, latitude points in Mercator meters.
 *
 *
 * these have been projected into MapLibre GL's Mercator projection. Instead of decimal
 * degrees, it uses Mercator meters (which are notably not equivalent to SI meters)
 * except at the equator.
 *
 */
class ProjectedMeters : Parcelable {
    /**
     * Get projected meters in north direction.
     *
     * @return Projected meters in north.
     */
    var northing: Double
        private set

    /**
     * Get projected meters in east direction.
     *
     * @return Projected meters in east.
     */
    var easting: Double
        private set

    /**
     * Creates a ProjectedMeters based on projected meters in north and east direction.
     *
     * @param northing the northing in meters
     * @param easting  the easting in meters
     */
    @Keep
    constructor(northing: Double, easting: Double) {
        this.northing = northing
        this.easting = easting
    }

    /**
     * Creates a ProjecteMeters based on another set of projected meters.
     *
     * @param projectedMeters The projected meters to be based on.
     */
    constructor(projectedMeters: ProjectedMeters) {
        northing = projectedMeters.northing
        easting = projectedMeters.easting
    }

    /**
     * Creates a ProjectedMeters from a Parcel.
     *
     * @param `in` The parcel to create from
     * @return a bitmask indicating the set of special object types marshaled by this Parcelable object instance.
     */
    private constructor(parcel: Parcel) {
        northing = parcel.readDouble()
        easting = parcel.readDouble()
    }

    /**
     * Indicates whether some other object is "equal to" this one.
     *
     * @param other The object to compare this to
     * @return true if equal, false if not
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }
        val projectedMeters = other as ProjectedMeters
        return (java.lang.Double.compare(projectedMeters.easting, easting) == 0 && java.lang.Double.compare(projectedMeters.northing, northing) == 0)
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return the hash code of this
     */
    override fun hashCode(): Int {
        var result: Int
        var temp: Long
        temp = java.lang.Double.doubleToLongBits(easting)
        result = (temp xor (temp ushr 32)).toInt()
        temp = java.lang.Double.doubleToLongBits(northing)
        result = 31 * result + (temp xor (temp ushr 32)).toInt()
        return result
    }

    /**
     * Returns a string representation of the object.
     *
     * @return the string representation of this
     */
    override fun toString(): String {
        return "ProjectedMeters [northing=$northing, easting=$easting]"
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
        out.writeDouble(northing)
        out.writeDouble(easting)
    }

    companion object {
        /**
         * Inner class responsible for recreating Parcels into objects.
         */
        @JvmField
        val CREATOR: Parcelable.Creator<ProjectedMeters> = object : Parcelable.Creator<ProjectedMeters> {
            override fun createFromParcel(parcel: Parcel): ProjectedMeters {
                return ProjectedMeters(parcel)
            }

            override fun newArray(size: Int): Array<ProjectedMeters?> {
                return arrayOfNulls(size)
            }
        }
    }
}
