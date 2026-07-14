package org.maplibre.android.style.terrain

/**
 * The style's 3D terrain configuration.
 *
 * Terrain drapes the map over the elevation data of a raster-dem source.
 * Per the style specification, `exaggeration` is a vertical multiplier where
 * 1.0 renders true-scale elevation.
 *
 * It is recommended to give terrain its own raster-dem source (even when it
 * uses the same tiles as a hillshade source) so styles stay portable with
 * maplibre-gl-js, which keeps a separate terrain source cache.
 *
 * @property sourceId the id of the raster-dem source providing the elevation data
 * @property exaggeration the vertical exaggeration multiplier, 1.0 by default
 */
class Terrain @JvmOverloads constructor(
    val sourceId: String,
    val exaggeration: Float = 1.0f
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is Terrain) return false
        return sourceId == other.sourceId && exaggeration == other.exaggeration
    }

    override fun hashCode(): Int {
        return 31 * sourceId.hashCode() + exaggeration.hashCode()
    }

    override fun toString(): String {
        return "Terrain{sourceId=$sourceId, exaggeration=$exaggeration}"
    }
}
