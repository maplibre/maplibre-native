package org.maplibre.android.testapp.activity.storage

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.ModuleProvider
import org.maplibre.android.ModuleProviderImpl
import org.maplibre.android.maps.MapView
import org.maplibre.android.MapLibre
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.storage.FileSource
import org.maplibre.android.storage.FileSource.ResourceTransformCallback
import org.maplibre.android.storage.Resource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.utils.ApiKeyUtils
import org.maplibre.android.testapp.utils.ExampleCustomModuleProviderImpl
import timber.log.Timber

/**
 * This example activity shows how to provide your own HTTP request implementation.
 */
class CustomHttpRequestImplActivity : AppCompatActivity() {
    private lateinit var mapView: MapView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_data_driven_style)

        // Set a custom module provider that provides our custom HTTPRequestImpl
        MapLibre.setModuleProvider(ExampleCustomModuleProviderImpl() as ModuleProvider)

        // Initialize map with a style
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { maplibreMap: MapLibreMap ->
                maplibreMap.setStyle(Style.Builder().fromUri("https://demotiles.maplibre.org/style.json"))
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

        // Example of how to reset the module provider
        MapLibre.setModuleProvider(ModuleProviderImpl())
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
