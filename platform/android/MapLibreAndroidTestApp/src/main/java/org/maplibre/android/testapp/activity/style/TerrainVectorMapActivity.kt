package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.json.JSONArray
import org.json.JSONObject
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import java.net.URL
import kotlin.concurrent.thread

/**
 * Test activity showcasing 3D terrain on a full-planet vector basemap:
 * the OpenFreeMap Liberty style combined with the Mapterhorn raster-dem
 * tiles (https://mapterhorn.com), both with worldwide coverage.
 *
 * The terrain root property has no runtime styling API yet, so the style
 * JSON is fetched and patched with the DEM sources, a hillshade layer,
 * and the terrain configuration before it is loaded.
 */
class TerrainVectorMapActivity : AppCompatActivity() {
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
                .target(LatLng(47.26475, 11.40416)) // Innsbruck
                .zoom(12.0)
                .tilt(60.0)
                .bearing(20.0)
                .build()
            loadTerrainStyle()
        }
    }

    private fun loadTerrainStyle() {
        thread {
            val styleJson = try {
                buildTerrainStyle(URL(TestStyles.OPENFREEMAP_LIBERTY).readText())
            } catch (e: Exception) {
                runOnUiThread {
                    Toast.makeText(this, "Failed to load style: ${e.message}", Toast.LENGTH_LONG).show()
                }
                return@thread
            }
            runOnUiThread {
                if (!isFinishing && !isDestroyed) {
                    maplibreMap.setStyle(Style.Builder().fromJson(styleJson))
                }
            }
        }
    }

    companion object {
        private const val DEM_TILEJSON = "https://tiles.mapterhorn.com/tilejson.json"

        /**
         * Add the Mapterhorn DEM sources, a hillshade layer, and the terrain
         * configuration to a fetched style. Terrain gets its own raster-dem
         * source (same tiles) per the maplibre-gl-js recommendation.
         */
        private fun buildTerrainStyle(baseStyle: String): String {
            val style = JSONObject(baseStyle)

            val demSource = JSONObject()
                .put("type", "raster-dem")
                .put("url", DEM_TILEJSON)
                .put("encoding", "terrarium")
                .put("attribution", "<a href=\"https://mapterhorn.com\">Mapterhorn</a>")
            val sources = style.getJSONObject("sources")
            sources.put("mapterhorn", demSource)
            sources.put("mapterhorn-terrain", JSONObject(demSource.toString()))

            val hillshadeLayer = JSONObject()
                .put("id", "mapterhorn-hillshade")
                .put("type", "hillshade")
                .put("source", "mapterhorn")
                .put(
                    "paint",
                    JSONObject()
                        .put("hillshade-method", "igor")
                        .put("hillshade-exaggeration", 0.4)
                )

            // Insert the hillshade below the first symbol layer so labels stay on top
            val layers = style.getJSONArray("layers")
            val patchedLayers = JSONArray()
            var inserted = false
            for (i in 0 until layers.length()) {
                val layer = layers.getJSONObject(i)
                if (!inserted && layer.optString("type") == "symbol") {
                    patchedLayers.put(hillshadeLayer)
                    inserted = true
                }
                patchedLayers.put(layer)
            }
            if (!inserted) {
                patchedLayers.put(hillshadeLayer)
            }
            style.put("layers", patchedLayers)

            style.put(
                "terrain",
                JSONObject()
                    .put("source", "mapterhorn-terrain")
                    .put("exaggeration", 1.0)
            )

            return style.toString()
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
}
