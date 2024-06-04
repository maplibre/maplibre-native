package org.maplibre.android.testapp.activity.maplayout

import android.graphics.Color
import android.os.Bundle
import android.view.*
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing restricting user gestures to a bounds around Iceland, almost worldview and IDL.
 */
class LatLngBoundsForCameraActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_restricted_bounds)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"))
        maplibreMap.setMinZoomPreference(2.0)
        maplibreMap.uiSettings.isFlingVelocityAnimationEnabled = false
        showCrosshair()
        setupBounds(ICELAND_BOUNDS)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_bounds, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.menu_action_allmost_world_bounds -> {
                setupBounds(ALMOST_WORLD_BOUNDS)
                return true
            }
            R.id.menu_action_cross_idl -> {
                setupBounds(CROSS_IDL_BOUNDS)
                return true
            }
            R.id.menu_action_reset -> {
                setupBounds(null)
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    private fun setupBounds(bounds: LatLngBounds?) {
        maplibreMap.setLatLngBoundsForCameraTarget(bounds)
        showBoundsArea(bounds)
    }

    private fun showBoundsArea(bounds: LatLngBounds?) {
        maplibreMap.clear()
        if (bounds != null) {
            val boundsArea = PolygonOptions()
                .add(bounds.northWest)
                .add(bounds.northEast)
                .add(bounds.southEast)
                .add(bounds.southWest)
            boundsArea.alpha(0.25f)
            boundsArea.fillColor(Color.RED)
            maplibreMap.addPolygon(boundsArea)
        }
    }

    private fun showCrosshair() {
        val crosshair = View(this)
        crosshair.layoutParams = FrameLayout.LayoutParams(10, 10, Gravity.CENTER)
        crosshair.setBackgroundColor(Color.BLUE)
        mapView.addView(crosshair)
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
        private val ICELAND_BOUNDS = LatLngBounds.Builder()
            .include(LatLng(66.852863, -25.985652))
            .include(LatLng(62.985661, -12.626277))
            .build()
        private val ALMOST_WORLD_BOUNDS = LatLngBounds.Builder()
            .include(LatLng(20.0, 170.0))
            .include(LatLng((-20).toDouble(), -170.0))
            .build()
        private val CROSS_IDL_BOUNDS = LatLngBounds.Builder()
            .include(LatLng(20.0, 170.0))
            .include(LatLng((-20).toDouble(), 190.0))
            .build()
    }
}
