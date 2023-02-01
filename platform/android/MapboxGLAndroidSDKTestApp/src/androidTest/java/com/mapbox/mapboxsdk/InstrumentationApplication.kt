package com.mapbox.mapboxsdk

import com.mapbox.mapboxsdk.testapp.MapLibreApplication

class InstrumentationApplication : MapLibreApplication() {
    fun initializeLeakCanary(): Boolean {
        // do not initialize leak canary during instrumentation tests
        return true
    }
}
