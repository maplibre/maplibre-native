package org.maplibre.android.annotations

import android.graphics.Color
import androidx.annotation.Keep
import org.maplibre.android.geometry.LatLng

/**
 * Polygon is a geometry annotation that's a closed loop of coordinates.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class Polygon internal constructor() : BasePointCollection() {
    /**
     * The color of the fill region of the polygon, in ARGB format.
     */
    @field:Keep
    var fillColor: Int = Color.BLACK // default fillColor is black
        set(value) {
            field = value
            update()
        }

    /**
     * The color of the stroke of the polygon, in ARGB format.
     */
    @field:Keep
    var strokeColor: Int = Color.BLACK // default strokeColor is black
        set(value) {
            field = value
            update()
        }

    /**
     * The holes of this polygon.
     *
     * Reading returns a copy of the holes. Writing takes a copy of the holes, so further
     * mutations to the provided list will have no effect on this polygon.
     */
    @field:Keep
    var holes: List<List<LatLng>> = ArrayList()
        get() = ArrayList(field)
        set(value) {
            field = ArrayList(value)
            update()
        }

    /**
     * Add a hole to the polygon.
     *
     * @param hole A [List] of [LatLng] points making up the hole to be added.
     */
    internal fun addHole(hole: List<LatLng>) {
        holes = holes + listOf(hole)
    }

    override fun update() {
        getMapLibreMap()?.updatePolygon(this)
    }
}
