package org.maplibre.android.annotations

import android.os.Parcel
import android.os.Parcelable
import org.maplibre.android.geometry.LatLng

/**
 * Builder for composing [Polygon] objects.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class PolygonOptions : Parcelable {
    /**
     * Do not use this property. Used internally by the SDK.
     */
    val polygon: Polygon = Polygon()

    /**
     * Defines options for a polygon.
     */
    constructor()

    @Suppress("DEPRECATION", "UNCHECKED_CAST")
    private constructor(parcel: Parcel) {
        val pointsList = ArrayList<Any?>()
        parcel.readList(pointsList, LatLng::class.java.classLoader)
        addAll(pointsList.map { it as LatLng })
        val holesList = ArrayList<Any?>()
        parcel.readList(holesList, LatLng::class.java.classLoader)
        addAllHoles(holesList.map { it as List<LatLng> })
        alpha(parcel.readFloat())
        fillColor(parcel.readInt())
        strokeColor(parcel.readInt())
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
        out.writeList(getHoles())
        out.writeFloat(getAlpha())
        out.writeInt(getFillColor())
        out.writeInt(getStrokeColor())
    }

    /**
     * Adds a vertex to the outline of the polygon being built.
     *
     * @param point [LatLng] point to be added to polygon geometry.
     * @return This [PolygonOptions] object with the given point added to the outline.
     */
    fun add(point: LatLng): PolygonOptions {
        polygon.addPoint(point)
        return this
    }

    /**
     * Adds vertices to the outline of the polygon being built.
     *
     * @param points [LatLng] points to be added to polygon geometry.
     * @return This [PolygonOptions] object with the given points added to the outline.
     */
    fun add(vararg points: LatLng): PolygonOptions {
        for (point in points) {
            add(point)
        }
        return this
    }

    /**
     * Adds vertices to the outline of the polygon being built.
     *
     * @param points [Iterable] list made up of [LatLng] points defining the polygon geometry
     * @return This [PolygonOptions] object with the given points added to the outline.
     */
    fun addAll(points: Iterable<LatLng>): PolygonOptions {
        for (point in points) {
            add(point)
        }
        return this
    }

    /**
     * Adds a hole to the outline of the polygon being built.
     *
     * @param hole [List] list made up of [LatLng] points defining the hole
     * @return This [PolygonOptions] object with the given hole added to the outline.
     */
    fun addHole(hole: List<LatLng>): PolygonOptions {
        polygon.addHole(hole)
        return this
    }

    /**
     * Adds holes to the outline of the polygon being built.
     *
     * @param holes [List] list made up of [LatLng] holes to be added to polygon geometry
     * @return This [PolygonOptions] object with the given holes added to the outline.
     */
    fun addHole(vararg holes: List<LatLng>): PolygonOptions {
        for (hole in holes) {
            addHole(hole)
        }
        return this
    }

    /**
     * Adds holes to the outline of the polygon being built.
     *
     * @param holes [Iterable] list made up of [List] list of [LatLng] holes defining the hole geometry
     * @return This [PolygonOptions] object with the given holes added to the outline.
     */
    fun addAllHoles(holes: Iterable<List<LatLng>>): PolygonOptions {
        for (hole in holes) {
            addHole(hole)
        }
        return this
    }

    /**
     * Set the alpha value of the polyline.
     *
     * @param alpha float value between 0 (not visible) and 1.
     * @return This [PolygonOptions] object with the given polygon alpha value.
     */
    fun alpha(alpha: Float): PolygonOptions {
        polygon.alpha = alpha
        return this
    }

    /**
     * Gets the alpha set for this [PolygonOptions] object.
     *
     * @return float value between 0 and 1 defining the alpha.
     */
    fun getAlpha(): Float = polygon.alpha

    /**
     * Specifies the polygon's fill color, as 32-bit ARGB. The default color is black.
     *
     * @param color 32-bit ARGB color.
     * @return This [PolygonOptions] object with a new color set.
     */
    fun fillColor(color: Int): PolygonOptions {
        polygon.fillColor = color
        return this
    }

    /**
     * Gets the fill color set for this [PolygonOptions] object.
     *
     * @return The fill color of the polygon in ARGB format.
     */
    fun getFillColor(): Int = polygon.fillColor

    /**
     * Specifies the polygon's stroke color, as 32-bit ARGB. The default color is black.
     *
     * @param color 32-bit ARGB color.
     * @return This [PolygonOptions] object with a new stroke color set.
     */
    fun strokeColor(color: Int): PolygonOptions {
        polygon.strokeColor = color
        return this
    }

    /**
     * Gets the stroke color set for this [PolygonOptions] object.
     *
     * @return The stroke color of the polygon in ARGB format.
     */
    fun getStrokeColor(): Int = polygon.strokeColor

    /**
     * Gets the points set for this [PolygonOptions] object.
     *
     * @return The list made up of [LatLng] points defining the polygon.
     */
    fun getPoints(): List<LatLng> {
        // the getter gives us a copy, which is the safe thing to do...
        return polygon.points
    }

    /**
     * Gets the holes set for this [PolygonOptions] object.
     *
     * @return The list made up of [List] of [List] of [LatLng] points defining the holes.
     */
    fun getHoles(): List<List<LatLng>> = polygon.holes

    /**
     * Compares this [PolygonOptions] object with another [PolygonOptions] and
     * determines if their color, alpha, stroke color, and vertices match.
     *
     * @param other Another [PolygonOptions] to compare with this object.
     * @return True if color, alpha, stroke color, vertices and holes match this [PolygonOptions]
     * object. Else, false.
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as PolygonOptions

        if (other.getAlpha().compareTo(getAlpha()) != 0) {
            return false
        }

        if (getFillColor() != other.getFillColor()) {
            return false
        }

        if (getStrokeColor() != other.getStrokeColor()) {
            return false
        }

        if (getPoints() != other.getPoints()) {
            return false
        }

        return getHoles() == other.getHoles()
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
        result = 31 * result + getFillColor()
        result = 31 * result + getStrokeColor()
        result = 31 * result + getPoints().hashCode()
        result = 31 * result + getHoles().hashCode()
        return result
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<PolygonOptions> =
            object : Parcelable.Creator<PolygonOptions> {
                override fun createFromParcel(parcel: Parcel): PolygonOptions = PolygonOptions(parcel)

                override fun newArray(size: Int): Array<PolygonOptions?> = arrayOfNulls(size)
            }
    }
}
