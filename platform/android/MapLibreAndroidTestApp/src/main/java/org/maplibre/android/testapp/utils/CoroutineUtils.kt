package org.maplibre.android.testapp.utils

import kotlinx.coroutines.suspendCancellableCoroutine
import org.maplibre.android.camera.CameraUpdate
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.maps.MapView
import kotlin.coroutines.resume

suspend fun MapView.setStyleSuspend(styleUrl: String): Boolean =
    suspendCancellableCoroutine { continuation ->
        lateinit var listener: MapView.OnDidFinishLoadingStyleListener
        lateinit var errorListener: MapView.OnDidFailLoadingMapListener

        var resumed = false
        val resume: (Boolean) -> Unit = { result ->
            if (!resumed) {
                resumed = true
                removeOnDidFinishLoadingStyleListener(listener)
                removeOnDidFailLoadingMapListener(errorListener)
                continuation.resume(result)
            }
        }

        listener = MapView.OnDidFinishLoadingStyleListener {
            resume(true)
        }

        errorListener = MapView.OnDidFailLoadingMapListener {
            resume(false)
        }

        addOnDidFinishLoadingStyleListener(listener)
        addOnDidFailLoadingMapListener(errorListener)
        getMapAsync { map -> map.setStyle(styleUrl) }

        continuation.invokeOnCancellation {
            removeOnDidFinishLoadingStyleListener(listener)
            removeOnDidFailLoadingMapListener(errorListener)
        }

    }

suspend fun MapLibreMap.animateCameraSuspend(cameraUpdate: CameraUpdate, durationMs: Int): Unit =
    suspendCancellableCoroutine { continuation ->
        animateCamera(cameraUpdate, durationMs, object : CancelableCallback {
            var resumed = false

            override fun onCancel() {
                continuation.cancel()
            }

            override fun onFinish() {
                if (!resumed) {
                    resumed = true
                    continuation.resume(Unit)
                }
            }
        })
    }
