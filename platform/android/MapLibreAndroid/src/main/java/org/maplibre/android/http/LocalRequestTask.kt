package org.maplibre.android.http

import android.content.res.AssetManager
import android.os.AsyncTask
import org.maplibre.android.MapStrictMode
import org.maplibre.android.log.Logger
import java.io.IOException
import java.io.InputStream

@Suppress("DEPRECATION")
internal class LocalRequestTask(
    private val assets: AssetManager,
    private val requestResponse: OnLocalRequestResponse?,
) : AsyncTask<String, Void, ByteArray?>() {
    override fun doInBackground(vararg strings: String): ByteArray? =
        loadFile(
            assets,
            "integration/" +
                strings[0]
                    .substring(8)
                    .replace("%20", " ")
                    .replace("%2c", ","),
        )

    override fun onPostExecute(bytes: ByteArray?) {
        super.onPostExecute(bytes)
        if (bytes != null && requestResponse != null) {
            requestResponse.onResponse(bytes)
        }
    }

    interface OnLocalRequestResponse {
        fun onResponse(bytes: ByteArray?)
    }

    companion object {
        private const val TAG = "Mbgl-LocalRequestTask"

        private fun loadFile(
            assets: AssetManager,
            path: String,
        ): ByteArray? {
            var buffer: ByteArray? = null
            var input: InputStream? = null
            try {
                input = assets.open(path)
                val size = input.available()
                buffer = ByteArray(size)
                input.read(buffer)
            } catch (exception: IOException) {
                logFileError(exception)
            } finally {
                if (input != null) {
                    try {
                        input.close()
                    } catch (exception: IOException) {
                        logFileError(exception)
                    }
                }
            }
            return buffer
        }

        private fun logFileError(exception: Exception) {
            val message = "Load file failed"
            Logger.e(TAG, message, exception)
            MapStrictMode.strictModeViolation(message, exception)
        }
    }
}
