package org.maplibre.android.http

import android.content.res.AssetManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.MapLibre
import org.maplibre.android.MapStrictMode
import org.maplibre.android.log.Logger
import java.io.IOException
import java.io.InputStream

internal class LocalRequestTask(private val scope: CoroutineScope, private val onCompletion: ((ByteArray?) -> Unit)?) {

	fun execute(url: String) {
		scope.launch(Dispatchers.IO) {
			val bytes = loadFile(
				MapLibre.getApplicationContext().assets,
				"integration/" + url
					.substring(8)
					.replace("%20".toRegex(), " ")
					.replace("%2c".toRegex(), ",")
			)
			withContext(Dispatchers.Main) {
				onCompletion?.invoke(bytes)
			}
		}
	}

	companion object {
		private const val TAG = "Mbgl-LocalRequestTask"

		private fun loadFile(assets: AssetManager, path: String): ByteArray? {
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
