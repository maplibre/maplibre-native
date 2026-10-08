package org.maplibre.android.testapp.activity.stability

import android.annotation.SuppressLint
import android.location.Location
import android.os.Bundle
import android.os.SystemClock
import android.util.TypedValue
import android.view.Choreographer
import android.view.MotionEvent
import android.view.View
import android.view.ViewConfiguration
import android.view.ViewGroup
import android.view.ViewTreeObserver
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentActivationOptions
import org.maplibre.android.location.LocationComponentOptions
import org.maplibre.android.location.OnCameraTrackingChangedListener
import org.maplibre.android.location.OnLocationCameraTransitionListener
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.BuildConfig
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.setStyleSuspend
import java.util.ArrayDeque
import kotlin.math.cos
import kotlin.math.ln
import kotlin.math.pow
import kotlin.math.tan
import kotlin.random.Random
import kotlin.time.Duration.Companion.milliseconds
import kotlin.time.Duration.Companion.seconds

/**
 * Navigation map that draws a route as a GeoJSON line and trims it with the location puck.
 */
class DisappearingRouteActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var statsView: TextView
    private var messageToast: Toast? = null
    private var messageToastText: String? = null
    private var messageToastShownAt = 0L
    private var pendingMessage: Runnable? = null
    private lateinit var frameGraph: FrameTimeGraphView
    private lateinit var trackingButton: Button
    private lateinit var lineModeButton: Button

    private var map: MapLibreMap? = null
    private var route: GeoJsonRoute? = null
    private var routeJob: Job? = null
    private var simulationJob: Job? = null
    private var styleJob: Job? = null
    private var stopped = false
    private var waitedForInitialStyle = false
    private var styleCycleIndex = 0
    private var routeGeneration = 0
    private var splitLine = true
    private var activeSpeedMultiplier = 1.0
    private var userSpeedMultiplier: Double? = null
    private var userUpdateIntervalMs: Long? = null
    private var handlingThreeFingers = false
    private var threeFingerStartX = 0f
    private var threeFingerStartY = 0f
    private var speedAtGestureStart = 1.0
    private var intervalAtGestureStart = DEFAULT_UPDATE_INTERVAL_MS.toDouble()
    private var panAxis = PanAxis.UNDECIDED

    private val frameTimes = ArrayDeque<Long>()
    private var lastStatsPublish = 0L
    private val choreographer = Choreographer.getInstance()
    private var trimCallbackPosted = false

    private val frameListener = MapView.OnDidFinishRenderingFrameWithStatsListener { _, stats ->
        if (stopped) {
            return@OnDidFinishRenderingFrameWithStatsListener
        }
        frameGraph.addFrameDuration(stats.encodingTime)
        scheduleTrimFrames()
        publishStats()
    }

    /**
     * Same vsync clock as the location puck. Trimming from the map's frame-finished callback
     * instead applies the line a rendered frame later, so the puck pulls ahead whenever the map
     * cannot keep up with the display.
     */
    private val trimFrameCallback = Choreographer.FrameCallback { frameTimeNanos ->
        trimCallbackPosted = false
        if (stopped) {
            return@FrameCallback
        }
        val active = route
        if (active == null || !active.isTrimAnimating) {
            return@FrameCallback
        }
        active.tickDisplayedGeometry(elapsedRealtimeAtFrame(frameTimeNanos))
        map?.triggerRepaint()
        scheduleTrimFrames()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_disappearing_route)
        mapView = findViewById(R.id.mapView)
        statsView = findViewById(R.id.stats)
        placeReadoutBelowTitleBar(findViewById(R.id.readout))
        frameGraph = findViewById(R.id.frameGraph)
        trackingButton = findViewById(R.id.trackingMode)
        lineModeButton = findViewById(R.id.lineMode)

        mapView.onCreate(savedInstanceState)
        mapView.addOnDidFinishRenderingFrameListener(frameListener)
        mapView.setOnTouchListener { _, event -> onMapTouch(event) }

        findViewById<Button>(R.id.nextRoute).setOnClickListener { skipToNextRoute() }
        findViewById<Button>(R.id.nextStyle).setOnClickListener { cycleToNextStyle() }
        trackingButton.setOnClickListener { cycleTrackingMode() }
        lineModeButton.setOnClickListener { toggleLineMode() }
        updateLineModeButton()

        mapView.getMapAsync { maplibreMap ->
            map = maplibreMap
            maplibreMap.uiSettings.isTiltGesturesEnabled = true
            maplibreMap.addOnMapClickListener {
                restoreTracking()
                true
            }
            maplibreMap.locationComponent.addOnCameraTrackingChangedListener(object : OnCameraTrackingChangedListener {
                override fun onCameraTrackingDismissed() {
                    updateTrackingButton()
                }

                override fun onCameraTrackingChanged(currentMode: Int) {
                    updateTrackingButton()
                }
            })
            val styleUrl = STYLES.random(RANDOM)
            showMessage("Loading $styleUrl", 2000)
            lifecycleScope.launch {
                mapView.setStyleSuspend(styleUrl)
                enableLocation()
                startNewRoute(nextStyle = false, delay = false)
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun enableLocation() {
        val maplibreMap = map ?: return
        val style = maplibreMap.style ?: return
        val options = LocationComponentOptions.builder(this)
            .trackingAnimationDurationMultiplier(1f)
            .build()
        maplibreMap.locationComponent.activateLocationComponent(
            LocationComponentActivationOptions.builder(this, style)
                .locationComponentOptions(options)
                .useDefaultLocationEngine(false)
                .useSpecializedLocationLayer(true)
                .build()
        )
        maplibreMap.locationComponent.isLocationComponentEnabled = true
        maplibreMap.locationComponent.renderMode = RenderMode.GPS
    }

    private fun startNewRoute(nextStyle: Boolean, delay: Boolean) {
        if (stopped) {
            return
        }
        val generation = ++routeGeneration
        simulationJob?.cancel()
        styleJob?.cancel()
        routeJob?.cancel()
        routeJob = lifecycleScope.launch {
            if (delay) {
                val waitSeconds = RANDOM.nextDouble(5.0, 10.0)
                showMessage("Waiting ${"%.1f".format(waitSeconds)}s …")
                delay(waitSeconds.seconds)
            }
            if (!isActive || stopped || generation != routeGeneration) {
                return@launch
            }

            val previous = route
            route = null
            previous?.unload()

            if (waitedForInitialStyle && nextStyle) {
                val styleUrl = STYLES.random(RANDOM)
                showMessage("Loading $styleUrl")
                mapView.setStyleSuspend(styleUrl)
            } else {
                waitedForInitialStyle = true
            }
            if (!isActive || stopped || generation != routeGeneration) {
                return@launch
            }

            val loaded = LocalRoute.load(this@DisappearingRouteActivity, RANDOM)
            if (loaded == null) {
                showMessage("Failed to load a route", 3000)
                return@launch
            }
            val maplibreMap = map ?: return@launch
            showMessage("Loading '${loaded.fileName}' …")
            val geoJsonRoute = GeoJsonRoute(maplibreMap, mapView, loaded.path, loaded.destination)
            geoJsonRoute.splitLine = splitLine
            route = geoJsonRoute

            val kilometers = loaded.distance / 1000.0
            showMessage(
                "Loaded route from '${loaded.fileName}' with ${loaded.path.coordinates.size} points, " +
                    "${"%.2f".format(kilometers)} km",
                3000,
            )

            val altitude = RANDOM.nextDouble(800.0, 2000.0)
            val tilt = RANDOM.nextDouble(30.0, 60.0)
            val start = loaded.path.point(0.0)
            val zoom = zoomForAltitude(altitude, start.latitude, mapView.height)
            maplibreMap.moveCamera(
                CameraUpdateFactory.newCameraPosition(
                    CameraPosition.Builder()
                        .target(start)
                        .zoom(zoom)
                        .tilt(tilt)
                        .bearing(0.0)
                        .build()
                )
            )

            if (delay) {
                delay(RANDOM.nextLong(5000, 10001).milliseconds)
            }
            if (!isActive || stopped || generation != routeGeneration) {
                return@launch
            }

            followCourse(zoom, tilt)
            simulate(generation, loaded)
        }
    }

    private fun simulate(generation: Int, loaded: LocalRoute) {
        val speed = if (loaded.duration > 0) loaded.distance / loaded.duration else 20.0
        val multiplier = userSpeedMultiplier ?: RANDOM.nextDouble(0.7, 2.0)
        activeSpeedMultiplier = multiplier
        simulationJob = lifecycleScope.launch {
            pushLocation(loaded, 0.0, speed * multiplier)
            var distance = 0.0
            while (isActive && generation == routeGeneration && distance < loaded.path.totalDistance) {
                val interval = userUpdateIntervalMs ?: DEFAULT_UPDATE_INTERVAL_MS
                delay(interval.milliseconds)
                if (!isActive || generation != routeGeneration) {
                    return@launch
                }
                val stepMultiplier = userSpeedMultiplier ?: multiplier
                distance += speed * stepMultiplier * (interval / 1000.0)
                val clamped = distance.coerceAtMost(loaded.path.totalDistance)
                pushLocation(loaded, clamped, speed * stepMultiplier)
                if (clamped >= loaded.path.totalDistance) {
                    startNewRoute(nextStyle = true, delay = true)
                    return@launch
                }
            }
        }
    }

    private fun pushLocation(loaded: LocalRoute, distance: Double, speedMetersPerSecond: Double) {
        val maplibreMap = map ?: return
        val component = maplibreMap.locationComponent
        val ahead = loaded.path.point((distance + 10.0).coerceAtMost(loaded.path.totalDistance))
        // Visit each vertex between the marker and this sample. One straight animation cuts
        // those corners, so a cut measured along the route runs ahead of the marker and only
        // meets it when the animation ends.
        val points = route?.animationPoints(distance).orEmpty()
        if (points.size > 1) {
            val locations = points.mapIndexed { index, point ->
                val next = points.getOrNull(index + 1) ?: ahead
                simulatedLocation(point, RoutePath.direction(point, next).toFloat(), speedMetersPerSecond)
            }
            component.forceLocationUpdate(locations, false)
        } else {
            val point = points.firstOrNull() ?: loaded.path.point(distance)
            component.forceLocationUpdate(
                simulatedLocation(point, RoutePath.direction(point, ahead).toFloat(), speedMetersPerSecond),
            )
        }
        route?.updateDisplayedGeometry(
            distance,
            component.userLocationAnimationReferenceTime,
            component.userLocationAnimationDuration,
        )
        scheduleTrimFrames()
    }

    private fun simulatedLocation(point: LatLng, bearing: Float, speedMetersPerSecond: Double): Location {
        return Location("DisappearingRoute").apply {
            latitude = point.latitude
            longitude = point.longitude
            this.bearing = bearing
            speed = speedMetersPerSecond.toFloat()
            accuracy = 20f
            time = System.currentTimeMillis()
        }
    }

    private fun scheduleTrimFrames() {
        if (stopped || trimCallbackPosted || route?.isTrimAnimating != true) {
            return
        }
        trimCallbackPosted = true
        choreographer.postFrameCallback(trimFrameCallback)
    }

    /** [Choreographer] frame time shares the [System.nanoTime] clock, trim timestamps use elapsed realtime. */
    private fun elapsedRealtimeAtFrame(frameTimeNanos: Long): Long {
        val skew = SystemClock.elapsedRealtimeNanos() - System.nanoTime()
        return ((frameTimeNanos + skew) / 1_000_000L).coerceAtLeast(0L)
    }

    private fun followCourse(zoom: Double, tilt: Double) {
        val component = map?.locationComponent ?: return
        component.renderMode = RenderMode.GPS
        component.setCameraMode(
            CameraMode.TRACKING_GPS,
            object : OnLocationCameraTransitionListener {
                override fun onLocationCameraTransitionFinished(cameraMode: Int) {
                    component.zoomWhileTracking(zoom)
                    component.tiltWhileTracking(tilt)
                    updateTrackingButton()
                }

                override fun onLocationCameraTransitionCanceled(cameraMode: Int) {
                    updateTrackingButton()
                }
            }
        )
        updateTrackingButton()
    }

    private fun skipToNextRoute() {
        startNewRoute(nextStyle = false, delay = false)
    }

    private fun cycleToNextStyle() {
        if (stopped) {
            return
        }
        val next = pendingStyle()
        styleJob?.cancel()
        val loading = routeJob
        styleJob = lifecycleScope.launch {
            if (route == null && loading != null) {
                loading.join()
            }
            if (!isActive || stopped) {
                return@launch
            }
            showMessage("Loading $next")
            mapView.setStyleSuspend(next)
            if (!isActive || stopped) {
                return@launch
            }
            styleCycleIndex = STYLES.indexOf(next).coerceAtLeast(0)
            route?.reinstallDisplayedLine()
        }
    }

    private fun pendingStyle(): String {
        val current = map?.style?.uri
        val index = STYLES.indexOf(current)
        val nextIndex = if (index >= 0) {
            (index + 1) % STYLES.size
        } else {
            (styleCycleIndex + 1) % STYLES.size
        }
        return STYLES[nextIndex]
    }

    private fun cycleTrackingMode() {
        val component = map?.locationComponent ?: return
        val next = when (component.cameraMode) {
            CameraMode.NONE -> CameraMode.TRACKING
            CameraMode.TRACKING -> CameraMode.TRACKING_COMPASS
            CameraMode.TRACKING_COMPASS -> CameraMode.TRACKING_GPS
            CameraMode.TRACKING_GPS -> CameraMode.NONE
            else -> CameraMode.TRACKING
        }
        component.renderMode = when (next) {
            CameraMode.TRACKING_COMPASS -> RenderMode.COMPASS
            CameraMode.TRACKING_GPS -> RenderMode.GPS
            else -> RenderMode.NORMAL
        }
        component.cameraMode = next
        updateTrackingButton()
        showMessage(trackingMessage(next), 2000)
    }

    private fun restoreTracking() {
        val component = map?.locationComponent ?: return
        if (!component.isLocationComponentEnabled) {
            return
        }
        component.renderMode = RenderMode.GPS
        component.cameraMode = CameraMode.TRACKING_GPS
        updateTrackingButton()
        showMessage(trackingMessage(CameraMode.TRACKING_GPS), 2000)
    }

    private fun toggleLineMode() {
        splitLine = !splitLine
        route?.splitLine = splitLine
        updateLineModeButton()
        showMessage(
            if (splitLine) {
                "Splitting the route line"
            } else {
                "Submitting the entire line every frame"
            },
            2000,
        )
    }

    private fun updateLineModeButton() {
        lineModeButton.text = if (splitLine) "Split" else "Whole line"
    }

    private fun updateTrackingButton() {
        val mode = map?.locationComponent?.cameraMode ?: CameraMode.NONE
        trackingButton.text = trackingTitle(mode)
        trackingButton.alpha = if (mode == CameraMode.NONE) 0.45f else 0.92f
    }

    private fun trackingTitle(mode: Int): String {
        return when (mode) {
            CameraMode.NONE -> "Tracking Off"
            CameraMode.TRACKING -> "Follow"
            CameraMode.TRACKING_COMPASS -> "Heading"
            CameraMode.TRACKING_GPS -> "Course"
            else -> "Track"
        }
    }

    private fun trackingMessage(mode: Int): String {
        return when (mode) {
            CameraMode.NONE -> "Tracking off"
            CameraMode.TRACKING -> "Following location"
            CameraMode.TRACKING_COMPASS -> "Following heading"
            CameraMode.TRACKING_GPS -> "Following course"
            else -> "Tracking"
        }
    }

    private fun showMessage(text: String, autoHideAfterMs: Long? = null) {
        val now = SystemClock.elapsedRealtime()
        if (text == messageToastText && now - messageToastShownAt < 1000L) {
            return
        }
        val duration = if ((autoHideAfterMs ?: Long.MAX_VALUE) > 2000L) {
            Toast.LENGTH_LONG
        } else {
            Toast.LENGTH_SHORT
        }
        messageToastText = text
        messageToastShownAt = now
        // Text toasts are drawn by the system. Updating one that is already visible is ignored,
        // and cancel() can hide a replacement shown in the same turn, so show the next one after.
        messageToast?.cancel()
        messageToast = null
        pendingMessage?.let { mapView.removeCallbacks(it) }
        val showNext = Runnable {
            pendingMessage = null
            if (stopped) {
                return@Runnable
            }
            val toast = Toast.makeText(this, text, duration)
            messageToast = toast
            toast.show()
        }
        pendingMessage = showNext
        mapView.post(showNext)
    }

    private fun placeReadoutBelowTitleBar(readout: View) {
        readout.viewTreeObserver.addOnGlobalLayoutListener(object : ViewTreeObserver.OnGlobalLayoutListener {
            override fun onGlobalLayout() {
                if (!readout.isAttachedToWindow) {
                    return
                }
                val titleBar = titleBarView()
                if (titleBar == null) {
                    readout.viewTreeObserver.removeOnGlobalLayoutListener(this)
                    applyReadoutTopMargin(readout, actionBarHeight())
                    return
                }
                if (titleBar.height == 0) {
                    return
                }
                readout.viewTreeObserver.removeOnGlobalLayoutListener(this)
                val titleLocation = IntArray(2)
                val readoutLocation = IntArray(2)
                titleBar.getLocationOnScreen(titleLocation)
                readout.getLocationOnScreen(readoutLocation)
                val overlap = titleLocation[1] + titleBar.height - readoutLocation[1]
                if (overlap > 0) {
                    val params = readout.layoutParams as ViewGroup.MarginLayoutParams
                    applyReadoutTopMargin(readout, params.topMargin + overlap)
                }
            }
        })
    }

    private fun titleBarView(): View? {
        val decor = window.decorView
        return decor.findViewById(androidx.appcompat.R.id.action_bar)
            ?: decor.findViewById(androidx.appcompat.R.id.action_bar_container)
    }

    private fun actionBarHeight(): Int {
        val typed = TypedValue()
        if (!theme.resolveAttribute(android.R.attr.actionBarSize, typed, true)) {
            return 0
        }
        return TypedValue.complexToDimensionPixelSize(typed.data, resources.displayMetrics)
    }

    private fun applyReadoutTopMargin(readout: View, topMargin: Int) {
        val params = readout.layoutParams as ViewGroup.MarginLayoutParams
        if (params.topMargin == topMargin) {
            return
        }
        params.topMargin = topMargin
        readout.layoutParams = params
    }

    private fun publishStats() {
        val now = SystemClock.elapsedRealtime()
        frameTimes.addLast(now)
        while (frameTimes.isNotEmpty() && now - frameTimes.first() > 1000) {
            frameTimes.removeFirst()
        }
        if (frameTimes.size < 2 || now - lastStatsPublish < 250) {
            return
        }
        val elapsedSeconds = (frameTimes.last() - frameTimes.first()) / 1000.0
        if (elapsedSeconds <= 0) {
            return
        }
        lastStatsPublish = now
        val framesPerSecond = (frameTimes.size - 1) / elapsedSeconds
        val zoom = map?.cameraPosition?.zoom ?: 0.0
        val points = route?.displayedPointCount ?: 0
        val debug = if (BuildConfig.DEBUG) "\nDEBUG" else ""
        statsView.text = "%3.0f FPS\nz %5.2f\n%5d points%s".format(framesPerSecond, zoom, points, debug)
    }

    private fun onMapTouch(event: MotionEvent): Boolean {
        val pointerCount = event.pointerCount
        val action = event.actionMasked
        if (!handlingThreeFingers && pointerCount < 3) {
            return false
        }
        if (!handlingThreeFingers) {
            val cancel = MotionEvent.obtain(event)
            cancel.action = MotionEvent.ACTION_CANCEL
            mapView.onTouchEvent(cancel)
            cancel.recycle()
            handlingThreeFingers = true
            panAxis = PanAxis.UNDECIDED
            threeFingerStartX = averageX(event)
            threeFingerStartY = averageY(event)
            speedAtGestureStart = currentSpeedMultiplier()
            intervalAtGestureStart = (userUpdateIntervalMs ?: DEFAULT_UPDATE_INTERVAL_MS).toDouble()
        }
        if (handlingThreeFingers) {
            val translationX = averageX(event) - threeFingerStartX
            val translationY = averageY(event) - threeFingerStartY
            val finished = action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL ||
                (action == MotionEvent.ACTION_POINTER_UP && pointerCount <= 3)
            if (action == MotionEvent.ACTION_MOVE || finished) {
                updateThreeFingerPan(translationX, translationY, finished)
            }
            if (finished) {
                val slop = ViewConfiguration.get(this).scaledTouchSlop
                if (kotlin.math.hypot(translationX.toDouble(), translationY.toDouble()) < slop) {
                    skipToNextRoute()
                }
                handlingThreeFingers = false
            }
            return true
        }
        return false
    }

    private fun updateThreeFingerPan(translationX: Float, translationY: Float, finished: Boolean) {
        if (panAxis == PanAxis.UNDECIDED) {
            val lock = 12f * resources.displayMetrics.density
            if (maxOf(kotlin.math.abs(translationX), kotlin.math.abs(translationY)) < lock) {
                return
            }
            panAxis = if (kotlin.math.abs(translationX) > kotlin.math.abs(translationY)) {
                PanAxis.HORIZONTAL
            } else {
                PanAxis.VERTICAL
            }
        }
        val hideAfter = if (finished) 2000L else null
        val pointsPerDecade = 280f * resources.displayMetrics.density
        when (panAxis) {
            PanAxis.VERTICAL -> {
                val decades = (-translationY / pointsPerDecade).toDouble()
                val value = (speedAtGestureStart * 10.0.pow(decades)).coerceIn(0.1, 10.0)
                userSpeedMultiplier = value
                showMessage("Speed ${formatSpeed(value)}", hideAfter)
            }
            PanAxis.HORIZONTAL -> {
                val startHz = 1000.0 / intervalAtGestureStart
                val decades = (translationX / pointsPerDecade).toDouble()
                val hz = (startHz * 10.0.pow(decades)).coerceIn(0.2, 60.0)
                userUpdateIntervalMs = (1000.0 / hz).toLong().coerceAtLeast(1)
                showMessage("Update ${formatHz(hz)}", hideAfter)
            }
            PanAxis.UNDECIDED -> Unit
        }
    }

    private fun currentSpeedMultiplier(): Double {
        return userSpeedMultiplier ?: activeSpeedMultiplier
    }

    private fun averageX(event: MotionEvent): Float {
        var sum = 0f
        for (index in 0 until event.pointerCount) {
            sum += event.getX(index)
        }
        return sum / event.pointerCount
    }

    private fun averageY(event: MotionEvent): Float {
        var sum = 0f
        for (index in 0 until event.pointerCount) {
            sum += event.getY(index)
        }
        return sum / event.pointerCount
    }

    private fun formatSpeed(value: Double): String {
        return if (value >= 9.95) {
            "%.0f×".format(value)
        } else {
            "%.1f×".format(value)
        }
    }

    private fun formatHz(hz: Double): String {
        return when {
            hz >= 9.95 -> "%.0f Hz".format(hz)
            hz >= 0.95 -> "%.1f Hz".format(hz)
            else -> "%.2f Hz".format(hz)
        }
    }

    private fun zoomForAltitude(altitudeMeters: Double, latitude: Double, viewHeightPx: Int): Double {
        val height = viewHeightPx.coerceAtLeast(1)
        val metersTall = 2.0 * altitudeMeters * tan(Math.toRadians(36.87) / 2.0)
        val metersPerPixel = metersTall / height
        val circumference = 2.0 * Math.PI * 6_378_137.0
        val worldPixels = circumference * cos(Math.toRadians(latitude)) / metersPerPixel
        return ln(worldPixels / 512.0) / ln(2.0)
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
        route?.tickDisplayedGeometry()
        scheduleTrimFrames()
    }

    override fun onPause() {
        stopTrimFrames()
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        stopped = true
        stopTrimFrames()
        pendingMessage?.let { mapView.removeCallbacks(it) }
        pendingMessage = null
        messageToast?.cancel()
        messageToast = null
        routeJob?.cancel()
        simulationJob?.cancel()
        styleJob?.cancel()
        route?.unload()
        route = null
        mapView.removeOnDidFinishRenderingFrameListener(frameListener)
        mapView.onDestroy()
        super.onDestroy()
    }

    private fun stopTrimFrames() {
        trimCallbackPosted = false
        choreographer.removeFrameCallback(trimFrameCallback)
    }

    private enum class PanAxis {
        UNDECIDED,
        VERTICAL,
        HORIZONTAL,
    }

    companion object {
        private const val DEFAULT_UPDATE_INTERVAL_MS = 1000L
        private val RANDOM = Random(42)
        private val STYLES = listOf(
            TestStyles.AMERICANA,
            TestStyles.OPENFREEMAP_LIBERTY,
            TestStyles.OPENFREEMAP_BRIGHT,
            TestStyles.PROTOMAPS_LIGHT,
            TestStyles.PROTOMAPS_DARK,
            TestStyles.PROTOMAPS_GRAYSCALE,
            TestStyles.PROTOMAPS_WHITE,
            TestStyles.PROTOMAPS_BLACK,
        )
    }
}
