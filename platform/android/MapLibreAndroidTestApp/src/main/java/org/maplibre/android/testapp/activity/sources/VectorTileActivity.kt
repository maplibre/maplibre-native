package org.maplibre.android.testapp.activity.sources

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapView
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.PropertyFactory.lineColor
import org.maplibre.android.style.layers.PropertyFactory.lineWidth
import org.maplibre.android.style.sources.TileSet
import org.maplibre.android.style.sources.VectorSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles


class VectorTileActivity : AppCompatActivity() {
    private lateinit var mapView: MapView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_vector_tile)
        mapView = findViewById<MapView>(R.id.mapView)

        mapView.getMapAsync {
            it.animateCamera(
                CameraUpdateFactory.newLatLngBounds(
                    // z: 12, x: 2177, y: 1436 is one of the available tiles:
                    // https://github.com/maplibre/demotiles/tree/gh-pages/tiles-omt/12/2177
                    LatLngBounds.from(12, 2177, 1436),
                    0
                )
            )
            it.setStyle(TestStyles.PROTOMAPS_GRAYSCALE) { style ->
                // --8<-- [start:addTileSet]
                val tileset = TileSet(
                    "openmaptiles",
                    "https://demotiles.maplibre.org/tiles-omt/{z}/{x}/{y}.pbf"
                )
                val openmaptiles = VectorSource("openmaptiles", tileset)
                style.addSource(openmaptiles)
                val roadLayer = LineLayer("road", "openmaptiles").apply {
                    setSourceLayer("transportation")
                    setProperties(
                        lineColor("red"),
                        lineWidth(2.0f)
                    )
                }
                // --8<-- [end:addTileSet]

                style.addLayer(roadLayer)
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

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }
}
