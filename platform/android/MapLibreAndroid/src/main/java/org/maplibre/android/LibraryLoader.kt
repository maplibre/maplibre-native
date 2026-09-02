package org.maplibre.android

import org.maplibre.android.log.Logger
import org.maplibre.android.utils.PlatformUtils

/**
 * Loads the mapbox-gl shared library
 *
 * By default uses System.loadLibrary
 * use [setLibraryLoader] to provide an alternative library loading hook.
 */
abstract class LibraryLoader {
    abstract fun load(name: String)

    companion object {
        private const val TAG = "Mbgl-LibraryLoader"

        private val DEFAULT: LibraryLoader =
            MapLibre
                .getModuleProvider()
                .createLibraryLoaderProvider()
                .getDefaultLibraryLoader()

        @Volatile
        private var loader: LibraryLoader = DEFAULT

        private var loaded = false
        private var handleLoadError = false

        /**
         * Set the library loader that loads the shared library.
         *
         * @param libraryLoader the library loader
         */
        @JvmStatic
        fun setLibraryLoader(libraryLoader: LibraryLoader) {
            loader = libraryLoader
        }

        /**
         * Catch UnsatisfiedLinkErrors on load
         */
        @JvmStatic
        fun enableErrorHandling(value: Boolean) {
            handleLoadError = value
        }

        /**
         * Loads "libmaplibre.so" native shared library.
         *
         * Catches UnsatisfiedLinkErrors (if enabled) and prints a warning to logcat.
         */
        @JvmStatic
        @Synchronized
        fun load() {
            try {
                if (!loaded) {
                    loaded = true
                    loader.load("maplibre")
                }
            } catch (error: UnsatisfiedLinkError) {
                loaded = false
                val message = "Failed to load native shared library."
                Logger.e(TAG, message, error)
                MapStrictMode.strictModeViolation(message, error)

                if (!handleLoadError && !PlatformUtils.isTest()) {
                    throw error
                }
            }
        }
    }
}
