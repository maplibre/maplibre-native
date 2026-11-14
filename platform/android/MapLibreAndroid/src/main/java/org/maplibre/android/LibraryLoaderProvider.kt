package org.maplibre.android

/**
 * Injects the default library loader.
 */
interface LibraryLoaderProvider {
    /**
     * Creates and returns a the default Library Loader.
     *
     * @return the default library loader
     */
    fun getDefaultLibraryLoader(): LibraryLoader
}
