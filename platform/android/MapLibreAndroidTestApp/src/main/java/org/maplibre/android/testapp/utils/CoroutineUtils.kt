package org.maplibre.android.testapp.utils

import kotlinx.coroutines.suspendCancellableCoroutine
import org.maplibre.android.camera.CameraUpdate
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.maps.MapView
import kotlin.coroutines.resume

suspend fun MapView.setStyleSuspend(styleUrl: String): Unit =
    suspendCancellableCoroutine { continuation ->
        var listener: MapView.OnDidFinishLoadingStyleListener? = null

        var resumed = false
        listener = MapView.OnDidFinishLoadingStyleListener {
            if (!resumed) {
                resumed = true
                listener?.let { removeOnDidFinishLoadingStyleListener(it) }
                continuation.resume(Unit)
            }
        }
        addOnDidFinishLoadingStyleListener(listener)
        getMapAsync { map -> map.setStyle(styleUrl) }

        continuation.invokeOnCancellation {
            removeOnDidFinishLoadingStyleListener(listener)
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
