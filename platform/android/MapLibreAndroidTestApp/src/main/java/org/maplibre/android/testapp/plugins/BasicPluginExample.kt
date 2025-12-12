package org.maplibre.android.testapp.plugins

import androidx.annotation.Keep

/**
 * A basic example MapLibre plugin that demonstrates the cross-platform plugin architecture.
 *
 * This plugin logs lifecycle events and provides a method to navigate to San Francisco.
 * The native implementation is built into libmaplibre.so.
 */
@Keep
class BasicPluginExample : org.maplibre.android.XPlatformPluginBridge {

    private var nativePtr: Long = nativeCreate()

    override fun getNativePlugin(): Long = nativePtr

    fun showSanFrancisco() {
        nativeShowSanFrancisco(nativePtr)
    }

    protected fun finalize() {
        nativeDestroy(nativePtr)
    }

    private external fun nativeCreate(): Long
    private external fun nativeDestroy(nativePtr: Long)
    private external fun nativeShowSanFrancisco(nativePtr: Long)
}
