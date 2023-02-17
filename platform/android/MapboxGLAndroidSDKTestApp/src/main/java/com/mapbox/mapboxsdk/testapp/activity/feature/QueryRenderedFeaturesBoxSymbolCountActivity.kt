package com.mapbox.mapboxsdk.testapp.activity.feature

import android.graphics.BitmapFactory
import android.graphics.RectF
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.expressions.Expression
import com.mapbox.mapboxsdk.style.layers.BackgroundLayer
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.style.layers.SymbolLayer
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.testapp.utils.ResourceUtils
import timber.log.Timber
import java.io.IOException

/**
 * Test activity showcasing using the query rendered features API to count Symbols in a rectangle.
 */
class QueryRenderedFeaturesBoxSymbolCountActivity : AppCompatActivity() {
    lateinit var mapView: MapView
    lateinit var mapboxMap: MapboxMap
        private set
    private lateinit var toast: Toast
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_query_features_box)
        val selectionBox = findViewById<View>(R.id.selection_box)

        // Initialize map as normal
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { mapboxMap: MapboxMap ->
            this@QueryRenderedFeaturesBoxSymbolCountActivity.mapboxMap = mapboxMap
            try {
                val testPoints = ResourceUtils.readRawResource(
                    mapView.context,
                    R.raw.test_points_utrecht
                )
                val markerImage =
                    BitmapFactory.decodeResource(resources, R.drawable.maplibre_marker_icon_default)
                mapboxMap.setStyle(
                    Style.Builder()
                        .withLayer(
                            BackgroundLayer("bg")
                                .withProperties(
                                    PropertyFactory.backgroundColor(Expression.rgb(120, 161, 226))
                                )
                        )
                        .withLayer(
                            SymbolLayer("symbols-layer", "symbols-source")
                                .withProperties(
                                    PropertyFactory.iconImage("test-icon")
                                )
                        )
                        .withSource(
                            GeoJsonSource("symbols-source", testPoints)
                        )
                        .withImage("test-icon", markerImage)
                )
            } catch (exception: IOException) {
                exception.printStackTrace()
            }
            selectionBox.setOnClickListener { view: View? ->
                // Query
                val top = selectionBox.top - mapView.top
                val left = selectionBox.left - mapView.left
                val box = RectF(
                    left.toFloat(),
                    top.toFloat(),
                    (left + selectionBox.width).toFloat(),
                    (top + selectionBox.height).toFloat()
                )
                Timber.i("Querying box %s", box)
                val features = mapboxMap.queryRenderedFeatures(box, "symbols-layer")

                // Show count
                if (toast != null) {
                    toast!!.cancel()
                }
                toast = Toast.makeText(
                    this@QueryRenderedFeaturesBoxSymbolCountActivity,
                    String.format("%s features in box", features.size),
                    Toast.LENGTH_SHORT
                )
                toast.show()
            }
        }
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
}
