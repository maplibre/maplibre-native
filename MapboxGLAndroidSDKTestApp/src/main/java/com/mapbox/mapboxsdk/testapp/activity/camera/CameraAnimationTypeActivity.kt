package com.mapbox.mapboxsdk.testapp.activity.camera

import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.MapboxMap.CancelableCallback
import com.mapbox.mapboxsdk.maps.MapboxMap.OnCameraIdleListener
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.testapp.R
import timber.log.Timber

/**
 * Test activity showcasing the Camera API and listen to camera updates by animating the camera
 * above London.
 *
 * Shows how to use animate, ease and move camera update factory methods.
 */
class CameraAnimationTypeActivity : AppCompatActivity(), OnMapReadyCallback {
    private val callback: CancelableCallback =
        object : CancelableCallback {
            override fun onCancel() {
                Timber.i("Duration onCancel Callback called.")
                Toast.makeText(
                    this@CameraAnimationTypeActivity.applicationContext,
                    "Ease onCancel Callback called.",
                    Toast.LENGTH_LONG
                )
                    .show()
            }

            override fun onFinish() {
                Timber.i("Duration onFinish Callback called.")
                Toast.makeText(
                    this@CameraAnimationTypeActivity.applicationContext,
                    "Ease onFinish Callback called.",
                    Toast.LENGTH_LONG
                )
                    .show()
            }
        }
    private lateinit var mapboxMap: MapboxMap
    private lateinit var mapView: MapView
    private var cameraState = false
    private val cameraIdleListener = OnCameraIdleListener {
        if (mapboxMap != null) {
            Timber.w(mapboxMap!!.cameraPosition.toString())
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_camera_animation_types)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapboxMap) {
        mapboxMap = map
        mapboxMap!!.setStyle(Style.Builder().fromUri(Style.getPredefinedStyle("Streets")))
        mapboxMap!!.uiSettings.isAttributionEnabled = false
        mapboxMap!!.uiSettings.isLogoEnabled = false
        mapboxMap!!.addOnCameraIdleListener(cameraIdleListener)

        // handle move button clicks
        val moveButton = findViewById<View>(R.id.cameraMoveButton)
        moveButton?.setOnClickListener { view: View? ->
            val cameraPosition =
                CameraPosition.Builder()
                    .target(nextLatLng)
                    .zoom(14.0)
                    .tilt(30.0)
                    .tilt(0.0)
                    .build()
            mapboxMap!!.moveCamera(CameraUpdateFactory.newCameraPosition(cameraPosition))
        }

        // handle ease button clicks
        val easeButton = findViewById<View>(R.id.cameraEaseButton)
        easeButton?.setOnClickListener { view: View? ->
            val cameraPosition =
                CameraPosition.Builder()
                    .target(nextLatLng)
                    .zoom(15.0)
                    .bearing(180.0)
                    .tilt(30.0)
                    .build()
            mapboxMap!!.easeCamera(
                CameraUpdateFactory.newCameraPosition(cameraPosition),
                7500,
                callback
            )
        }

        // handle animate button clicks
        val animateButton = findViewById<View>(R.id.cameraAnimateButton)
        animateButton?.setOnClickListener { view: View? ->
            val cameraPosition =
                CameraPosition.Builder().target(nextLatLng).bearing(270.0).tilt(20.0).build()
            mapboxMap!!.animateCamera(
                CameraUpdateFactory.newCameraPosition(cameraPosition),
                7500,
                callback
            )
        }
    }

    private val nextLatLng: LatLng
        private get() {
            cameraState = !cameraState
            return if (cameraState) LAT_LNG_TOWER_BRIDGE else LAT_LNG_LONDON_EYE
        }

    override fun onStart() {
        super.onStart()
        mapView!!.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView!!.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView!!.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView!!.onStop()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView!!.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        if (mapboxMap != null) {
            mapboxMap!!.removeOnCameraIdleListener(cameraIdleListener)
        }
        mapView!!.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }

    companion object {
        private val LAT_LNG_LONDON_EYE = LatLng(51.50325, -0.11968)
        private val LAT_LNG_TOWER_BRIDGE = LatLng(51.50550, -0.07520)
    }
}
