package org.maplibre.android.location.engine

import android.annotation.SuppressLint
import android.app.PendingIntent
import android.content.Context
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.os.Bundle
import android.os.Looper
import android.util.Log
import androidx.annotation.VisibleForTesting

/**
 * A location engine that uses core android.location and has no external dependencies
 * https://developer.android.com/guide/topics/location/strategies.html
 */
open class AndroidLocationEngineImpl(
    context: Context,
) : LocationEngineImpl<LocationListener> {
    internal val locationManager: LocationManager =
        context.getSystemService(Context.LOCATION_SERVICE) as LocationManager

    internal var currentProvider: String = LocationManager.PASSIVE_PROVIDER

    override fun createListener(callback: LocationEngineCallback<LocationEngineResult>): LocationListener =
        AndroidLocationEngineCallbackTransport(callback)

    @Throws(SecurityException::class)
    override fun getLastLocation(callback: LocationEngineCallback<LocationEngineResult>) {
        val lastLocation = getLastLocationFor(currentProvider)
        if (lastLocation != null) {
            callback.onSuccess(LocationEngineResult.create(lastLocation))
            return
        }

        for (provider in locationManager.allProviders) {
            val location = getLastLocationFor(provider)
            if (location != null) {
                callback.onSuccess(LocationEngineResult.create(location))
                return
            }
        }
        callback.onFailure(Exception("Last location unavailable"))
    }

    @SuppressLint("MissingPermission")
    @Throws(SecurityException::class)
    internal fun getLastLocationFor(provider: String): Location? =
        try {
            locationManager.getLastKnownLocation(provider)
        } catch (iae: IllegalArgumentException) {
            Log.e(TAG, iae.toString())
            null
        }

    @SuppressLint("MissingPermission")
    @Throws(SecurityException::class)
    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        listener: LocationListener,
        looper: Looper?,
    ) {
        // Pick best provider only if user has not explicitly chosen passive mode
        currentProvider = getBestProvider(request.priority)
        locationManager.requestLocationUpdates(
            currentProvider,
            request.interval,
            request.displacement,
            listener,
            looper,
        )
    }

    @SuppressLint("MissingPermission")
    @Throws(SecurityException::class)
    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        pendingIntent: PendingIntent?,
    ) {
        // Pick best provider only if user has not explicitly chosen passive mode
        currentProvider = getBestProvider(request.priority)
        locationManager.requestLocationUpdates(
            currentProvider,
            request.interval,
            request.displacement,
            pendingIntent!!,
        )
    }

    @SuppressLint("MissingPermission")
    override fun removeLocationUpdates(listener: LocationListener) {
        locationManager.removeUpdates(listener)
    }

    override fun removeLocationUpdates(pendingIntent: PendingIntent?) {
        if (pendingIntent != null) {
            locationManager.removeUpdates(pendingIntent)
        }
    }

    private fun getBestProvider(priority: Int): String {
        var provider: String? = null
        // Pick best provider only if user has not explicitly chosen passive mode
        if (priority != LocationEngineRequest.PRIORITY_NO_POWER) {
            provider = locationManager.getBestProvider(getCriteria(priority), true)
        }
        return provider ?: LocationManager.PASSIVE_PROVIDER
    }

    @VisibleForTesting
    internal class AndroidLocationEngineCallbackTransport(
        private val callback: LocationEngineCallback<LocationEngineResult>,
    ) : LocationListener {
        override fun onLocationChanged(location: Location) {
            callback.onSuccess(LocationEngineResult.create(location))
        }

        override fun onStatusChanged(
            s: String?,
            i: Int,
            bundle: Bundle?,
        ) {
            // noop
        }

        override fun onProviderEnabled(s: String) {
            // noop
        }

        override fun onProviderDisabled(s: String) {
            callback.onFailure(Exception("Current provider disabled"))
        }
    }

    companion object {
        private const val TAG = "AndroidLocationEngine"

        @VisibleForTesting
        @JvmStatic
        internal fun getCriteria(priority: Int): Criteria {
            val criteria = Criteria()
            criteria.accuracy = priorityToAccuracy(priority)
            criteria.isCostAllowed = true
            criteria.powerRequirement = priorityToPowerRequirement(priority)
            return criteria
        }

        private fun priorityToAccuracy(priority: Int): Int =
            when (priority) {
                LocationEngineRequest.PRIORITY_HIGH_ACCURACY,
                LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY,
                -> Criteria.ACCURACY_FINE

                else -> Criteria.ACCURACY_COARSE
            }

        private fun priorityToPowerRequirement(priority: Int): Int =
            when (priority) {
                LocationEngineRequest.PRIORITY_HIGH_ACCURACY -> Criteria.POWER_HIGH
                LocationEngineRequest.PRIORITY_BALANCED_POWER_ACCURACY -> Criteria.POWER_MEDIUM
                else -> Criteria.POWER_LOW
            }
    }
}
