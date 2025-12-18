package org.maplibre.android.testapp.activity.maplayout

import android.os.Bundle
import android.view.MenuItem
import android.widget.Button
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import com.example.GluecodiumPlugin.ArrowPolylineConfig
import com.example.GluecodiumPlugin.ArrowPolylineExample
import com.example.GluecodiumPlugin.LatLng as GlueLatLng
import org.maplibre.android.testapp.utils.ApiKeyUtils
import org.maplibre.android.testapp.utils.NavUtils

/**
 * Test activity showcasing the arrow polyline example plugin.
 * Tap points on the map to build an arrow polyline.
 */
class ArrowPolylineExampleActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var plugin: ArrowPolylineExample
    private val coordinates = mutableListOf<LatLng>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        onBackPressedDispatcher.addCallback(this, object: OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                NavUtils.navigateHome(this@ArrowPolylineExampleActivity)
            }
        })
        setContentView(R.layout.activity_arrow_polyline_example)

        plugin = ArrowPolylineExample()
        mapView = findViewById(R.id.mapView)
        mapView.addPluginByPtr(plugin.ptr)

        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            // Set camera to San Francisco at zoom 10
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(37.7749, -122.4194))
                .zoom(10.0)
                .build()

            val key = ApiKeyUtils.getApiKey(applicationContext)
            if (key == null || key == "YOUR_API_KEY_GOES_HERE") {
                map.setStyle(
                    Style.Builder().fromUri("https://demotiles.maplibre.org/style.json")
                )
            } else {
                val styles = Style.getPredefinedStyles()
                if (styles.isNotEmpty()) {
                    val styleUrl = styles[0].url
                    map.setStyle(Style.Builder().fromUri(styleUrl))
                }
            }

            // Add points on map click
            map.addOnMapClickListener { point ->
                coordinates.add(point)
                updateArrow()
                true
            }
        }

        findViewById<Button>(R.id.clearButton).setOnClickListener {
            coordinates.clear()
            plugin.removeArrowPolyline()
        }

        findViewById<Button>(R.id.demoButton).setOnClickListener {
            // Demo arrow around SF Bay Area
            coordinates.clear()
            coordinates.add(LatLng(37.8044, -122.2712)) // Oakland
            coordinates.add(LatLng(37.7955, -122.3937)) // Bay Bridge mid-point
            coordinates.add(LatLng(37.7749, -122.4194)) // Downtown SF
            coordinates.add(LatLng(37.7599, -122.4370)) // Twin Peaks area
            coordinates.add(LatLng(37.7339, -122.4467)) // Glen Park
            coordinates.add(LatLng(37.7127, -122.4530)) // SF border near Daly City
            updateArrow()
        }
    }

    private fun updateArrow() {
        if (coordinates.size >= 2) {
            val config = ArrowPolylineConfig().apply {
                headLength = 30.0  // 30 pixels for arrow head
                headAngle = 25.0
                lineColor = "#0066FF"
                lineWidth = 4.0
            }
            val glueCoords = coordinates.map { GlueLatLng(it.latitude, it.longitude) }
            plugin.addArrowPolyline(glueCoords, config)
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
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            android.R.id.home -> {
                onBackPressedDispatcher.onBackPressed()
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }
}
