package org.maplibre.android.location.engine

import android.annotation.SuppressLint
import android.app.PendingIntent
import android.content.Context
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.os.Bundle
import android.os.Looper
import org.maplibre.android.location.engine.Utils.isBetterLocation
import timber.log.Timber

/**
 * MapLibre replacement for Google Play Services Fused Location Client
 *
 * Note: fusion will not work in background mode.
 */
class MapLibreFusedLocationEngineImpl(
    context: Context,
) : AndroidLocationEngineImpl(context) {
    override fun createListener(callback: LocationEngineCallback<LocationEngineResult>): LocationListener =
        MapLibreLocationEngineCallbackTransport(callback)

    @Throws(SecurityException::class)
    override fun getLastLocation(callback: LocationEngineCallback<LocationEngineResult>) {
        val bestLastLocation = getBestLastLocation()
        if (bestLastLocation != null) {
            callback.onSuccess(LocationEngineResult.create(bestLastLocation))
        } else {
            callback.onFailure(Exception("Last location unavailable"))
        }
    }

    @SuppressLint("MissingPermission")
    @Throws(SecurityException::class)
    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        listener: LocationListener,
        looper: Looper?,
    ) {
        super.requestLocationUpdates(request, listener, looper)

        // Start network provider along with gps
        if (shouldStartNetworkProvider(request.priority)) {
            try {
                locationManager.requestLocationUpdates(
                    LocationManager.NETWORK_PROVIDER,
                    request.interval,
                    request.displacement,
                    listener,
                    looper,
                )
            } catch (iae: IllegalArgumentException) {
                iae.printStackTrace()
            }
        }
    }

    @SuppressLint("MissingPermission")
    @Throws(SecurityException::class)
    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        pendingIntent: PendingIntent?,
    ) {
        super.requestLocationUpdates(request, pendingIntent)

        // Start network provider along with gps
        if (shouldStartNetworkProvider(request.priority)) {
            try {
                locationManager.requestLocationUpdates(
                    LocationManager.NETWORK_PROVIDER,
                    request.interval,
                    request.displacement,
                    pendingIntent!!,
                )
            } catch (iae: IllegalArgumentException) {
                iae.printStackTrace()
            }
        }
    }

    private fun getBestLastLocation(): Location? {
        var bestLastLocation: Location? = null
        for (provider in locationManager.allProviders) {
            val location = getLastLocationFor(provider) ?: continue

            if (isBetterLocation(location, bestLastLocation)) {
                bestLastLocation = location
            }
        }
        return bestLastLocation
    }

    private fun shouldStartNetworkProvider(priority: Int): Boolean =
        (
            priority == LocationEngineRequest.PRIORITY_HIGH_ACCURACY ||
                priority == LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY
        ) &&
            currentProvider == LocationManager.GPS_PROVIDER

    private class MapLibreLocationEngineCallbackTransport(
        private val callback: LocationEngineCallback<LocationEngineResult>,
    ) : LocationListener {
        private var currentBestLocation: Location? = null

        override fun onLocationChanged(location: Location) {
            if (isBetterLocation(location, currentBestLocation)) {
                currentBestLocation = location
            }

            callback.onSuccess(LocationEngineResult.create(currentBestLocation))
        }

        override fun onStatusChanged(
            provider: String?,
            status: Int,
            extras: Bundle?,
        ) {
            Timber.d("onStatusChanged: %s", provider)
        }

        override fun onProviderEnabled(provider: String) {
            Timber.d("onProviderEnabled: %s", provider)
        }

        override fun onProviderDisabled(provider: String) {
            Timber.d("onProviderDisabled: %s", provider)
        }
    }
}
