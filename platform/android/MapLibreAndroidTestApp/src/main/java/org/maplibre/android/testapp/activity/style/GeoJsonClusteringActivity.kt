package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.graphics.Point
import android.os.Bundle
import android.view.MenuItem
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import org.maplibre.geojson.Feature
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
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
import org.maplibre.android.utils.BitmapUtils
import timber.log.Timber
import java.net.URI
import java.net.URISyntaxException
import java.util.Objects

/**
 * Test activity showcasing using a geojson source and visualise that source as a cluster by using filters.
 */
class GeoJsonClusteringActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var clusterSource: GeoJsonSource? = null
    private var clickOptionCounter = 0
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_geojson_clustering)

        // Initialize map as normal
        mapView = findViewById(R.id.mapView)
        // noinspection ConstantConditions
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap? ->
                if (map != null) {
                    maplibreMap = map
                }
                maplibreMap.animateCamera(
                    CameraUpdateFactory.newLatLngZoom(
                        LatLng(37.7749, 122.4194),
                        0.0
                    )
                )
                val clusterLayers = arrayOf(
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
                try {
                    maplibreMap.setStyle(
                        Style.Builder()
                            .fromUri(TestStyles.getPredefinedStyleWithFallback("Bright"))
                            .withSource(createClusterSource().also { clusterSource = it })
                            .withLayer(createSymbolLayer())
                            .withLayer(createClusterLevelLayer(0, clusterLayers))
                            .withLayer(createClusterLevelLayer(1, clusterLayers))
                            .withLayer(createClusterLevelLayer(2, clusterLayers))
                            .withLayer(createClusterTextLayer())
                            .withImage(
                                "icon-id",
                                Objects.requireNonNull(
                                    BitmapUtils.getBitmapFromDrawable(ResourcesCompat.getDrawable(resources, R.drawable.ic_hearing_black_24dp, null))
                                )!!,
                                true
                            )
                    )
                } catch (exception: URISyntaxException) {
                    Timber.e(exception)
                }
                maplibreMap.addOnMapClickListener { latLng: LatLng? ->
                    val point = maplibreMap.projection.toScreenLocation(latLng!!)
                    val features =
                        maplibreMap.queryRenderedFeatures(point, "cluster-0", "cluster-1", "cluster-2")
                    if (!features.isEmpty()) {
                        onClusterClick(features[0], Point(point.x.toInt(), point.y.toInt()))
                    }
                    true
                }
            }
        )
        findViewById<View>(R.id.fab).setOnClickListener { v: View? ->
            updateClickOptionCounter()
            notifyClickOptionUpdate()
        }
    }

    private fun onClusterClick(cluster: Feature, clickPoint: Point) {
        if (clickOptionCounter == 0) {
            val nextZoomLevel = clusterSource!!.getClusterExpansionZoom(cluster).toDouble()
            val zoomDelta = nextZoomLevel - maplibreMap.cameraPosition.zoom
            maplibreMap.animateCamera(
                CameraUpdateFactory.zoomBy(
                    zoomDelta + CAMERA_ZOOM_DELTA,
                    clickPoint
                )
            )
            Toast.makeText(this, "Zooming to $nextZoomLevel", Toast.LENGTH_SHORT).show()
        } else if (clickOptionCounter == 1) {
            val collection = clusterSource!!.getClusterChildren(cluster)
            Toast.makeText(this, "Children: " + collection.toJson(), Toast.LENGTH_SHORT).show()
        } else {
            val collection = clusterSource!!.getClusterLeaves(cluster, 2, 1)
            Toast.makeText(this, "Leaves: " + collection.toJson(), Toast.LENGTH_SHORT).show()
        }
    }

    private fun createClusterSource(): GeoJsonSource {
        return GeoJsonSource(
            "earthquakes",
            URI("asset://earthquakes.geojson"),
            GeoJsonOptions()
                .withCluster(true)
                .withClusterMaxZoom(14)
                .withClusterRadius(50)
                .withClusterProperty(
                    "max",
                    Expression.max(Expression.accumulated(), Expression.get("max")),
                    Expression.get("mag")
                )
                .withClusterProperty("sum", Expression.literal("+"), Expression.get("mag"))
                .withClusterProperty(
                    "felt",
                    Expression.literal("any"),
                    Expression.neq(Expression.get("felt"), Expression.literal("null"))
                )
        )
    }

    private fun createSymbolLayer(): SymbolLayer {
        return SymbolLayer("unclustered-points", "earthquakes")
            .withProperties(
                PropertyFactory.iconImage("icon-id"),
                PropertyFactory.iconSize(
                    Expression.division(
                        Expression.get("mag"),
                        Expression.literal(4.0f)
                    )
                ),
                PropertyFactory.iconColor(
                    Expression.interpolate(
                        Expression.exponential(1),
                        Expression.get("mag"),
                        Expression.stop(2.0, Expression.rgb(0, 255, 0)),
                        Expression.stop(4.5, Expression.rgb(0, 0, 255)),
                        Expression.stop(7.0, Expression.rgb(255, 0, 0))
                    )
                )
            )
            .withFilter(Expression.has("mag"))
    }

    private fun createClusterLevelLayer(level: Int, layerColors: Array<IntArray>): CircleLayer {
        val circles = CircleLayer("cluster-$level", "earthquakes")
        circles.setProperties(
            PropertyFactory.circleColor(layerColors[level][1]),
            PropertyFactory.circleRadius(18f)
        )
        val pointCount = Expression.toNumber(Expression.get("point_count"))
        circles.setFilter(
            if (level == 0) {
                Expression.all(
                    Expression.has("point_count"),
                    Expression.gte(
                        pointCount,
                        Expression.literal(layerColors[level][0])
                    )
                )
            } else {
                Expression.all(
                    Expression.has("point_count"),
                    Expression.gt(
                        pointCount,
                        Expression.literal(layerColors[level][0])
                    ),
                    Expression.lt(
                        pointCount,
                        Expression.literal(layerColors[level - 1][0])
                    )
                )
            }
        )
        return circles
    }

    private fun createClusterTextLayer(): SymbolLayer {
        return SymbolLayer("property", "earthquakes")
            .withProperties(
                PropertyFactory.textField(
                    Expression.concat(
                        Expression.get("point_count"),
                        Expression.literal(", "),
                        Expression.get("max")
                    )
                ),
                PropertyFactory.textSize(12f),
                PropertyFactory.textColor(Color.WHITE),
                PropertyFactory.textIgnorePlacement(true),
                PropertyFactory.textAllowOverlap(true)
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
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            android.R.id.home -> {
                onBackPressed()
                true
            }

            else -> super.onOptionsItemSelected(item)
        }
    }

    private fun updateClickOptionCounter() {
        if (clickOptionCounter == 2) {
            clickOptionCounter = 0
        } else {
            clickOptionCounter++
        }
    }

    private fun notifyClickOptionUpdate() {
        if (clickOptionCounter == 0) {
            Toast.makeText(
                this@GeoJsonClusteringActivity,
                "Clicking a cluster will zoom to the level where it dissolves",
                Toast.LENGTH_SHORT
            ).show()
        } else if (clickOptionCounter == 1) {
            Toast.makeText(
                this@GeoJsonClusteringActivity,
                "Clicking a cluster will show the details of the cluster children",
                Toast.LENGTH_SHORT
            ).show()
        } else {
            Toast.makeText(
                this@GeoJsonClusteringActivity,
                "Clicking a cluster will show the details of the cluster leaves with an offset and limit",
                Toast.LENGTH_SHORT
            ).show()
        }
    }

    companion object {
        private const val CAMERA_ZOOM_DELTA = 0.01
    }
}
