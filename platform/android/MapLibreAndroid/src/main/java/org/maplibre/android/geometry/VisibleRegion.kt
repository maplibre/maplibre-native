package org.maplibre.android.geometry

import android.os.Build
import android.os.Parcel
import android.os.Parcelable

/**
 * Contains the four points defining the four-sided polygon that is visible in a map's camera.
 * This polygon can be a trapezoid instead of a rectangle, because a camera can have tilt.
 * If the camera is directly over the center of the camera, the shape is rectangular,
 * but if the camera is tilted, the shape will appear to be a trapezoid whose
 * smallest side is closest to the point of view.
 */
class VisibleRegion : Parcelable {
    /**
     * LatLng object that defines the far left corner of the camera.
     */
    @JvmField
    val farLeft: LatLng?

    /**
     * LatLng object that defines the far right corner of the camera.
     */
    @JvmField
    val farRight: LatLng?

    /**
     * LatLng object that defines the bottom left corner of the camera.
     */
    @JvmField
    val nearLeft: LatLng?

    /**
     * LatLng object that defines the bottom right corner of the camera.
     */
    @JvmField
    val nearRight: LatLng?

    /**
     * The smallest bounding box that includes the visible region defined in this class.
     */
    @JvmField
    val latLngBounds: LatLngBounds

    /**
     * Creates a VisibleRegion from a Parcel.
     *
     * @param `in` The Parcel to create this from
     */
    @Suppress("DEPRECATION")
    private constructor(parcel: Parcel) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            farLeft = parcel.readParcelable<LatLng>(LatLng::class.java.classLoader, LatLng::class.java)
            farRight = parcel.readParcelable(LatLng::class.java.classLoader, LatLng::class.java)
            nearLeft = parcel.readParcelable(LatLng::class.java.classLoader, LatLng::class.java)
            nearRight = parcel.readParcelable(LatLng::class.java.classLoader, LatLng::class.java)
            latLngBounds = parcel.readParcelable(LatLngBounds::class.java.classLoader, LatLngBounds::class.java)!!
        } else {
            farLeft = parcel.readParcelable<LatLng>(LatLng::class.java.classLoader)
            farRight = parcel.readParcelable(LatLng::class.java.classLoader)
            nearLeft = parcel.readParcelable(LatLng::class.java.classLoader)
            nearRight = parcel.readParcelable(LatLng::class.java.classLoader)
            latLngBounds = parcel.readParcelable(LatLngBounds::class.java.classLoader)!!
        }
    }

    /**
     * Creates a VisibleRegion given the four corners of the camera.
     *
     * @param farLeft      A LatLng object containing the latitude and longitude of the far left corner of the region.
     * @param farRight     A LatLng object containing the latitude and longitude of the far right corner of the region.
     * @param nearLeft     A LatLng object containing the latitude and longitude of the near left corner of the region.
     * @param nearRight    A LatLng object containing the latitude and longitude of the near right corner of the region.
     * @param latLngBounds The smallest bounding box that includes the visible region defined in this class.
     */
    constructor(farLeft: LatLng, farRight: LatLng, nearLeft: LatLng, nearRight: LatLng, latLngBounds: LatLngBounds) {
        this.farLeft = farLeft
        this.farRight = farRight
        this.nearLeft = nearLeft
        this.nearRight = nearRight
        this.latLngBounds = latLngBounds
    }

    /**
     * Compares this VisibleRegion to another object.
     * If the other object is actually a pointer to this object,
     * or if all four corners and the bounds of the two objects are the same,
     * this method returns true. Otherwise, this method returns false.
     *
     * @param other The Object to compare with.
     * @return true if both objects are the same object.
     */
    override fun equals(other: Any?): Boolean {
        if (other !is VisibleRegion) {
            return false
        }
        if (other === this) {
            return true
        }
        return farLeft == other.farLeft && farRight == other.farRight && nearLeft == other.nearLeft && nearRight == other.nearRight && latLngBounds == other.latLngBounds
    }

    /**
     * The string representation of the object.
     *
     * @return the string representation of this
     */
    override fun toString(): String {
        return ("[farLeft [" + farLeft + "], farRight [" + farRight + "], nearLeft [" + nearLeft + "], nearRight [" + nearRight + "], latLngBounds [" + latLngBounds + "]]")
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return the hash code
     */
    override fun hashCode(): Int {
        return farLeft.hashCode() + 90 + (farRight.hashCode() + 90) * 1000 + (nearLeft.hashCode() + 180) * 1000000 + (nearRight.hashCode() + 180) * 1000000000
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
        out.writeParcelable(farLeft, flags)
        out.writeParcelable(farRight, flags)
        out.writeParcelable(nearLeft, flags)
        out.writeParcelable(nearRight, flags)
        out.writeParcelable(latLngBounds, flags)
    }

    companion object {
        /**
         * Inner class responsible for recreating Parcels into objects.
         */
        @JvmField
        val CREATOR: Parcelable.Creator<VisibleRegion> = object : Parcelable.Creator<VisibleRegion> {
            override fun createFromParcel(parcel: Parcel): VisibleRegion {
                return VisibleRegion(parcel)
            }

            override fun newArray(size: Int): Array<VisibleRegion?> {
                return arrayOfNulls(size)
            }
        }
    }
}
