package org.maplibre.android.utils

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.log.Logger
import java.io.File

private const val TAG = "Mbgl-FileUtils"

/**
 * Deletes a file asynchronously in a separate coroutine.
 *
 * @param path the path of the file that should be deleted
 */
fun deleteFileAsync(path: String, scope: CoroutineScope) {
	// Delete the file in a separate coroutine to avoid affecting the UI
	scope.launch(Dispatchers.IO) {
		try {
			val file = File(path)
			if (file.exists()) {
				if (file.delete()) {
					Logger.d(TAG, "File deleted to save space: $path")
				} else {
					Logger.e(TAG, "Failed to delete file: $path")
				}
			}
		} catch (exception: Exception) {
			Logger.e(TAG, "Failed to delete file: ", exception)
		}
	}
}

fun checkFileReadPermissionAsync(file: File, scope: CoroutineScope, onCompletion: (Boolean, Throwable?) -> Unit) {
	scope.launch(Dispatchers.IO) {
		try {
			val result = file.canRead()
			withContext(Dispatchers.Main) {
				onCompletion(result, null)
			}
		} catch (e: Exception) {
			withContext(Dispatchers.Main) {
				onCompletion(false, e)
			}
		}
	}
}

fun checkFileWritePermissionAsync(file: File, scope: CoroutineScope, onCompletion: (Boolean, Throwable?) -> Unit) {
	scope.launch(Dispatchers.IO) {
		try {
			val result = file.canWrite()
			withContext(Dispatchers.Main) {
				onCompletion(result, null)
			}
		} catch (e: Exception) {
			withContext(Dispatchers.Main) {
				onCompletion(false, e)
			}
		}
	}
}
