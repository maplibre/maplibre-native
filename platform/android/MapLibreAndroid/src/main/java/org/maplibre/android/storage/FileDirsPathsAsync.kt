package org.maplibre.android.storage

import android.content.Context
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

fun fileDirsPathsAsync(
	context: Context,
	onCompletion: (resourcesCachePath: String, internalCachePath: String) -> Unit
) {
	CoroutineScope(Dispatchers.IO).launch {
		try {
			val path1 = FileSource.getCachePath(context)
			val path2 = context.cacheDir.absolutePath
			withContext(Dispatchers.Main) {
				onCompletion(path1, path2)
			}
		} finally {
			withContext(Dispatchers.Main) {
				FileSource.unlockPathLoaders()
			}
		}
	}
}