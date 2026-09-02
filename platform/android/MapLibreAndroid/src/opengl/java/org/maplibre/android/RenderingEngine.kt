package org.maplibre.android

import android.content.Context
import androidx.annotation.Keep

/**
 * The rendering engine baked into this build of the SDK.
 *
 * In single-backend flavors (opengl, vulkan, ...) the engine is fixed at build
 * time and [setCurrentType] only accepts the matching value. In the
 * multiBackend flavor this class is mutable and `setCurrentType(...)` must
 * be called before [MapLibre.getInstance] to pick a backend for the process lifetime.
 */
@Keep
object RenderingEngine {
    /** Supported rendering backends. WebGPU flavors report the closest match. */
    enum class Type {
        OPENGL,
        VULKAN,
    }

    /**
     * The rendering backend used by the currently loaded library.
     */
    @JvmStatic
    @Volatile
    var currentType: Type = Type.OPENGL
        private set

    /**
     * This flavor's engine is fixed at compile time, so there's nothing to select —
     * always [Type.OPENGL].
     */
    @Suppress("UNUSED_PARAMETER")
    internal fun getDefaultRenderingEngine(context: Context): Type = Type.OPENGL

    /**
     * Single-backend flavor: this is a no-op when [type] matches the
     * compiled-in backend, and throws otherwise. Use the multiBackend flavor of
     * the SDK if you need to switch backends at runtime.
     *
     * @param type the desired backend
     * @throws IllegalStateException if [MapLibre.getInstance] has already been called in this process.
     * @throws UnsupportedOperationException if [type] differs from the compiled-in backend
     */
    internal fun setCurrentType(type: Type) {
        if (MapLibre.hasInstance()) {
            throw IllegalStateException(
                "RenderingEngine.setCurrentType() must be called before MapLibre.getInstance().",
            )
        }
        if (type != Type.OPENGL) {
            throw UnsupportedOperationException(
                "This MapLibre Android build supports only " + Type.OPENGL +
                    ". Use the multiBackend flavor to switch backends at runtime.",
            )
        }
        currentType = type
    }
}
