package org.maplibre.android.testapp.activity.fragment

import android.graphics.Color
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import androidx.core.graphics.drawable.toBitmap
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import org.maplibre.android.annotations.IconFactory
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.annotations.PolylineOptions
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdate
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import java.lang.reflect.Field
import kotlin.coroutines.resume
import kotlin.math.min
import kotlin.random.Random

class LongRunningActivity : AppCompatActivity() {
    private lateinit var mapView1: MapView
    private lateinit var mapView2: MapView
    private lateinit var bitmapDrawables: List<Drawable>

    // config
    companion object {
        private const val RANDOM_SEED = 42
        private val RANDOM = Random(RANDOM_SEED)

        // each run the time scale gets updated
        private const val RUN_COUNT = 5
        private const val RUN_TIME_SCALE_FACTOR = 1.5
        private var TIME_SCALE = 1.0

        private val STYLES = arrayListOf(
            TestStyles.DEMOTILES,
            TestStyles.AMERICANA,
            TestStyles.OPENFREEMAP_LIBERTY,
            TestStyles.OPENFREEMAP_BRIGHT,
            TestStyles.AWS_OPEN_DATA_STANDARD_LIGHT,
            TestStyles.PROTOMAPS_LIGHT,
            TestStyles.PROTOMAPS_DARK,
            TestStyles.PROTOMAPS_GRAYSCALE,
            TestStyles.PROTOMAPS_WHITE,
            TestStyles.PROTOMAPS_BLACK,
        )

        private val PLACES = arrayOf(
            LatLng(37.7749, -122.4194), // SF
            LatLng(38.9072, -77.0369), // DC
            LatLng(52.3702, 4.8952), // AMS
            LatLng(60.1699, 24.9384), // HEL
            LatLng(-13.1639, -74.2236), // AYA
            LatLng(52.5200, 13.4050), // BER
            LatLng(12.9716, 77.5946), // BAN
            LatLng(31.2304, 121.4737) // SHA
        )

        private const val USE_ALL_DRAWABLES = false
        private val ICONS = arrayListOf(
            org.maplibre.android.R.drawable.maplibre_info_icon_default,
            org.maplibre.android.R.drawable.maplibre_user_icon,
            org.maplibre.android.R.drawable.maplibre_marker_icon_default,
            org.maplibre.android.R.drawable.maplibre_compass_icon,
            org.maplibre.android.R.drawable.maplibre_user_puck_icon
        )

        private fun random(min: Double, max: Double): Double {
            return min + RANDOM.nextDouble() * (max - min)
        }

        private fun random(min: Int, max: Int): Int {
            return RANDOM.nextInt(min, max)
        }

        private fun randomSlowDuration(): Double {
            return random(3000.0, 5000.0) * TIME_SCALE
        }

        private fun randomFastDuration(): Double {
            return random(500.0, 1000.0) * TIME_SCALE
        }

        private fun randomPlacePoints(): Int {
            return random(10, 20)
        }

        private fun randomPlaceActions(): Int {
            return random(10, 20)
        }

        private fun randomLatLng(bounds: LatLngBounds): LatLng {
            return LatLng(
                random(bounds.latitudeSouth, bounds.latitudeNorth),
                random(bounds.longitudeWest, bounds.longitudeEast)
            )
        }

        private fun randomLatLng(map: MapLibreMap): LatLng {
            return randomLatLng(map.projection.visibleRegion.latLngBounds)
        }

        private fun randomZoom(): Double {
            return random(14.0, 20.0)
        }

        private fun randomTilt(): Double {
            return random(0.0, 60.0)
        }

        private fun randomBearing(): Double {
            return random(0.0, 360.0)
        }

        private fun randomAnnotationRemove(): Int {
            return random(4, 8)
        }

        private fun randomAnnotationAdd(): Int {
            return random(2, 4)
        }

        private fun randomPolyPoints(bounds: LatLngBounds): List<LatLng> {
            val boundMultiplier = 0.25
            val lat = random(bounds.latitudeSouth, bounds.latitudeNorth)
            val lng = random(bounds.longitudeWest, bounds.longitudeEast)
            val latRange = bounds.latitudeSpan * boundMultiplier / 2.0
            val lngRange = bounds.longitudeSpan * boundMultiplier / 2.0

            return List(random(3, 15)) {
                LatLng(
                    lat + random(-latRange, latRange),
                    lng + random(-lngRange, lngRange)
                )
            }
        }

        private fun randomColor(): Int {
            return Color.argb(
                RANDOM.nextInt(127,256),
                RANDOM.nextInt(256),
                RANDOM.nextInt(256),
                RANDOM.nextInt(256)
            )
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_long_running_maps)

        if (USE_ALL_DRAWABLES) {
            bitmapDrawables =
                org.maplibre.android.R.drawable::class.java.fields.map { field: Field ->
                    ResourcesCompat.getDrawable(
                        this.resources,
                        field.getInt(null),
                        theme
                    )
                }.filterIsInstance<BitmapDrawable>()
        } else {
            bitmapDrawables = ICONS.map { id ->
                ResourcesCompat.getDrawable(
                    this.resources,
                    id,
                    theme
                )!!
            }
        }

        mapView1 = supportFragmentManager.findFragmentById(R.id.map1)!!.view as MapView
        mapView2 = supportFragmentManager.findFragmentById(R.id.map2)!!.view as MapView

        mapView1.getMapAsync { map: MapLibreMap -> run(map, mapView1) }

        // TODO setup this for slow navigation
        mapView2.getMapAsync { map: MapLibreMap -> map.setStyle(TestStyles.DEMOTILES) }
    }

    private fun run(map: MapLibreMap, mapView: MapView) {
        lifecycleScope.launch {

            // since the random generator was not reset each run will have different values
            repeat(RUN_COUNT) {
                TIME_SCALE *= RUN_TIME_SCALE_FACTOR

                runStyleActions(map, mapView)
            }
        }
    }

    private suspend fun runStyleActions(map: MapLibreMap, mapView: MapView) {
        for (style in STYLES.shuffled(RANDOM)) {
            println("Running test using $style at $TIME_SCALE speed")
            mapView.setStyleSuspend(style)
            runCameraActions(map)
        }
    }

    private suspend fun runCameraActions(map: MapLibreMap) {
        for (placeCenter in PLACES) {
            // update all values to simulate a long jump
            // (generated by the app, searching for a city/street, etc)
            val cameraPosition = CameraPosition.Builder()
                .target(placeCenter)
                .zoom(randomZoom())
                .tilt(randomTilt())
                .bearing(randomBearing())
                .build()

            map.animateCameraSuspend(
                CameraUpdateFactory.newCameraPosition(cameraPosition),
                randomSlowDuration()
            )

            val actions = arrayOf(
                { CameraUpdateFactory.newLatLng(randomLatLng(map)) },
                { CameraUpdateFactory.zoomTo(randomZoom()) },
                { CameraUpdateFactory.tiltTo(randomTilt()) },
                { CameraUpdateFactory.bearingTo(randomBearing()) },
            )

            repeat(randomPlacePoints()) {
                // perform a series of fast camera actions
                repeat(randomPlaceActions()) {
                    // update each value individually to simulate user interaction
                    map.animateCameraSuspend(
                        actions.random()(),
                        randomFastDuration()
                    )
                }

                // TODO add geojsons?

                runAnnotationActions(map)
            }

            map.removeAnnotations()
        }
    }

    private suspend fun runAnnotationActions(map: MapLibreMap) {
        // TODO add/remove annotations (random?) -> use android `markers` (deprecated) or annotation plugin?

        // remove some annotations
        repeat(min(map.annotations.size, randomAnnotationRemove())) {
            map.removeAnnotation(map.annotations.random(RANDOM))
        }

        // add some annotations
        val bounds = map.projection.visibleRegion.latLngBounds

        // markers
        repeat(randomAnnotationAdd()) {
            // get a random drawable
            map.addMarker(MarkerOptions()
                .position(randomLatLng(bounds))
                .icon(IconFactory.getInstance(this).fromBitmap(
                    bitmapDrawables.random().mutate().apply { setTint(randomColor()) }.toBitmap()
                ))
            )
        }

        // polylines
        repeat(randomAnnotationAdd()) {
            map.addPolyline(PolylineOptions()
                .color(randomColor()).
                addAll(randomPolyPoints(bounds))
            )
        }

        // polygons
        repeat(randomAnnotationAdd()) {
            map.addPolygon(PolygonOptions()
                .fillColor(randomColor())
                .strokeColor(randomColor())
                .addAll(randomPolyPoints(bounds))
            )
        }
    }
}

suspend fun MapView.setStyleSuspend(styleUrl: String): Unit =
    suspendCancellableCoroutine { continuation ->
        var listener: MapView.OnDidFinishLoadingStyleListener? = null

        var resumed = false
        listener = MapView.OnDidFinishLoadingStyleListener {
            if (!resumed) {
                resumed = true
                listener?.let { removeOnDidFinishLoadingStyleListener(it) }
                continuation.resume(Unit)
            }
        }

        addOnDidFinishLoadingStyleListener(listener)
        getMapAsync { map -> map.setStyle(styleUrl) }

        continuation.invokeOnCancellation {
            removeOnDidFinishLoadingStyleListener(listener)
        }

    }

suspend fun MapLibreMap.animateCameraSuspend(
    cameraUpdate: CameraUpdate,
    durationMs: Double
): Unit = suspendCancellableCoroutine { continuation ->
    animateCamera(cameraUpdate, durationMs.toInt(), object : CancelableCallback {
        var resumed = false

        override fun onCancel() {
            continuation.cancel()
        }

        override fun onFinish() {
            if (!resumed) {
                resumed = true
                continuation.resume(Unit)
            }
        }
    })
}
