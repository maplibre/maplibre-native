package org.maplibre.android.testapp.activity.projection

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.PropertyFactory.circleOpacity
import org.maplibre.android.style.layers.PropertyFactory.circleRadius
import org.maplibre.android.style.layers.PropertyFactory.circleStrokeColor
import org.maplibre.android.style.layers.PropertyFactory.circleStrokeWidth
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.projection.NationalGrid
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point

/**
 * Test activity showing tiles cut on the British National Grid, EPSG:27700, a projection registered from Kotlin. The
 * tiles carry red crosshairs at known eastings and northings, and each blue ring is a GeoJSON point set from Kotlin at
 * the WGS 84 position of one crosshair, so it centers on that crosshair.
 */
class NationalGridActivity : AppCompatActivity() {
    private lateinit var mapView: MapView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Style.addProjection(NationalGrid.coordinateReferenceSystem)
        setContentView(R.layout.activity_map_simple)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            map.setStyle(Style.Builder().fromUri("asset://national_grid_crosshairs.json")) { style ->
                map.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(51.96064818208741, -1.2773320477729189), 2.0))
                val rings = CROSSHAIRS.map { (easting, northing) ->
                    val (latitude, longitude) = NationalGrid.wgs84FromEastingNorthing(easting, northing)
                    Feature.fromGeometry(Point.fromLngLat(longitude, latitude))
                }
                style.addSource(GeoJsonSource(RINGS_SOURCE, FeatureCollection.fromFeatures(rings)))
                style.addLayer(
                    CircleLayer(RINGS_LAYER, RINGS_SOURCE).withProperties(
                        circleRadius(8f),
                        circleOpacity(0f),
                        circleStrokeWidth(2f),
                        circleStrokeColor("#0000ff")
                    )
                )
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

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
        Style.removeProjection(NationalGrid.NAME)
    }

    companion object {
        const val RINGS_SOURCE = "rings"
        const val RINGS_LAYER = "rings"

        /** The eastings and northings of the crosshairs on the tiles, each on a pixel center at OS level 0. */
        val CROSSHAIRS = listOf(
            310425.0 to 386624.0,
            452889.0 to 279104.0,
            611481.0 to 94528.0,
            274585.0 to 193088.0
        )
    }
}
