package org.maplibre.android.http

import android.util.Log
import org.maplibre.android.log.Logger

object HttpLogger {
    private const val TAG = "Mbgl-HttpRequest"

    @JvmField
    var logRequestUrl: Boolean = false

    @JvmField
    var logEnabled: Boolean = true

    @JvmStatic
    fun logFailure(
        type: Int,
        errorMessage: String,
        requestUrl: String,
    ) {
        val temporary = type == HttpRequest.TEMPORARY_ERROR
        val connection = type == HttpRequest.CONNECTION_ERROR
        log(
            if (temporary) {
                Log.DEBUG
            } else if (connection) {
                Log.INFO
            } else {
                Log.WARN
            },
            String.format(
                "Request failed due to a %s error: %s %s",
                if (temporary) {
                    "temporary"
                } else if (connection) {
                    "connection"
                } else {
                    "permanent"
                },
                errorMessage,
                if (logRequestUrl) requestUrl else "",
            ),
        )
    }

    @JvmStatic
    fun log(
        type: Int,
        errorMessage: String,
    ) {
        if (logEnabled) {
            Logger.log(type, TAG, errorMessage)
        }
    }
}
