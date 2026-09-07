package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.databinding.ActivityMapSimpleBinding

/**
 * Activity showcasing a style that renders the map as a globe.
 */
class GlobeActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMapSimpleBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMapSimpleBinding.inflate(layoutInflater)
        setContentView(binding.root)
        // # --8<-- [start:setup]
        binding.mapView.getMapAsync { map ->
            map.moveCamera(CameraUpdateFactory.newLatLngZoom(cameraTarget, cameraZoom))
            map.setStyle(Style.Builder().fromUri("asset://globe.json"))
        }
        // # --8<-- [end:setup]
    }

    override fun onStart() {
        super.onStart()
        binding.mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        binding.mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        binding.mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        binding.mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        binding.mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        binding.mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        binding.mapView.onSaveInstanceState(outState)
    }

    companion object {
        const val cameraZoom = 1.2
        val cameraTarget = LatLng(25.0, 10.0)
    }
}
