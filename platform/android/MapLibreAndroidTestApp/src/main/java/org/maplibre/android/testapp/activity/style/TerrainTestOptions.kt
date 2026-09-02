package org.maplibre.android.testapp.activity.style

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Handler
import android.os.Looper
import android.view.Gravity
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import org.maplibre.android.camera.CameraUpdateFactory
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
 * mode_performance, stats_on|stats_off|stats_toggle, burst_on|burst_off|burst_toggle,
 * abovelog_on|abovelog_off|abovelog_toggle, exag_up|exag_down|exag_reset|
 * exag_sweep_on|exag_sweep_off|exag_sweep_toggle.
 * Initial state can also be set with launch extras: --ez terrain false --es mode performance
 * --ez stats true.
 *
 * `burst_*` drives a deterministic zoom-burst: it jumps the camera between a low and high
 * zoom every few seconds, forcing a large tile-build + drape re-render burst on each arrival.
 * Unlike smooth panning, this actually makes the [TerrainLoadMode] budgets bind, so an A/B run
 * (set a mode, start the burst, read the PERF-HUD worst-frame/jank log) shows their real trade
 * (Quality: one big hitch, instant detail; Performance: smoother, progressive pop-in).
 *
 * `exag_sweep_*` ramps `exaggeration` down and back up in steps, replacing the style's
 * terrain on every step. That is the one runtime terrain change none of the fixed-exaggeration
 * activities used to make, and it is how an orphaned terrain mesh per replacement went
 * unnoticed until it was found on iOS/Metal (see TERRAIN.md, Backend parity). Lowering is the
 * telling direction: stale surfaces taller than the live one occlude it.
 *
 * @param terrainSourceId the raster-dem source id the style wires terrain to; used to
 *   re-enable terrain after it was toggled off.
 * @param exaggeration terrain exaggeration the style starts at, and the base the runtime
 *   exaggeration controls move from.
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

    // Persists the load mode across activity/app reloads within the app, so a mode picked in
    // one 3D terrain test carries into the next launch until changed again (menu or adb).
    // Lazy: this controller is built as an activity field initializer (constructor time), when
    // the activity's base Context is not yet attached, so it must not be touched until onMapReady.
    private val prefs by lazy { activity.getSharedPreferences(PREFS, Context.MODE_PRIVATE) }

    var terrainEnabled = true
        private set
    var loadMode = TerrainLoadMode.QUALITY
        private set
    var statsEnabled = false
        private set
    var aboveGroundLog = false
        private set
    var exaggerationValue = exaggeration
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
                "burst_on" -> setBursting(true)
                "burst_off" -> setBursting(false)
                "burst_toggle" -> setBursting(!bursting)
                "abovelog_on" -> setAboveGroundLog(true)
                "abovelog_off" -> setAboveGroundLog(false)
                "abovelog_toggle" -> setAboveGroundLog(!aboveGroundLog)
                "exag_up" -> setExaggeration(exaggerationValue + EXAG_STEP)
                "exag_down" -> setExaggeration(exaggerationValue - EXAG_STEP)
                "exag_reset" -> setExaggeration(exaggeration)
                "exag_sweep_on" -> setExagSweeping(true)
                "exag_sweep_off" -> setExagSweeping(false)
                "exag_sweep_toggle" -> setExagSweeping(!exagSweeping)
                else -> return
            }
            activity.invalidateOptionsMenu()
        }
    }

    /** Call once the map and its style are ready. */
    fun onMapReady(map: MapLibreMap, style: Style) {
        this.map = map
        this.style = style
        // Restore the persisted load mode (prefs is safe to touch now the Context is attached),
        // then apply it before launch extras so an adb `--es mode ...` can still override it.
        loadMode = readPersistedLoadMode()
        map.setTerrainLoadMode(loadMode)
        applyLaunchExtras()
        ContextCompat.registerReceiver(
            activity, cmdReceiver, IntentFilter(ACTION_CMD), ContextCompat.RECEIVER_EXPORTED
        )
        // onCreateOptionsMenu already ran (before this async callback) with the default state,
        // so rebuild the menu to reflect the restored mode / any launch-extra overrides.
        activity.invalidateOptionsMenu()
    }

    fun onDestroy() {
        setBursting(false)
        setExagSweeping(false)
        runCatching { activity.unregisterReceiver(cmdReceiver) }
    }

    // Deterministic zoom-burst driver (adb `burst_*`). Each jump between the low and high zoom
    // forces a big tile-build + drape re-render burst, which is what makes the load-mode budgets
    // actually bind (smooth panning does not). Uses jumpTo (moveCamera) so each burst is a single
    // hard arrival, keeping the current center so it works from any activity's viewpoint.
    private val burstHandler = Handler(Looper.getMainLooper())
    private var bursting = false
    private var burstHigh = false
    private val burstRunnable = object : Runnable {
        override fun run() {
            val m = map ?: return
            burstHigh = !burstHigh
            // easeCamera (animated), not a jump: models interactive browsing-zoom - the scenario
            // the load-mode budgets target ("zoom-in is laggy"). During the animation tiles stream
            // in, so the per-frame budget governs how much builds/re-drapes each animation frame.
            m.easeCamera(
                CameraUpdateFactory.zoomTo(if (burstHigh) BURST_ZOOM_HIGH else BURST_ZOOM_LOW),
                BURST_EASE_MS
            )
            if (bursting) burstHandler.postDelayed(this, BURST_INTERVAL_MS)
        }
    }

    private fun setBursting(on: Boolean) {
        if (bursting == on) return
        bursting = on
        burstHandler.removeCallbacks(burstRunnable)
        if (on) burstHandler.post(burstRunnable)
        toast("Zoom burst ${if (on) "on" else "off"}")
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
        menu.add(0, MENU_TOGGLE_ABOVELOG, 2, "Above-ground log").apply {
            isCheckable = true
            isChecked = aboveGroundLog
        }
        menu.add(0, MENU_EXAG_UP, 3, "Exaggeration +$EXAG_STEP")
        menu.add(0, MENU_EXAG_DOWN, 4, "Exaggeration -$EXAG_STEP")
        menu.add(0, MENU_TOGGLE_EXAG_SWEEP, 5, "Exaggeration sweep").apply {
            isCheckable = true
            isChecked = exagSweeping
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
            MENU_TOGGLE_ABOVELOG -> setAboveGroundLog(!aboveGroundLog)
            MENU_MODE_QUALITY -> setLoadMode(TerrainLoadMode.QUALITY)
            MENU_MODE_BALANCED -> setLoadMode(TerrainLoadMode.BALANCED)
            MENU_MODE_PERFORMANCE -> setLoadMode(TerrainLoadMode.PERFORMANCE)
            MENU_EXAG_UP -> setExaggeration(exaggerationValue + EXAG_STEP)
            MENU_EXAG_DOWN -> setExaggeration(exaggerationValue - EXAG_STEP)
            MENU_TOGGLE_EXAG_SWEEP -> setExagSweeping(!exagSweeping)
            else -> return false
        }
        item.isChecked = when (item.itemId) {
            MENU_TOGGLE_TERRAIN -> terrainEnabled
            MENU_TOGGLE_STATS -> statsEnabled
            MENU_TOGGLE_ABOVELOG -> aboveGroundLog
            MENU_TOGGLE_EXAG_SWEEP -> exagSweeping
            else -> true // exclusive mode group unchecks the others
        }
        return true
    }

    private fun setTerrainEnabled(on: Boolean) {
        terrainEnabled = on
        style?.setTerrain(if (on) Terrain(source = terrainSourceId, exaggeration = exaggerationValue) else null)
        toast("Terrain ${if (on) "on" else "off"}")
    }

    // Replaces the style's terrain rather than toggling it, which is the path that used to
    // orphan the previous terrain mesh in the orchestrator.
    private fun setExaggeration(value: Float, notify: Boolean = true) {
        exaggerationValue = value.coerceIn(EXAG_MIN, EXAG_MAX)
        if (terrainEnabled) {
            style?.setTerrain(Terrain(source = terrainSourceId, exaggeration = exaggerationValue))
        }
        if (notify) toast("Exaggeration %.1f".format(exaggerationValue))
    }

    private val exagHandler = Handler(Looper.getMainLooper())
    private var exagSweeping = false
    private var exagRising = false
    private val exagSweepRunnable = object : Runnable {
        override fun run() {
            if (exagRising) {
                if (exaggerationValue >= EXAG_SWEEP_HIGH) exagRising = false
            } else if (exaggerationValue <= EXAG_SWEEP_LOW) {
                exagRising = true
            }
            setExaggeration(exaggerationValue + if (exagRising) EXAG_STEP else -EXAG_STEP, notify = false)
            if (exagSweeping) exagHandler.postDelayed(this, EXAG_SWEEP_INTERVAL_MS)
        }
    }

    private fun setExagSweeping(on: Boolean) {
        if (exagSweeping == on) return
        exagSweeping = on
        exagHandler.removeCallbacks(exagSweepRunnable)
        if (on) exagHandler.post(exagSweepRunnable)
        toast("Exaggeration sweep ${if (on) "on" else "off"}")
    }

    private fun setLoadMode(mode: TerrainLoadMode) {
        loadMode = mode
        prefs.edit().putString(KEY_LOAD_MODE, mode.name).apply()
        map?.setTerrainLoadMode(mode)
        toast("Terrain load mode: ${mode.name}")
    }

    // Debug: the native above-ground clearance log (ABOVE-GROUND ...). Off by default and gated in
    // the renderer, so the per-frame elevation sampling only runs while this is on.
    private fun setAboveGroundLog(on: Boolean) {
        aboveGroundLog = on
        map?.setDebugAboveGroundLog(on)
        toast("Above-ground log ${if (on) "on" else "off"}")
    }

    private fun readPersistedLoadMode(): TerrainLoadMode =
        prefs.getString(KEY_LOAD_MODE, null)
            ?.let { name -> runCatching { TerrainLoadMode.valueOf(name) }.getOrNull() }
            ?: TerrainLoadMode.QUALITY

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
        private const val PREFS = "terrain_test_options"
        private const val KEY_LOAD_MODE = "load_mode"

        // Zoom-burst A/B parameters: a typical browsing zoom range (not an extreme teleport),
        // animated over BURST_EASE_MS so tiles stream in during the animation - the interactive
        // case the budgets target. The interval leaves the scene time to settle between bursts.
        private const val BURST_ZOOM_LOW = 11.0
        private const val BURST_ZOOM_HIGH = 14.5
        private const val BURST_EASE_MS = 1200
        private const val BURST_INTERVAL_MS = 3000L
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
        private const val MENU_TOGGLE_ABOVELOG = 6
        private const val MENU_EXAG_UP = 7
        private const val MENU_EXAG_DOWN = 8
        private const val MENU_TOGGLE_EXAG_SWEEP = 9

        // Exaggeration sweep: a range wide enough that a stale surface left behind by one step
        // visibly towers over the next one down.
        private const val EXAG_STEP = 0.5f
        private const val EXAG_MIN = 0.0f
        private const val EXAG_MAX = 10.0f
        private const val EXAG_SWEEP_LOW = 1.0f
        private const val EXAG_SWEEP_HIGH = 4.0f
        private const val EXAG_SWEEP_INTERVAL_MS = 700L
    }
}
