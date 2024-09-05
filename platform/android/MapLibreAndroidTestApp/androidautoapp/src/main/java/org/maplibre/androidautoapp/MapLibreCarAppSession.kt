package org.maplibre.androidautoapp

import android.content.Intent
import androidx.car.app.Screen
import androidx.car.app.Session
import org.maplibre.android.MapLibre

class MapLibreCarAppSession : Session() {
    override fun onCreateScreen(intent: Intent): Screen {
        MapLibre.getInstance(carContext)
        val carSurface = MapLibreAndroidAutoSurface(carContext, lifecycle)
        return MapLibreCarAppScreen(carContext, carSurface)
    }
}