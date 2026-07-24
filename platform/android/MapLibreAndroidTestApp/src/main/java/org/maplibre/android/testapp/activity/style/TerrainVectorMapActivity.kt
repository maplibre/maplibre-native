package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.TerrainLoadMode
import org.maplibre.android.style.layers.HillshadeLayer
import org.maplibre.android.style.layers.PropertyFactory.hillshadeExaggeration
import org.maplibre.android.style.layers.PropertyFactory.hillshadeMethod
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.RasterDemSource
import org.maplibre.android.style.terrain.Terrain
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing 3D terrain on a full-planet vector basemap:
 * the OpenFreeMap Liberty style combined with the Mapterhorn raster-dem
 * tiles (https://mapterhorn.com), both with worldwide coverage.
 *
 * The DEM sources, hillshade layer, and terrain configuration are added
 * at runtime with the style API. Terrain gets its own raster-dem source
 * (same tiles) per the maplibre-gl-js recommendation.
 */
class TerrainVectorMapActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var style: Style? = null
    private var terrainEnabled = true

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map
            // Let the tilt gesture reach the full supported pitch over 3D terrain
            map.setMaxPitchPreference(MapLibreConstants.MAXIMUM_PITCH_LIMIT.toDouble())
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(47.26475, 11.40416)) // Innsbruck
                .zoom(12.0)
                .tilt(60.0)
                .bearing(20.0)
                .build()
            map.setStyle(Style.Builder().fromUri(TestStyles.OPENFREEMAP_LIBERTY)) { style ->
                this.style = style
                addTerrain(style)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menu.add(0, MENU_TOGGLE_TERRAIN, 0, "Toggle terrain")
        val sub = menu.addSubMenu("Terrain load mode")
        sub.add(0, MENU_MODE_QUALITY, 0, "Quality (default)")
        sub.add(0, MENU_MODE_BALANCED, 0, "Balanced")
        sub.add(0, MENU_MODE_PERFORMANCE, 0, "Performance")
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            MENU_TOGGLE_TERRAIN -> {
                terrainEnabled = !terrainEnabled
                style?.setTerrain(
                    if (terrainEnabled) Terrain(source = SOURCE_ID_TERRAIN, exaggeration = 1.0f) else null
                )
                toast("Terrain ${if (terrainEnabled) "on" else "off"}")
            }
            MENU_MODE_QUALITY -> setLoadMode(TerrainLoadMode.QUALITY, "Quality")
            MENU_MODE_BALANCED -> setLoadMode(TerrainLoadMode.BALANCED, "Balanced")
            MENU_MODE_PERFORMANCE -> setLoadMode(TerrainLoadMode.PERFORMANCE, "Performance")
            else -> return super.onOptionsItemSelected(item)
        }
        return true
    }

    private fun setLoadMode(mode: TerrainLoadMode, name: String) {
        maplibreMap.setTerrainLoadMode(mode)
        toast("Terrain load mode: $name")
    }

    private fun toast(message: String) = Toast.makeText(this, message, Toast.LENGTH_SHORT).show()

    private fun addTerrain(style: Style) {
        // The Mapterhorn TileJSON declares "encoding": "terrarium", which the
        // tileset parser picks up
        style.addSource(RasterDemSource(SOURCE_ID_HILLSHADE, DEM_TILEJSON))
        style.addSource(RasterDemSource(SOURCE_ID_TERRAIN, DEM_TILEJSON))

        // Insert the hillshade below the first symbol layer so labels stay on top
        val hillshade = HillshadeLayer(LAYER_ID_HILLSHADE, SOURCE_ID_HILLSHADE)
            .withProperties(
                hillshadeMethod("igor"),
                hillshadeExaggeration(0.4f)
            )
        val firstSymbolLayer = style.layers.firstOrNull { it is SymbolLayer }
        if (firstSymbolLayer != null) {
            style.addLayerBelow(hillshade, firstSymbolLayer.id)
        } else {
            style.addLayer(hillshade)
        }

        style.setTerrain(Terrain(source = SOURCE_ID_TERRAIN, exaggeration = 1.0f))
    }

    companion object {
        private const val MENU_TOGGLE_TERRAIN = 1
        private const val MENU_MODE_QUALITY = 2
        private const val MENU_MODE_BALANCED = 3
        private const val MENU_MODE_PERFORMANCE = 4
        private const val DEM_TILEJSON = "https://tiles.mapterhorn.com/tilejson.json"
        private const val SOURCE_ID_HILLSHADE = "mapterhorn"
        private const val SOURCE_ID_TERRAIN = "mapterhorn-terrain"
        private const val LAYER_ID_HILLSHADE = "mapterhorn-hillshade"
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
