package org.maplibre.android.maps

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import org.maplibre.android.maps.Style.Builder.ImageWrapper
import java.lang.ref.WeakReference

internal class BitmapImageConversionTask(
	nativeMap: NativeMap,
) {
	private val nativeMap = WeakReference(nativeMap)
	private val job: Job = SupervisorJob(null)
	private val scope = CoroutineScope(Dispatchers.IO + job)

	fun execute(vararg params: ImageWrapper) {
		scope.launch {
			val images = params.map { Style.toImage(it) }
			val nativeMap = nativeMap.get()
			if (nativeMap != null && !nativeMap.isDestroyed) {
				nativeMap.addImages(images.toTypedArray())
			}
		}
	}

	/** Cancel any currently executing coroutines. */
	fun cancelAll() {
		job.cancel()
	}
}
