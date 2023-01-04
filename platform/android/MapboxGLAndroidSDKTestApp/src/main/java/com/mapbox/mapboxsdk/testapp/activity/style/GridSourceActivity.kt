package com.mapbox.mapboxsdk.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.MultiLineString
import com.mapbox.geojson.Point
import com.mapbox.mapboxsdk.geometry.LatLngBounds
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.layers.*
import com.mapbox.mapboxsdk.style.sources.CustomGeometrySource
import com.mapbox.mapboxsdk.style.sources.GeometryTileProvider
import com.mapbox.mapboxsdk.testapp.R
import java.util.*

/**
 * Test activity showcasing using CustomGeometrySource to create a grid overlay on the map.
 */
class GridSourceActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapView: MapView

    // public for testing purposes
    var source: CustomGeometrySource? = null
    var layer: LineLayer? = null

    /**
     * Implementation of GeometryTileProvider that returns features representing a zoom-dependent
     * grid.
     */
    internal class GridProvider : GeometryTileProvider {
        override fun getFeaturesForBounds(bounds: LatLngBounds, zoom: Int): FeatureCollection {
            val features: MutableList<Feature> = ArrayList()
            val gridSpacing: Double
            gridSpacing = if (zoom >= 13) {
                0.01
            } else if (zoom >= 11) {
                0.05
            } else if (zoom == 10) {
                .1
            } else if (zoom == 9) {
                0.25
            } else if (zoom == 8) {
                0.5
            } else if (zoom >= 6) {
                1.0
            } else if (zoom == 5) {
                2.0
            } else if (zoom >= 4) {
                5.0
            } else if (zoom == 2) {
                10.0
            } else {
                20.0
            }
            var gridLines: MutableList<Any?> = ArrayList<Any?>()
            var y = Math.ceil(bounds.latitudeNorth / gridSpacing) * gridSpacing
            while (y >= Math.floor(bounds.latitudeSouth / gridSpacing) * gridSpacing) {
                gridLines.add(
                    Arrays.asList(
                        Point.fromLngLat(bounds.longitudeWest, y),
                        Point.fromLngLat(bounds.longitudeEast, y)
                    )
                )
                y -= gridSpacing
            }
            features.add(Feature.fromGeometry(MultiLineString.fromLngLats(gridLines as MutableList<MutableList<Point>>)))
            gridLines = ArrayList<Any?>()
            var x = Math.floor(bounds.longitudeWest / gridSpacing) * gridSpacing
            while (x <= Math.ceil(bounds.longitudeEast / gridSpacing) * gridSpacing) {
                gridLines.add(
                    Arrays.asList(
                        Point.fromLngLat(x, bounds.latitudeSouth),
                        Point.fromLngLat(x, bounds.latitudeNorth)
                    )
                )
                x += gridSpacing
            }
            features.add(Feature.fromGeometry(MultiLineString.fromLngLats(gridLines as MutableList<MutableList<Point>>)))
            return FeatureCollection.fromFeatures(features)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_grid_source)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapboxMap) {
        source = CustomGeometrySource(ID_GRID_SOURCE, GridProvider())
        layer = LineLayer(ID_GRID_LAYER, ID_GRID_SOURCE)
        layer!!.setProperties(
            PropertyFactory.lineColor(Color.parseColor("#000000"))
        )
        map.setStyle(
            Style.Builder()
                .fromUri(Style.getPredefinedStyle("Streets"))
                .withLayer(layer!!)
                .withSource(source!!)
        )
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    public override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    public override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    companion object {
        const val ID_GRID_SOURCE = "grid_source"
        const val ID_GRID_LAYER = "grid_layer"
    }
}
