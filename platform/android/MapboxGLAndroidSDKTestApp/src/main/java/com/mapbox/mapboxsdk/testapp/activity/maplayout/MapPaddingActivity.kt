package com.mapbox.mapboxsdk.testapp.activity.maplayout

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.annotations.MarkerOptions
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.*
import com.mapbox.mapboxsdk.testapp.R

/**
 * Test activity showcasing using the map padding API.
 */
class MapPaddingActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var mapboxMap: MapboxMap
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_padding)
        mapView = findViewById(R.id.mapView)
        mapView.setTag(true)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { mapboxMap: MapboxMap ->
                this@MapPaddingActivity.mapboxMap = mapboxMap
                mapboxMap.setStyle(Style.getPredefinedStyle("Streets"))
                val paddingLeft = resources.getDimension(R.dimen.map_padding_left).toInt()
                val paddingBottom = resources.getDimension(R.dimen.map_padding_bottom).toInt()
                val paddingRight = resources.getDimension(R.dimen.map_padding_right).toInt()
                val paddingTop = resources.getDimension(R.dimen.map_padding_top).toInt()
                mapboxMap.setPadding(paddingLeft, paddingTop, paddingRight, paddingBottom)
                val settings = mapboxMap.uiSettings
                settings.setLogoMargins(paddingLeft, 0, 0, paddingBottom)
                settings.setCompassMargins(0, paddingTop, paddingRight, 0)
                settings.isAttributionEnabled = false
                moveToBangalore()
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

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_padding, menu)
        return true
    }

    private fun moveToBangalore() {
        val bangalore = LatLng(12.9810816, 77.6368034)
        val cameraPosition = CameraPosition.Builder()
            .zoom(16.0)
            .target(bangalore)
            .bearing(40.0)
            .tilt(45.0)
            .build()
        mapboxMap.moveCamera(CameraUpdateFactory.newCameraPosition(cameraPosition))
        mapboxMap.addMarker(MarkerOptions().title("Center map").position(bangalore))
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_bangalore -> {
                if (mapboxMap != null) {
                    moveToBangalore()
                }
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
}
