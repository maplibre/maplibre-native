package org.maplibre.android.annotations

import androidx.annotation.Keep
import org.maplibre.android.geometry.LatLng

/**
 * Multipoint is an abstract annotation for combining geographical locations.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
abstract class BasePointCollection protected constructor() : Annotation() {
    /**
     * The points making up this point collection.
     *
     * Reading returns a copy of the points. Writing takes a copy of the points, so further
     * mutations to the provided list will have no effect on this annotation.
     */
    @field:Keep
    var points: List<LatLng> = ArrayList()
        get() = ArrayList(field)
        set(value) {
            field = ArrayList(value)
            update()
        }

    /**
     * Value between 0 and 1 defining the polyline alpha.
     */
    @field:Keep
    var alpha: Float = 1.0f
        set(value) {
            field = value
            update()
        }

    /**
     * Add a point to the polyline.
     *
     * @param point A [LatLng] point to be added.
     */
    fun addPoint(point: LatLng) {
        points = points + point
    }

    protected abstract fun update()
}
