package org.maplibre.android.testapp.activity.feature

import android.graphics.Color
import android.graphics.PointF
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.gson.JsonObject
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression.*
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.PropertyFactory.fillColor
import org.maplibre.android.style.layers.PropertyFactory.fillOpacity
import org.maplibre.android.style.layers.PropertyFactory.lineColor
import org.maplibre.android.style.layers.PropertyFactory.lineWidth
import org.maplibre.android.style.sources.VectorSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing interactive styling with feature state on a vector
 * tile source.
 *
 * This mirrors [FeatureStateActivity] (which uses a GeoJSON source) but shows the
 * two things that differ for a vector source:
 *
 * 1. Every feature-state call takes a source-layer id, because vector tiles are
 *    organized into named source layers. (A GeoJSON source has a single implicit
 *    layer, so its API omits it.)
 * 2. The target features must carry an id in the vector tile data — feature state
 *    is keyed by feature identifier. The MapLibre demo tiles used here assign one
 *    per country, so selection binds correctly.
 */
class FeatureStateVectorActivity : AppCompatActivity() {
    companion object {
        private const val COUNTRIES_SOURCE_ID = "countries-source"
        // The source layer that holds the country polygons in the demo tiles.
        private const val COUNTRIES_SOURCE_LAYER = "countries"
        private const val FILL_LAYER_ID = "country-fills"
        private const val LINE_LAYER_ID = "country-borders"
        private const val TILES_URL = "https://demotiles.maplibre.org/tiles/tiles.json"
    }

    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var countriesSource: VectorSource

    private val mapClickListener = MapLibreMap.OnMapClickListener { point ->
        handleMapClick(point)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_query_features_point)

        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            maplibreMap.moveCamera(
                CameraUpdateFactory.newLatLngZoom(LatLng(20.0, 0.0), 1.0)
            )
            maplibreMap.setStyle(TestStyles.DEMOTILES) { style ->
                addCountriesLayer(style)
                maplibreMap.addOnMapClickListener(mapClickListener)
            }
        }
    }

    private fun addCountriesLayer(style: Style) {
        // The MapLibre demo tiles are a world map of country polygons.
        val source = VectorSource(COUNTRIES_SOURCE_ID, TILES_URL)
        style.addSource(source)
        countriesSource = source

        val selectedFill = Color.parseColor("#d94d41")
        val selectedBorder = Color.parseColor("#8f241f")
        val defaultBorder = Color.parseColor("#888888")

        // Fill layer: transparent by default so the basemap shows through, and
        // highlighted only where the "selected" feature state is set.
        style.addLayer(
            FillLayer(FILL_LAYER_ID, COUNTRIES_SOURCE_ID)
                // A vector style layer must be told which source layer to draw.
                .withSourceLayer(COUNTRIES_SOURCE_LAYER)
                .withProperties(
                    fillColor(color(selectedFill)),
                    fillOpacity(
                        switchCase(
                            toBool(featureState("selected")),
                            literal(0.5f),
                            literal(0.0f)
                        )
                    )
                )
        )

        // Border layer: a thin border everywhere, thicker and red where selected.
        style.addLayer(
            LineLayer(LINE_LAYER_ID, COUNTRIES_SOURCE_ID)
                .withSourceLayer(COUNTRIES_SOURCE_LAYER)
                .withProperties(
                    lineColor(
                        switchCase(
                            toBool(featureState("selected")),
                            color(selectedBorder),
                            color(defaultBorder)
                        )
                    ),
                    lineWidth(
                        switchCase(
                            toBool(featureState("selected")),
                            literal(2.0f),
                            literal(0.5f)
                        )
                    )
                )
        )
    }

    private fun handleMapClick(point: LatLng): Boolean {
        val screenPoint: PointF = maplibreMap.projection.toScreenLocation(point)
        val features = maplibreMap.queryRenderedFeatures(screenPoint, FILL_LAYER_ID)
        val feature = features.firstOrNull() ?: return false
        val featureId = feature.id() ?: return false

        // Note the source-layer id argument, which is required for a vector source.
        val currentState = countriesSource.getFeatureState(COUNTRIES_SOURCE_LAYER, featureId)
        val isSelected = currentState
            ?.get("selected")
            ?.takeUnless { it.isJsonNull }
            ?.asBoolean
            ?: false

        // Toggle the selection state of the tapped country. Selection is not
        // exclusive: any number of countries can be selected at the same time.
        val nextState = JsonObject().apply { addProperty("selected", !isSelected) }
        countriesSource.setFeatureState(COUNTRIES_SOURCE_LAYER, featureId, nextState)

        val countryName = feature.getStringProperty("NAME") ?: "Unknown"
        Toast.makeText(
            this,
            "$countryName ${if (!isSelected) "selected" else "deselected"}",
            Toast.LENGTH_SHORT
        ).show()
        return true
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
        if (this::maplibreMap.isInitialized) {
            maplibreMap.removeOnMapClickListener(mapClickListener)
        }
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
