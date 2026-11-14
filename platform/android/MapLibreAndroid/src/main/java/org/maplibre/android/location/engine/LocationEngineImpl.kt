package org.maplibre.android.location.engine

import android.app.PendingIntent
import android.os.Looper

/**
 * Internal location engine implementation interface.
 *
 * @param T location listener object type
 */
interface LocationEngineImpl<T> {
    fun createListener(callback: LocationEngineCallback<LocationEngineResult>): T

    @Throws(SecurityException::class)
    fun getLastLocation(callback: LocationEngineCallback<LocationEngineResult>)

    @Throws(SecurityException::class)
    fun requestLocationUpdates(
        request: LocationEngineRequest,
        listener: T,
        looper: Looper?,
    )

    @Throws(SecurityException::class)
    fun requestLocationUpdates(
        request: LocationEngineRequest,
        pendingIntent: PendingIntent?,
    )

    fun removeLocationUpdates(listener: T)

    fun removeLocationUpdates(pendingIntent: PendingIntent?)
}
