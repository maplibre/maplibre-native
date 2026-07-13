package org.maplibre.android.testapp.activity.surfaceview

import android.animation.ValueAnimator
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test resizing a [android.view.SurfaceView] backed map on the fly.
 */
class SurfaceViewResizeActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private var animatedValue: ValueAnimator? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_surfaceview_resize)
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
        mapView.getMapAsync { setupMap(it) }
    }

    private fun setupMap(maplibreMap: MapLibreMap) {
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
    }

    private fun setupFab() {
        val fabDebug = findViewById<FloatingActionButton>(R.id.fabResize)
        fabDebug.setOnClickListener { view: View? ->
            val parent = findViewById<View>(R.id.coordinator_layout)
            val params = mapView.layoutParams

            if (animatedValue != null) {
                animatedValue?.cancel()
                animatedValue = null

                params.width = parent.width
                params.height = parent.height
            }

            if (this::mapView.isInitialized) {
                params.width = if (parent.width == params.width) parent.width / 2 else parent.width
                params.height =
                    if (parent.height == params.height) parent.height / 2 else parent.height
            }

            mapView.requestLayout()
        }

        val fabAnimatedDebug = findViewById<FloatingActionButton>(R.id.fabAnimatedResize)
        fabAnimatedDebug.setOnClickListener { view: View? ->
            val parent = findViewById<View>(R.id.coordinator_layout)

            if (animatedValue != null) {
                animatedValue?.cancel()
                animatedValue = null

                mapView.layoutParams.width = parent.width
                mapView.layoutParams.height = parent.height
                mapView.requestLayout()
            } else if (this::mapView.isInitialized) {
                val minValue = parent.height / 2
                val maxValue = parent.height

                animatedValue = ValueAnimator.ofInt(maxValue, minValue)
                animatedValue?.apply {
                    duration = 1000
                    repeatCount = ValueAnimator.INFINITE
                    repeatMode = ValueAnimator.REVERSE

                    addUpdateListener { animator ->
                        val height = animator.animatedValue as Int
                        mapView.layoutParams.height = height
                        mapView.requestLayout()
                    }

                    start()
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

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
