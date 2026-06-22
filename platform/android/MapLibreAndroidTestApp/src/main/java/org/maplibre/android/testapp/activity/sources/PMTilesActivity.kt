package org.maplibre.android.testapp.activity.sources

import android.graphics.Color
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.RasterLayer
import org.maplibre.android.style.sources.RasterSource
import org.maplibre.android.style.sources.VectorSource
import org.maplibre.android.testapp.R

class PMTilesActivity : AppCompatActivity() {
    private lateinit var mapView: MapView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_pmtiles)
        mapView = findViewById(R.id.mapView)

        mapView.getMapAsync { map ->
            // --8<-- [start:loadStyle]
            map.setStyle("https://demotiles.maplibre.org/pmtiles/raster/style-imagery.json") { style ->
                map.cameraPosition = CameraPosition.Builder()
                    .target(LatLng(47.5, 12.0))
                    .zoom(9.0)
                    .build()
                // --8<-- [end:loadStyle]
                addPlaces(style)
            }
        }
    }

    private fun addPlaces(style: Style) {
        // --8<-- [start:addSource]
        val overtureSource = VectorSource(
            "overture-places",
            "pmtiles://https://overturemaps-tiles-us-west-2-beta.s3.amazonaws.com/2026-01-21/places.pmtiles"
        )
        style.addSource(overtureSource)
        val overtureLayer = CircleLayer("overture-layer", "overture-places")
        overtureLayer.setSourceLayer("place")
        overtureLayer.setProperties(
            PropertyFactory.circleColor(Color.parseColor("#f5c800")),
            PropertyFactory.circleRadius(4f),
            PropertyFactory.circleOpacity(0.6f)
        )
        style.addLayer(overtureLayer)
        // --8<-- [end:addSource]

        val foursquareSource = VectorSource(
            "foursquare-places",
            "pmtiles://https://oliverwipfli.ch/data/foursquare-os-places-10M-2024-11-20.pmtiles"
        )
        style.addSource(foursquareSource)
        val foursquareLayer = CircleLayer("foursquare-layer", "foursquare-places")
        foursquareLayer.setSourceLayer("place")
        foursquareLayer.setProperties(
            PropertyFactory.circleColor(Color.parseColor("#2dd9fe")),
            PropertyFactory.circleRadius(4f),
            PropertyFactory.circleOpacity(0.6f)
        )
        style.addLayer(foursquareLayer)
    }

    @Suppress("unused")
    private fun addLocalRasterSource(style: Style) {
        // --8<-- [start:loadFromFile]
        val path = getExternalFilesDir(null)?.absolutePath + "/watercolor.pmtiles"
        val source = RasterSource(
            "watercolor",
            "pmtiles://file://$path",
            256
        )
        style.addSource(source)
        style.addLayer(RasterLayer("watercolor", "watercolor"))
        // --8<-- [end:loadFromFile]
    }

    override fun onStart() { super.onStart(); mapView.onStart() }
    override fun onResume() { super.onResume(); mapView.onResume() }
    override fun onPause() { super.onPause(); mapView.onPause() }
    override fun onStop() { super.onStop(); mapView.onStop() }
    override fun onLowMemory() { super.onLowMemory(); mapView.onLowMemory() }
    override fun onDestroy() { super.onDestroy(); mapView.onDestroy() }
}
