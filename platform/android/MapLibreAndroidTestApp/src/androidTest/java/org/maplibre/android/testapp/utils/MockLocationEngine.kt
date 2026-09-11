package org.maplibre.android.testapp.utils

import android.app.PendingIntent
import android.location.Location
import android.os.Looper
import org.maplibre.android.location.engine.LocationEngine
import org.maplibre.android.location.engine.LocationEngineCallback
import org.maplibre.android.location.engine.LocationEngineRequest
import org.maplibre.android.location.engine.LocationEngineResult

class MockLocationEngine() : LocationEngine {
    private val listeners = mutableSetOf<LocationEngineCallback<LocationEngineResult>>()
    private var currentLocation: Location = Location("mock").apply {
        latitude = 0.0
        longitude = 0.0
        accuracy = 1.0f
    }

    constructor(location: Location) : this() {
        currentLocation = location
    }

    fun setLocation(newLocation: Location) {
        currentLocation = newLocation
        for (listener in listeners) {
            listener.onSuccess(LocationEngineResult.create(currentLocation))
        }
    }

    override fun getLastLocation(callback: LocationEngineCallback<LocationEngineResult>) {
        callback.onSuccess(LocationEngineResult.create(currentLocation))
    }

    override fun requestLocationUpdates(
        request: LocationEngineRequest,
        listener: LocationEngineCallback<LocationEngineResult>,
        looper: Looper?
    ) {
        listeners.add(listener)
        listener.onSuccess(LocationEngineResult.create(currentLocation))
    }

    override fun requestLocationUpdates(
        request: LocationEngineRequest, pendingIntent: PendingIntent?
    ) {
    }

    override fun removeLocationUpdates(listener: LocationEngineCallback<LocationEngineResult>) {
        listeners.remove(listener)
    }

    override fun removeLocationUpdates(pendingIntent: PendingIntent?) {}
}
