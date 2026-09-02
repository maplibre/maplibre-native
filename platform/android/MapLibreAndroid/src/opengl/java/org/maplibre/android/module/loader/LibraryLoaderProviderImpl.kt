package org.maplibre.android.module.loader

import org.maplibre.android.LibraryLoader
import org.maplibre.android.LibraryLoaderProvider

/**
 * OpenGL flavor: loads the single-backend libmaplibre.so via System.loadLibrary.
 */
class LibraryLoaderProviderImpl : LibraryLoaderProvider {
    override fun getDefaultLibraryLoader(): LibraryLoader = SystemLibraryLoader()

    private class SystemLibraryLoader : LibraryLoader() {
        override fun load(name: String) {
            System.loadLibrary(name)
        }
    }
}
