package org.maplibre.android.http

object HttpRequestUrl {
    /**
     * Adapts a resource request url based on the host, query size, and offline requirement.
     * MapLibre resources downloaded for offline use are subject to separate Vector Tile and
     * Raster Tile API pricing and are not included in the Maps SDK’s “unlimited” requests.
     * See [our pricing page](https://www.mapbox.com/pricing) for more information.
     *
     * @param host        the host used as endpoint
     * @param resourceUrl the resource to download
     * @param querySize   the query size of the resource request
     * @param offline     the type of resource, either offline or online
     * @return the adapted resource url
     */
    @JvmStatic
    fun buildResourceUrl(
        host: String,
        resourceUrl: String,
        querySize: Int,
        offline: Boolean,
    ): String {
        var url = resourceUrl
        if (isValidMapboxEndpoint(host)) {
            url = if (querySize == 0) "$url?" else "$url&"
            // Only add SKU token to requests not tagged as "offline" usage.
            if (offline) {
                url = url + "offline=true"
            }
        }
        return url
    }

    /**
     * Validates if the host used as endpoint is a valid MapLibre endpoint.
     *
     * @param host the host used as endpoint
     * @return true if a valid MapLibre endpoint
     */
    private fun isValidMapboxEndpoint(host: String): Boolean =
        host == "mapbox.com" ||
            host.endsWith(".mapbox.com") ||
            host == "mapbox.cn" ||
            host.endsWith(".mapbox.cn")
}
