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
 * FPV-style continuous flight over 3D terrain, on the same planet vector
 * basemap as [TerrainVectorMapActivity] (OpenFreeMap Liberty + Mapterhorn
 * raster-dem). The camera flies a looping path through the Alps at a low,
 * heavily-pitched viewpoint, aiming at a point ahead on the path so turns
 * stay smooth.
 *
 * It is meant as a moving stress test: DEM/drape tiles stream in over time,
 * so it surfaces loading smoothness, LOD-transition seams, culling as the
 * camera rotates, and label behaviour while travelling. Tap to pause/resume.
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
        // A slow heading rotation layered on the travel direction, to survey the horizon at the
        // high points (long period = gentle).
        val heading = bearingBetween(here, ahead) +
            ROTATION_AMPLITUDE_DEG * sin(distance / ROTATION_PERIOD_KM * 2.0 * Math.PI)
        return CameraPosition.Builder()
            .target(here)
            .zoom(zoom)
            .tilt(tilt)
            .bearing((heading % 360.0 + 360.0) % 360.0)
            .build()
    }

    /** Pitch tied to zoom: [PITCH_HIGH] toward the horizon at the zoomed-out overview (ZOOM_FAR),
     *  [PITCH_LOW] looking down on the close passes (ZOOM_CLOSE). Mirrors plan_flight.py. */
    private fun pitchAt(zoom: Double): Double {
        val t = ((zoom - ZOOM_CLOSE_REF) / (ZOOM_FAR_REF - ZOOM_CLOSE_REF)).coerceIn(0.0, 1.0)
        return PITCH_LOW + (PITCH_HIGH - PITCH_LOW) * t
    }

    /** Baked terrain-following zoom at [distance] along the loop (linear interp between bins). */
    private fun zoomAt(distance: Double): Double {
        val n = zoomProfile.size
        val f = (((distance % totalLength) / totalLength) + 1.0) % 1.0 * n // fractional bin, wrapped
        val i = f.toInt() % n
        val t = f - f.toInt()
        return zoomProfile[i] + (zoomProfile[(i + 1) % n] - zoomProfile[i]) * t
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
            14.357f, 14.563f, 14.322f, 13.549f, 12.596f, 11.573f, 10.600f, 9.790f, 9.237f, 9.005f, 9.121f, 9.573f,
            10.306f, 11.237f, 12.256f, 12.792f, 12.771f, 12.714f, 12.738f, 12.697f, 12.498f, 12.526f, 12.622f, 11.915f,
            10.913f, 10.036f, 9.388f, 9.044f, 9.044f, 9.387f, 10.034f, 10.910f, 11.913f, 12.456f, 12.464f, 12.508f,
            12.609f, 12.558f, 12.524f, 12.509f, 12.552f, 12.259f, 11.239f, 10.308f, 9.574f, 9.122f, 9.005f, 9.236f,
            9.788f, 10.598f, 11.571f, 12.546f, 12.873f, 12.999f, 12.986f, 13.122f, 13.102f, 13.246f, 13.162f, 12.598f,
            11.576f, 10.602f, 9.792f, 9.238f, 9.005f, 9.121f, 9.571f, 10.304f, 11.234f, 12.254f, 12.975f, 12.926f,
            12.979f, 13.158f, 13.119f, 13.668f, 13.395f, 12.930f, 11.918f, 10.915f, 10.038f, 9.390f, 9.045f, 9.043f,
            9.386f, 10.033f, 10.908f, 11.911f, 12.614f, 12.813f, 12.601f, 12.593f, 12.660f, 12.679f, 12.613f, 12.599f,
            12.261f, 11.241f, 10.310f, 9.576f, 9.123f, 9.005f, 9.235f, 9.787f, 10.596f, 11.568f, 12.591f, 12.585f,
            12.558f, 12.596f, 12.606f, 12.714f, 12.660f, 12.827f, 12.601f, 11.578f, 10.605f, 9.793f, 9.239f, 9.005f,
            9.120f, 9.570f, 10.302f, 11.232f, 12.251f, 13.151f, 13.235f, 13.122f, 13.227f, 13.282f, 13.068f, 12.965f,
            12.933f, 11.920f, 10.917f, 10.040f, 9.391f, 9.045f, 9.043f, 9.385f, 10.031f, 10.906f, 11.908f, 12.690f,
            12.686f, 12.662f, 12.612f, 12.777f, 12.713f, 12.808f, 12.763f, 12.263f, 11.244f, 10.312f, 9.577f, 9.123f,
            9.005f, 9.234f, 9.785f, 10.594f, 11.566f, 12.570f, 12.637f, 12.610f, 12.702f, 12.665f, 12.635f, 12.520f,
            12.803f, 12.603f, 11.580f, 10.607f, 9.795f, 9.240f, 9.005f, 9.119f, 9.568f, 10.300f, 11.230f, 12.249f
        )
        // Slow enough that DEM/drape tiles stream in ahead of the camera instead
        // of the near-field rendering empty (the loop is ~255 km).
        private const val FLIGHT_DURATION_MS = 420_000L
        private const val TERRAIN_EXAGGERATION = 1.2f

        // Pitch tied to zoom (see pitchAt): high toward the horizon/sky at the zoomed-out
        // overview (ZOOM_FAR_REF), looking down at terrain/buildings on the close passes
        // (ZOOM_CLOSE_REF). Max device pitch is 85. Keep in sync with plan_flight.py
        // (the above-ground zoom profile was planned with this pitch).
        private const val PITCH_LOW = 50.0
        private const val PITCH_HIGH = 74.0
        private const val ZOOM_FAR_REF = 9.0
        private const val ZOOM_CLOSE_REF = 15.0
        // A slow heading rotation (long period) layered on the travel direction, to gently
        // survey the horizon at the high overview points.
        private const val ROTATION_AMPLITUDE_DEG = 38.0
        private const val ROTATION_PERIOD_KM = 40.0
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
