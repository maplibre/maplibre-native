package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.MultiLineString
import org.maplibre.geojson.Point
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.*
import org.maplibre.android.style.sources.CustomGeometrySource
import org.maplibre.android.style.sources.GeometryTileProvider
import org.maplibre.android.testapp.R
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
        override fun getFeaturesForBounds(bounds: LatLngBounds, zoomLevel: Int): FeatureCollection {
            val features: MutableList<Feature> = ArrayList()
            val gridSpacing = if (zoomLevel >= 13) {
                0.01
            } else if (zoomLevel >= 11) {
                0.05
            } else if (zoomLevel == 10) {
                .1
            } else if (zoomLevel == 9) {
                0.25
            } else if (zoomLevel == 8) {
                0.5
            } else if (zoomLevel >= 6) {
                1.0
            } else if (zoomLevel == 5) {
                2.0
            } else if (zoomLevel >= 4) {
                5.0
            } else if (zoomLevel == 2) {
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

    override fun onMapReady(map: MapLibreMap) {
        source = CustomGeometrySource(ID_GRID_SOURCE, GridProvider())
        layer = LineLayer(ID_GRID_LAYER, ID_GRID_SOURCE)
        layer!!.setProperties(
            PropertyFactory.lineColor(Color.parseColor("#000000"))
        )
        map.setStyle(
            Style.Builder()
                .fromUri(Style.getPredefinedStyles()[0].url)
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
