package org.maplibre.android.geometry

import android.os.Parcel
import android.os.Parcelable

/**
 * A geographical span defined by its latitude and longitude span.
 */
class LatLngSpan : Parcelable {
    /**
     * Returns the latitude span.
     *
     * @return The latitude span.
     */
    /**
     * Sets the latitude span.
     *
     * @var latitudeSpan The latitude span to set.
     */
    var latitudeSpan: Double
    /**
     * Returns to longitude span.
     *
     * @return The longitude span.
     */
    /**
     * Sets the longitude span.
     *
     * @var longitudeSpan The longitude span to set.
     */
    var longitudeSpan: Double

    private constructor(parcel: Parcel) {
        latitudeSpan = parcel.readDouble()
        longitudeSpan = parcel.readDouble()
    }

    /**
     * Creates a LatLgnSpan.
     *
     * @param latitudeSpan  The span used for latitude.
     * @param longitudeSpan The span used for longitude.
     */
    constructor(latitudeSpan: Double, longitudeSpan: Double) {
        this.latitudeSpan = latitudeSpan
        this.longitudeSpan = longitudeSpan
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
        if (other is LatLngSpan) {
            return (longitudeSpan == other.longitudeSpan && latitudeSpan == other.latitudeSpan)
        }
        return false
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
     * @param out   Parcel in which the object should be written
     * @param flags Additional flags about how the object should be written
     */
    override fun writeToParcel(out: Parcel, flags: Int) {
        out.writeDouble(latitudeSpan)
        out.writeDouble(longitudeSpan)
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return hash code value of this
     */
    override fun hashCode(): Int {
        var result: Int
        var temp: Long
        temp = java.lang.Double.doubleToLongBits(latitudeSpan)
        result = (temp xor (temp ushr 32)).toInt()
        temp = java.lang.Double.doubleToLongBits(longitudeSpan)
        result = 31 * result + (temp xor (temp ushr 32)).toInt()
        return result
    }

    companion object {
        /**
         * Inner class responsible for recreating Parcels into objects.
         */
        @JvmField
        val CREATOR: Parcelable.Creator<LatLngSpan> = object : Parcelable.Creator<LatLngSpan> {
            override fun createFromParcel(parcel: Parcel): LatLngSpan {
                return LatLngSpan(parcel)
            }

            override fun newArray(size: Int): Array<LatLngSpan?> {
                return arrayOfNulls(size)
            }
        }
    }
}
