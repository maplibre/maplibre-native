package com.mapbox.mapboxsdk.location.engine

import android.content.Context

object LocationEngineDefault {
    /**
     * Returns the default `LocationEngine`.
     */
    fun getDefaultLocationEngine(context: Context): LocationEngine {
        return LocationEngineProxy(MapboxFusedLocationEngineImpl(context.applicationContext))
    }
}
