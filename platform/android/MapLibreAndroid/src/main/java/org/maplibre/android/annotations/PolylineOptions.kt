package org.maplibre.android.annotations

import android.os.Parcel
import android.os.Parcelable
import org.maplibre.android.geometry.LatLng

/**
 * Builder for composing [Polyline] objects.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class PolylineOptions : Parcelable {
    /**
     * Do not use this property. Used internally by the SDK.
     */
    val polyline: Polyline = Polyline()

    /**
     * Defines options for a polyline.
     */
    constructor()

    @Suppress("DEPRECATION")
    private constructor(parcel: Parcel) {
        val pointsList = ArrayList<Any?>()
        parcel.readList(pointsList, LatLng::class.java.classLoader)
        addAll(pointsList.map { it as LatLng })
        alpha(parcel.readFloat())
        color(parcel.readInt())
        width(parcel.readFloat())
    }

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
        out.writeList(getPoints())
        out.writeFloat(getAlpha())
        out.writeInt(getColor())
        out.writeFloat(getWidth())
    }

    /**
     * Adds a vertex to the end of the polyline being built.
     *
     * @param point [LatLng] point to be added to polyline geometry.
     * @return This [PolylineOptions] object with the given point on the end.
     */
    fun add(point: LatLng): PolylineOptions {
        polyline.addPoint(point)
        return this
    }

    /**
     * Adds vertices to the end of the polyline being built.
     *
     * @param points [LatLng] points defining the polyline geometry.
     * @return This [PolylineOptions] object with the given point on the end.
     */
    fun add(vararg points: LatLng): PolylineOptions {
        for (point in points) {
            add(point)
        }
        return this
    }

    /**
     * Adds vertices to the end of the polyline being built.
     *
     * @param points [Iterable] list made up of [LatLng] points defining the polyline geometry
     * @return This [PolylineOptions] object with the given points on the end.
     */
    fun addAll(points: Iterable<LatLng>): PolylineOptions {
        for (point in points) {
            add(point)
        }
        return this
    }

    /**
     * Set the alpha value of the polyline.
     *
     * @param alpha float value between 0 (not visible) and 1.
     * @return This [PolylineOptions] object with the given polyline alpha value.
     */
    fun alpha(alpha: Float): PolylineOptions {
        polyline.alpha = alpha
        return this
    }

    /**
     * Gets the alpha set for this [PolylineOptions] object.
     *
     * @return float value between 0 and 1 defining the alpha.
     */
    fun getAlpha(): Float = polyline.alpha

    /**
     * Sets the color of the polyline as a 32-bit ARGB color. The default color is black.
     *
     * @param color 32-bit ARGB color.
     * @return This [PolylineOptions] object with a new color set.
     */
    fun color(color: Int): PolylineOptions {
        polyline.color = color
        return this
    }

    /**
     * Gets the color set for this [PolylineOptions] object.
     *
     * @return The color of the polyline in ARGB format.
     */
    fun getColor(): Int = polyline.color

    /**
     * Gets the width set for this [PolylineOptions] object.
     *
     * @return The width of the polyline in screen pixels.
     */
    fun getWidth(): Float = polyline.width

    /**
     * Sets the width of the polyline in screen pixels. The default is 10.
     *
     * @param width float value defining width of polyline using unit pixels.
     * @return This [PolylineOptions] object with a new width set.
     */
    fun width(width: Float): PolylineOptions {
        polyline.width = width
        return this
    }

    /**
     * Gets the points set for this [PolylineOptions] object.
     *
     * @return a [List] of [LatLng]s specifying the vertices of the polyline.
     */
    fun getPoints(): List<LatLng> {
        // the getter gives us a copy, which is the safe thing to do...
        return polyline.points
    }

    /**
     * Compares this [PolylineOptions] object with another [PolylineOptions] and
     * determines if their color, alpha, width, and vertices match.
     *
     * @param other Another [PolylineOptions] to compare with this object.
     * @return True if color, alpha, width, and vertices match this [PolylineOptions] object.
     * Else, false.
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as PolylineOptions

        if (other.getAlpha().compareTo(getAlpha()) != 0) {
            return false
        }
        if (getColor() != other.getColor()) {
            return false
        }
        if (other.getWidth().compareTo(getWidth()) != 0) {
            return false
        }
        return getPoints() == other.getPoints()
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
        result = 31 * result + (if (getAlpha() != +0.0f) getAlpha().toBits() else 0)
        result = 31 * result + getColor()
        result = 31 * result + (if (getWidth() != +0.0f) getWidth().toBits() else 0)
        result = 31 * result + getPoints().hashCode()
        return result
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<PolylineOptions> =
            object : Parcelable.Creator<PolylineOptions> {
                override fun createFromParcel(parcel: Parcel): PolylineOptions = PolylineOptions(parcel)

                override fun newArray(size: Int): Array<PolylineOptions?> = arrayOfNulls(size)
            }
    }
}
