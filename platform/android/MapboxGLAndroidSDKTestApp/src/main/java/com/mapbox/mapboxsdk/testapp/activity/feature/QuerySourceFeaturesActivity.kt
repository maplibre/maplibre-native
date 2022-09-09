package com.mapbox.mapboxsdk.testapp.activity.feature

import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.gson.JsonObject
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.Point
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.expressions.Expression
import com.mapbox.mapboxsdk.style.layers.CircleLayer
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.R

/**
 * Test activity showcasing using the query source features API to query feature counts
 */
class QuerySourceFeaturesActivity : AppCompatActivity() {
    var mapView: MapView? = null
    private var mapboxMap: MapboxMap? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_query_source_features)

        // Initialize map as normal
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView!!.onCreate(savedInstanceState)
        mapView!!.getMapAsync { map: MapboxMap? ->
            mapboxMap = map
            mapboxMap!!.getStyle { style: Style -> initStyle(style) }
            mapboxMap!!.setStyle(Style.getPredefinedStyle("Streets"))
        }
    }

    private fun initStyle(style: Style) {
        val properties = JsonObject()
        properties.addProperty("key1", "value1")
        val source = GeoJsonSource(
            "test-source",
            FeatureCollection.fromFeatures(
                arrayOf(
                    Feature.fromGeometry(Point.fromLngLat(17.1, 51.0), properties),
                    Feature.fromGeometry(Point.fromLngLat(17.2, 51.0), properties),
                    Feature.fromGeometry(Point.fromLngLat(17.3, 51.0), properties),
                    Feature.fromGeometry(Point.fromLngLat(17.4, 51.0), properties)
                )
            )
        )
        style.addSource(source)
        val visible = Expression.eq(Expression.get("key1"), Expression.literal("value1"))
        val invisible = Expression.neq(Expression.get("key1"), Expression.literal("value1"))
        val layer = CircleLayer("test-layer", source.id)
            .withFilter(visible)
        style.addLayer(layer)

        // Add a click listener
        mapboxMap!!.addOnMapClickListener { point: LatLng? ->
            // Query
            val features = source.querySourceFeatures(
                Expression.eq(
                    Expression.get("key1"),
                    Expression.literal("value1")
                )
            )
            Toast.makeText(
                this@QuerySourceFeaturesActivity,
                String.format(
                    "Found %s features",
                    features.size
                ),
                Toast.LENGTH_SHORT
            ).show()
            false
        }
        val fab = findViewById<View>(R.id.fab) as FloatingActionButton
        fab.setColorFilter(ContextCompat.getColor(this, R.color.primary))
        fab.setOnClickListener { view: View? ->
            val visibility = layer.filter
            if (visibility != null && visibility == visible) {
                layer.setFilter(invisible)
                fab.setImageResource(R.drawable.ic_layers_clear)
            } else {
                layer.setFilter(visible)
                fab.setImageResource(R.drawable.ic_layers)
            }
        }
    }

    override fun onStart() {
        super.onStart()
        mapView!!.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView!!.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView!!.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView!!.onStop()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView!!.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView!!.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }
}
