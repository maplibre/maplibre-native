package org.maplibre.android.testapp.activity.annotation

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.content.res.ResourcesCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.IconUtils

/**
 * Test activity showcasing updating a Marker position, title, icon and snippet.
 */
class DynamicMarkerChangeActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var marker: Marker? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_dynamic_marker)
        mapView = findViewById(R.id.mapView)
        mapView.setTag(false)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { maplibreMap: MapLibreMap ->
                maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
                this@DynamicMarkerChangeActivity.maplibreMap = maplibreMap
                // Create marker
                val markerOptions = MarkerOptions()
                    .position(LAT_LNG_CHELSEA)
                    .icon(
                        IconUtils.drawableToIcon(
                            this@DynamicMarkerChangeActivity,
                            R.drawable.ic_stars,
                            ResourcesCompat.getColor(resources, R.color.blueAccent, theme)
                        )
                    )
                    .title(getString(R.string.dynamic_marker_chelsea_title))
                    .snippet(getString(R.string.dynamic_marker_chelsea_snippet))
                marker = maplibreMap.addMarker(markerOptions)
            }
        )
        val fab = findViewById<FloatingActionButton>(R.id.fab)
        fab.setColorFilter(ContextCompat.getColor(this, R.color.primary))
        fab.setOnClickListener { view: View? ->
            updateMarker()
        }
    }

    private fun updateMarker() {
        // update model
        val first = mapView.tag as Boolean
        mapView.tag = !first

        // update marker
        marker!!.position =
            if (first) LAT_LNG_CHELSEA else LAT_LNG_ARSENAL
        marker!!.icon = IconUtils.drawableToIcon(
            this,
            R.drawable.ic_stars,
            if (first) {
                ResourcesCompat.getColor(
                    resources,
                    R.color.blueAccent,
                    theme
                )
            } else {
                ResourcesCompat.getColor(
                    resources,
                    R.color.redAccent,
                    theme
                )
            }
        )
        marker!!.title =
            if (first) getString(R.string.dynamic_marker_chelsea_title) else getString(R.string.dynamic_marker_arsenal_title)
        marker!!.snippet =
            if (first) getString(R.string.dynamic_marker_chelsea_snippet) else getString(R.string.dynamic_marker_arsenal_snippet)
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

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    companion object {
        private val LAT_LNG_CHELSEA = LatLng(51.481670, -0.190849)
        private val LAT_LNG_ARSENAL = LatLng(51.555062, -0.108417)
    }
}
