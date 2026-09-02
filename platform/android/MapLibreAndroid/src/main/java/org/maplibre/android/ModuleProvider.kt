package org.maplibre.android

import org.maplibre.android.http.HttpRequest

/**
 * Injects concrete instances of configurable abstractions
 */
interface ModuleProvider {
    /**
     * Create a new concrete implementation of HttpRequest.
     *
     * @return a new instance of an HttpRequest
     */
    fun createHttpRequest(): HttpRequest

    /**
     * Get the concrete implementation of LibraryLoaderProvider
     *
     * @return a new instance of LibraryLoaderProvider
     */
    fun createLibraryLoaderProvider(): LibraryLoaderProvider
}
