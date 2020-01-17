package com.mapbox.mapboxsdk.http;

import androidx.annotation.NonNull;

import com.mapbox.mapboxsdk.Mapbox;

public class HttpRequestUrl {

  private static final StringBuilder requestBuilder = new StringBuilder();

  private HttpRequestUrl() {
  }

  /**
   * Adapts a resource request url based on the host, query size, and offline requirement.
   * Mapbox resources downloaded for offline use are subject to separate Vector Tile and
   * Raster Tile API pricing and are not included in the Maps SDK’s “unlimited” requests.
   * See <a href="https://www.mapbox.com/pricing">our pricing page</a> for more information.
   *
   * @param host        the host used as endpoint
   * @param resourceUrl the resource to download
   * @param querySize   the query size of the resource request
   * @param offline     the type of resource, either offline or online
   * @return the adapted resource url
   */
  public static String buildResourceUrl(@NonNull String host, String resourceUrl, int querySize, boolean offline) {
    requestBuilder.setLength(0); // clear previous builder
    requestBuilder.append(resourceUrl);
    if (isValidMapboxEndpoint(host)) {
      if (querySize == 0) {
        requestBuilder.append("?");
      } else {
        requestBuilder.append("&");
      }
      if (offline) {
        requestBuilder.append("offline=true&");
      }
      requestBuilder.append("sku=").append(Mapbox.getSkuToken());
    }
    return requestBuilder.toString();
  }

  /**
   * Validates if the host used as endpoint is a valid Mapbox endpoint.
   *
   * @param host the host used as endpoint
   * @return true if a valid Mapbox endpoint
   */
  private static boolean isValidMapboxEndpoint(String host) {
    return host.equals("mapbox.com")
      || host.endsWith(".mapbox.com")
      || host.equals("mapbox.cn")
      || host.endsWith(".mapbox.cn");
  }
}
