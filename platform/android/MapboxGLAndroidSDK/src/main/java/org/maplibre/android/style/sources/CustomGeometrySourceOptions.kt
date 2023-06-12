package org.maplibre.android.style.sources

/**
 * Builder class for composing CustomGeometrySource objects.
 */
class CustomGeometrySourceOptions : HashMap<String?, Any?>() {
    /**
     * If the data includes wrapped coordinates, setting this to true unwraps the coordinates.
     *
     * @param wrap defaults to false
     * @return the current instance for chaining
     */
    fun withWrap(wrap: Boolean): CustomGeometrySourceOptions {
        this["wrap"] = wrap
        return this
    }

    /**
     * If the data includes geometry outside the tile boundaries, setting this to true clips the geometry
     * to the tile boundaries.
     *
     * @param clip defaults to false
     * @return the current instance for chaining
     */
    fun withClip(clip: Boolean): CustomGeometrySourceOptions {
        this["clip"] = clip
        return this
    }

    /**
     * Minimum zoom level at which to create vector tiles (lower means more field of view detail at low zoom levels).
     *
     * @param minZoom the minimum zoom - Defaults to 0.
     * @return the current instance for chaining
     */
    fun withMinZoom(minZoom: Int): CustomGeometrySourceOptions {
        this["minzoom"] = minZoom
        return this
    }

    /**
     * Maximum zoom level at which to create vector tiles (higher means greater detail at high zoom levels).
     *
     * @param maxZoom the maximum zoom - Defaults to 25.5
     * @return the current instance for chaining
     */
    fun withMaxZoom(maxZoom: Int): CustomGeometrySourceOptions {
        this["maxzoom"] = maxZoom
        return this
    }

    /**
     * Tile buffer size on each side (measured in 1/512ths of a tile; higher means fewer rendering artifacts near tile
     * edges but slower performance).
     *
     * @param buffer the buffer size - Defaults to 128.
     * @return the current instance for chaining
     */
    fun withBuffer(buffer: Int): CustomGeometrySourceOptions {
        this["buffer"] = buffer
        return this
    }

    /**
     * Douglas-Peucker simplification tolerance (higher means simpler geometries and faster performance).
     *
     * @param tolerance the tolerance - Defaults to 0.375
     * @return the current instance for chaining
     */
    fun withTolerance(tolerance: Float): CustomGeometrySourceOptions {
        this["tolerance"] = tolerance
        return this
    }
}
