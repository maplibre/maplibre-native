package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import androidx.core.content.res.ResourcesCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber
import java.net.URI
import java.net.URISyntaxException

/**
 * Test activity showcasing adding a Circle Layer to the Map
 *
 *
 * Uses bus stop data from Singapore as a source and allows to filter into 1 specific route with a line layer.
 *
 */
class CircleLayerActivity : AppCompatActivity(), View.OnClickListener {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var styleFab: FloatingActionButton
    private lateinit var routeFab: FloatingActionButton
    private var layer: CircleLayer? = null
    private var source: GeoJsonSource? = null
    private var currentStyleIndex = 0
    private var isLoadingStyle = true
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_circle_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map: MapLibreMap? ->
            if (map != null) {
                maplibreMap = map
            }
            maplibreMap.setStyle(TestStyles.PROTOMAPS_WHITE) {
                maplibreMap.getStyle { style ->
                    addBusStopSource(style)
                    addBusStopCircleLayer(style)
                    initFloatingActionButtons()
                    isLoadingStyle = false

                    style.addImage(
                        "bus-icon",
                        AppCompatResources.getDrawable(this, R.drawable.ic_directions_bus_black)!!
                    )
                }
            }
        }
    }

    private fun addBusStopSource(style: Style) {
        // --8<-- [start:addBusStopSource]
        try {
            source = GeoJsonSource(SOURCE_ID, URI(URL_BUS_ROUTES))
        } catch (exception: URISyntaxException) {
            Timber.e(exception, "That's not an url... ")
        }
        style.addSource(source!!)
        // --8<-- [end:addBusStopSource]
    }

    private fun addBusStopCircleLayer(style: Style) {
        // --8<-- [start:addBusStopCircleLayer]
        layer = CircleLayer(LAYER_ID, SOURCE_ID)
        layer!!.setProperties(
            PropertyFactory.circleColor(Color.parseColor("#FF9800")),
            PropertyFactory.circleRadius(2.0f)
        )
        style.addLayer(layer!!)
        // --8<-- [end:addBusStopCircleLayer]

    }

    private fun initFloatingActionButtons() {
        routeFab = findViewById(R.id.fab_route)
        routeFab.setColorFilter(ContextCompat.getColor(this, R.color.primary))
        routeFab.setOnClickListener(this)
        styleFab = findViewById(R.id.fab_style)
        styleFab.setOnClickListener(this)
    }

    override fun onClick(view: View) {
        if (isLoadingStyle) {
            return
        }
        if (view.id == R.id.fab_route) {
            showBusCluster()
        } else if (view.id == R.id.fab_style) {
            changeMapStyle()
        }
    }

    private fun showBusCluster() {
        removeFabs()
        removeOldSource()
        addClusteredSource()
    }

    private fun removeOldSource() {
        maplibreMap.style!!.removeSource(SOURCE_ID)
        maplibreMap.style!!.removeLayer(LAYER_ID)
    }

    private fun addClusteredSource() {
        maplibreMap.getStyle { style ->
            try {
                // --8<-- [start:addClusteredSource]
                style.addSource(
                    GeoJsonSource(
                        SOURCE_ID_CLUSTER,
                        URI(URL_BUS_ROUTES),
                        GeoJsonOptions()
                            .withCluster(true)
                            .withClusterMaxZoom(14)
                            .withClusterRadius(50)
                    )
                )
                // --8<-- [end:addClusteredSource]
            } catch (malformedUrlException: URISyntaxException) {
                Timber.e(malformedUrlException, "That's not an url... ")
            }

            // --8<-- [start:unclusteredLayer]
            val unclustered = SymbolLayer("unclustered-points", SOURCE_ID_CLUSTER)
            unclustered.setProperties(
                PropertyFactory.iconImage("bus-icon"),
            )
            unclustered.setFilter(
                Expression.neq(Expression.get("cluster"), true)
            )
            style.addLayer(unclustered)
            // --8<-- [end:unclusteredLayer]

            // --8<-- [start:clusteredCircleLayers]
            val layers = arrayOf(
                150 to ResourcesCompat.getColor(
                    resources,
                    R.color.redAccent,
                    theme
                ),
                20 to ResourcesCompat.getColor(resources, R.color.greenAccent, theme),
                0 to ResourcesCompat.getColor(
                    resources,
                    R.color.blueAccent,
                    theme
                )
            )
            // --8<-- [end:clusteredCircleLayers]

            // --8<-- [start:clusteredCircleLayersLoop]
            for (i in layers.indices) {
                // Add some nice circles
                val circles = CircleLayer("cluster-$i", SOURCE_ID_CLUSTER)
                circles.setProperties(
                    PropertyFactory.circleColor(layers[i].second),
                    PropertyFactory.circleRadius(18f)
                )

                val pointCount = Expression.toNumber(Expression.get("point_count"))
                circles.setFilter(
                    if (i == 0) {
                        Expression.all(
                            Expression.has("point_count"),
                            Expression.gte(
                                pointCount,
                                Expression.literal(layers[i].first)
                            )
                        )
                    } else {
                        Expression.all(
                            Expression.has("point_count"),
                            Expression.gt(
                                pointCount,
                                Expression.literal(layers[i].first)
                            ),
                            Expression.lt(
                                pointCount,
                                Expression.literal(layers[i - 1].first)
                            )
                        )
                    }
                )

                style.addLayer(circles)
            }
            // --8<-- [end:clusteredCircleLayersLoop]

            // --8<-- [start:countLabels]
            val count = SymbolLayer("count", SOURCE_ID_CLUSTER)
            count.setProperties(
                PropertyFactory.textField(Expression.toString(Expression.get("point_count"))),
                PropertyFactory.textFont(arrayOf("Noto Sans Medium")),
                PropertyFactory.textSize(12f),
                PropertyFactory.textColor(Color.WHITE),
                PropertyFactory.textIgnorePlacement(true),
                PropertyFactory.textAllowOverlap(true)
            )
            style.addLayer(count)
            // --8<-- [end:countLabels]
        }
    }

    private fun removeFabs() {
        routeFab.visibility = View.GONE
        styleFab.visibility = View.GONE
    }

    private fun changeMapStyle() {
        isLoadingStyle = true
        removeBusStop()
        loadNewStyle()
    }

    private fun removeBusStop() {
        maplibreMap.style!!.removeLayer(layer!!)
        maplibreMap.style!!.removeSource(source!!)
    }

    private fun loadNewStyle() {
        maplibreMap.setStyle(Style.Builder().fromUri(nextStyle))
    }

    private val nextStyle: String
        get() {
            currentStyleIndex++
            if (currentStyleIndex == Data.STYLES.size) {
                currentStyleIndex = 0
            }
            return Data.STYLES[currentStyleIndex]
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

    private object Data {
        val STYLES = arrayOf(
            TestStyles.PROTOMAPS_WHITE,
            TestStyles.PROTOMAPS_LIGHT
        )
    }

    companion object {
        const val SOURCE_ID = "bus_stop"
        const val SOURCE_ID_CLUSTER = "bus_stop_cluster"
        const val URL_BUS_ROUTES =
            "https://s3.eu-central-1.amazonaws.com/maplibre-native/android-documentation-resources/bus-stops.geojson"
        const val LAYER_ID = "stops_layer"
    }
}
