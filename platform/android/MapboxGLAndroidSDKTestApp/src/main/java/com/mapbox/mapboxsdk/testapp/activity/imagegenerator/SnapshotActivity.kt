package com.mapbox.mapboxsdk.testapp.activity.imagegenerator

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.log.Logger
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.testapp.databinding.ActivitySnapshotBinding
import timber.log.Timber

/**
 * Test activity showcasing the Snapshot API to create and display a bitmap of the current shown Map.
 */
class SnapshotActivity : AppCompatActivity(), OnMapReadyCallback {

    private lateinit var binding: ActivitySnapshotBinding

    private lateinit var mapboxMap: MapboxMap

    private val idleListener = object : MapView.OnDidFinishRenderingFrameListener {
        override fun onDidFinishRenderingFrame(fully: Boolean) {
            if (fully) {
                binding.mapView.removeOnDidFinishRenderingFrameListener(this)
                Logger.v(TAG, LOG_MESSAGE)
                mapboxMap.snapshot { snapshot ->
                    binding.imageView.setImageBitmap(snapshot)
                    binding.mapView.addOnDidFinishRenderingFrameListener(this)
                }
            }
        }
    }

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySnapshotBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.mapView.onCreate(savedInstanceState)
        binding.mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapboxMap) {
        mapboxMap = map
        mapboxMap.setStyle(Style.Builder().fromUri(Style.getPredefinedStyle("Outdoor"))) { binding.mapView.addOnDidFinishRenderingFrameListener(idleListener) }
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
        mapboxMap.snapshot {
            Timber.e("Regression test for https://github.com/mapbox/mapbox-gl-native/pull/11358")
        }
        binding.mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        binding.mapView.onStop()
    }

    public override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        binding.mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        binding.mapView.onLowMemory()
    }

    public override fun onDestroy() {
        super.onDestroy()
        binding.mapView.removeOnDidFinishRenderingFrameListener(idleListener)
        binding.mapView.onDestroy()
    }

    companion object {
        const val TAG = "Mbgl-SnapshotActivity"
        const val LOG_MESSAGE = "OnSnapshot"
    }
}
