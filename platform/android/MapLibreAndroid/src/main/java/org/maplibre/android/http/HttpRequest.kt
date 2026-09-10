package org.maplibre.android.http

/**
 * Interface definition for performing http requests.
 *
 * This allows to provide alternative implementations for the http interaction of this library.
 */
interface HttpRequest {
    /**
     * Executes the request.
     *
     * @param httpRequest  callback to be invoked when we receive a response
     * @param nativePtr    the pointer associated to the request
     * @param resourceUrl  the resource url to download
     * @param dataRange    http header, used to indicate the part of a resource that the server should return
     * @param etag         http header, identifier for a specific version of a resource
     * @param modified     http header, used to determine if a resource hasn't been modified since
     * @param offlineUsage flag to indicate a resource will be used for offline, appends offline=true as a query parameter
     */
    fun executeRequest(
        httpRequest: HttpResponder,
        nativePtr: Long,
        resourceUrl: String,
        dataRange: String,
        etag: String,
        modified: String,
        offlineUsage: Boolean,
    )

    /**
     * Cancels the request.
     */
    fun cancelRequest()

    companion object {
        const val CONNECTION_ERROR: Int = 0
        const val TEMPORARY_ERROR: Int = 1
        const val PERMANENT_ERROR: Int = 2
    }
}
