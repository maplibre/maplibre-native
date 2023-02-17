package com.mapbox.mapboxsdk.testapp.activity.style

import android.os.Bundle
import android.os.Handler
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.maps.*
import com.mapbox.mapboxsdk.style.layers.*
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.R
import timber.log.Timber
import java.net.URI
import java.net.URISyntaxException

/**
 * Test activity showcasing using realtime GeoJSON to move a symbol on your map
 *
 *
 * GL-native equivalent of https://maplibre.org/maplibre-gl-js-docs/example/live-geojson/
 *
 */
class RealTimeGeoJsonActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapView: MapView
    private lateinit var mapboxMap: MapboxMap
    private var handler: Handler? = null
    private var runnable: Runnable? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_default)
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapboxMap) {
        mapboxMap = map
        mapboxMap.setStyle(Style.getPredefinedStyle("Streets")) { style -> // add source
            try {
                style.addSource(GeoJsonSource(ID_GEOJSON_SOURCE, URI(URL_GEOJSON_SOURCE)))
            } catch (malformedUriException: URISyntaxException) {
                Timber.e(malformedUriException, "Invalid URL")
            }

            // add layer
            val layer = SymbolLayer(ID_GEOJSON_LAYER, ID_GEOJSON_SOURCE)
            layer.setProperties(PropertyFactory.iconImage("rocket-15"))
            style.addLayer(layer)

            // loop refresh geojson
            handler = Handler()
            runnable = RefreshGeoJsonRunnable(mapboxMap!!, handler!!)
            runnable?.let {
                handler!!.postDelayed(it, 2000)
            }
        }
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    public override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    public override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
        runnable?.let {
            handler!!.removeCallbacks(it)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    private class RefreshGeoJsonRunnable internal constructor(
        private val mapboxMap: MapboxMap,
        private val handler: Handler
    ) : Runnable {
        override fun run() {
            val geoJsonSource = mapboxMap.style!!.getSource(ID_GEOJSON_SOURCE) as GeoJsonSource
            geoJsonSource.url = URL_GEOJSON_SOURCE
            handler.postDelayed(this, 2000)
        }
    }

    companion object {
        private const val ID_GEOJSON_LAYER = "wanderdrone"
        private const val ID_GEOJSON_SOURCE = ID_GEOJSON_LAYER
        private const val URL_GEOJSON_SOURCE = "https://wanderdrone.appspot.com/"
    }
}
