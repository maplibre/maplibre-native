package org.maplibre.android.annotations

import android.graphics.Color
import androidx.annotation.Keep

/**
 * Polyline is a geometry feature with an unclosed list of coordinates drawn as a line
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class Polyline internal constructor() : BasePointCollection() {
    /**
     * The color of this polyline, in ARGB format.
     */
    @field:Keep
    var color: Int = Color.BLACK // default color is black
        set(value) {
            field = value
            update()
        }

    /**
     * The width of this polyline, in screen pixels.
     */
    @field:Keep
    var width: Float = 10f // As specified by Google API Docs (in pixels)
        set(value) {
            field = value
            update()
        }

    override fun update() {
        getMapLibreMap()?.updatePolyline(this)
    }
}
