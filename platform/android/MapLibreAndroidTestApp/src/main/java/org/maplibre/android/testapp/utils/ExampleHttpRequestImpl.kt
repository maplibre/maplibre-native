package org.maplibre.android.testapp.utils

import org.maplibre.android.http.HttpResponder
import org.maplibre.android.http.HttpRequest
import org.maplibre.android.module.http.HttpRequestImpl

/*
 * An example implementation of HttpRequest for use with a custom implementation of
 * the ModuleProvider. This allows you to provide your own transport mechanism for HTTP
 * requests made by the library.
 */
class ExampleHttpRequestImpl : HttpRequest {
    override fun executeRequest(httpRequest: HttpResponder, nativePtr: Long, resourceUrl: String,
                                etag: String, modified: String, offlineUsage: Boolean)
    {
        // Load all json documents and any pbf ending with a 0.
        if (resourceUrl.endsWith(".json") || resourceUrl.endsWith("0.pbf")) {
            impl.executeRequest(httpRequest, nativePtr, resourceUrl, etag, modified, offlineUsage)
        } else {
            // All other requests get an instant 404!
            httpRequest.onResponse(
                    /* responseCode */ 404,
                    /* eTag */ etag,
                    /* lastModified */ "",
                    /* cacheControl */ "",
                    /* expires */ "",
                    /* retryAfter */ "",
                    /* xRateLimitReset */ "",
                    /* body */ byteArrayOf(0x0)
            )
        }
    }

    override fun cancelRequest() {
        impl.cancelRequest()
    }

    // The default implementation using okhttp
    private var impl: HttpRequestImpl = HttpRequestImpl()
}
