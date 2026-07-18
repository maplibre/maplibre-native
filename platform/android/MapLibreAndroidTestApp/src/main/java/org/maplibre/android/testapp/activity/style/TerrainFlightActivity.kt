package org.maplibre.android.testapp.activity.style

import android.animation.ValueAnimator
import android.os.Bundle
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

    // Looping flight path over the Ötztal/Stubai Alps and the Dolomites, chosen
    // for dramatic relief. Closed loop: the last point flies back to the first.
    private val flightPath = listOf(
        LatLng(47.09, 11.02), // Ötztal Alps
        LatLng(47.01, 11.34), // Brenner
        LatLng(46.90, 11.44), // Sterzing / Vipiteno
        LatLng(46.70, 11.63), // toward Bolzano
        LatLng(46.51, 11.79), // Sella group, Dolomites
        LatLng(46.54, 12.05), // Marmolada area
        LatLng(46.72, 11.94), // back north over the Puster valley
        LatLng(46.93, 11.60), // Zillertal Alps
        LatLng(47.05, 11.28)  // toward Stubai, closing the loop
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
                startFlight()
            }
            mapView.setOnClickListener { toggleFlight() }
        }
    }

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
        // Layer independent oscillations on the path-following camera: a heading
        // sweep (look left/right of travel), a pitch bob, and a zoom dive/climb,
        // on distinct periods so the loop covers a varied mix of the three and
        // exercises the tile cover / culling / label placement across heights.
        val heading = bearingBetween(here, ahead) +
            ROTATION_AMPLITUDE_DEG * sin(distance / ROTATION_PERIOD_KM * 2.0 * Math.PI)
        val tilt = FLIGHT_TILT + TILT_AMPLITUDE_DEG * sin(distance / TILT_PERIOD_KM * 2.0 * Math.PI)
        // Start at the high-altitude (low-zoom) end of the sweep (-cos is +1 at
        // distance 0), so the flight opens well above the terrain and only dives
        // to the low FPV zoom later - the camera is sea-level-anchored, so opening
        // low over a peak would start inside the mountain (TERRAIN.md Phase 4).
        val zoom = ZOOM_MID - ZOOM_AMPLITUDE * cos(distance / ZOOM_PERIOD_KM * 2.0 * Math.PI)
        return CameraPosition.Builder()
            .target(here)
            .zoom(zoom)
            .tilt(tilt)
            .bearing((heading % 360.0 + 360.0) % 360.0)
            .build()
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

        // The camera dives and climbs through a zoom range as it flies: some
        // artifacts (LOD seams, culling, label placement, and camera-in-terrain
        // clipping) only appear at certain heights, so one loop sweeps the range.
        // Note MapLibre's camera altitude is sea-level-anchored (not terrain-aware),
        // so the low (zoomed-in) end can pass *through* peaks until the
        // terrain-anchored camera (TERRAIN.md Phase 4) lands.
        private const val ZOOM_MID = 12.5
        private const val ZOOM_AMPLITUDE = 1.5 // sweeps ~11.0 (overview) .. 14.0 (low FPV)
        // Periods are short (the flight is slow, ~0.36 km/s) so a full dive/climb
        // and heading sweep are visible within a ~70 s window.
        private const val ZOOM_PERIOD_KM = 24.0
        private const val FLIGHT_TILT = 55.0
        // Slow enough that DEM/drape tiles stream in ahead of the camera instead
        // of the near-field rendering empty (the loop is ~150 km).
        private const val FLIGHT_DURATION_MS = 420_000L
        private const val TERRAIN_EXAGGERATION = 1.2f

        // Heading sweep (look left/right of travel) and pitch bob layered on the
        // path-following camera, on periods distinct from the zoom sweep so the
        // flight covers a varied mix of heading/pitch/altitude as it travels.
        private const val ROTATION_AMPLITUDE_DEG = 35.0
        private const val ROTATION_PERIOD_KM = 16.0
        private const val TILT_AMPLITUDE_DEG = 8.0
        private const val TILT_PERIOD_KM = 20.0
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
        mapView.onDestroy()
    }
}
