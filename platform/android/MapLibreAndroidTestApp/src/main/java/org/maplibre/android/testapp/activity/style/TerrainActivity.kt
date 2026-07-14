package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R

/**
 * Test activity showcasing 3D terrain together with hillshade and color-relief
 * layers, driven by the Mapterhorn raster-dem tiles (https://mapterhorn.com).
 *
 * The terrain root property has no runtime styling API yet, so the whole style
 * is provided as JSON.
 */
class TerrainActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(45.9763, 7.6586)) // Matterhorn
                .zoom(11.5)
                .tilt(60.0)
                .bearing(35.0)
                .build()
            map.setStyle(Style.Builder().fromJson(STYLE_JSON))
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

    companion object {
        // A self-contained style: color-relief provides the base coloring,
        // hillshade adds shading, and the terrain property drapes both over
        // the 3D elevation mesh. All three read the same raster-dem source.
        // The color ramp and hillshade tuning follow the WDB_COLOR_RELIEF_DARK
        // tileserver-gl style.
        private val STYLE_JSON = """
            {
              "version": 8,
              "name": "Terrain demo",
              "sources": {
                "mapterhorn": {
                  "type": "raster-dem",
                  "url": "https://tiles.mapterhorn.com/tilejson.json",
                  "encoding": "terrarium",
                  "attribution": "<a href=\"https://mapterhorn.com\">Mapterhorn</a>"
                }
              },
              "layers": [
                {
                  "id": "background",
                  "type": "background",
                  "paint": { "background-color": "#0e1512" }
                },
                {
                  "id": "color-relief",
                  "type": "color-relief",
                  "source": "mapterhorn",
                  "paint": {
                    "color-relief-color": [
                      "interpolate", ["linear"], ["elevation"],
                      0, "rgba(0,0,0,0)",
                      0.1, "#121d10",
                      500, "#152010",
                      1000, "#1a2610",
                      2000, "#1e2a14",
                      2500, "#232e18",
                      3500, "#2a3420",
                      4000, "#343c2c"
                    ]
                  }
                },
                {
                  "id": "hillshade",
                  "type": "hillshade",
                  "source": "mapterhorn",
                  "paint": {
                    "hillshade-method": "igor",
                    "hillshade-illumination-anchor": "map",
                    "hillshade-exaggeration": 0.5,
                    "hillshade-shadow-color": "#000000",
                    "hillshade-highlight-color": "#2a3820",
                    "hillshade-accent-color": "#050a04"
                  }
                }
              ],
              "terrain": {
                "source": "mapterhorn",
                "exaggeration": 1.0
              }
            }
        """.trimIndent()
    }
}
