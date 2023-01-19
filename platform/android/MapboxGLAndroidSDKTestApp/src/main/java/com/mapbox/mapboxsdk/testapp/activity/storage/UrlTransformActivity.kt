package com.mapbox.mapboxsdk.testapp.activity.storage

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.storage.FileSource
import com.mapbox.mapboxsdk.storage.FileSource.ResourceTransformCallback
import com.mapbox.mapboxsdk.storage.Resource
import com.mapbox.mapboxsdk.testapp.R
import timber.log.Timber

/**
 * Test activity showcasing the url transform
 */
class UrlTransformActivity : AppCompatActivity() {
    private lateinit var mapView: MapView

    /**
     * Be sure to use an isolated class so the activity is not leaked when
     * the activity goes out of scope (static class in this case).
     *
     *
     * Alternatively, unregister the callback in [Activity.onDestroy]
     */
    private class Transform : ResourceTransformCallback {
        override fun onURL(@Resource.Kind kind: Int, url: String): String {
            Timber.i("[%s] Could be rewriting %s", Thread.currentThread().name, url)
            return url
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_data_driven_style)

        // Initialize map as normal
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)

        // Get a handle to the file source and set the resource transform
        FileSource.getInstance(this@UrlTransformActivity).setResourceTransform(Transform())
        mapView.getMapAsync { map: MapboxMap ->
            Timber.i("Map loaded")
            map.setStyle(Style.getPredefinedStyle("Streets"))
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

        // Example of how to reset the transform callback
        FileSource.getInstance(this@UrlTransformActivity).setResourceTransform(null)
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
