package com.mapbox.mapboxsdk.testapp.activity.fragment

import android.os.Bundle // ktlint-disable import-ordering
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.* // ktlint-disable no-wildcard-imports
import com.mapbox.mapboxsdk.maps.MapFragment.OnMapViewReadyCallback
import com.mapbox.mapboxsdk.maps.MapView.OnDidFinishRenderingFrameListener
import com.mapbox.mapboxsdk.testapp.R

/**
 * Test activity showcasing using the MapFragment API using Support Library Fragments.
 *
 *
 * Uses MapboxMapOptions to initialise the Fragment.
 *
 */
class SupportMapFragmentActivity :
    AppCompatActivity(),
    OnMapViewReadyCallback,
    OnMapReadyCallback,
    OnDidFinishRenderingFrameListener {
    private var mapboxMap: MapboxMap? = null
    private var mapView: MapView? = null
    private var initialCameraAnimation = true
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_fragment)
        val mapFragment: SupportMapFragment?
        if (savedInstanceState == null) {
            mapFragment = SupportMapFragment.newInstance(createFragmentOptions())
            supportFragmentManager
                .beginTransaction()
                .add(R.id.fragment_container, mapFragment, TAG)
                .commit()
        } else {
            mapFragment = supportFragmentManager.findFragmentByTag(TAG) as SupportMapFragment?
        }
        mapFragment!!.getMapAsync(this)
    }

    private fun createFragmentOptions(): MapboxMapOptions {
        val options = MapboxMapOptions.createFromAttributes(this, null)
        options.scrollGesturesEnabled(false)
        options.zoomGesturesEnabled(false)
        options.tiltGesturesEnabled(false)
        options.rotateGesturesEnabled(false)
        options.debugActive(false)
        val dc = LatLng(38.90252, -77.02291)
        options.minZoomPreference(9.0)
        options.maxZoomPreference(11.0)
        options.camera(
            CameraPosition.Builder()
                .target(dc)
                .zoom(11.0)
                .build()
        )
        return options
    }

    override fun onMapViewReady(map: MapView) {
        mapView = map
        mapView!!.addOnDidFinishRenderingFrameListener(this)
    }

    override fun onMapReady(map: MapboxMap) {
        mapboxMap = map
        mapboxMap!!.setStyle(Style.getPredefinedStyle("Satellite Hybrid"))
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView!!.removeOnDidFinishRenderingFrameListener(this)
    }

    override fun onDidFinishRenderingFrame(fully: Boolean) {
        if (initialCameraAnimation && fully && mapboxMap != null) {
            mapboxMap!!.animateCamera(
                CameraUpdateFactory.newCameraPosition(CameraPosition.Builder().tilt(45.0).build()),
                5000
            )
            initialCameraAnimation = false
        }
    }

    companion object {
        private const val TAG = "com.mapbox.map"
    }
}
