package org.maplibre.android

import org.maplibre.android.testapp.MapLibreApplication

class InstrumentationApplication : MapLibreApplication() {
    fun initializeLeakCanary(): Boolean {
        // do not initialize leak canary during instrumentation tests
        return true
    }
}
