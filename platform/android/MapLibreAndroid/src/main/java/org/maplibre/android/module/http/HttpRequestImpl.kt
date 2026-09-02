package org.maplibre.android.module.http

import android.os.Build
import android.text.TextUtils
import android.util.Log
import androidx.annotation.VisibleForTesting
import okhttp3.Call
import okhttp3.Callback
import okhttp3.Dispatcher
import okhttp3.HttpUrl.Companion.toHttpUrlOrNull
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import org.maplibre.android.BuildConfig
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.http.HttpIdentifier
import org.maplibre.android.http.HttpLogger
import org.maplibre.android.http.HttpRequest
import org.maplibre.android.http.HttpRequestUrl
import org.maplibre.android.http.HttpResponder
import org.maplibre.android.module.http.HttpRequestUtil.toHumanReadableAscii
import java.io.IOException
import java.io.InterruptedIOException
import java.net.NoRouteToHostException
import java.net.ProtocolException
import java.net.SocketException
import java.net.UnknownHostException
import javax.net.ssl.SSLException

class HttpRequestImpl : HttpRequest {
    private var call: Call? = null

    override fun executeRequest(
        httpRequest: HttpResponder,
        nativePtr: Long,
        resourceUrl: String,
        dataRange: String,
        etag: String,
        modified: String,
        offlineUsage: Boolean,
    ) {
        val callback = OkHttpCallback(httpRequest)
        try {
            val httpUrl = resourceUrl.toHttpUrlOrNull()
            if (httpUrl == null) {
                HttpLogger.log(Log.ERROR, String.format("[HTTP] Unable to parse resourceUrl %s", resourceUrl))
                return
            }

            val host = httpUrl.host.lowercase(MapLibreConstants.MAPLIBRE_LOCALE)
            val url = HttpRequestUrl.buildResourceUrl(host, resourceUrl, httpUrl.querySize, offlineUsage)

            val builder =
                Request
                    .Builder()
                    .url(url)
                    .tag(url.lowercase(MapLibreConstants.MAPLIBRE_LOCALE))
                    .addHeader("User-Agent", userAgentString)

            if (dataRange.isNotEmpty()) {
                builder.addHeader("Range", dataRange)
            }

            if (etag.isNotEmpty()) {
                builder.addHeader("If-None-Match", etag)
            } else if (modified.isNotEmpty()) {
                builder.addHeader("If-Modified-Since", modified)
            }

            val newCall = getHttpClient().newCall(builder.build())
            call = newCall
            newCall.enqueue(callback)
        } catch (exception: Exception) {
            callback.handleFailure(call, exception)
        }
    }

    override fun cancelRequest() {
        // call can be null if the constructor gets aborted (e.g, under a NoRouteToHostException).
        call?.let {
            HttpLogger.log(
                Log.DEBUG,
                String.format(
                    "[HTTP] This request was cancelled (%s). This is expected for tiles" +
                        " that were being prefetched but are no longer needed for the map to render.",
                    it.request().url,
                ),
            )
            it.cancel()
        }
    }

    private class OkHttpCallback(
        private val httpRequest: HttpResponder,
    ) : Callback {
        override fun onFailure(
            call: Call,
            e: IOException,
        ) {
            handleFailure(call, e)
        }

        override fun onResponse(
            call: Call,
            response: Response,
        ) {
            if (response.isSuccessful) {
                HttpLogger.log(
                    Log.VERBOSE,
                    String.format("[HTTP] Request was successful (code = %s).", response.code),
                )
            } else {
                // We don't want to call this unsuccessful because a 304 isn't really an error
                val message = if (!TextUtils.isEmpty(response.message)) response.message else "No additional information"
                HttpLogger.log(
                    Log.DEBUG,
                    String.format("[HTTP] Request with response = %s: %s", response.code, message),
                )
            }

            val responseBody = response.body
            if (responseBody == null) {
                HttpLogger.log(Log.ERROR, "[HTTP] Received empty response body")
                return
            }

            val body: ByteArray
            try {
                body = responseBody.bytes()
            } catch (ioException: IOException) {
                onFailure(call, ioException)
                // throw ioException;
                return
            } finally {
                response.close()
            }

            httpRequest.onResponse(
                response.code,
                response.header("ETag"),
                response.header("Last-Modified"),
                response.header("Cache-Control"),
                response.header("Expires"),
                response.header("Retry-After"),
                response.header("x-rate-limit-reset"),
                body,
            )
        }

        fun handleFailure(
            call: Call?,
            e: Exception,
        ) {
            val errorMessage = e.message ?: "Error processing the request"
            val type = getFailureType(e)

            if (HttpLogger.logEnabled && call != null) {
                val requestUrl = call.request().url.toString()
                HttpLogger.logFailure(type, errorMessage, requestUrl)
            }
            httpRequest.handleFailure(type, errorMessage)
        }

        private fun getFailureType(e: Exception): Int {
            if (e is NoRouteToHostException || e is UnknownHostException || e is SocketException ||
                e is ProtocolException || e is SSLException
            ) {
                return HttpRequest.CONNECTION_ERROR
            } else if (e is InterruptedIOException) {
                return HttpRequest.TEMPORARY_ERROR
            }
            return HttpRequest.PERMANENT_ERROR
        }
    }

    companion object {
        private val userAgentString: String =
            toHumanReadableAscii(
                String.format(
                    "%s %s (%s) Android/%s (%s)",
                    HttpIdentifier.getIdentifier(),
                    BuildConfig.MAPLIBRE_VERSION_STRING,
                    BuildConfig.GIT_REVISION_SHORT,
                    Build.VERSION.SDK_INT,
                    Build.SUPPORTED_ABIS[0],
                ),
            )

        @VisibleForTesting
        @Volatile
        internal var defaultClient: OkHttpClient? = null

        @VisibleForTesting
        @Volatile
        internal var client: Call.Factory? = null

        @JvmStatic
        fun enablePrintRequestUrlOnFailure(enabled: Boolean) {
            HttpLogger.logRequestUrl = enabled
        }

        @JvmStatic
        fun enableLog(enabled: Boolean) {
            HttpLogger.logEnabled = enabled
        }

        @JvmStatic
        fun setOkHttpClient(client: Call.Factory?) {
            this.client = client ?: getOrCreateDefaultClient()
        }

        private fun getHttpClient(): Call.Factory {
            if (client == null) {
                synchronized(HttpRequestImpl::class.java) {
                    if (client == null) {
                        client = getOrCreateDefaultClient()
                    }
                }
            }

            return client!!
        }

        private fun getOrCreateDefaultClient(): OkHttpClient {
            synchronized(HttpRequestImpl::class.java) {
                var httpClient = defaultClient
                if (httpClient == null) {
                    httpClient = OkHttpClient.Builder().dispatcher(getDispatcher()).build()
                    defaultClient = httpClient
                }

                return httpClient
            }
        }

        private fun getDispatcher(): Dispatcher {
            val dispatcher = Dispatcher()
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                // Matches core limit set on
                // https://github.com/mapbox/mapbox-gl-native/blob/master/platform/android/src/http_file_source.cpp#L192
                dispatcher.maxRequestsPerHost = 20
            } else {
                // Limiting concurrent request on Android 4.4, to limit impact of SSL handshake platform library crash
                // https://github.com/mapbox/mapbox-gl-native/issues/14910
                dispatcher.maxRequestsPerHost = 10
            }
            return dispatcher
        }
    }
}
