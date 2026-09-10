package org.maplibre.android.annotations

import android.graphics.Bitmap
import android.os.Parcel
import android.os.Parcelable
import org.maplibre.android.exceptions.InvalidMarkerPositionException
import org.maplibre.android.geometry.LatLng

/**
 * Builder for composing [Marker] objects. See [Marker] for additional information.
 *
 * ### Example
 *
 * ```kotlin
 * mapView.addMarker(
 *     MarkerOptions()
 *         .title("Intersection")
 *         .snippet("H St NW with 15th St NW")
 *         .position(LatLng(38.9002073, -77.03364419))
 * )
 * ```
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class MarkerOptions : BaseMarkerOptions<Marker, MarkerOptions> {
    /**
     * Defines options for a Marker.
     */
    constructor()

    private constructor(parcel: Parcel) {
        position(parcel.readParcelable<LatLng>(LatLng::class.java.classLoader))
        snippet(parcel.readString())
        title(parcel.readString())
        if (parcel.readByte() != 0.toByte()) {
            // this means we have an icon
            val iconId = parcel.readString()
            val iconBitmap = parcel.readParcelable<Bitmap>(Bitmap::class.java.classLoader)
            icon(Icon(iconId, iconBitmap))
        }
    }

    override fun getThis(): MarkerOptions = this

    /**
     * Describe the kinds of special objects contained in this Parcelable's
     * marshalled representation.
     *
     * @return integer 0.
     */
    override fun describeContents(): Int = 0

    /**
     * Flatten this object in to a Parcel.
     *
     * @param out   The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written. May be 0 or
     *              `PARCELABLE_WRITE_RETURN_VALUE`.
     */
    override fun writeToParcel(
        out: Parcel,
        flags: Int,
    ) {
        out.writeParcelable(getPosition(), flags)
        out.writeString(getSnippet())
        out.writeString(getTitle())
        val icon = getIcon()
        out.writeByte(if (icon != null) 1.toByte() else 0.toByte())
        if (icon != null) {
            out.writeString(icon.id)
            out.writeParcelable(icon.bitmap, flags)
        }
    }

    /**
     * Do not use this property. Used internally by the SDK.
     */
    override val marker: Marker
        get() {
            if (position == null) {
                throw InvalidMarkerPositionException()
            }
            return Marker(position, icon, title, snippet)
        }

    /**
     * Returns the position set for this [MarkerOptions] object.
     *
     * @return A [LatLng] object specifying the marker's current position.
     */
    fun getPosition(): LatLng? = position

    /**
     * Gets the snippet set for this [MarkerOptions] object.
     *
     * @return A string containing the marker's snippet.
     */
    fun getSnippet(): String? = snippet

    /**
     * Gets the title set for this [MarkerOptions] object.
     *
     * @return A string containing the marker's title.
     */
    fun getTitle(): String? = title

    /**
     * Gets the custom icon set for this [MarkerOptions] object.
     *
     * @return A [Icon] object that the marker is using. If the icon wasn't set, default icon
     * will return.
     */
    fun getIcon(): Icon? = icon

    /**
     * Compares this [MarkerOptions] object with another [MarkerOptions] and
     * determines if their properties match.
     *
     * @param other Another [MarkerOptions] to compare with this object.
     * @return True if marker properties match this [MarkerOptions] object. Else, false.
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as MarkerOptions
        return getPosition() == other.getPosition() &&
            getSnippet() == other.getSnippet() &&
            getIcon() == other.getIcon() &&
            getTitle() == other.getTitle()
    }

    /**
     * Gives an integer which can be used as the bucket number for storing elements of the set/map.
     * This bucket number is the address of the element inside the set/map. There's no guarantee
     * that this hash value will be consistent between different Java implementations, or even
     * between different execution runs of the same program.
     *
     * @return integer value you can use for storing element.
     */
    override fun hashCode(): Int {
        var result = 1
        result = 31 * result + (getPosition()?.hashCode() ?: 0)
        result = 31 * result + (getSnippet()?.hashCode() ?: 0)
        result = 31 * result + (getIcon()?.hashCode() ?: 0)
        result = 31 * result + (getTitle()?.hashCode() ?: 0)
        return result
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<MarkerOptions> =
            object : Parcelable.Creator<MarkerOptions> {
                override fun createFromParcel(parcel: Parcel): MarkerOptions = MarkerOptions(parcel)

                override fun newArray(size: Int): Array<MarkerOptions?> = arrayOfNulls(size)
            }
    }
}
