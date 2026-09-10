package org.maplibre.android.maps

import androidx.annotation.Keep

/**
 * Holds the raw data of an image that is added to the map's style.
 *
 * The fields of this class are read from the native peer, they must not be renamed.
 */
@Keep
@Suppress("unused", "LongParameterList")
class Image(
    private val buffer: ByteArray,
    private val pixelRatio: Float,
    private val name: String,
    private val width: Int,
    private val height: Int,
    private val sdf: Boolean,
    private val stretchX: FloatArray? = null,
    private val stretchY: FloatArray? = null,
    private val content: FloatArray? = null,
)
