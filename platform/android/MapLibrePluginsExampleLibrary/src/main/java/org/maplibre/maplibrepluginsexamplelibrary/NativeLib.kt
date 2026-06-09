package org.maplibre.maplibrepluginsexamplelibrary

class NativeLib {

    /**
     * A native method that is implemented by the 'maplibrepluginsexamplelibrary' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    /**
     * Register plugins with MapLibre.
     */
    external fun registerPlugins()

    companion object {
        // Used to load the 'maplibrepluginsexamplelibrary' library on application startup.
        init {
            // Load maplibre first - the plugin library depends on its symbols
            System.loadLibrary("maplibre")
            System.loadLibrary("maplibrepluginsexamplelibrary")
        }
    }
}