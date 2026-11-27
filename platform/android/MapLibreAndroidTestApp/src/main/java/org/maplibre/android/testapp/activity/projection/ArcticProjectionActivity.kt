package org.maplibre.android.testapp.activity.projection

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.projection.ArcticProjection

/**
 * Test activity showing NASA GIBS's sea ice map of the Arctic in EPSG:3413, a projection registered from Kotlin.
 */
class ArcticProjectionActivity : AppCompatActivity() {
    private lateinit var mapView: MapView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // --8<-- [start:addProjection]
        Style.addProjection(ArcticProjection.coordinateReferenceSystem)
        // --8<-- [end:addProjection]
        setContentView(R.layout.activity_map_simple)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        // --8<-- [start:setStyle]
        mapView.getMapAsync { map ->
            map.setStyle(Style.Builder().fromUri("asset://arctic.json")) {
                map.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(90.0, -45.0), 1.0))
            }
        }
        // --8<-- [end:setStyle]
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
        Style.removeProjection(ArcticProjection.NAME)
    }
}
