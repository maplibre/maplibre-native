package org.maplibre.android.utils

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.log.Logger
import java.io.File

object FileUtils {
	private const val TAG = "Mbgl-FileUtils"

	/**
	 * Deletes a file asynchronously in a separate thread.
	 *
	 * @param path the path of the file that should be deleted
	 */
	fun deleteFile(path: String) {
		// Delete the file in a separate thread to avoid affecting the UI
		Thread {
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
		}.start()
	}
}

/**
 * Task checking whether app's process can read a file.
 */
class CheckFileReadPermissionTask(private val scope: CoroutineScope, private val onCompletion: (Boolean, Throwable?) -> Unit)  {
	fun execute(file: File) {
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
}

/**
 * Task checking whether app's process can write to a file.
 */
class CheckFileWritePermissionTask(private val scope: CoroutineScope, private val onCompletion: (Boolean, Throwable?) -> Unit)  {
	fun execute(file: File) {
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
}
