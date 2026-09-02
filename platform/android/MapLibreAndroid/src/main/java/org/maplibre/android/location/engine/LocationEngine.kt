package org.maplibre.android.location.engine

import android.Manifest.permission.ACCESS_COARSE_LOCATION
import android.Manifest.permission.ACCESS_FINE_LOCATION
import android.app.PendingIntent
import android.os.Looper
import androidx.annotation.RequiresPermission

/**
 * Generic location engine interface wrapper for the location providers.
 * Default providers bundled with MapLibre location library:
 * Android location provider and Google Play Services fused location provider
 *
 * @since 1.0.0
 */
interface LocationEngine {
    /**
     * Returns the most recent location currently available.
     *
     * If a location is not available, which should happen very rarely, null will be returned.
     *
     * @param callback [LocationEngineCallback] for the location result [LocationEngineResult].
     * @throws SecurityException if permission is not granted to access location services.
     * @since 1.0.0
     */
    @RequiresPermission(anyOf = [ACCESS_COARSE_LOCATION, ACCESS_FINE_LOCATION])
    @Throws(SecurityException::class)
    fun getLastLocation(callback: LocationEngineCallback<LocationEngineResult>)

    /**
     * Requests location updates with a callback on the specified Looper thread.
     *
     * @param request  [LocationEngineRequest] for the updates.
     * @param callback [LocationEngineCallback] for the location result [LocationEngineResult].
     * @param looper   The Looper object whose message queue will be used to implement the callback mechanism,
     *                 or null to invoke callbacks on the main thread.
     * @throws SecurityException if permission is not granted to access location services.
     * @since 1.0.0
     */
    @RequiresPermission(anyOf = [ACCESS_COARSE_LOCATION, ACCESS_FINE_LOCATION])
    @Throws(SecurityException::class)
    fun requestLocationUpdates(
        request: LocationEngineRequest,
        callback: LocationEngineCallback<LocationEngineResult>,
        looper: Looper?,
    )

    /**
     * Requests location updates with callback on the specified PendingIntent.
     *
     * @param request       [LocationEngineRequest] for the updates.
     * @param pendingIntent [PendingIntent] for the location result [LocationEngineResult].
     * @throws SecurityException if permission is not granted to access location services.
     * @since 1.1.0
     */
    @RequiresPermission(anyOf = [ACCESS_COARSE_LOCATION, ACCESS_FINE_LOCATION])
    @Throws(SecurityException::class)
    fun requestLocationUpdates(
        request: LocationEngineRequest,
        pendingIntent: PendingIntent?,
    )

    /**
     * Removes location updates for the given location engine callback.
     *
     * It is recommended to remove location requests when the activity is in a paused or
     * stopped state, doing so helps battery performance.
     *
     * @param callback [LocationEngineCallback] to remove.
     * @since 1.0.0
     */
    fun removeLocationUpdates(callback: LocationEngineCallback<LocationEngineResult>)

    /**
     * Removes location updates for the given pending intent.
     *
     * It is recommended to remove location requests when the activity is in a paused or
     * stopped state, doing so helps battery performance.
     *
     * @param pendingIntent [PendingIntent] to remove.
     * @since 1.1.0
     */
    fun removeLocationUpdates(pendingIntent: PendingIntent?)
}
