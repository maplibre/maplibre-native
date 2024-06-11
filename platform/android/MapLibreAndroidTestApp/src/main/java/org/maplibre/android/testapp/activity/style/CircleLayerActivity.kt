package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.content.res.ResourcesCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapView.OnDidFinishLoadingStyleListener
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
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
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap? ->
                if (map != null) {
                    maplibreMap = map
                }
                maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"))
                mapView.addOnDidFinishLoadingStyleListener(
                    OnDidFinishLoadingStyleListener {
                        val style = maplibreMap.style
                        addBusStopSource(style)
                        addBusStopCircleLayer(style)
                        initFloatingActionButtons()
                        isLoadingStyle = false
                    }
                )
            }
        )
    }

    private fun addBusStopSource(style: Style?) {
        try {
            source = GeoJsonSource(SOURCE_ID, URI(URL_BUS_ROUTES))
        } catch (exception: URISyntaxException) {
            Timber.e(exception, "That's not an url... ")
        }
        style!!.addSource(source!!)
    }

    private fun addBusStopCircleLayer(style: Style?) {
        layer = CircleLayer(LAYER_ID, SOURCE_ID)
        layer!!.setProperties(
            PropertyFactory.circleColor(Color.parseColor("#FF9800")),
            PropertyFactory.circleRadius(2.0f)
        )
        style!!.addLayerBelow(layer!!, "water_intermittent")
    }

    private fun initFloatingActionButtons() {
        routeFab = findViewById(R.id.fab_route)
        routeFab.setColorFilter(ContextCompat.getColor(this@CircleLayerActivity, R.color.primary))
        routeFab.setOnClickListener(this@CircleLayerActivity)
        styleFab = findViewById(R.id.fab_style)
        styleFab.setOnClickListener(this@CircleLayerActivity)
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
        try {
            maplibreMap.style!!.addSource(
                GeoJsonSource(
                    SOURCE_ID_CLUSTER,
                    URI(URL_BUS_ROUTES),
                    GeoJsonOptions()
                        .withCluster(true)
                        .withClusterMaxZoom(14)
                        .withClusterRadius(50)
                )
            )
        } catch (malformedUrlException: URISyntaxException) {
            Timber.e(malformedUrlException, "That's not an url... ")
        }

        // Add unclustered layer
        val layers = arrayOf(
            intArrayOf(
                150,
                ResourcesCompat.getColor(
                    resources,
                    R.color.redAccent,
                    theme
                )
            ),
            intArrayOf(20, ResourcesCompat.getColor(resources, R.color.greenAccent, theme)),
            intArrayOf(
                0,
                ResourcesCompat.getColor(
                    resources,
                    R.color.blueAccent,
                    theme
                )
            )
        )
        val unclustered = SymbolLayer("unclustered-points", SOURCE_ID_CLUSTER)
        unclustered.setProperties(
            PropertyFactory.iconImage("bus-15")
        )
        maplibreMap.style!!.addLayer(unclustered)
        for (i in layers.indices) {
            // Add some nice circles
            val circles = CircleLayer("cluster-$i", SOURCE_ID_CLUSTER)
            circles.setProperties(
                PropertyFactory.circleColor(layers[i][1]),
                PropertyFactory.circleRadius(18f)
            )
            val pointCount = Expression.toNumber(Expression.get("point_count"))
            circles.setFilter(
                if (i == 0) {
                    Expression.all(
                        Expression.has("point_count"),
                        Expression.gte(
                            pointCount,
                            Expression.literal(
                                layers[i][0]
                            )
                        )

                    )
                } else {
                    Expression.all(
                        Expression.has("point_count"),
                        Expression.gt(
                            pointCount,
                            Expression.literal(
                                layers[i][0]
                            )
                        ),
                        Expression.lt(
                            pointCount,
                            Expression.literal(
                                layers[i - 1][0]
                            )
                        )
                    )
                }
            )
            maplibreMap.style!!.addLayer(circles)
        }

        // Add the count labels
        val count = SymbolLayer("count", SOURCE_ID_CLUSTER)
        count.setProperties(
            PropertyFactory.textField(Expression.toString(Expression.get("point_count"))),
            PropertyFactory.textSize(12f),
            PropertyFactory.textColor(Color.WHITE),
            PropertyFactory.textIgnorePlacement(true),
            PropertyFactory.textAllowOverlap(true)
        )
        maplibreMap.style!!.addLayer(count)
    }

    private fun removeFabs() {
        routeFab!!.visibility = View.GONE
        styleFab!!.visibility = View.GONE
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

    private fun addBusStop() {
        maplibreMap.style!!.addLayer(layer!!)
        maplibreMap.style!!.addSource(source!!)
    }

    private val nextStyle: String
        private get() {
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
            TestStyles.getPredefinedStyleWithFallback("Streets"),
            TestStyles.getPredefinedStyleWithFallback("Outdoor"),
            TestStyles.getPredefinedStyleWithFallback("Bright"),
            TestStyles.getPredefinedStyleWithFallback("Pastel"),
            TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"),
            TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid")
        )
    }

    companion object {
        const val SOURCE_ID = "bus_stop"
        const val SOURCE_ID_CLUSTER = "bus_stop_cluster"
        const val URL_BUS_ROUTES =
            "https://raw.githubusercontent.com/cheeaun/busrouter-sg/master/data/2/bus-stops.geojson"
        const val LAYER_ID = "stops_layer"
    }
}
