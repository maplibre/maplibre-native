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
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import java.net.URI

/**
 * Test activity showcasing interactive styling with feature state,
 * matching the iOS feature-state example.
 *
 * Loads US states from a remote GeoJSON source and uses feature-state
 * expressions to highlight tapped states.
 */
class FeatureStateActivity : AppCompatActivity() {
    companion object {
        private const val STATES_SOURCE_ID = "states"
        private const val FILL_LAYER_ID = "state-fills"
        private const val LINE_LAYER_ID = "state-borders"
        private const val STATES_URL =
            "https://maplibre.org/maplibre-gl-js/docs/assets/us_states.geojson"
    }

    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap

    private val mapClickListener = MapLibreMap.OnMapClickListener { point ->
        handleMapClick(point)
    }

    // # --8<-- [start:onCreate]
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_query_features_point)

        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            maplibreMap.moveCamera(
                CameraUpdateFactory.newLatLngZoom(LatLng(42.619626, -103.523181), 3.0)
            )
            maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style ->
                addStatesLayer(style)
                maplibreMap.addOnMapClickListener(mapClickListener)
            }
        }
    }
    // # --8<-- [end:onCreate]

    // # --8<-- [start:addStatesLayer]
    private fun addStatesLayer(style: Style) {
        style.addSource(GeoJsonSource(STATES_SOURCE_ID, URI(STATES_URL)))

        val selectedFill = Color.parseColor("#d94d41")
        val defaultFill = Color.parseColor("#2f7de1")
        val selectedBorder = Color.parseColor("#8f241f")
        val defaultBorder = Color.parseColor("#2f7de1")

        style.addLayer(
            FillLayer(FILL_LAYER_ID, STATES_SOURCE_ID).withProperties(
                fillColor(
                    switchCase(
                        toBool(featureState("selected")),
                        color(selectedFill),
                        color(defaultFill)
                    )
                ),
                fillOpacity(
                    switchCase(
                        toBool(featureState("selected")),
                        literal(0.7f),
                        literal(0.5f)
                    )
                )
            )
        )

        style.addLayer(
            LineLayer(LINE_LAYER_ID, STATES_SOURCE_ID).withProperties(
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
                        literal(1.0f)
                    )
                )
            )
        )
    }
    // # --8<-- [end:addStatesLayer]

    // # --8<-- [start:handleMapClick]
    private fun handleMapClick(point: LatLng): Boolean {
        val screenPoint: PointF = maplibreMap.projection.toScreenLocation(point)
        val features = maplibreMap.queryRenderedFeatures(screenPoint, FILL_LAYER_ID)
        val feature = features.firstOrNull() ?: return false
        val featureId = feature.id() ?: return false

        val currentState = maplibreMap.getFeatureState(STATES_SOURCE_ID, null, featureId)
        val isSelected = currentState
            ?.get("selected")
            ?.takeUnless { it.isJsonNull }
            ?.asBoolean
            ?: false

        val nextState = JsonObject().apply { addProperty("selected", !isSelected) }
        maplibreMap.setFeatureState(STATES_SOURCE_ID, null, featureId, nextState)

        val stateName = feature.getStringProperty("STATE_NAME") ?: "Unknown"
        Toast.makeText(
            this,
            "$stateName ${if (!isSelected) "selected" else "deselected"}",
            Toast.LENGTH_SHORT
        ).show()
        return true
    }
    // # --8<-- [end:handleMapClick]

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
