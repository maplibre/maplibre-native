package org.maplibre.android.testapp.activity.feature

import android.graphics.Color
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
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.geojson.Polygon

/**
 * Test activity showcasing interactive styling with feature state.
 */
class FeatureStateActivity : AppCompatActivity() {
    companion object {
        private const val SOURCE_ID = "feature-state-source"
        private const val FILL_LAYER_ID = "feature-state-fill-layer"
        private const val LINE_LAYER_ID = "feature-state-line-layer"
        private const val PROPERTY_NAME = "name"
        private const val STATE_SELECTED = "selected"
    }

    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap

    private val mapClickListener = MapLibreMap.OnMapClickListener { point ->
        toggleFeatureSelection(point)
    }

    // # --8<-- [start:onCreate]
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_query_features_point)

        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(39.5, -96.5), 2.6))
            maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style ->
                addFeatureStateLayers(style)
                maplibreMap.addOnMapClickListener(mapClickListener)
            }
        }
    }
    // # --8<-- [end:onCreate]

    // # --8<-- [start:addFeatureStateLayers]
    private fun addFeatureStateLayers(style: Style) {
        style.addSource(GeoJsonSource(SOURCE_ID, createRegions()))

        val selectedFill = Color.parseColor("#d94d41")
        val defaultFill = Color.parseColor("#2f7de1")
        val selectedLine = Color.parseColor("#8f241f")
        val defaultLine = Color.parseColor("#194a80")

        style.addLayer(
            FillLayer(FILL_LAYER_ID, SOURCE_ID).withProperties(
                fillColor(
                    switchCase(
                        toBool(featureState(STATE_SELECTED)),
                        color(selectedFill),
                        color(defaultFill)
                    )
                ),
                fillOpacity(
                    switchCase(
                        toBool(featureState(STATE_SELECTED)),
                        literal(0.8f),
                        literal(0.45f)
                    )
                )
            )
        )

        style.addLayer(
            LineLayer(LINE_LAYER_ID, SOURCE_ID).withProperties(
                lineColor(
                    switchCase(
                        toBool(featureState(STATE_SELECTED)),
                        color(selectedLine),
                        color(defaultLine)
                    )
                ),
                lineWidth(
                    switchCase(
                        toBool(featureState(STATE_SELECTED)),
                        literal(3.0f),
                        literal(1.25f)
                    )
                )
            )
        )
    }
    // # --8<-- [end:addFeatureStateLayers]

    // # --8<-- [start:toggleFeatureSelection]
    private fun toggleFeatureSelection(point: LatLng): Boolean {
        val screenCoordinate = maplibreMap.projection.toScreenLocation(point)
        val feature = maplibreMap.queryRenderedFeatures(screenCoordinate, FILL_LAYER_ID).firstOrNull() ?: return false
        val featureId = feature.id() ?: return false

        val currentState = maplibreMap.getFeatureState(SOURCE_ID, null, featureId)
        val isSelected = currentState
            ?.get(STATE_SELECTED)
            ?.takeUnless { it.isJsonNull }
            ?.asBoolean
            ?: false

        val nextState = JsonObject().apply { addProperty(STATE_SELECTED, !isSelected) }
        maplibreMap.setFeatureState(SOURCE_ID, null, featureId, nextState)

        val featureName = feature.properties()
            ?.get(PROPERTY_NAME)
            ?.takeUnless { it.isJsonNull }
            ?.asString
            ?: "Region"
        Toast.makeText(
            this,
            "$featureName ${if (!isSelected) "selected" else "deselected"}",
            Toast.LENGTH_SHORT
        ).show()
        return true
    }
    // # --8<-- [end:toggleFeatureSelection]

    // # --8<-- [start:createRegions]
    private fun createRegions(): FeatureCollection {
        return FeatureCollection.fromFeatures(
            arrayOf(
                createRegion("west", "West", -124.5, 32.5, -109.0, 49.0),
                createRegion("central", "Central", -109.0, 31.0, -94.0, 49.0),
                createRegion("east", "East", -94.0, 28.0, -67.0, 47.5)
            )
        )
    }
    // # --8<-- [end:createRegions]

    private fun createRegion(
        id: String,
        name: String,
        minLongitude: Double,
        minLatitude: Double,
        maxLongitude: Double,
        maxLatitude: Double
    ): Feature {
        val ring = listOf(
            Point.fromLngLat(minLongitude, minLatitude),
            Point.fromLngLat(maxLongitude, minLatitude),
            Point.fromLngLat(maxLongitude, maxLatitude),
            Point.fromLngLat(minLongitude, maxLatitude),
            Point.fromLngLat(minLongitude, minLatitude)
        )
        val properties = JsonObject().apply { addProperty(PROPERTY_NAME, name) }
        return Feature.fromGeometry(Polygon.fromLngLats(listOf(ring)), properties, id)
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
