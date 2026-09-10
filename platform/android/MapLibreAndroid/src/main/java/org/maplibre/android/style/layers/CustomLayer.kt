package org.maplibre.android.style.layers

import androidx.annotation.Keep

/**
 * Custom layer.
 *
 * Experimental feature. Do not use.
 */
class CustomLayer : Layer {
    constructor(id: String?, host: Long) {
        initialize(id, host)
    }

    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Triggers map re-paint.
     */
    @Deprecated("Use MapLibreMap.triggerRepaint() instead.")
    @Keep
    fun update() {
    }

    @Keep
    protected external fun initialize(
        id: String?,
        host: Long,
    )

    @Keep
    @Throws(Throwable::class)
    protected external override fun finalize()
}
