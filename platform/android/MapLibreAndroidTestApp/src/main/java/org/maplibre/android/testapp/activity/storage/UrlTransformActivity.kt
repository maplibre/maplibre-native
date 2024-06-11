package org.maplibre.android.testapp.activity.storage

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.storage.FileSource
import org.maplibre.android.storage.FileSource.ResourceTransformCallback
import org.maplibre.android.storage.Resource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
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
        mapView.getMapAsync { map: MapLibreMap ->
            Timber.i("Map loaded")
            map.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
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
