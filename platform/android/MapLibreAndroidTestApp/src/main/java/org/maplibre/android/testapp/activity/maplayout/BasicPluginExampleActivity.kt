package org.maplibre.android.testapp.activity.maplayout

import android.os.Bundle
import android.view.MenuItem
import android.widget.Button
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.plugins.BasicPluginExample
import org.maplibre.android.testapp.utils.ApiKeyUtils
import org.maplibre.android.testapp.utils.NavUtils

/**
 * Test activity showcasing the basic cross-platform plugin example.
 */
class BasicPluginExampleActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var plugin: BasicPluginExample

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        onBackPressedDispatcher.addCallback(this, object: OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                NavUtils.navigateHome(this@BasicPluginExampleActivity)
            }
        })
        setContentView(R.layout.activity_basic_plugin_example)

        plugin = BasicPluginExample()
        mapView = findViewById(R.id.mapView)
        mapView.addPluginBridge(plugin)

        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync {
            val key = ApiKeyUtils.getApiKey(applicationContext)
            if (key == null || key == "YOUR_API_KEY_GOES_HERE") {
                it.setStyle(
                    Style.Builder().fromUri("https://demotiles.maplibre.org/style.json")
                )
            } else {
                val styles = Style.getPredefinedStyles()
                if (styles.isNotEmpty()) {
                    val styleUrl = styles[0].url
                    it.setStyle(Style.Builder().fromUri(styleUrl))
                }
            }
        }

        findViewById<Button>(R.id.showSanFranciscoButton).setOnClickListener {
            plugin.showSanFrancisco()
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
