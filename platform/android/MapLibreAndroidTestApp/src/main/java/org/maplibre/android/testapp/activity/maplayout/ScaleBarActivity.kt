package org.maplibre.android.testapp.activity.maplayout

import android.os.Bundle
import android.view.Gravity
import android.view.Menu
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Activity demonstrating the ScaleBarView widget.
 *
 * The scale bar shows the current map scale and updates dynamically
 * as the user pans and zooms.
 */
class ScaleBarActivity : AppCompatActivity() {

    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var isMetric = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_simple)

        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            map.setStyle(Style.Builder().fromUri(TestStyles.OPENFREEMAP_LIBERTY)) {
                // Enable scale bar
                map.uiSettings.isScaleBarEnabled = true
                map.uiSettings.scaleBarGravity = Gravity.TOP or Gravity.START

                // Set initial camera position
                map.cameraPosition = CameraPosition.Builder()
                    .target(LatLng(40.7128, -74.0060)) // New York
                    .zoom(12.0)
                    .build()
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_scale_bar, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_toggle_units -> {
                isMetric = !isMetric
                maplibreMap.uiSettings.isScaleBarIsMetric = isMetric
                item.title = if (isMetric) "Switch to Imperial" else "Switch to Metric"
                true
            }
            R.id.action_position_top_start -> {
                maplibreMap.uiSettings.scaleBarGravity = Gravity.TOP or Gravity.START
                true
            }
            R.id.action_position_top_end -> {
                maplibreMap.uiSettings.scaleBarGravity = Gravity.TOP or Gravity.END
                true
            }
            R.id.action_position_bottom_start -> {
                maplibreMap.uiSettings.scaleBarGravity = Gravity.BOTTOM or Gravity.START
                true
            }
            R.id.action_position_bottom_end -> {
                maplibreMap.uiSettings.scaleBarGravity = Gravity.BOTTOM or Gravity.END
                true
            }
            else -> super.onOptionsItemSelected(item)
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

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }
}
