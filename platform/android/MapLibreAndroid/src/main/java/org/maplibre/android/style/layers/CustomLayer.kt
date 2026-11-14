package org.maplibre.android.style.layers

import androidx.annotation.Keep

/**
 * A style layer that is rendered by native drawing code that you provide.
 *
 * The `host` passed to the constructor is a pointer to a native
 * `mln::style::CustomLayerHost` implementation, created in your own JNI code.
 * The layer takes ownership of the host. Its callbacks run on the render thread and
 * issue draw calls directly with the graphics API of the active rendering backend
 * (OpenGL ES or Vulkan).
 *
 * **This is an advanced feature. Use it at your own risk.**
 * Bugs in your drawing code can break rendering or crash the app, and keeping it
 * working with future versions of MapLibre Native is up to you.
 */
open class CustomLayer : Layer {
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
    open fun update() {
    }

    @Keep
    protected open external fun initialize(
        id: String?,
        host: Long,
    )

    @Keep
    @Throws(Throwable::class)
    protected external override fun finalize()
}
