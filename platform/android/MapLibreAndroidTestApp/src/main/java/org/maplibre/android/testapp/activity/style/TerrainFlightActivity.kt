package org.maplibre.android.testapp.activity.style

import android.animation.ValueAnimator
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.animation.LinearInterpolator
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.HillshadeLayer
import org.maplibre.android.style.layers.PropertyFactory.hillshadeExaggeration
import org.maplibre.android.style.layers.PropertyFactory.hillshadeMethod
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.RasterDemSource
import org.maplibre.android.style.terrain.Terrain
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.sin

/**
 * Drone-style continuous flight over 3D terrain, on the same planet vector basemap as
 * [TerrainVectorMapActivity] (OpenFreeMap Liberty + Mapterhorn raster-dem). Starts low over
 * Innsbruck then tours the surrounding Tyrol Alps: a baked, terrain-following zoom profile
 * (planned offline from real elevations - see zoomProfile) dives close over low/dense terrain
 * (valley towns, buildings) and pulls back to a wide overview over the peaks, keeping the
 * sea-level-anchored camera above the terrain. At each overview it ramps pitch toward the
 * horizon and sweeps ~360 deg to survey the panorama (and the black sky/background beyond it).
 *
 * It is meant as a moving stress test: DEM/drape tiles stream in over time, so it surfaces
 * loading smoothness, LOD-transition seams, culling as the camera rotates, and label behaviour
 * while travelling - and it exercises the full range of zoom, altitude, pitch, and rotation.
 * Tap to pause/resume.
 */
class TerrainFlightActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var animator: ValueAnimator? = null
    // OpenFreeMap serves Noto glyphs, so the stats HUD text uses Noto Sans Regular.
    private val options = TerrainTestOptions(this, SOURCE_ID_TERRAIN, TERRAIN_EXAGGERATION, "Noto Sans Regular")

    // Drone tour: starts low over Innsbruck (our default scene) then tours the surrounding
    // Ötztal/Stubai Alps and the Dolomites. Closed loop: the last point flies back to the first.
    // zoomProfile below is planned for THIS path (and the tilt/bearing below); keep them in sync.
    private val flightPath = listOf(
        LatLng(47.2654, 11.3927), // Innsbruck (low start over the city)
        LatLng(47.09, 11.02),     // Ötztal Alps
        LatLng(47.01, 11.34),     // Brenner
        LatLng(46.90, 11.44),     // Sterzing / Vipiteno
        LatLng(46.70, 11.63),     // toward Bolzano
        LatLng(46.51, 11.79),     // Sella group, Dolomites
        LatLng(46.54, 12.05),     // Marmolada area
        LatLng(46.72, 11.94),     // back north over the Puster valley
        LatLng(46.93, 11.60),     // Zillertal Alps
        LatLng(47.05, 11.28)      // toward Stubai, closing the loop back to Innsbruck
    )

    // Distance (in the same units as [haversine]) to look ahead along the path
    // for the camera's aim point; larger = gentler turns.
    private val lookAheadKm = 4.0

    private lateinit var cumulative: DoubleArray
    private var totalLength = 0.0

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        buildPathMetrics()
        mapView.getMapAsync { map ->
            maplibreMap = map
            map.setMaxPitchPreference(MapLibreConstants.MAXIMUM_PITCH_LIMIT.toDouble())
            map.uiSettings.setAllGesturesEnabled(false)
            map.cameraPosition = cameraAt(0.0)
            map.setStyle(Style.Builder().fromUri(TestStyles.OPENFREEMAP_LIBERTY)) { style ->
                addTerrain(style)
                options.onMapReady(map, style)
                startFlight()
            }
            mapView.setOnClickListener { toggleFlight() }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean = options.onCreateOptionsMenu(menu)

    override fun onOptionsItemSelected(item: MenuItem): Boolean =
        options.onOptionsItemSelected(item) || super.onOptionsItemSelected(item)

    private fun addTerrain(style: Style) {
        style.addSource(RasterDemSource(SOURCE_ID_HILLSHADE, DEM_TILEJSON))
        style.addSource(RasterDemSource(SOURCE_ID_TERRAIN, DEM_TILEJSON))

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

        style.setTerrain(Terrain(source = SOURCE_ID_TERRAIN, exaggeration = TERRAIN_EXAGGERATION))
    }

    private fun startFlight() {
        animator?.cancel()
        animator = ValueAnimator.ofFloat(0f, 1f).apply {
            duration = FLIGHT_DURATION_MS
            repeatCount = ValueAnimator.INFINITE
            interpolator = LinearInterpolator()
            addUpdateListener { a ->
                if (!::maplibreMap.isInitialized) return@addUpdateListener
                val distance = (a.animatedValue as Float).toDouble() * totalLength
                maplibreMap.moveCamera(CameraUpdateFactory.newCameraPosition(cameraAt(distance)))
            }
            start()
        }
    }

    private fun toggleFlight() {
        val a = animator ?: return
        if (a.isPaused) a.resume() else a.pause()
    }

    /** Camera at [distance] along the loop, aiming [lookAheadKm] further ahead. */
    private fun cameraAt(distance: Double): CameraPosition {
        val here = pointAtDistance(distance)
        val ahead = pointAtDistance(distance + lookAheadKm)
        // Zoom follows the baked terrain-following profile (zoomAt): it dives close over low/
        // dense terrain (the Innsbruck start, valley towns) and pulls back over peaks, keeping
        // the sea-level-anchored eye above the terrain (TERRAIN.md Phase 4). The profile was
        // planned offline from real elevations so no live elevation query is needed at runtime.
        val zoom = zoomAt(distance)
        // Pitch is tied to the zoom: high (toward the horizon/sky background) at the zoomed-out
        // overview points, looking down at terrain/buildings on the close passes. The above-ground
        // profile was planned WITH this pitch, so keep pitchAt in sync with plan_flight.py.
        val tilt = pitchAt(zoom)
        // Heading = travel direction + the baked bearing offset: a gentle drift on the close/mid
        // passes, sweeping ~360 deg during each overview dwell so the high-pitch overview surveys
        // the full panorama + sky.
        val heading = bearingBetween(here, ahead) + bearingAt(distance)
        return CameraPosition.Builder()
            .target(here)
            .zoom(zoom)
            .tilt(tilt)
            .bearing((heading % 360.0 + 360.0) % 360.0)
            .build()
    }

    /** Pitch tied to zoom: stays [PITCH_LOW] on the mid/close passes (safe altitude), then ramps to
     *  [PITCH_HIGH] toward the horizon/sky only at the deep, high-clearance overview (zoom < KNEE).
     *  Nonlinear so max pitch happens only where it is safe. Mirrors plan_flight.py. */
    private fun pitchAt(zoom: Double): Double {
        val f = ((PITCH_KNEE_ZOOM - zoom) / (PITCH_KNEE_ZOOM - ZOOM_FAR_REF)).coerceIn(0.0, 1.0)
        return PITCH_LOW + (PITCH_HIGH - PITCH_LOW) * f
    }

    /** Baked terrain-following zoom at [distance] along the loop (linear interp between bins). */
    private fun zoomAt(distance: Double): Double {
        val n = zoomProfile.size
        val f = (((distance % totalLength) / totalLength) + 1.0) % 1.0 * n // fractional bin, wrapped
        val i = f.toInt() % n
        val t = f - f.toInt()
        return zoomProfile[i] + (zoomProfile[(i + 1) % n] - zoomProfile[i]) * t
    }

    /** Baked cumulative bearing offset (deg) at [distance]. The profile is a whole number of turns
     *  over the loop, so past the last bin it continues to BEARING_LOOP_DEG (== the wrap to 0). */
    private fun bearingAt(distance: Double): Double {
        val n = bearingOffsetProfile.size
        val f = (((distance % totalLength) / totalLength) + 1.0) % 1.0 * n
        val i = f.toInt() % n
        val t = f - f.toInt()
        val cur = bearingOffsetProfile[i].toDouble()
        val nxt = if (i + 1 < n) bearingOffsetProfile[i + 1].toDouble() else BEARING_LOOP_DEG
        return cur + (nxt - cur) * t
    }

    private fun buildPathMetrics() {
        cumulative = DoubleArray(flightPath.size + 1)
        for (i in flightPath.indices) {
            val next = flightPath[(i + 1) % flightPath.size]
            cumulative[i + 1] = cumulative[i] + haversine(flightPath[i], next)
        }
        totalLength = cumulative[flightPath.size]
    }

    /** Point at [distance] km along the closed loop (wraps around). */
    private fun pointAtDistance(distance: Double): LatLng {
        val d = ((distance % totalLength) + totalLength) % totalLength
        var seg = 0
        while (seg < flightPath.size && cumulative[seg + 1] < d) seg++
        val segStart = cumulative[seg]
        val segLen = cumulative[seg + 1] - segStart
        val f = if (segLen > 0) (d - segStart) / segLen else 0.0
        val a = flightPath[seg]
        val b = flightPath[(seg + 1) % flightPath.size]
        return LatLng(a.latitude + (b.latitude - a.latitude) * f, a.longitude + (b.longitude - a.longitude) * f)
    }

    companion object {
        private const val DEM_TILEJSON = "https://tiles.mapterhorn.com/tilejson.json"
        private const val SOURCE_ID_HILLSHADE = "mapterhorn"
        private const val SOURCE_ID_TERRAIN = "mapterhorn-terrain"
        private const val LAYER_ID_HILLSHADE = "mapterhorn-hillshade"

        // Terrain-following zoom profile (180 bins over the loop, indexed by loop fraction),
        // PLANNED OFFLINE from real elevations (tileserver-gl elevation API) so the app needs
        // no live elevation query. For each bin the camera altitude the sea-level-anchored eye
        // reaches at a given zoom (calibrated c2c) is held >= the terrain (exaggeration 1.2)
        // over the eye's rear footprint + a terrain-proportional clearance (~370 m low, more over
        // peaks to absorb dataset differences). Result: it dives close (zoom ~14)
        // over low/dense terrain - the Innsbruck start and valley towns - and pulls back
        // (zoom 9, wide overview) over the high peaks, staying above ground. MapLibre's camera
        // is sea-level-anchored (TERRAIN.md Phase 4); a true terrain-anchored camera would let
        // this be a constant-clearance path instead of a baked per-position zoom. To regenerate
        // if the path/tilt/bearing change: sample a terrain-elevation endpoint over each bin's
        // eye footprint (windowed max) and solve zoom for altitude = terrain + clearance.
        private val zoomProfile = floatArrayOf(
            14.315f, 14.471f, 14.398f, 14.388f, 12.924f, 11.339f, 9.830f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f,
            9.375f, 10.817f, 12.397f, 13.086f, 12.962f, 12.969f, 12.988f, 12.926f, 12.820f, 12.786f, 12.860f, 11.869f,
            10.314f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 10.311f, 11.865f, 12.777f, 12.725f, 12.779f,
            12.739f, 12.736f, 12.716f, 12.805f, 12.818f, 12.401f, 10.821f, 9.378f, 9.000f, 9.000f, 9.000f, 9.000f,
            9.000f, 9.827f, 11.335f, 12.920f, 13.167f, 13.233f, 13.236f, 13.223f, 13.328f, 13.442f, 13.191f, 12.923f,
            11.342f, 9.834f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 9.372f, 10.813f, 12.393f, 13.169f, 13.173f,
            13.156f, 13.261f, 13.179f, 13.781f, 13.745f, 13.397f, 11.873f, 10.318f, 9.000f, 9.000f, 9.000f, 9.000f,
            9.000f, 9.000f, 10.307f, 11.861f, 12.941f, 12.966f, 12.858f, 12.859f, 12.830f, 12.826f, 12.817f, 12.864f,
            12.404f, 10.824f, 9.381f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 9.824f, 11.331f, 12.834f, 12.852f,
            13.059f, 12.758f, 12.974f, 12.926f, 12.901f, 12.957f, 12.931f, 11.346f, 9.837f, 9.000f, 9.000f, 9.000f,
            9.000f, 9.000f, 9.368f, 10.810f, 12.390f, 13.400f, 13.367f, 13.388f, 13.389f, 13.333f, 13.186f, 13.363f,
            13.033f, 11.876f, 10.321f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 10.304f, 11.858f, 12.884f,
            12.846f, 12.920f, 12.791f, 12.825f, 12.903f, 12.944f, 12.838f, 12.408f, 10.828f, 9.384f, 9.000f, 9.000f,
            9.000f, 9.000f, 9.000f, 9.820f, 11.327f, 12.828f, 12.764f, 12.909f, 12.947f, 12.742f, 12.750f, 12.979f,
            13.019f, 12.935f, 11.350f, 9.840f, 9.000f, 9.000f, 9.000f, 9.000f, 9.000f, 9.365f, 10.806f, 12.386f
        )

        // Bearing offset (deg) added to the travel heading, baked per bin: a slow drift on the
        // close/mid passes, sweeping ~360 deg during each overview dwell so the high-pitch overview
        // surveys the full panorama + sky. Normalized to whole turns so it loops seamlessly.
        private val bearingOffsetProfile = floatArrayOf(
            0.0f, 3.0f, 5.9f, 8.9f, 11.8f, 14.8f, 17.7f, 32.7f, 106.5f, 180.3f, 254.1f, 327.9f,
            401.7f, 448.9f, 451.8f, 454.8f, 457.7f, 460.7f, 463.6f, 466.6f, 469.5f, 472.5f, 475.4f, 478.4f,
            481.4f, 484.3f, 558.1f, 631.9f, 705.7f, 779.5f, 853.3f, 927.1f, 930.0f, 933.0f, 935.9f, 938.9f,
            941.8f, 944.8f, 947.7f, 950.7f, 953.6f, 956.6f, 959.5f, 962.5f, 1009.5f, 1083.3f, 1157.1f, 1230.9f,
            1304.6f, 1378.4f, 1393.6f, 1396.6f, 1399.5f, 1402.5f, 1405.5f, 1408.4f, 1411.4f, 1414.3f, 1417.3f, 1420.2f,
            1423.2f, 1426.1f, 1440.8f, 1514.6f, 1588.4f, 1662.2f, 1736.0f, 1809.8f, 1857.2f, 1860.2f, 1863.1f, 1866.1f,
            1869.0f, 1872.0f, 1874.9f, 1877.9f, 1880.8f, 1883.8f, 1886.7f, 1889.7f, 1892.6f, 1966.4f, 2040.2f, 2114.0f,
            2187.8f, 2261.6f, 2335.4f, 2338.3f, 2341.3f, 2344.2f, 2347.2f, 2350.1f, 2353.1f, 2356.0f, 2359.0f, 2361.9f,
            2364.9f, 2367.9f, 2370.8f, 2417.6f, 2491.4f, 2565.2f, 2639.0f, 2712.8f, 2786.6f, 2802.0f, 2804.9f, 2807.9f,
            2810.8f, 2813.8f, 2816.7f, 2819.7f, 2822.6f, 2825.6f, 2828.5f, 2831.5f, 2834.4f, 2848.9f, 2922.7f, 2996.5f,
            3070.3f, 3144.1f, 3217.9f, 3265.6f, 3268.6f, 3271.5f, 3274.5f, 3277.4f, 3280.4f, 3283.3f, 3286.3f, 3289.2f,
            3292.2f, 3295.1f, 3298.1f, 3301.0f, 3374.8f, 3448.6f, 3522.4f, 3596.2f, 3670.0f, 3743.8f, 3746.7f, 3749.7f,
            3752.6f, 3755.6f, 3758.5f, 3761.5f, 3764.4f, 3767.4f, 3770.4f, 3773.3f, 3776.3f, 3779.2f, 3825.8f, 3899.6f,
            3973.4f, 4047.2f, 4121.0f, 4194.8f, 4210.5f, 4213.4f, 4216.4f, 4219.3f, 4222.3f, 4225.2f, 4228.2f, 4231.1f,
            4234.1f, 4237.0f, 4240.0f, 4242.9f, 4257.2f, 4331.0f, 4404.8f, 4478.6f, 4552.4f, 4626.2f, 4674.1f, 4677.0f
        )
        // Slow enough that DEM/drape tiles stream in ahead of the camera instead
        // of the near-field rendering empty (the loop is ~255 km, so ~0.47 km/s here).
        private const val FLIGHT_DURATION_MS = 540_000L
        private const val TERRAIN_EXAGGERATION = 1.2f

        // Pitch tied to zoom (see pitchAt): stays PITCH_LOW on mid/close passes, then ramps to
        // PITCH_HIGH (near-horizontal => sky) only at the deep overview below PITCH_KNEE_ZOOM,
        // where clearance is huge. Max device pitch is 85. Keep in sync with plan_flight.py
        // (the above-ground zoom profile was planned with this pitch).
        private const val PITCH_LOW = 52.0
        private const val PITCH_HIGH = 85.0
        private const val PITCH_KNEE_ZOOM = 10.5
        private const val ZOOM_FAR_REF = 9.0
        // Total cumulative spin of bearingOffsetProfile over the loop (whole turns, so bearingAt
        // wraps seamlessly). Update if the bearing profile is regenerated with different params.
        private const val BEARING_LOOP_DEG = 4680.0
        private const val EARTH_RADIUS_KM = 6371.0

        /** Great-circle distance between two points, in kilometres. */
        private fun haversine(a: LatLng, b: LatLng): Double {
            val dLat = Math.toRadians(b.latitude - a.latitude)
            val dLon = Math.toRadians(b.longitude - a.longitude)
            val lat1 = Math.toRadians(a.latitude)
            val lat2 = Math.toRadians(b.latitude)
            val h = sin(dLat / 2) * sin(dLat / 2) +
                sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2)
            return 2 * EARTH_RADIUS_KM * atan2(Math.sqrt(h), Math.sqrt(1 - h))
        }

        /** Initial bearing (degrees, 0..360) from [a] to [b]. */
        private fun bearingBetween(a: LatLng, b: LatLng): Double {
            val lat1 = Math.toRadians(a.latitude)
            val lat2 = Math.toRadians(b.latitude)
            val dLon = Math.toRadians(b.longitude - a.longitude)
            val y = sin(dLon) * cos(lat2)
            val x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon)
            return (Math.toDegrees(atan2(y, x)) + 360.0) % 360.0
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
        animator?.cancel()
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
        animator?.cancel()
        options.onDestroy()
        mapView.onDestroy()
    }
}
