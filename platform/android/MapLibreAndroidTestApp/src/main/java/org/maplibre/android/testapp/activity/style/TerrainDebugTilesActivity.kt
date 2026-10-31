package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R

/**
 * Test activity loading the maplibre-gl-js terrain debug style
 * (https://demotiles.maplibre.org/debug-tiles), which drapes numbered
 * debug tiles over synthetic "ruffled" terrain. Useful for verifying
 * tile zoom variation, draping, and terrain mesh behavior against the
 * gl-js reference rendering of the same style.
 */
class TerrainDebugTilesActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    // The upstream debug-tiles style has no glyphs source (its tile numbers are raster
    // tiles, not text), so the built-in stats HUD - a symbol layer - could not render its
    // text. We inline the same style with a glyphs endpoint added (STYLE_JSON below), and
    // point the HUD at Noto Sans Regular, which that endpoint serves.
    private val options = TerrainTestOptions(this, "terrainSource", 1.0f, "Noto Sans Regular")

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            // Let the tilt gesture reach the full supported pitch over 3D terrain
            // (also required for the 70° start tilt below, which exceeds the 60° default)
            map.setMaxPitchPreference(MapLibreConstants.MAXIMUM_PITCH_LIMIT.toDouble())
            // Match the gl-js debug page's default view (#9/0/0/0/70)
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(0.0, 0.0))
                .zoom(9.0)
                .tilt(70.0)
                .build()
            map.setStyle(Style.Builder().fromJson(STYLE_JSON)) { style ->
                options.onMapReady(map, style)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: android.view.Menu): Boolean = options.onCreateOptionsMenu(menu)

    override fun onOptionsItemSelected(item: android.view.MenuItem): Boolean =
        options.onOptionsItemSelected(item) || super.onOptionsItemSelected(item)

    companion object {
        // The upstream https://demotiles.maplibre.org/debug-tiles/style.json verbatim
        // (already using absolute URLs) plus a "glyphs" endpoint so the stats HUD can
        // render text. The demotiles font server serves the composite default stack and
        // Noto Sans Regular (github.com/maplibre/demotiles/tree/gh-pages/font).
        private val STYLE_JSON = """
            {
              "version": 8,
              "glyphs": "https://demotiles.maplibre.org/font/{fontstack}/{range}.pbf",
              "sources": {
                "number": {
                  "type": "raster",
                  "url": "https://demotiles.maplibre.org/debug-tiles/number/tiles.json",
                  "tileSize": 256,
                  "maxzoom": 22
                },
                "terrainSource": {
                  "type": "raster-dem",
                  "url": "https://demotiles.maplibre.org/debug-tiles/terrain-ruffles/tiles.json",
                  "tileSize": 256
                },
                "hillshadeSource": {
                  "type": "raster-dem",
                  "url": "https://demotiles.maplibre.org/debug-tiles/number-hillshade/tiles.json",
                  "tileSize": 256
                }
              },
              "layers": [
                { "id": "number", "type": "raster", "source": "number" },
                {
                  "id": "hills",
                  "type": "hillshade",
                  "source": "hillshadeSource",
                  "layout": { "visibility": "visible" },
                  "paint": { "hillshade-shadow-color": "#473B24" }
                }
              ],
              "terrain": { "source": "terrainSource", "exaggeration": 1 }
            }
        """.trimIndent()
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
        options.onDestroy()
        mapView.onDestroy()
    }
}
