package org.maplibre.android.testapp.activity.maplayout

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing visibility changes to the mapview.
 */
class VisibilityChangeActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private val handler = Handler(Looper.getMainLooper())
    private var runnable: Runnable? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_visibility)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap? ->
                if (map != null) {
                    maplibreMap = map
                }
                maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
                maplibreMap.animateCamera(
                    CameraUpdateFactory.newLatLngZoom(
                        LatLng(55.754020, 37.620948),
                        12.0
                    ),
                    9000
                )
            }
        )
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
        handler.post(
            VisibilityRunner(
                mapView,
                findViewById(R.id.viewParent),
                handler
            ).also { runnable = it }
        )
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    private class VisibilityRunner internal constructor(
        private val mapView: MapView?,
        private val viewParent: View?,
        private val handler: Handler
    ) : Runnable {
        private var currentStep = 0
        override fun run() {
            if (isViewHiearchyReady) {
                if (isEvenStep) {
                    viewParent!!.visibility = View.VISIBLE
                    mapView?.visibility = View.VISIBLE
                } else if (isFirstOrThirdStep) {
                    mapView?.visibility = visibilityForStep
                } else if (isFifthOrSeventhStep) {
                    viewParent!!.visibility = visibilityForStep
                }
                updateStep()
            }
            handler.postDelayed(this, 1500)
        }

        private fun updateStep() {
            if (currentStep == 7) {
                currentStep = 0
            } else {
                currentStep++
            }
        }

        private val visibilityForStep: Int
            get() = if (currentStep == 1 || currentStep == 5) View.GONE else View.INVISIBLE
        private val isFifthOrSeventhStep: Boolean
            get() = currentStep == 5 || currentStep == 7
        private val isFirstOrThirdStep: Boolean
            get() = currentStep == 1 || currentStep == 3
        private val isEvenStep: Boolean
            get() = currentStep == 0 || currentStep % 2 == 0
        private val isViewHiearchyReady: Boolean
            get() = mapView != null && viewParent != null
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        runnable?.let {
            handler.removeCallbacks(it)
            runnable = null
        }
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
}
