package org.maplibre.android.testapp.activity.textureview

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.coordinatorlayout.widget.CoordinatorLayout
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test resizing a [android.view.TextureView] backed map on the fly.
 */
class TextureViewResizeActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_textureview_resize)
        setupToolbar()
        setupMapView(savedInstanceState)
        setupFab()
    }

    private fun setupToolbar() {
        val actionBar = supportActionBar
        if (actionBar != null) {
            supportActionBar!!.setDisplayHomeAsUpEnabled(true)
            supportActionBar!!.setHomeButtonEnabled(true)
        }
    }

    private fun setupMapView(savedInstanceState: Bundle?) {
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(OnMapReadyCallback { maplibreMap: MapLibreMap -> setupMap(maplibreMap) })
    }

    private fun setupMap(maplibreMap: MapLibreMap) {
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
    }

    private fun setupFab() {
        val fabDebug = findViewById<FloatingActionButton>(R.id.fabResize)
        fabDebug.setOnClickListener { view: View? ->
            if (mapView != null) {
                val parent = findViewById<View>(R.id.coordinator_layout)
                val width = if (parent.width == mapView.width) parent.width / 2 else parent.width
                val height =
                    if (parent.height == mapView.height) parent.height / 2 else parent.height
                mapView.layoutParams = CoordinatorLayout.LayoutParams(width, height)
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

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
