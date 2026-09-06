package org.maplibre.android

import android.content.Context
import androidx.annotation.Keep

/**
 * Vulkan (and WebGPU) flavor: the rendering engine is fixed at build time.
 * See the opengl flavor's KDoc for the multiBackend contrast.
 */
@Keep
object RenderingEngine {
    enum class Type {
        OPENGL,
        VULKAN,
    }

    /**
     * The rendering backend used by the currently loaded library.
     */
    @JvmStatic
    @Volatile
    var currentType: Type = Type.VULKAN
        private set

    /**
     * This flavor's engine is fixed at compile time, so there's nothing to select —
     * always [Type.VULKAN].
     */
    @Suppress("UNUSED_PARAMETER")
    internal fun getDefaultRenderingEngine(context: Context): Type = Type.VULKAN

    /**
     * @throws IllegalStateException if [MapLibre.getInstance] has already been called in this process.
     * @throws UnsupportedOperationException if [type] differs from the compiled-in backend
     */
    internal fun setCurrentType(type: Type) {
        if (MapLibre.hasInstance()) {
            throw IllegalStateException(
                "RenderingEngine.setCurrentType() must be called before MapLibre.getInstance().",
            )
        }
        if (type != Type.VULKAN) {
            throw UnsupportedOperationException(
                "This MapLibre Android build supports only " + Type.VULKAN +
                    ". Use the multiBackend flavor to switch backends at runtime.",
            )
        }
        currentType = type
    }
}
