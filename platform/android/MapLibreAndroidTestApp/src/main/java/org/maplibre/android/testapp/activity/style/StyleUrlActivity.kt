package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.AutoCompleteTextView
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Activity to demonstrate loading a style dynamically from a URL.
 */
class StyleUrlActivity : AppCompatActivity() {

    private lateinit var mapView: MapView
    private var mapLibreMap: MapLibreMap? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_style_url)

        val urlInput = findViewById<AutoCompleteTextView>(R.id.urlAutoCompleteTextView)
        val loadButton = findViewById<Button>(R.id.loadButton)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)

        val styles = arrayOf(
            TestStyles.DEMOTILES,
            TestStyles.AMERICANA,
            TestStyles.OPENFREEMAP_LIBERTY,
            TestStyles.OPENFREEMAP_BRIGHT,
            TestStyles.AWS_OPEN_DATA_STANDARD_LIGHT,
            TestStyles.PROTOMAPS_LIGHT,
            TestStyles.PROTOMAPS_DARK,
            TestStyles.PROTOMAPS_GRAYSCALE,
            TestStyles.PROTOMAPS_WHITE,
            TestStyles.PROTOMAPS_BLACK
        )
        val adapter = ArrayAdapter(this, android.R.layout.simple_dropdown_item_1line, styles)
        urlInput.setAdapter(adapter)

        urlInput.setOnClickListener {
            urlInput.showDropDown()
        }

        mapView.getMapAsync { map ->
            mapLibreMap = map
            map.setStyle(Style.Builder().fromUri(urlInput.text.toString()))
        }

        loadButton.setOnClickListener {
            val url = urlInput.text.toString()
            if (url.isNotEmpty()) {
                mapLibreMap?.setStyle(Style.Builder().fromUri(url)) {
                    Toast.makeText(this, "Style loaded", Toast.LENGTH_SHORT).show()
                }
            }
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
