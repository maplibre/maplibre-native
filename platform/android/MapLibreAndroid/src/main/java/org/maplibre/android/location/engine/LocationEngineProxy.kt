package org.maplibre.android.location.engine

import android.app.PendingIntent
import android.os.Looper
import androidx.annotation.VisibleForTesting
import java.util.concurrent.ConcurrentHashMap

open class LocationEngineProxy<T>(
    private val locationEngineImpl: LocationEngineImpl<T>,
) : LocationEngine {
    private var listeners: MutableMap<LocationEngineCallback<LocationEngineResult>, T>? = null

    @Throws(SecurityException::class)
    override fun getLastLocation(callback: LocationEngineCallback<LocationEngineResult>) {
        locationEngineImpl.getLastLocation(callback)
    }

    @Throws(SecurityException::class)
    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        callback: LocationEngineCallback<LocationEngineResult>,
        looper: Looper?,
    ) {
        locationEngineImpl.requestLocationUpdates(request, getListener(callback), looper ?: Looper.getMainLooper())
    }

    @Throws(SecurityException::class)
    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        pendingIntent: PendingIntent?,
    ) {
        locationEngineImpl.requestLocationUpdates(request, pendingIntent)
    }

    override fun removeLocationUpdates(callback: LocationEngineCallback<LocationEngineResult>) {
        removeListener(callback)?.let { locationEngineImpl.removeLocationUpdates(it) }
    }

    override fun removeLocationUpdates(pendingIntent: PendingIntent?) {
        locationEngineImpl.removeLocationUpdates(pendingIntent)
    }

    @get:VisibleForTesting
    internal val listenersCount: Int
        get() = listeners?.size ?: 0

    @VisibleForTesting
    internal fun getListener(callback: LocationEngineCallback<LocationEngineResult>): T {
        val listeners =
            this.listeners ?: ConcurrentHashMap<LocationEngineCallback<LocationEngineResult>, T>()
                .also { this.listeners = it }

        val listener = listeners[callback] ?: locationEngineImpl.createListener(callback)
        listeners[callback] = listener
        return listener
    }

    @VisibleForTesting
    internal fun removeListener(callback: LocationEngineCallback<LocationEngineResult>): T? = listeners?.remove(callback)
}
