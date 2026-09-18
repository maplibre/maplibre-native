package org.maplibre.plugins.android

internal object NativePlugin {
    init {
        // The plugin retains native callbacks for the process lifetime; never unload it.
        System.loadLibrary("ngon-plugin")
    }

    @JvmStatic
    external fun registerNgon(libraryName: String)
}
