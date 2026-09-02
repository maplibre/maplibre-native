package org.maplibre.android.location

import android.content.Context
import android.location.Location
import android.view.MotionEvent
import androidx.annotation.VisibleForTesting
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.gestures.AndroidGesturesManager
import org.maplibre.android.gestures.MoveGestureDetector
import org.maplibre.android.gestures.RotateGestureDetector
import org.maplibre.android.location.LocationComponentConstants.TRANSITION_ANIMATION_DURATION_MS
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Transform

internal class LocationCameraController {
    private val maplibreMap: MapLibreMap
    private val transform: Transform
    private val internalCameraTrackingChangedListener: OnCameraTrackingChangedListener
    private val moveGestureDetector: MoveGestureDetector
    private val onCameraMoveInvalidateListener: OnCameraMoveInvalidateListener

    private val initialGesturesManager: AndroidGesturesManager
    private val internalGesturesManager: AndroidGesturesManager

    private lateinit var options: LocationComponentOptions

    var isTransitioning = false
        private set

    private var lastLocation: LatLng? = null
    private var isEnabled = false

    constructor(
        context: Context,
        maplibreMap: MapLibreMap,
        transform: Transform,
        internalCameraTrackingChangedListener: OnCameraTrackingChangedListener,
        options: LocationComponentOptions,
        onCameraMoveInvalidateListener: OnCameraMoveInvalidateListener,
    ) {
        this.maplibreMap = maplibreMap
        this.transform = transform

        initialGesturesManager = maplibreMap.gesturesManager
        internalGesturesManager = LocationGesturesManager(context)
        moveGestureDetector = internalGesturesManager.moveGestureDetector
        maplibreMap.addOnRotateListener(onRotateListener)
        maplibreMap.addOnFlingListener(onFlingListener)
        maplibreMap.addOnMoveListener(onMoveListener)
        maplibreMap.addOnCameraMoveListener(onCameraMoveListener)
        this.internalCameraTrackingChangedListener = internalCameraTrackingChangedListener
        this.onCameraMoveInvalidateListener = onCameraMoveInvalidateListener
        initializeOptions(options)
    }

    // Visible for testing purposes
    constructor(
        maplibreMap: MapLibreMap,
        transform: Transform,
        moveGestureDetector: MoveGestureDetector,
        internalCameraTrackingChangedListener: OnCameraTrackingChangedListener,
        onCameraMoveInvalidateListener: OnCameraMoveInvalidateListener,
        initialGesturesManager: AndroidGesturesManager,
        internalGesturesManager: AndroidGesturesManager,
    ) {
        this.maplibreMap = maplibreMap
        maplibreMap.addOnCameraMoveListener(onCameraMoveListener)
        this.transform = transform
        this.moveGestureDetector = moveGestureDetector
        this.internalCameraTrackingChangedListener = internalCameraTrackingChangedListener
        this.onCameraMoveInvalidateListener = onCameraMoveInvalidateListener
        this.internalGesturesManager = internalGesturesManager
        this.initialGesturesManager = initialGesturesManager
    }

    fun initializeOptions(options: LocationComponentOptions) {
        this.options = options
        if (options.trackingGesturesManagement()) {
            if (maplibreMap.gesturesManager !== internalGesturesManager) {
                maplibreMap.setGesturesManager(internalGesturesManager, true, true)
            }
            adjustGesturesThresholds()
        } else if (maplibreMap.gesturesManager !== initialGesturesManager) {
            maplibreMap.setGesturesManager(initialGesturesManager, true, true)
        }
    }

    @CameraMode.Mode
    private var currentCameraMode: Int = 0

    @get:CameraMode.Mode
    @setparam:CameraMode.Mode
    var cameraMode: Int
        get() = currentCameraMode
        set(value) = setCameraMode(value, null, TRANSITION_ANIMATION_DURATION_MS, null, null, null, null)

    fun setCameraMode(
        @CameraMode.Mode cameraMode: Int,
        lastLocation: Location?,
        transitionDuration: Long,
        zoom: Double?,
        bearing: Double?,
        tilt: Double?,
        internalTransitionListener: OnLocationCameraTransitionListener?,
    ) {
        if (currentCameraMode == cameraMode) {
            internalTransitionListener?.onLocationCameraTransitionFinished(cameraMode)
            return
        }

        val wasTracking = isLocationTracking()
        currentCameraMode = cameraMode

        if (cameraMode != CameraMode.NONE) {
            maplibreMap.cancelTransitions()
        }

        adjustGesturesThresholds()
        notifyCameraTrackingChangeListener(wasTracking)
        transitionToCurrentLocation(
            wasTracking,
            lastLocation,
            transitionDuration,
            zoom,
            bearing,
            tilt,
            internalTransitionListener,
        )
    }

    /**
     * Initiates a camera animation to the current location if location tracking was engaged.
     * Notifies an internal listener when the transition's finished to invalidate animators and notify external listeners.
     */
    private fun transitionToCurrentLocation(
        wasTracking: Boolean,
        lastLocation: Location?,
        transitionDuration: Long,
        zoom: Double?,
        bearing: Double?,
        tilt: Double?,
        internalTransitionListener: OnLocationCameraTransitionListener?,
    ) {
        if (!wasTracking && isLocationTracking() && lastLocation != null && isEnabled) {
            isTransitioning = true
            val target = LatLng(lastLocation)

            val builder = CameraPosition.Builder().target(target)
            if (zoom != null) {
                builder.zoom(zoom)
            }
            if (tilt != null) {
                builder.tilt(tilt)
            }
            if (bearing != null) {
                builder.bearing(bearing)
            } else if (isLocationBearingTracking()) {
                builder.bearing(
                    if (cameraMode == CameraMode.TRACKING_GPS_NORTH) 0.0 else lastLocation.bearing.toDouble(),
                )
            }

            val update = CameraUpdateFactory.newCameraPosition(builder.build())
            val callback =
                object : MapLibreMap.CancelableCallback {
                    override fun onCancel() {
                        isTransitioning = false
                        internalTransitionListener?.onLocationCameraTransitionCanceled(cameraMode)
                    }

                    override fun onFinish() {
                        isTransitioning = false
                        internalTransitionListener?.onLocationCameraTransitionFinished(cameraMode)
                    }
                }

            val currentPosition = maplibreMap.cameraPosition
            if (Utils.immediateAnimation(maplibreMap.projection, currentPosition.target!!, target)) {
                transform.moveCamera(maplibreMap, update, callback)
            } else {
                transform.animateCamera(maplibreMap, update, transitionDuration.toInt(), callback)
            }
        } else {
            internalTransitionListener?.onLocationCameraTransitionFinished(cameraMode)
        }
    }

    private fun setBearing(bearing: Float) {
        if (isTransitioning) {
            return
        }

        transform.moveCamera(maplibreMap, CameraUpdateFactory.bearingTo(bearing.toDouble()), null)
        onCameraMoveInvalidateListener.onInvalidateCameraMove()
    }

    private fun setLatLng(latLng: LatLng) {
        if (isTransitioning) {
            return
        }
        lastLocation = latLng
        transform.moveCamera(maplibreMap, CameraUpdateFactory.newLatLng(latLng), null)
        onCameraMoveInvalidateListener.onInvalidateCameraMove()
    }

    private fun setZoom(zoom: Float) {
        if (isTransitioning) {
            return
        }

        transform.moveCamera(maplibreMap, CameraUpdateFactory.zoomTo(zoom.toDouble()), null)
        onCameraMoveInvalidateListener.onInvalidateCameraMove()
    }

    private fun setPadding(padding: DoubleArray) {
        if (isTransitioning) {
            return
        }

        transform.moveCamera(maplibreMap, CameraUpdateFactory.paddingTo(padding), null)
        onCameraMoveInvalidateListener.onInvalidateCameraMove()
    }

    private fun setTilt(tilt: Float) {
        if (isTransitioning) {
            return
        }

        transform.moveCamera(maplibreMap, CameraUpdateFactory.tiltTo(tilt.toDouble()), null)
        onCameraMoveInvalidateListener.onInvalidateCameraMove()
    }

    private val latLngValueListener =
        AnimationsValueChangeListener<LatLng> { value ->
            setLatLng(value)
        }

    private val gpsBearingValueListener =
        AnimationsValueChangeListener<Float> { value ->
            val trackingNorth =
                cameraMode == CameraMode.TRACKING_GPS_NORTH &&
                    maplibreMap.cameraPosition.bearing == 0.0

            if (!trackingNorth) {
                setBearing(value)
            }
        }

    private val compassBearingValueListener =
        AnimationsValueChangeListener<Float> { value ->
            if (cameraMode == CameraMode.TRACKING_COMPASS || cameraMode == CameraMode.NONE_COMPASS) {
                setBearing(value)
            }
        }

    private val zoomValueListener = AnimationsValueChangeListener<Float> { value -> setZoom(value) }

    private val paddingValueListener = AnimationsValueChangeListener<DoubleArray> { value -> setPadding(value) }

    private val tiltValueListener = AnimationsValueChangeListener<Float> { value -> setTilt(value) }

    val animationListeners: Set<AnimatorListenerHolder>
        get() {
            val holders = mutableSetOf<AnimatorListenerHolder>()
            if (isLocationTracking()) {
                holders.add(AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_CAMERA_LATLNG, latLngValueListener))
            }

            if (isLocationBearingTracking()) {
                holders.add(
                    AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING, gpsBearingValueListener),
                )
            }

            if (isConsumingCompass) {
                holders.add(
                    AnimatorListenerHolder(
                        MapLibreAnimator.ANIMATOR_CAMERA_COMPASS_BEARING,
                        compassBearingValueListener,
                    ),
                )
            }

            holders.add(AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_ZOOM, zoomValueListener))
            holders.add(AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_TILT, tiltValueListener))
            holders.add(AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_PADDING, paddingValueListener))
            return holders
        }

    private fun adjustGesturesThresholds() {
        if (options.trackingGesturesManagement()) {
            if (isLocationTracking()) {
                moveGestureDetector.moveThreshold = options.trackingInitialMoveThreshold()
            } else {
                moveGestureDetector.moveThreshold = initialGesturesManager.moveGestureDetector.moveThreshold
                moveGestureDetector.moveThresholdRect = null
            }
        }
    }

    val isConsumingCompass: Boolean
        get() = cameraMode == CameraMode.TRACKING_COMPASS || cameraMode == CameraMode.NONE_COMPASS

    fun setEnabled(enabled: Boolean) {
        isEnabled = enabled
    }

    private fun isLocationTracking(): Boolean =
        cameraMode == CameraMode.TRACKING ||
            cameraMode == CameraMode.TRACKING_COMPASS ||
            cameraMode == CameraMode.TRACKING_GPS ||
            cameraMode == CameraMode.TRACKING_GPS_NORTH

    private fun isBearingTracking(): Boolean =
        cameraMode == CameraMode.NONE_COMPASS ||
            cameraMode == CameraMode.TRACKING_COMPASS ||
            cameraMode == CameraMode.NONE_GPS ||
            cameraMode == CameraMode.TRACKING_GPS ||
            cameraMode == CameraMode.TRACKING_GPS_NORTH

    private fun isLocationBearingTracking(): Boolean =
        cameraMode == CameraMode.TRACKING_GPS ||
            cameraMode == CameraMode.TRACKING_GPS_NORTH ||
            cameraMode == CameraMode.NONE_GPS

    private fun notifyCameraTrackingChangeListener(wasTracking: Boolean) {
        internalCameraTrackingChangedListener.onCameraTrackingChanged(cameraMode)
        if (wasTracking && !isLocationTracking()) {
            maplibreMap.uiSettings.focalPoint = null
            internalCameraTrackingChangedListener.onCameraTrackingDismissed()
        }
    }

    private val onCameraMoveListener =
        MapLibreMap.OnCameraMoveListener {
            val currentLastLocation = lastLocation
            if (isLocationTracking() && currentLastLocation != null && options.trackingGesturesManagement()) {
                val focalPoint = maplibreMap.projection.toScreenLocation(currentLastLocation)
                maplibreMap.uiSettings.focalPoint = focalPoint
            }
        }

    @VisibleForTesting
    val onMoveListener: MapLibreMap.OnMoveListener =
        object : MapLibreMap.OnMoveListener {
            private var interrupt = false

            override fun onMoveBegin(detector: MoveGestureDetector) {
                if (options.trackingGesturesManagement() && isLocationTracking()) {
                    if (detector.pointersCount > 1) {
                        applyMultiFingerThresholdArea(detector)
                        applyMultiFingerMoveThreshold(detector)
                    } else {
                        applySingleFingerMoveThreshold(detector)
                    }
                } else {
                    cameraMode = CameraMode.NONE
                }
            }

            private fun applyMultiFingerThresholdArea(detector: MoveGestureDetector) {
                val currentRect = detector.moveThresholdRect
                if (currentRect != null && currentRect != options.trackingMultiFingerProtectedMoveArea()) {
                    detector.moveThresholdRect = options.trackingMultiFingerProtectedMoveArea()
                    interrupt = true
                } else if (currentRect == null && options.trackingMultiFingerProtectedMoveArea() != null) {
                    detector.moveThresholdRect = options.trackingMultiFingerProtectedMoveArea()
                    interrupt = true
                }
            }

            private fun applyMultiFingerMoveThreshold(detector: MoveGestureDetector) {
                if (detector.moveThreshold != options.trackingMultiFingerMoveThreshold()) {
                    detector.moveThreshold = options.trackingMultiFingerMoveThreshold()
                    interrupt = true
                }
            }

            private fun applySingleFingerMoveThreshold(detector: MoveGestureDetector) {
                if (detector.moveThreshold != options.trackingInitialMoveThreshold()) {
                    detector.moveThreshold = options.trackingInitialMoveThreshold()
                    interrupt = true
                }
            }

            override fun onMove(detector: MoveGestureDetector) {
                if (interrupt) {
                    detector.interrupt()
                    return
                }

                if (isLocationTracking() || isBearingTracking()) {
                    cameraMode = CameraMode.NONE
                    detector.interrupt()
                }
            }

            override fun onMoveEnd(detector: MoveGestureDetector) {
                if (options.trackingGesturesManagement() && !interrupt && isLocationTracking()) {
                    detector.moveThreshold = options.trackingInitialMoveThreshold()
                    detector.moveThresholdRect = null
                }
                interrupt = false
            }
        }

    private val onRotateListener =
        object : MapLibreMap.OnRotateListener {
            override fun onRotateBegin(detector: RotateGestureDetector) {
                if (isBearingTracking()) {
                    cameraMode = CameraMode.NONE
                }
            }

            override fun onRotate(detector: RotateGestureDetector) {
                // no implementation
            }

            override fun onRotateEnd(detector: RotateGestureDetector) {
                // no implementation
            }
        }

    private val onFlingListener =
        MapLibreMap.OnFlingListener {
            cameraMode = CameraMode.NONE
        }

    private inner class LocationGesturesManager(
        context: Context,
    ) : AndroidGesturesManager(context) {
        override fun onTouchEvent(motionEvent: MotionEvent?): Boolean {
            if (motionEvent != null) {
                if (motionEvent.actionMasked == MotionEvent.ACTION_UP) {
                    adjustGesturesThresholds()
                }
            }
            return super.onTouchEvent(motionEvent)
        }
    }
}
