package org.maplibre.android.testapp.activity.style

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.view.Gravity
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.TerrainLoadMode
import org.maplibre.android.style.layers.PropertyFactory.textFont
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.terrain.Terrain

/**
 * Shared options menu for the "3D Terrain" test activities. Adds a runtime menu that
 * toggles terrain on/off, switches the [TerrainLoadMode], and toggles the built-in
 * rendering-stats HUD - the same controls across every 3D terrain test.
 *
 * The controls are also driveable over adb so device tests (and manual debugging) can
 * flip them without tapping the overflow menu:
 *
 *   adb shell am broadcast -p <pkg> \
 *     -a org.maplibre.android.testapp.action.TERRAIN_CMD --es cmd terrain_toggle
 *
 * cmd values: terrain_on|terrain_off|terrain_toggle, mode_quality|mode_balanced|
 * mode_performance, stats_on|stats_off|stats_toggle. Initial state can also be set with
 * launch extras: --ez terrain false --es mode performance --ez stats true.
 *
 * @param terrainSourceId the raster-dem source id the style wires terrain to; used to
 *   re-enable terrain after it was toggled off.
 * @param exaggeration terrain exaggeration to restore on re-enable (match the style).
 * @param statsFont font for the stats HUD text. The built-in HUD is a SymbolLayer with no
 *   text-font, so it falls back to Open Sans, which not every glyph server provides. Pass a
 *   font the hosting style's glyph endpoint actually serves (e.g. "Noto Sans Regular" for
 *   OpenFreeMap), or null to keep the SymbolLayer default. Styles with no glyphs source
 *   cannot render the HUD text at all.
 */
class TerrainTestOptions(
    private val activity: AppCompatActivity,
    private val terrainSourceId: String,
    private val exaggeration: Float = 1.0f,
    private val statsFont: String? = null
) {
    private var map: MapLibreMap? = null
    private var style: Style? = null

    var terrainEnabled = true
        private set
    var loadMode = TerrainLoadMode.QUALITY
        private set
    var statsEnabled = false
        private set

    private val cmdReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            when (intent?.getStringExtra(EXTRA_CMD)) {
                "terrain_on" -> setTerrainEnabled(true)
                "terrain_off" -> setTerrainEnabled(false)
                "terrain_toggle" -> setTerrainEnabled(!terrainEnabled)
                "mode_quality" -> setLoadMode(TerrainLoadMode.QUALITY)
                "mode_balanced" -> setLoadMode(TerrainLoadMode.BALANCED)
                "mode_performance" -> setLoadMode(TerrainLoadMode.PERFORMANCE)
                "stats_on" -> setStatsEnabled(true)
                "stats_off" -> setStatsEnabled(false)
                "stats_toggle" -> setStatsEnabled(!statsEnabled)
                else -> return
            }
            activity.invalidateOptionsMenu()
        }
    }

    /** Call once the map and its style are ready. */
    fun onMapReady(map: MapLibreMap, style: Style) {
        this.map = map
        this.style = style
        applyLaunchExtras()
        ContextCompat.registerReceiver(
            activity, cmdReceiver, IntentFilter(ACTION_CMD), ContextCompat.RECEIVER_EXPORTED
        )
    }

    fun onDestroy() {
        runCatching { activity.unregisterReceiver(cmdReceiver) }
    }

    fun onCreateOptionsMenu(menu: Menu): Boolean {
        menu.add(0, MENU_TOGGLE_TERRAIN, 0, "Terrain enabled").apply {
            isCheckable = true
            isChecked = terrainEnabled
        }
        menu.add(0, MENU_TOGGLE_STATS, 1, "Rendering stats").apply {
            isCheckable = true
            isChecked = statsEnabled
        }
        val sub = menu.addSubMenu("Terrain load mode")
        sub.add(MODE_GROUP, MENU_MODE_QUALITY, 0, "Quality (default)")
        sub.add(MODE_GROUP, MENU_MODE_BALANCED, 1, "Balanced")
        sub.add(MODE_GROUP, MENU_MODE_PERFORMANCE, 2, "Performance")
        sub.setGroupCheckable(MODE_GROUP, true, true)
        val checkedId = when (loadMode) {
            TerrainLoadMode.QUALITY -> MENU_MODE_QUALITY
            TerrainLoadMode.BALANCED -> MENU_MODE_BALANCED
            TerrainLoadMode.PERFORMANCE -> MENU_MODE_PERFORMANCE
        }
        sub.findItem(checkedId).isChecked = true
        return true
    }

    fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            MENU_TOGGLE_TERRAIN -> setTerrainEnabled(!terrainEnabled)
            MENU_TOGGLE_STATS -> setStatsEnabled(!statsEnabled)
            MENU_MODE_QUALITY -> setLoadMode(TerrainLoadMode.QUALITY)
            MENU_MODE_BALANCED -> setLoadMode(TerrainLoadMode.BALANCED)
            MENU_MODE_PERFORMANCE -> setLoadMode(TerrainLoadMode.PERFORMANCE)
            else -> return false
        }
        item.isChecked = when (item.itemId) {
            MENU_TOGGLE_TERRAIN -> terrainEnabled
            MENU_TOGGLE_STATS -> statsEnabled
            else -> true // exclusive mode group unchecks the others
        }
        return true
    }

    private fun setTerrainEnabled(on: Boolean) {
        terrainEnabled = on
        style?.setTerrain(if (on) Terrain(source = terrainSourceId, exaggeration = exaggeration) else null)
        toast("Terrain ${if (on) "on" else "off"}")
    }

    private fun setLoadMode(mode: TerrainLoadMode) {
        loadMode = mode
        map?.setTerrainLoadMode(mode)
        toast("Terrain load mode: ${mode.name}")
    }

    private fun setStatsEnabled(on: Boolean) {
        statsEnabled = on
        map?.enableRenderingStatsView(on)
        if (on && statsFont != null) {
            // Point the built-in stats layer at a font the hosting style actually serves,
            // otherwise its default (Open Sans) glyphs 404 and the HUD text stays blank.
            (style?.getLayer(STATS_LAYER_ID) as? SymbolLayer)
                ?.setProperties(textFont(arrayOf(statsFont)))
        }
        // The stats HUD sits top-right where the compass lives; move the compass aside
        // while the HUD is up so it does not cover the encoding/rendering-time lines.
        map?.uiSettings?.compassGravity = if (on) Gravity.TOP or Gravity.START else Gravity.TOP or Gravity.END
    }

    private fun applyLaunchExtras() {
        val extras = activity.intent?.extras ?: return
        if (extras.containsKey(EXTRA_TERRAIN)) setTerrainEnabled(extras.getBoolean(EXTRA_TERRAIN))
        when (extras.getString(EXTRA_MODE)?.lowercase()) {
            "quality" -> setLoadMode(TerrainLoadMode.QUALITY)
            "balanced" -> setLoadMode(TerrainLoadMode.BALANCED)
            "performance" -> setLoadMode(TerrainLoadMode.PERFORMANCE)
        }
        if (extras.containsKey(EXTRA_STATS)) setStatsEnabled(extras.getBoolean(EXTRA_STATS))
    }

    private fun toast(message: String) = Toast.makeText(activity, message, Toast.LENGTH_SHORT).show()

    companion object {
        const val ACTION_CMD = "org.maplibre.android.testapp.action.TERRAIN_CMD"
        const val EXTRA_CMD = "cmd"
        const val EXTRA_TERRAIN = "terrain"
        const val EXTRA_MODE = "mode"
        const val EXTRA_STATS = "stats"

        // Layer id the native RenderingStatsView creates (see gfx/rendering_stats.cpp).
        private const val STATS_LAYER_ID = "rendering-stats"

        private const val MODE_GROUP = 1
        private const val MENU_TOGGLE_TERRAIN = 1
        private const val MENU_TOGGLE_STATS = 5
        private const val MENU_MODE_QUALITY = 2
        private const val MENU_MODE_BALANCED = 3
        private const val MENU_MODE_PERFORMANCE = 4
    }
}
