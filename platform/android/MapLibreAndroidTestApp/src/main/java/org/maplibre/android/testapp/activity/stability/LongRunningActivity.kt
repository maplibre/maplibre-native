package org.maplibre.android.testapp.activity.stability

import android.annotation.SuppressLint
import android.app.ActivityManager
import android.graphics.Color
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.location.Location
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import androidx.core.graphics.drawable.toBitmap
import androidx.lifecycle.lifecycleScope
import com.google.gson.Gson
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.decodeFromJsonElement
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonObject
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.maplibre.android.annotations.IconFactory
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.annotations.PolylineOptions
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.location.LocationComponentActivationOptions
import org.maplibre.android.location.OnLocationCameraTransitionListener
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.GeoParseUtil
import org.maplibre.android.testapp.utils.animateCameraSuspend
import org.maplibre.android.testapp.utils.setStyleSuspend
import org.maplibre.geojson.Point
import org.maplibre.navigation.android.navigation.v5.location.replay.ReplayRouteLocationEngine
import org.maplibre.navigation.android.navigation.v5.models.DirectionsResponse
import org.maplibre.navigation.android.navigation.v5.models.DirectionsRoute
import org.maplibre.navigation.android.navigation.v5.models.RouteOptions
import org.maplibre.navigation.android.navigation.v5.navigation.MapLibreNavigation
import org.maplibre.navigation.android.navigation.v5.navigation.MapLibreNavigationOptions
import org.maplibre.navigation.android.navigation.v5.navigation.NavigationMapRoute
import org.maplibre.navigation.android.navigation.v5.routeprogress.RouteProgress
import java.lang.reflect.Field
import java.util.Locale
import java.util.logging.Logger
import kotlin.math.min
import kotlin.random.Random

class LongRunningActivity : AppCompatActivity() {
    private lateinit var bitmapDrawables: List<Drawable>
    private lateinit var navigation: MapLibreNavigation
    private lateinit var navigationMapRoute: NavigationMapRoute
    private val replayRouteLocationEngine = ReplayRouteLocationEngine()
    private var routeUpdateTimer = 0.0

    // config
    companion object {
        private val LOG = Logger.getLogger(LongRunningActivity::class.java.name)
        // activity lifetime (seconds)
        private const val DURATION = 10 * 60 * 60

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

        private fun newDoubleRandom(random: Random): (min: Double, max: Double) -> Double =
            { min, max -> min + random.nextDouble() * (max - min) }

        private fun newIntRandom(random: Random): (min: Int, max: Int) -> Int =
            { min, max -> random.nextInt(min, max) }

        private fun random(min: Double, max: Double): Double = newDoubleRandom(RANDOM).invoke(min, max)
        private fun random(min: Int, max: Int): Int = newIntRandom(RANDOM).invoke(min, max)

        private fun <T> weightedRandom(values :List<Pair<T, Double>>): T {
            val cumulativeWeights = values.scan(0.0) { acc, value -> acc + value.second }
            var index = cumulativeWeights.binarySearch(random(0.00001, cumulativeWeights.last()))

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

        private fun randomLatLng(map: MapLibreMap): LatLng = randomLatLng(map.projection.visibleRegion.latLngBounds)
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

        private fun randomColor(): Int =
            Color.argb(
                RANDOM.nextInt(127, 256),
                RANDOM.nextInt(256),
                RANDOM.nextInt(256),
                RANDOM.nextInt(256)
            )

        // use a different random generator for navigation since web requests
        // can reorder invocations between the 2 views
        private val NAV_RANDOM = Random(RANDOM_SEED + 1)

        enum class RouteProvider {
            Local,
            OSRM,
            Valhalla,
        }

        private val ROUTE_PROVIDER = RouteProvider.Local
        private const val ROUTE_UPDATE_INTERVAL = 10.0

        // used by remote providers
        private val ROUTES = arrayOf(
            Pair(LatLng(52.521487, 13.409961), LatLng(52.502501, 13.423342)), // BER short
            Pair(LatLng(52.46314, 13.339116), LatLng(52.545929, 13.457635)), // BER long
            Pair(LatLng(38.903727, -77.000668), LatLng(38.890509, -77.025958)), // DC short
            Pair(LatLng(38.876664, -77.206788), LatLng(38.957716, -77.027674)), // DC long
            Pair(LatLng(37.781832, -122.401477), LatLng(37.771035, -122.410592)), // SF short
            Pair(LatLng(37.736431, -122.504263), LatLng(37.785594, -122.401209)), // SF long
        )

        private fun navRandom(min: Double, max: Double): Double = newDoubleRandom(NAV_RANDOM).invoke(min, max)
        private fun navRandom(min: Int, max: Int): Int = newIntRandom(NAV_RANDOM).invoke(min, max)

        private fun randomNavZoom(): Double = navRandom(13.0, 18.0)
        private fun randomNavTilt(): Double = navRandom(30.0, 60.0)
        private fun randomNavSpeed(): Int = navRandom(30, 130) // in km/h
        private fun randomNavWaitTime(): Long = navRandom(5000, 10000).toLong()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_long_running_maps)

        LOG.info("Running activity with seed $RANDOM_SEED for $DURATION seconds")

        val mapView1 = supportFragmentManager.findFragmentById(R.id.map1)!!.view as MapView
        val mapView2 = supportFragmentManager.findFragmentById(R.id.map2)!!.view as MapView

        mapView1.getMapAsync { map: MapLibreMap -> run(map, mapView1) }
        mapView2.getMapAsync { map: MapLibreMap -> runNavigation(map, mapView2) }

        lifecycleScope.launch {
            delay(DURATION * 1000L)

            LOG.info("Finished running activity with seed $RANDOM_SEED")

            printStats()
            finish()
        }
    }

    private fun run(map: MapLibreMap, mapView: MapView) {
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

        lifecycleScope.launch {
            // since the random generator was not reset each run will have different values
            while (true) {
                runStyleActions(map, mapView)
            }
        }
    }

    private suspend fun runStyleActions(map: MapLibreMap, mapView: MapView) {
        for (style in STYLES.shuffled(RANDOM)) {
            mapView.setStyleSuspend(style)
            runCameraActions(map)
        }
    }

    private suspend fun runCameraActions(map: MapLibreMap) {
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

                runAnnotationActions(map)
            }

            map.removeAnnotations()
        }
    }

    private fun runAnnotationActions(map: MapLibreMap) {
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
                        IconFactory.getInstance(this).fromBitmap(
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

    private fun runNavigation(map: MapLibreMap, mapView: MapView) {
        navigation = MapLibreNavigation(this,
            MapLibreNavigationOptions.builder().snapToRoute(true).build()).apply {
                snapEngine
                addProgressChangeListener { location: Location, progress: RouteProgress ->
                    map.locationComponent.forceLocationUpdate(location)

                    if (routeUpdateTimer - progress.durationRemaining() > ROUTE_UPDATE_INTERVAL) {
                        map.locationComponent.zoomWhileTracking(randomNavZoom())
                        map.locationComponent.tiltWhileTracking(randomNavTilt())

                        replayRouteLocationEngine.updateSpeed(randomNavSpeed())

                        routeUpdateTimer = progress.durationRemaining()

                        LOG.info("Navigation - duration ${progress.durationRemaining()}")
                    }

                    if (progress.fractionTraveled() > 0.99) {
                        startNewRoute(map, mapView)
                    }
                }
            }

        navigation.locationEngine = replayRouteLocationEngine
        navigationMapRoute = NavigationMapRoute(navigation, mapView, map)

        startNewRoute(map, mapView)
    }

    private fun startNewRoute(map: MapLibreMap, mapView: MapView) {
        lifecycleScope.launch {
            delay(randomNavWaitTime())

            navigation.stopNavigation()
            navigationMapRoute.removeRoute()

            mapView.setStyleSuspend(STYLES.random(NAV_RANDOM))
            enableLocation(map)

            val route = getRoute() ?: return@launch

            // display route
            navigationMapRoute.addRoute(route)

            // force tile load at starting position
            val startingPoint = route.routeOptions()?.coordinates()?.firstOrNull()
            if (startingPoint != null) {
                map.locationComponent.forceLocationUpdate(
                    Location("StartingLocation").apply {
                        latitude = startingPoint.latitude()
                        longitude = startingPoint.longitude()
                    }
                )
            }

            delay(randomNavWaitTime())

            replayRouteLocationEngine.assign(route)
            routeUpdateTimer = route.duration()
            navigation.startNavigation(route)
        }
    }

    @SuppressLint("MissingPermission")
    private fun enableLocation(map: MapLibreMap) {
        map.locationComponent.activateLocationComponent(
            LocationComponentActivationOptions
                .builder(this, map.style!!)
                .useDefaultLocationEngine(false)
                .build()
        )

        map.locationComponent.isLocationComponentEnabled = true
        map.locationComponent.renderMode = RenderMode.GPS
        map.locationComponent.setCameraMode(
            CameraMode.TRACKING_GPS,
            object :
                OnLocationCameraTransitionListener {
                override fun onLocationCameraTransitionFinished(cameraMode: Int) {
                    map.locationComponent.zoomWhileTracking(randomNavZoom())
                    map.locationComponent.tiltWhileTracking(randomNavTilt())
                }

                override fun onLocationCameraTransitionCanceled(cameraMode: Int) {}
            }
        )
    }

    private fun getRoute(): DirectionsRoute? {
        var location: LatLng? = null
        var destination: LatLng? = null

        val routeString: String? = when (ROUTE_PROVIDER) {
            RouteProvider.Local -> {
                val routeFile = this.assets.list("routes/")!!.random(NAV_RANDOM)
                LOG.info("Navigation - local route: $routeFile")

                val routeStr = GeoParseUtil.loadStringFromAssets(this, "routes/$routeFile")

                // quick parse to get the waypoints before starting the route
                val json = Json.parseToJsonElement(routeStr).jsonObject

                val waypoints = json["waypoints"]?.jsonArray
                val startWaypoint = waypoints?.first()?.jsonObject?.get("location")?.jsonArray
                val endWaypoint = waypoints?.last()?.jsonObject?.get("location")?.jsonArray

                if (startWaypoint != null) {
                    location = LatLng(
                        Json.decodeFromJsonElement<Double>(startWaypoint.last()),
                        Json.decodeFromJsonElement<Double>(startWaypoint.first()),
                    )
                }

                if (endWaypoint != null) {
                    destination = LatLng(
                        Json.decodeFromJsonElement<Double>(endWaypoint.last()),
                        Json.decodeFromJsonElement<Double>(endWaypoint.first()),
                    )
                }

                routeStr
            }

            RouteProvider.OSRM -> {
                val routePoints = ROUTES.random(NAV_RANDOM)
                location = routePoints.first
                destination = routePoints.second

                val get = "${location.longitude},${location.latitude};" +
                        "${destination.longitude},${destination.latitude}" +
                        "?steps=true"

                val request = Request
                    .Builder()
                    .header("User-Agent", "MapLibre Android")
                    .url("https://router.project-osrm.org/route/v1/driving/$get")
                    .get()
                    .build()

                LOG.info("Navigation - OSRM route request " +
                        "(${location.longitude},${location.latitude}) -> " +
                        "(${destination.longitude},${destination.latitude})"
                )

                val response = OkHttpClient().newCall(request).execute()
                response.body?.string()
            }

            RouteProvider.Valhalla -> {
                val routePoints = ROUTES.random(NAV_RANDOM)
                location = routePoints.first
                destination = routePoints.second

                val requestBody = Gson().toJson(
                    mapOf(
                        "format" to "osrm",
                        "costing" to "auto",
                        "banner_instructions" to true,
                        "voice_instructions" to true,
                        "language" to Locale.getDefault().language,
                        "directions_options" to mapOf(
                            "units" to "kilometers"
                        ),
                        "costing_options" to mapOf(
                            "auto" to mapOf(
                                "top_speed" to 130
                            )
                        ),
                        "locations" to listOf(
                            mapOf(
                                "lon" to location.longitude,
                                "lat" to location.latitude,
                                "type" to "break"
                            ),
                            mapOf(
                                "lon" to destination.longitude,
                                "lat" to destination.latitude,
                                "type" to "break"
                            )
                        )
                    )
                ).toRequestBody("application/json; charset=utf-8".toMediaType())

                val request = Request
                    .Builder()
                    .header("User-Agent", "MapLibre Android")
                    .url("https://valhalla1.openstreetmap.de/route")
                    .post(requestBody)
                    .build()

                LOG.info("Navigation - Valhalla route request " +
                        "(${location.longitude},${location.latitude}) -> " +
                        "(${destination.longitude},${destination.latitude})"
                )

                val response = OkHttpClient().newCall(request).execute()
                response.body?.string()
            }
        }

        if (routeString == null) {
            LOG.warning("Failed to get route data")
            return null
        }

        val directionsResponse = DirectionsResponse.fromJson(routeString)
        val route = directionsResponse.routes().first()

        val routeOptions = RouteOptions
            .builder()
            .baseUrl("https://maplibre.org")
            .profile("maplibre")
            .user("maplibre")
            .accessToken("maplibre")
            .voiceInstructions(true)
            .bannerInstructions(true)
            .language(Locale.getDefault().language)
            .coordinates(mutableListOf(
                location?.let { Point.fromLngLat(it.longitude, location.latitude) },
                destination?.let { Point.fromLngLat(it.longitude, destination.latitude) },
            ))
            .requestUuid("0000-0000-0000-0000")
            .build()

        return route.toBuilder().routeOptions(routeOptions).build()
    }

    @SuppressLint("NewApi")
    private fun printStats() {
        val activityManager = this.getSystemService(ACTIVITY_SERVICE) as ActivityManager
        val sysMemInfo = ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(sysMemInfo)

        LOG.info("System: \n" +
            "\tavailable memory - ${sysMemInfo.availMem / 1048576} MB\n" +
            "\ttotal memory - ${sysMemInfo.totalMem / 1048576} MB\n" +
            "\tlow memory threshold - ${sysMemInfo.threshold / 1048576} MB\n" +
            "\tlow memory - ${sysMemInfo.lowMemory}\n"
        )

        val appMemInfo = activityManager.getProcessMemoryInfo(intArrayOf(android.os.Process.myPid())).first()

        LOG.info("Application memory: \n" +
            appMemInfo.memoryStats.map { "\t${it.key} - ${it.value} KB" }.joinToString("\n")
        )
    }
}
