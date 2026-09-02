package org.maplibre.android.storage

import androidx.annotation.IntDef

/**
 * Resource provides access to resource types.
 */
object Resource {
    // Note: Keep this in sync with include/mln/storage/resource.hpp

    /**
     * Resource type variants.
     */
    @IntDef(UNKNOWN, STYLE, SOURCE, TILE, GLYPHS, SPRITE_IMAGE, SPRITE_JSON)
    @Retention(AnnotationRetention.SOURCE)
    annotation class Kind

    /**
     * Unknown type
     */
    const val UNKNOWN = 0

    /**
     * Style sheet JSON file
     */
    const val STYLE = 1

    /**
     * TileJSON file as specified in https://maplibre.org/maplibre-style-spec/root/#sources
     */
    const val SOURCE = 2

    /**
     * A vector or raster tile as described in the style sheet at
     * https://maplibre.org/maplibre-style-spec/sources/
     */
    const val TILE = 3

    /**
     * Signed distance field glyphs for text rendering. These are the URLs specified in the style
     * in https://maplibre.org/maplibre-style-spec/root/#glyphs
     */
    const val GLYPHS = 4

    /**
     * Image part of a sprite sheet. It is constructed of the prefix in
     * https://maplibre.org/maplibre-style-spec/root/#sprite and a PNG file extension.
     */
    const val SPRITE_IMAGE = 5

    /**
     * JSON part of a sprite sheet. It is constructed of the prefix in
     * https://maplibre.org/maplibre-style-spec/root/#sprite and a JSON file extension.
     */
    const val SPRITE_JSON = 6
}
