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
        // the 3D elevation mesh. The colour ramp is a bright hypsometric tint
        // with many bands (green lowlands through gold and brown to snow) so the
        // color-relief output is clearly visible across the Matterhorn's ~1500 m
        // to ~4500 m range; the hillshade is kept subtle so it shades the relief
        // without crushing the colours.
        //
        // Terrain uses its own raster-dem source (same tiles) per the
        // maplibre-gl-js recommendation: gl-js keeps a separate terrain source
        // cache, so sharing one source between terrain and hillshade conflicts
        // there. Native shares one tile pyramid per source and tolerates
        // sharing, but the examples model the portable style structure.
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
                },
                "mapterhorn-terrain": {
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
                  "paint": { "background-color": "#d8e4ec" }
                },
                {
                  "id": "color-relief",
                  "type": "color-relief",
                  "source": "mapterhorn",
                  "paint": {
                    "color-relief-color": [
                      "interpolate", ["linear"], ["elevation"],
                      0, "rgba(0,0,0,0)",
                      250, "#aee0a0",
                      750, "#c8e878",
                      1250, "#eee8a0",
                      1750, "#e8cf7a",
                      2250, "#dcac5f",
                      2750, "#c8864a",
                      3250, "#b06a45",
                      3750, "#9c7f70",
                      4250, "#d8d4cc",
                      4478, "#ffffff"
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
                    "hillshade-exaggeration": 0.3,
                    "hillshade-shadow-color": "#4a4a4a",
                    "hillshade-highlight-color": "#ffffff",
                    "hillshade-accent-color": "#8a8a8a"
                  }
                }
              ],
              "terrain": {
                "source": "mapterhorn-terrain",
                "exaggeration": 1.0
              }
            }
        """.trimIndent()
    }
}
