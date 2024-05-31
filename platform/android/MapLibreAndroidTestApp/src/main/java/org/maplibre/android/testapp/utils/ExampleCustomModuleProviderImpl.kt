package org.maplibre.android.testapp.utils

import org.maplibre.android.LibraryLoaderProvider
import org.maplibre.android.ModuleProvider
import org.maplibre.android.http.HttpRequest
import org.maplibre.android.module.loader.LibraryLoaderProviderImpl
import org.maplibre.android.testapp.utils.ExampleHttpRequestImpl

/*
 * An example implementation of the ModuleProvider. This is useful primarily for providing
 * a custom implementation of HttpRequest used by the core.
 */
class ExampleCustomModuleProviderImpl : ModuleProvider {
    override fun createHttpRequest(): HttpRequest {
        return ExampleHttpRequestImpl()
    }

    override fun createLibraryLoaderProvider(): LibraryLoaderProvider {
        return LibraryLoaderProviderImpl()
    }
}
