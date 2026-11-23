package org.maplibre.android.module.http

import okhttp3.Call
import okio.Buffer

/**
 * Utility class for setting OkHttpRequest configurations
 */
object HttpRequestUtil {
    private const val MIN_PRINTABLE = 0x1f
    private const val MAX_PRINTABLE = 0x7f

    /**
     * Set the log state of OkHttpRequest. Default value is true.
     *
     * This configuration will outlast the lifecycle of the Map.
     *
     * @param enabled True will enable logging, false will disable
     */
    @JvmStatic
    fun setLogEnabled(enabled: Boolean) {
        HttpRequestImpl.enableLog(enabled)
    }

    /**
     * Enable printing of the request url when an error occurred. Default value is false.
     *
     * Requires [setLogEnabled] to be activated.
     *
     * This configuration will outlast the lifecycle of the Map.
     *
     * @param enabled True will print urls, false will disable
     */
    @JvmStatic
    fun setPrintRequestUrlOnFailure(enabled: Boolean) {
        HttpRequestImpl.enablePrintRequestUrlOnFailure(enabled)
    }

    /**
     * Set the OkHttp Call.Factory used for requesting map resources.
     *
     * This configuration survives across mapView instances.
     * Reset the OkHttpClient to the default by passing null as parameter.
     *
     * @param client the OkHttp Call.Factory, typically OkHttpClient.
     */
    @JvmStatic
    fun setOkHttpClient(client: Call.Factory?) {
        HttpRequestImpl.setOkHttpClient(client)
    }

    internal fun toHumanReadableAscii(s: String): String {
        val length = s.length
        var i = 0
        var c: Int
        while (i < length) {
            c = s.codePointAt(i)
            if (isPrintable(c)) {
                i += Character.charCount(c)
                continue
            }

            val buffer = Buffer()
            buffer.writeUtf8(s, 0, i)
            var j = i
            while (j < length) {
                c = s.codePointAt(j)
                buffer.writeUtf8CodePoint(if (isPrintable(c)) c else '?'.code)
                j += Character.charCount(c)
            }
            return buffer.readUtf8()
        }
        return s
    }

    private fun isPrintable(codePoint: Int) = codePoint > MIN_PRINTABLE && codePoint < MAX_PRINTABLE
}
