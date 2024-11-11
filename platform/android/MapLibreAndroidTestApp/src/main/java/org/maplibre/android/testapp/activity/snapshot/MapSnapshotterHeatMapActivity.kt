package org.maplibre.android.testapp.activity.snapshot

import android.annotation.SuppressLint
import android.os.Bundle
import android.view.View
import android.view.ViewTreeObserver.OnGlobalLayoutListener
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Style
import org.maplibre.android.snapshotter.MapSnapshot
import org.maplibre.android.snapshotter.MapSnapshotter
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.HeatmapLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.style.sources.Source
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber
import java.net.URI
import java.net.URISyntaxException

/**
 * Test activity showing how to use the [MapSnapshotter] and heatmap layer on it.
 */

// # --8<-- [start:class_declaration]
class MapSnapshotterHeatMapActivity : AppCompatActivity(), MapSnapshotter.SnapshotReadyCallback {
// # --8<-- [end:class_declaration]
    private var mapSnapshotter: MapSnapshotter? = null

    // # --8<-- [start:onCreate]
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_snapshotter_marker)
        val container = findViewById<View>(R.id.container)
        container.viewTreeObserver
            .addOnGlobalLayoutListener(object : OnGlobalLayoutListener {
                override fun onGlobalLayout() {
                    container.viewTreeObserver.removeOnGlobalLayoutListener(this)
                    Timber.i("Starting snapshot")
                    val builder = Style.Builder().fromUri(TestStyles.AMERICANA)
                        .withSource(earthquakeSource!!)
                        .withLayerAbove(heatmapLayer, "water")
                    mapSnapshotter = MapSnapshotter(
                        applicationContext,
                        MapSnapshotter.Options(container.measuredWidth, container.measuredHeight)
                            .withStyleBuilder(builder)
                            .withCameraPosition(
                                CameraPosition.Builder()
                                    .target(LatLng(15.0, (-94).toDouble()))
                                    .zoom(5.0)
                                    .padding(1.0, 1.0, 1.0, 1.0)
                                    .build()
                            )
                    )
                    mapSnapshotter!!.start(this@MapSnapshotterHeatMapActivity)
                }
            })
    }
    // # --8<-- [end:onCreate]

    // # --8<-- [start:heatmapLayer]
    private val heatmapLayer: HeatmapLayer
        get() {
            val layer = HeatmapLayer(HEATMAP_LAYER_ID, EARTHQUAKE_SOURCE_ID)
            layer.maxZoom = 9f
            layer.sourceLayer = HEATMAP_LAYER_SOURCE
            layer.setProperties(
                PropertyFactory.heatmapColor(
                    Expression.interpolate(
                        Expression.linear(), Expression.heatmapDensity(),
                        Expression.literal(0), Expression.rgba(33, 102, 172, 0),
                        Expression.literal(0.2), Expression.rgb(103, 169, 207),
                        Expression.literal(0.4), Expression.rgb(209, 229, 240),
                        Expression.literal(0.6), Expression.rgb(253, 219, 199),
                        Expression.literal(0.8), Expression.rgb(239, 138, 98),
                        Expression.literal(1), Expression.rgb(178, 24, 43)
                    )
                ),
                PropertyFactory.heatmapWeight(
                    Expression.interpolate(
                        Expression.linear(),
                        Expression.get("mag"),
                        Expression.stop(0, 0),
                        Expression.stop(6, 1)
                    )
                ),
                PropertyFactory.heatmapIntensity(
                    Expression.interpolate(
                        Expression.linear(),
                        Expression.zoom(),
                        Expression.stop(0, 1),
                        Expression.stop(9, 3)
                    )
                ),
                PropertyFactory.heatmapRadius(
                    Expression.interpolate(
                        Expression.linear(),
                        Expression.zoom(),
                        Expression.stop(0, 2),
                        Expression.stop(9, 20)
                    )
                ),
                PropertyFactory.heatmapOpacity(
                    Expression.interpolate(
                        Expression.linear(),
                        Expression.zoom(),
                        Expression.stop(7, 1),
                        Expression.stop(9, 0)
                    )
                )
            )
            return layer
        }
    // # --8<-- [end:heatmapLayer]

    // # --8<-- [start:onSnapshotReady]
    @SuppressLint("ClickableViewAccessibility")
    override fun onSnapshotReady(snapshot: MapSnapshot) {
        Timber.i("Snapshot ready")
        val imageView = findViewById<ImageView>(R.id.snapshot_image)
        imageView.setImageBitmap(snapshot.bitmap)
    }
    // # --8<-- [end:onSnapshotReady]

    // # --8<-- [start:onStop]
    override fun onStop() {
        super.onStop()
        mapSnapshotter?.cancel()
    }
    // # --8<-- [end:onStop]

    // # --8<-- [start:earthquakeSource]
    private val earthquakeSource: Source?
        get() {
            var source: Source? = null
            try {
                source = GeoJsonSource(EARTHQUAKE_SOURCE_ID, URI(EARTHQUAKE_SOURCE_URL))
            } catch (uriSyntaxException: URISyntaxException) {
                Timber.e(uriSyntaxException, "That's not a valid URL.")
            }
            return source
        }
    // # --8<-- [end:earthquakeSource]

    companion object {
        private const val EARTHQUAKE_SOURCE_URL =
            "https://maplibre.org/maplibre-gl-js/docs/assets/earthquakes.geojson"
        private const val EARTHQUAKE_SOURCE_ID = "earthquakes"
        private const val HEATMAP_LAYER_ID = "earthquakes-heat"
        private const val HEATMAP_LAYER_SOURCE = "earthquakes"
    }
}
