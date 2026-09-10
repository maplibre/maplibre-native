package org.maplibre.android

import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import androidx.annotation.Keep

/**
 * multiBackend flavor: the rendering engine is selectable at runtime.
 *
 * Call [setCurrentType] before the first call to [MapLibre.getInstance] (or any other path that
 * triggers native library loading). [setCurrentType] throws once [MapLibre.getInstance] has been
 * called, since the native library is loaded by then and the selection can no longer take effect for
 * the current process.
 */
@Keep
object RenderingEngine {
    /**
     * Matches the android.hardware.vulkan.version `uses-feature` declared for this flavor.
     */
    private const val REQUIRED_VULKAN_VERSION = 0x400003

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
     * @throws IllegalStateException if [MapLibre.getInstance] has already been called in this process.
     */
    internal fun setCurrentType(type: Type) {
        if (MapLibre.hasInstance()) {
            throw IllegalStateException(
                "RenderingEngine.setCurrentType() must be called before MapLibre.getInstance().",
            )
        }
        currentType = type
    }

    /**
     * Determines which rendering engine to use when none was explicitly requested:
     * Vulkan if this device's hardware supports it, OpenGL otherwise.
     */
    internal fun getDefaultRenderingEngine(context: Context): Type = if (isVulkanSupported(context)) Type.VULKAN else Type.OPENGL

    private fun isVulkanSupported(context: Context): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
            return false
        }
        return context.packageManager.hasSystemFeature(
            PackageManager.FEATURE_VULKAN_HARDWARE_VERSION,
            REQUIRED_VULKAN_VERSION,
        )
    }
}
