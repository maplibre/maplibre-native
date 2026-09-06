package org.maplibre.android.module.loader

import org.maplibre.android.LibraryLoader
import org.maplibre.android.LibraryLoaderProvider
import org.maplibre.android.RenderingEngine

/**
 * multiBackend flavor: when asked to load "maplibre", routes to either
 * libmaplibre.so (Vulkan, the default name) or libmaplibre-opengl.so based on
 * [RenderingEngine.getCurrentType]. Any other library name is loaded
 * verbatim.
 */
class LibraryLoaderProviderImpl : LibraryLoaderProvider {
    override fun getDefaultLibraryLoader(): LibraryLoader = MultiBackendLibraryLoader()

    private class MultiBackendLibraryLoader : LibraryLoader() {
        override fun load(name: String) {
            if ("maplibre" == name && RenderingEngine.currentType == RenderingEngine.Type.OPENGL) {
                System.loadLibrary("maplibre-opengl")
            } else {
                System.loadLibrary(name)
            }
        }
    }
}
