package org.maplibre.android.testapp.activity.stability

import android.graphics.Color
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import androidx.core.graphics.drawable.toBitmap
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.maplibre.android.annotations.IconFactory
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.annotations.PolylineOptions
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.SupportMapFragment
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.animateCameraSuspend
import org.maplibre.android.testapp.utils.setStyleSuspend
import java.lang.reflect.Field
import java.util.logging.Logger
import kotlin.math.min
import kotlin.random.Random

class UserMap : SupportMapFragment() {
    private lateinit var map: MapLibreMap
    private lateinit var mapView: MapView
    private lateinit var bitmapDrawables: List<Drawable>

    companion object {
        private val LOG = Logger.getLogger(UserMap::class.java.name)
        private const val RANDOM_SEED = 42
        private val RANDOM = Random(RANDOM_SEED)

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

        private val PLACES = arrayListOf(
            LatLng(37.7749, -122.4194), // SF
            LatLng(38.9072, -77.0369), // DC
            LatLng(52.3702, 4.8952), // AMS
            LatLng(60.1699, 24.9384), // HEL
            LatLng(-13.1639, -74.2236), // AYA
            LatLng(52.5200, 13.4050), // BER
            LatLng(12.9716, 77.5946), // BAN
            LatLng(31.2304, 121.4737), // SHA
        )

        // controls the list of icons available
        // false -> use `ICONS` list
        // true -> use all bitmap drawables in `org.maplibre.android.R.drawable.*`
        private const val USE_ALL_DRAWABLES = false
        private val ICONS = arrayListOf(
            org.maplibre.android.R.drawable.maplibre_info_icon_default,
            org.maplibre.android.R.drawable.maplibre_user_icon,
            org.maplibre.android.R.drawable.maplibre_marker_icon_default,
            org.maplibre.android.R.drawable.maplibre_compass_icon,
            org.maplibre.android.R.drawable.maplibre_user_puck_icon,
        )

        private fun random(min: Double, max: Double): Double = min + RANDOM.nextDouble() * (max - min)
        private fun random(min: Int, max: Int): Int = RANDOM.nextInt(min, max)

        private fun <T> weightedRandom(values: List<Pair<T, Double>>): T {
            val cumulativeWeights = values.scan(0.0) { acc, value -> acc + value.second }
            var index =
                cumulativeWeights.binarySearch(random(0.00001, cumulativeWeights.last()))

            if (index < 0)
                index = -index - 1

            return values[index - 1].first
        }

        private fun randomSlowDuration(): Int = random(3000, 5000)
        private fun randomFastDuration(): Int = random(500, 1000)

        private fun randomPlacePoints(): Int = random(10, 20)
        private fun randomPlaceActions(): Int = random(10, 20)

        private fun randomLatLng(bounds: LatLngBounds): LatLng =
            LatLng(
                random(bounds.latitudeSouth, bounds.latitudeNorth),
                random(bounds.longitudeWest, bounds.longitudeEast)
            )

        private fun randomLatLng(map: MapLibreMap): LatLng =
            randomLatLng(map.projection.visibleRegion.latLngBounds)

        private fun randomZoom(): Double = random(14.0, 20.0)
        private fun randomTilt(): Double = random(0.0, 60.0)
        private fun randomBearing(): Double = random(0.0, 360.0)

        private fun randomAnnotationRemove(): Int = random(4, 8)
        private fun randomAnnotationAdd(): Int = random(2, 4)

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

        private fun randomLineWidth(): Float = random(1.0, 4.0).toFloat()

        private fun randomColor(): Int =
            Color.argb(
                RANDOM.nextInt(127, 256),
                RANDOM.nextInt(256),
                RANDOM.nextInt(256),
                RANDOM.nextInt(256)
            )
    }

    override fun onMapReady(maplibreMap: MapLibreMap) {
        super.onMapReady(maplibreMap)

        this.map = maplibreMap
        this.mapView = view as MapView

        LOG.info("UserMap seed $RANDOM_SEED")
        run()
    }

    private fun run() {
        if (USE_ALL_DRAWABLES) {
            bitmapDrawables =
                org.maplibre.android.R.drawable::class.java.fields.map { field: Field ->
                    ResourcesCompat.getDrawable(
                        this.resources,
                        field.getInt(null),
                        requireActivity().theme
                    )
                }.filterIsInstance<BitmapDrawable>()
        } else {
            bitmapDrawables = ICONS.map { id ->
                ResourcesCompat.getDrawable(
                    this.resources,
                    id,
                    requireActivity().theme
                )!!
            }
        }

        lifecycleScope.launch {
            // since the random generator was not reset each run will have different values
            while (true) {
                runStyleActions()
            }
        }
    }

    private suspend fun runStyleActions() {
        for (style in STYLES.shuffled(RANDOM)) {
            mapView.setStyleSuspend(style)
            runCameraActions()
        }
    }

    private suspend fun runCameraActions() {
        // camera actions with different weights
        // position updates are more frequent
        val actions = listOf(
            Pair({ CameraUpdateFactory.newLatLng(randomLatLng(map)) }, 2.0),
            Pair({ CameraUpdateFactory.zoomTo(randomZoom()) }, 1.0),
            Pair({ CameraUpdateFactory.tiltTo(randomTilt()) }, 1.0),
            Pair({ CameraUpdateFactory.bearingTo(randomBearing()) }, 1.0),
        )

        for (placeCenter in PLACES.shuffled(RANDOM)) {
            // update all values to simulate a long jump
            // (generated by the app, searching for a city/street, etc)
            val cameraPosition = CameraPosition
                .Builder()
                .target(placeCenter)
                .zoom(randomZoom())
                .tilt(randomTilt())
                .bearing(randomBearing())
                .build()

            map.animateCameraSuspend(
                CameraUpdateFactory.newCameraPosition(cameraPosition),
                randomSlowDuration()
            )

            repeat(randomPlacePoints()) {
                // perform a series of fast camera actions
                repeat(randomPlaceActions()) {
                    // update each value individually to simulate user interaction
                    map.animateCameraSuspend(weightedRandom(actions)(), randomFastDuration())
                }

                runAnnotationActions()
            }

            map.removeAnnotations()
        }
    }

    private fun runAnnotationActions() {
        // remove some annotations
        repeat(min(map.annotations.size, randomAnnotationRemove())) {
            map.removeAnnotation(map.annotations.random(RANDOM))
        }

        // add some annotations
        val bounds = map.projection.visibleRegion.latLngBounds

        // markers
        repeat(randomAnnotationAdd()) {
            // get a random drawable
            map.addMarker(
                MarkerOptions()
                    .position(randomLatLng(bounds))
                    .icon(
                        IconFactory.getInstance(requireContext()).fromBitmap(
                            bitmapDrawables
                                .random(RANDOM)
                                .mutate()
                                .apply { setTint(randomColor()) }
                                .toBitmap(),
                        )
                    )
            )
        }

        // polylines
        repeat(randomAnnotationAdd()) {
            map.addPolyline(
                PolylineOptions()
                    .color(randomColor())
                    .width(randomLineWidth())
                    .addAll(randomPolyPoints(bounds)),
            )
        }

        // polygons
        repeat(randomAnnotationAdd()) {
            map.addPolygon(
                PolygonOptions()
                    .fillColor(randomColor())
                    .strokeColor(randomColor())
                    .addAll(randomPolyPoints(bounds)),
            )
        }
    }
}

class UserMapActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        supportFragmentManager
            .beginTransaction()
            .replace(android.R.id.content, UserMap())
            .commit()
    }
}
