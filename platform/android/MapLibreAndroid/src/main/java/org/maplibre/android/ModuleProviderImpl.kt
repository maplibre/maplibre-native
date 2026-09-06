package org.maplibre.android

import org.maplibre.android.http.HttpRequest
import org.maplibre.android.module.http.HttpRequestImpl
import org.maplibre.android.module.loader.LibraryLoaderProviderImpl

class ModuleProviderImpl : ModuleProvider {
    override fun createHttpRequest(): HttpRequest = HttpRequestImpl()

    override fun createLibraryLoaderProvider(): LibraryLoaderProvider = LibraryLoaderProviderImpl()
}
