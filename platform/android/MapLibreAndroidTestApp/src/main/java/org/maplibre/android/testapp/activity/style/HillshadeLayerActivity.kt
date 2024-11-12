package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.*
import org.maplibre.android.style.layers.HillshadeLayer
import org.maplibre.android.style.sources.RasterDemSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing using HillshadeLayer.
 */
class HillshadeLayerActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap? ->
                if (map != null) {
                    maplibreMap = map
                }
                val rasterDemSource = RasterDemSource(SOURCE_ID, SOURCE_URL)
                val hillshadeLayer = HillshadeLayer(LAYER_ID, SOURCE_ID)
                maplibreMap.setStyle(
                    Style.Builder()
                        .fromUri(TestStyles.getPredefinedStyleWithFallback("Streets"))
                        .withLayerBelow(hillshadeLayer, LAYER_BELOW_ID)
                        .withSource(rasterDemSource)
                )
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

    public override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    public override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    companion object {
        private const val LAYER_ID = "hillshade"
        private const val LAYER_BELOW_ID = "water_intermittent"
        private const val SOURCE_ID = "terrain-rgb"
        private const val SOURCE_URL = "maptiler://sources/terrain-rgb"
    }
}
