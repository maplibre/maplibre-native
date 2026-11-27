package org.maplibre.android.geometry

import androidx.annotation.Keep

/**
 * A planar coordinate reference system, such as a polar stereographic or a national grid, and the tile grid laid
 * over it, so a map can draw tiles that were cut in that system without warping them into Web Mercator.
 *
 * Register the system with [org.maplibre.android.maps.Style.addProjection] before loading a style that names it,
 * then select it by name with the style's `projection.type`. A style that names a system nobody registered falls
 * back to Web Mercator. Every tile source of such a map is taken to be cut in [tileMatrixSet], and the map counts the
 * system's units as meters wherever it converts meters. The map never draws world copies in such a system and never
 * wraps across the antimeridian.
 *
 * This API is experimental and may change in a future release.
 *
 * @property name The name the style's `projection.type` uses, such as `EPSG:3413`. It needs a colon: names without
 * one are reserved for built-in projections.
 * @property tileMatrixSet The tile grid the map's tiles were cut in.
 * @property projection Converts coordinates to positions in the system and back.
 */
@Keep
class CoordinateReferenceSystem(val name: String, val tileMatrixSet: TileMatrixSet, val projection: Projection) {

    /**
     * The square, power-of-two quad tile grid over a coordinate reference system that the map's tiles were cut in.
     *
     * @property originX The smallest x of tile 0/0/0, its left edge.
     * @property originY The largest y of tile 0/0/0, its top edge. The y axis grows toward the top of the map.
     * @property extentAtZoom0 The side of tile 0/0/0, in the system's units.
     */
    @Keep
    data class TileMatrixSet(val originX: Double, val originY: Double, val extentAtZoom0: Double)

    /**
     * Converts coordinates to positions in a coordinate reference system and back, many at a time.
     *
     * Both functions read the first `count` pairs of their input and write as many pairs to their output, so
     * `[longitude0, latitude0, longitude1, latitude1, ...]` or `[x0, y0, x1, y1, ...]`. The arrays can be longer than
     * that. The map calls both functions from any thread, several at once, and they must return the same answer for
     * the same input. They must not call back into the map or wait on the main thread. An exception thrown by either
     * counts as no answer for every pair in the call.
     */
    @Keep
    interface Projection {
        /**
         * Writes the position of each longitude and latitude in degrees, or `NaN` where a coordinate has no position.
         */
        fun forward(lngLats: DoubleArray, positions: DoubleArray, count: Int)

        /**
         * Writes the longitude and latitude in degrees of each position, or `NaN` where a position has no coordinate.
         */
        fun inverse(positions: DoubleArray, lngLats: DoubleArray, count: Int)
    }
}
