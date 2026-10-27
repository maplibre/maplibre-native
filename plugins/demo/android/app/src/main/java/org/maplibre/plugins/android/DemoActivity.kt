package org.maplibre.plugins.android

import android.app.Activity
import android.os.Bundle
import android.widget.Toast
import org.maplibre.android.MapLibre
import org.maplibre.android.RenderingEngine
import org.maplibre.android.WellKnownTileServer
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style

/**
 * Shows the demo style of the plugin named by the `plugin` extra. `-e style <uri>` loads another style instead and
 * `-e renderer opengl|vulkan` picks the renderer of the multi-backend flavor.
 */
class DemoActivity : Activity() {
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

        val demo = intent.getStringExtra("plugin")?.let { Demo.find(it) }
        if (demo == null) {
            Toast.makeText(this, "Unknown plugin: ${intent.getStringExtra("plugin")}", Toast.LENGTH_LONG).show()
            finish()
            return
        }
        // Register before loading a style that uses the plugin's layer types.
        if (!demo.register()) {
            val message = "${demo.displayName} does not support the ${RenderingEngine.getCurrentType()} renderer"
            Toast.makeText(this, message, Toast.LENGTH_LONG).show()
        }

        mapView = MapView(this)
        setContentView(mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            if (savedInstanceState == null) {
                map.moveCamera(CameraUpdateFactory.newCameraPosition(demo.camera))
            }
            // Plugin layers are loaded from JSON; the SDK has no Java Layer peer for them yet.
            map.setStyle(Style.Builder().fromUri(intent.getStringExtra("style") ?: demo.style))
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
        if (::mapView.isInitialized) mapView.onDestroy()
        super.onDestroy()
    }
}
