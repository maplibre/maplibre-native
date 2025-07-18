package org.maplibre.android.testapp.activity.stability

import android.annotation.SuppressLint
import android.location.Location
import android.os.Bundle
import android.os.PersistableBundle
import androidx.appcompat.app.AppCompatActivity
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
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentActivationOptions
import org.maplibre.android.location.OnLocationCameraTransitionListener
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.SupportMapFragment
import org.maplibre.android.testapp.activity.stability.LongRunningActivity.Companion.printStats
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.GeoParseUtil
import org.maplibre.android.testapp.utils.setStyleSuspend
import org.maplibre.geojson.Point
import org.maplibre.navigation.android.navigation.v5.location.replay.ReplayRouteLocationEngine
import org.maplibre.navigation.android.navigation.v5.milestone.BannerInstructionMilestone
import org.maplibre.navigation.android.navigation.v5.milestone.Milestone
import org.maplibre.navigation.android.navigation.v5.milestone.MilestoneEventListener
import org.maplibre.navigation.android.navigation.v5.models.DirectionsResponse
import org.maplibre.navigation.android.navigation.v5.models.DirectionsRoute
import org.maplibre.navigation.android.navigation.v5.models.RouteOptions
import org.maplibre.navigation.android.navigation.v5.navigation.MapLibreNavigation
import org.maplibre.navigation.android.navigation.v5.navigation.MapLibreNavigationOptions
import org.maplibre.navigation.android.navigation.v5.navigation.NavigationMapRoute
import org.maplibre.navigation.android.navigation.v5.routeprogress.ProgressChangeListener
import org.maplibre.navigation.android.navigation.v5.routeprogress.RouteProgress
import java.util.Locale
import java.util.logging.Logger
import kotlin.random.Random

class NavigationMap : SupportMapFragment(), ProgressChangeListener, MilestoneEventListener {
    private lateinit var map: MapLibreMap
    private lateinit var mapView: MapView

    private lateinit var navigation: MapLibreNavigation
    private lateinit var navigationMapRoute: NavigationMapRoute
    private val replayRouteLocationEngine = ReplayRouteLocationEngine()
    private var routeUpdateTimer = 0.0

    // config
    companion object {
        private val LOG = Logger.getLogger(NavigationMap::class.java.name)
        private const val RANDOM_SEED = 42
        private val RANDOM = Random(RANDOM_SEED)

        private val STYLES = arrayListOf(
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

        private fun random(min: Double, max: Double): Double = min + RANDOM.nextDouble() * (max - min)
        private fun random(min: Int, max: Int): Int = RANDOM.nextInt(min, max)

        private fun randomZoom(): Double = random(13.0, 18.0)
        private fun randomTilt(): Double = random(30.0, 60.0)
        private fun randomSpeed(): Int = random(30, 130) // in km/h
        private fun randomWaitTime(): Long = random(5000, 10000).toLong()
    }

    override fun onMapReady(maplibreMap: MapLibreMap) {
        super.onMapReady(maplibreMap)

        this.map = maplibreMap
        this.mapView = view as MapView

        LOG.info("NavigationMap seed $RANDOM_SEED")
        run()
    }

    private fun run() {
        lifecycleScope.launch {
            mapView.setStyleSuspend(STYLES.random(RANDOM))
            enableLocation()

            navigation = MapLibreNavigation(requireContext(), MapLibreNavigationOptions
                .builder()
                .snapToRoute(true)
                .build()
            ).apply {
                snapEngine
                addProgressChangeListener(this@NavigationMap)
                addMilestoneEventListener(this@NavigationMap)
            }

            navigationMapRoute = NavigationMapRoute(navigation, mapView, map)
            navigation.locationEngine = replayRouteLocationEngine

            startNewRoute()
        }
    }

    private fun startNewRoute() {
        lifecycleScope.launch {
            delay(randomWaitTime())

            printStats(requireContext())

            navigation.stopNavigation()
            navigationMapRoute.removeRoute()

            mapView.setStyleSuspend(STYLES.random(RANDOM))

            val route = getRoute() ?: return@launch

            // display route
            navigationMapRoute = NavigationMapRoute(navigation, mapView, map)
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

            delay(randomWaitTime())

            replayRouteLocationEngine.assign(route)
            routeUpdateTimer = route.duration()
            navigation.startNavigation(route)
        }
    }

    @SuppressLint("MissingPermission")
    private fun enableLocation() {
        map.locationComponent.activateLocationComponent(
            LocationComponentActivationOptions
                .builder(requireContext(), map.style!!)
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
                    map.locationComponent.zoomWhileTracking(randomZoom())
                    map.locationComponent.tiltWhileTracking(randomTilt())
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
                val context = requireContext()
                val routeFile = context.assets.list("routes/")!!.random(RANDOM)
                LOG.info("Navigation - local route: $routeFile")

                val routeStr = GeoParseUtil.loadStringFromAssets(context, "routes/$routeFile")

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
                val routePoints = ROUTES.random(RANDOM)
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
                val routePoints = ROUTES.random(RANDOM)
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

    override fun onProgressChange(location: Location, progress: RouteProgress) {
        map.locationComponent.forceLocationUpdate(location)

        if (routeUpdateTimer - progress.durationRemaining() > ROUTE_UPDATE_INTERVAL) {
            map.locationComponent.zoomWhileTracking(randomZoom())
            map.locationComponent.tiltWhileTracking(randomTilt())

            replayRouteLocationEngine.updateSpeed(randomSpeed())

            routeUpdateTimer = progress.durationRemaining()

            LOG.info("Navigation - remaining duration ${progress.durationRemaining()}")
        }
    }

    override fun onMilestoneEvent(routeProgress: RouteProgress, instruction: String, milestone: Milestone) {
        if (milestone !is BannerInstructionMilestone) {
            return
        }

        if (milestone.bannerInstructions.primary().type() == "arrive") {
            startNewRoute()
        }
    }
}

class NavigationMapActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        supportFragmentManager
            .beginTransaction()
            .replace(android.R.id.content, NavigationMap())
            .commit()
    }
}
