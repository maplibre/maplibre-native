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
 * Test activity showcasing OSM raster tiles draped over 3D terrain, following
 * the maplibre-gl-js "3D Terrain" example: a raster basemap, a hillshade
 * layer, and the terrain root property in a self-contained style JSON.
 * Exercises raster layer draping specifically.
 */
class TerrainOsmRasterActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            // Let the tilt gesture reach the full supported pitch over 3D terrain
            map.setMaxPitchPreference(MapLibreConstants.MAXIMUM_PITCH_LIMIT.toDouble())
            // Match the maplibre-gl-js 3D Terrain example's view of Innsbruck
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(47.27574, 11.39085))
                .zoom(12.0)
                .tilt(52.0)
                .build()
            map.setStyle(Style.Builder().fromJson(STYLE_JSON))
        }
    }

    companion object {
        private val STYLE_JSON = """
            {
              "version": 8,
              "sources": {
                "osm": {
                  "type": "raster",
                  "tiles": ["https://tile.openstreetmap.org/{z}/{x}/{y}.png"],
                  "tileSize": 256,
                  "attribution": "&copy; OpenStreetMap Contributors",
                  "maxzoom": 19
                },
                "terrain_source": {
                  "type": "raster-dem",
                  "url": "https://tiles.mapterhorn.com/tilejson.json"
                },
                "hillshade_source": {
                  "type": "raster-dem",
                  "url": "https://tiles.mapterhorn.com/tilejson.json"
                }
              },
              "layers": [
                {
                  "id": "osm",
                  "type": "raster",
                  "source": "osm"
                },
                {
                  "id": "hills",
                  "type": "hillshade",
                  "source": "hillshade_source",
                  "layout": {"visibility": "visible"},
                  "paint": {"hillshade-shadow-color": "#473B24"}
                }
              ],
              "terrain": {
                "source": "terrain_source",
                "exaggeration": 1
              }
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
        mapView.onDestroy()
    }
}
