package com.mapbox.mapboxsdk.testapp.activity.maplayout

import android.os.Bundle
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.maps.*
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.testapp.utils.ApiKeyUtils
import com.mapbox.mapboxsdk.testapp.utils.NavUtils

/**
 * Test activity showcasing a simple MapView without any MapboxMap interaction.
 */
class SimpleMapActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_simple)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { mapboxMap: MapboxMap ->
                var key = ApiKeyUtils.getApiKey(applicationContext)
                if (key == null || key == "YOUR_API_KEY_GOES_HERE") {
                    mapboxMap.setStyle(Style.Builder().fromUri("https://demotiles.maplibre.org/style.json"))
                } else {
                    val styles = Style.getPredefinedStyles()
                    if (styles != null && styles.size > 0) {
                        val styleUrl = styles[0].url
                        mapboxMap.setStyle(Style.Builder().fromUri(styleUrl))
                    }
                }
            }
        )
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
                // activity uses singleInstance for testing purposes
                // code below provides a default navigation when using the app
                onBackPressed()
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onBackPressed() {
        // activity uses singleInstance for testing purposes
        // code below provides a default navigation when using the app
        NavUtils.navigateHome(this)
    }
}
