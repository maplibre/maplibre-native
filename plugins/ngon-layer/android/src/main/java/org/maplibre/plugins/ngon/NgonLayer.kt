package org.maplibre.plugins.ngon

import org.maplibre.android.RenderingEngine

object NgonLayer {
    init {
        // The plugin retains native callbacks for the process lifetime; never unload it.
        System.loadLibrary("ngon-plugin")
    }

    /** Register after initializing MapLibre and before loading a style containing n-gon layers. */
    @JvmStatic
    fun register() {
        val library = if (BuildConfig.MULTI_BACKEND && RenderingEngine.getCurrentType() == RenderingEngine.Type.OPENGL) {
            "libmaplibre-opengl.so"
        } else {
            "libmaplibre.so"
        }
        registerNative(library)
    }

    @JvmStatic
    private external fun registerNative(libraryName: String)
}
