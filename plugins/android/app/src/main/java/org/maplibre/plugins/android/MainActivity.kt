package org.maplibre.plugins.android

import android.app.Activity
import android.os.Bundle
import org.maplibre.android.MapLibre
import org.maplibre.android.RenderingEngine
import org.maplibre.android.WellKnownTileServer
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.plugins.ngon.NgonLayer

class MainActivity : Activity() {
    private lateinit var mapView: MapView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val requestedRenderer = if (BuildConfig.MULTI_BACKEND) {
            when (intent.getStringExtra("renderer")) {
                "opengl" -> RenderingEngine.Type.OPENGL
                "vulkan" -> RenderingEngine.Type.VULKAN
                else -> null
            }
        } else {
            null
        }
        MapLibre.getInstance(this, null, WellKnownTileServer.MapLibre, requestedRenderer)
        NgonLayer.register()

        mapView = MapView(this)
        setContentView(mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            if (savedInstanceState == null) {
                map.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 11.3))
            }
            // Plugin layers are loaded from JSON; the SDK has no Java Layer peer for them yet.
            map.setStyle(Style.Builder().fromUri("asset://ngon.json"))
        }
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        mapView.onPause()
        super.onPause()
    }

    override fun onStop() {
        mapView.onStop()
        super.onStop()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        mapView.onDestroy()
        super.onDestroy()
    }
}
