package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import com.google.gson.JsonObject
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.OnMapClickListener
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber

/**
 * Test activity showcasing changing the icon with a zoom function and adding selection state to a SymbolLayer.
 */
class ZoomFunctionSymbolLayerActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var source: GeoJsonSource? = null
    private var layer: SymbolLayer? = null
    private var isInitialPosition = true
    private var isSelected = false
    private var isShowingSymbolLayer = true
    private val mapClickListener = OnMapClickListener { point ->
        val screenPoint = maplibreMap.projection.toScreenLocation(point)
        val featureList = maplibreMap.queryRenderedFeatures(screenPoint, LAYER_ID)
        if (!featureList.isEmpty()) {
            val feature = featureList[0]
            val selectedNow = feature.getBooleanProperty(KEY_PROPERTY_SELECTED)
            isSelected = !selectedNow
            updateSource(maplibreMap.style)
        } else {
            Timber.e("No features found")
        }
        true
    }

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_zoom_symbol_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap ->
                maplibreMap = map
                map.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style ->
                    updateSource(style)
                    addLayer(style)
                    map.addOnMapClickListener(mapClickListener)
                }
            }
        )
    }

    private fun updateSource(style: Style?) {
        val featureCollection = createFeatureCollection()
        if (source != null) {
            source!!.setGeoJson(featureCollection)
        } else {
            source = GeoJsonSource(SOURCE_ID, featureCollection)
            style!!.addSource(source!!)
        }
    }

    private fun toggleSymbolLayerVisibility() {
        layer!!.setProperties(
            PropertyFactory.visibility(if (isShowingSymbolLayer) Property.NONE else Property.VISIBLE)
        )
        isShowingSymbolLayer = !isShowingSymbolLayer
    }

    private fun createFeatureCollection(): FeatureCollection {
        val point = if (isInitialPosition) {
            Point.fromLngLat(-74.01618140, 40.701745)
        } else {
            Point.fromLngLat(-73.988097, 40.749864)
        }
        val properties = JsonObject()
        properties.addProperty(KEY_PROPERTY_SELECTED, isSelected)
        val feature = Feature.fromGeometry(point, properties)
        return FeatureCollection.fromFeatures(arrayOf(feature))
    }

    private fun addLayer(style: Style) {
        layer = SymbolLayer(LAYER_ID, SOURCE_ID)
        layer!!.setProperties(
            PropertyFactory.iconImage(
                Expression.step(
                    Expression.zoom(),
                    Expression.literal(BUS_MAKI_ICON_ID),
                    Expression.stop(ZOOM_STOP_MAX_VALUE, CAFE_MAKI_ICON_ID)
                )
            ),
            PropertyFactory.iconSize(
                Expression.switchCase(
                    Expression.get(KEY_PROPERTY_SELECTED),
                    Expression.literal(3.0f),
                    Expression.literal(1.0f)
                )
            ),
            PropertyFactory.iconAllowOverlap(true)
        )
        style.addLayer(layer!!)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_symbols, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (maplibreMap != null) {
            if (item.itemId == R.id.menu_action_change_location) {
                isInitialPosition = !isInitialPosition
                updateSource(maplibreMap.style)
            } else if (item.itemId == R.id.menu_action_toggle_source) {
                toggleSymbolLayerVisibility()
            }
        }
        return super.onOptionsItemSelected(item)
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
        private const val LAYER_ID = "symbolLayer"
        private const val SOURCE_ID = "poiSource"
        private const val BUS_MAKI_ICON_ID = "bus"
        private const val CAFE_MAKI_ICON_ID = "cafe-11"
        private const val KEY_PROPERTY_SELECTED = "selected"
        private const val ZOOM_STOP_MAX_VALUE = 12.0f
    }
}
