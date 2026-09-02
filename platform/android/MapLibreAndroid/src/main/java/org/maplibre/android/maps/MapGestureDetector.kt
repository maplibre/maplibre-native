package org.maplibre.android.maps

import android.animation.Animator
import android.animation.AnimatorListenerAdapter
import android.animation.ValueAnimator
import android.content.Context
import android.content.res.Resources
import android.graphics.PointF
import android.os.Handler
import android.os.Looper
import android.view.InputDevice
import android.view.MotionEvent
import android.view.animation.DecelerateInterpolator
import org.maplibre.android.R
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.constants.MapLibreConstants.MAXIMUM_ANGULAR_VELOCITY
import org.maplibre.android.constants.MapLibreConstants.MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE
import org.maplibre.android.constants.MapLibreConstants.QUICK_ZOOM_MAX_ZOOM_CHANGE
import org.maplibre.android.constants.MapLibreConstants.ROTATE_VELOCITY_RATIO_THRESHOLD
import org.maplibre.android.constants.MapLibreConstants.SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER
import org.maplibre.android.constants.MapLibreConstants.SCALE_VELOCITY_RATIO_THRESHOLD
import org.maplibre.android.constants.MapLibreConstants.ZOOM_RATE
import org.maplibre.android.gestures.AndroidGesturesManager
import org.maplibre.android.gestures.MoveGestureDetector
import org.maplibre.android.gestures.MultiFingerTapGestureDetector
import org.maplibre.android.gestures.RotateGestureDetector
import org.maplibre.android.gestures.ShoveGestureDetector
import org.maplibre.android.gestures.StandardGestureDetector
import org.maplibre.android.gestures.StandardScaleGestureDetector
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap.OnCameraMoveStartedListener.Companion.REASON_API_GESTURE
import org.maplibre.android.utils.MathUtils
import org.maplibre.android.utils.MathUtils.normalize
import java.util.concurrent.CopyOnWriteArrayList
import kotlin.math.E
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.atan
import kotlin.math.hypot
import kotlin.math.ln
import kotlin.math.pow

/**
 * Manages gestures events on a MapView.
 */
@Suppress("TooManyFunctions", "LargeClass")
internal class MapGestureDetector(
    context: Context?,
    private val transform: Transform,
    private val projection: Projection,
    private val uiSettings: UiSettings,
    private val annotationManager: AnnotationManager,
    private val cameraChangeDispatcher: CameraChangeDispatcher,
) {
    // new map touch API
    private val onMapClickListenerList = CopyOnWriteArrayList<MapLibreMap.OnMapClickListener>()

    private val onMapLongClickListenerList = CopyOnWriteArrayList<MapLibreMap.OnMapLongClickListener>()

    private val onFlingListenerList = CopyOnWriteArrayList<MapLibreMap.OnFlingListener>()

    private val onMoveListenerList = CopyOnWriteArrayList<MapLibreMap.OnMoveListener>()

    private val onRotateListenerList = CopyOnWriteArrayList<MapLibreMap.OnRotateListener>()

    private val onScaleListenerList = CopyOnWriteArrayList<MapLibreMap.OnScaleListener>()

    private val onShoveListenerList = CopyOnWriteArrayList<MapLibreMap.OnShoveListener>()

    /**
     * User-set focal point.
     */
    private var constantFocalPoint: PointF? = null

    private var doubleTapFocalPoint = PointF()

    private lateinit var gesturesManager: AndroidGesturesManager

    private var scaleAnimator: Animator? = null
    private var rotateAnimator: Animator? = null
    private val scheduledAnimators = mutableListOf<Animator>()

    /**
     * Cancels scheduled velocity animations if user doesn't lift fingers within
     * [MapLibreConstants.SCHEDULED_ANIMATION_TIMEOUT]
     */
    private val animationsTimeoutHandler = Handler(Looper.getMainLooper())

    private var doubleTapRegistered = false

    /**
     * Posted on main thread with [animationsTimeoutHandler]. Cancels all scheduled animators if needed.
     */
    private val cancelAnimatorsRunnable = Runnable { cancelAnimators() }

    init {
        // Checking for context != null for testing purposes
        if (context != null) {
            // Initialize gestures manager
            initializeGesturesManager(AndroidGesturesManager(context), true)

            // Initialize gesture listeners
            initializeGestureListeners(context, true)
        }
    }

    private fun initializeGestureListeners(
        context: Context,
        attachDefaultListeners: Boolean,
    ) {
        if (!attachDefaultListeners) {
            return
        }
        val resources = context.resources
        val standardGestureListener =
            StandardGestureListener(
                resources.getDimension(org.maplibre.android.gestures.R.dimen.mapbox_defaultScaleSpanSinceStartThreshold),
            )
        val moveGestureListener = MoveGestureListener()
        val scaleGestureListener =
            ScaleGestureListener(
                resources.getDimension(R.dimen.maplibre_density_constant).toDouble(),
                resources.getDimension(R.dimen.maplibre_minimum_scale_speed),
                resources.getDimension(R.dimen.maplibre_minimum_angled_scale_speed),
                resources.getDimension(R.dimen.maplibre_minimum_scale_velocity),
            )
        val rotateGestureListener =
            RotateGestureListener(
                resources.getDimension(R.dimen.maplibre_minimum_scale_span_when_rotating),
                resources.getDimension(R.dimen.maplibre_density_constant).toDouble(),
                resources.getDimension(R.dimen.maplibre_angular_velocity_multiplier),
                resources.getDimension(R.dimen.maplibre_minimum_angular_velocity),
                resources.getDimension(org.maplibre.android.gestures.R.dimen.mapbox_defaultScaleSpanSinceStartThreshold),
            )
        val shoveGestureListener = ShoveGestureListener()
        val tapGestureListener = TapGestureListener()

        gesturesManager.setStandardGestureListener(standardGestureListener)
        gesturesManager.setMoveGestureListener(moveGestureListener)
        gesturesManager.setStandardScaleGestureListener(scaleGestureListener)
        gesturesManager.setRotateGestureListener(rotateGestureListener)
        gesturesManager.setShoveGestureListener(shoveGestureListener)
        gesturesManager.setMultiFingerTapGestureListener(tapGestureListener)
    }

    private fun initializeGesturesManager(
        androidGesturesManager: AndroidGesturesManager,
        setDefaultMutuallyExclusives: Boolean,
    ) {
        if (setDefaultMutuallyExclusives) {
            val shoveScaleSet =
                setOf(
                    AndroidGesturesManager.GESTURE_TYPE_SHOVE,
                    AndroidGesturesManager.GESTURE_TYPE_SCALE,
                )
            val shoveRotateSet =
                setOf(
                    AndroidGesturesManager.GESTURE_TYPE_SHOVE,
                    AndroidGesturesManager.GESTURE_TYPE_ROTATE,
                )
            val scaleLongPressSet =
                setOf(
                    AndroidGesturesManager.GESTURE_TYPE_SCALE,
                    AndroidGesturesManager.GESTURE_TYPE_LONG_PRESS,
                )

            androidGesturesManager.setMutuallyExclusiveGestures(shoveScaleSet, shoveRotateSet, scaleLongPressSet)
        }

        // If this was 0°, every shove gesture (for tilting the map) would be detected as also a rotate
        androidGesturesManager.rotateGestureDetector.setAngleThreshold(3f)

        // If this was 0 (the default), a simple tap would also be detected as a move. A (very) small
        // move threshold solves this issue, while not making the map feel "sticky". (See #2792)
        androidGesturesManager.moveGestureDetector.setMoveThresholdResource(R.dimen.maplibre_minimum_move_threshold)

        gesturesManager = androidGesturesManager
    }

    /**
     * Set the gesture focal point.
     *
     * This is the center point used for calculate transformations from gestures, value is
     * overridden if end user provides his own through [UiSettings.focalPoint].
     *
     * @param focalPoint the center point for gestures
     */
    fun setFocalPoint(focalPoint: PointF?) {
        // resetting focal point, using user provided one to reset
        constantFocalPoint = focalPoint ?: uiSettings.focalPoint
    }

    /**
     * Called when user touches the screen, all positions are absolute.
     *
     * Forwards event to the related gesture detectors.
     *
     * @param motionEvent the MotionEvent
     * @return True if touch event is handled
     */
    fun onTouchEvent(motionEvent: MotionEvent?): Boolean {
        // Framework can return null motion events in edge cases #9432
        if (motionEvent == null) {
            return false
        }

        // Check and ignore non touch or left clicks
        if (motionEvent.buttonState != 0 && motionEvent.buttonState != MotionEvent.BUTTON_PRIMARY) {
            return false
        }

        if (motionEvent.actionMasked == MotionEvent.ACTION_DOWN) {
            cancelAnimators()
            transform.setGestureInProgress(true)
        }

        val result = gesturesManager.onTouchEvent(motionEvent)

        when (motionEvent.actionMasked) {
            MotionEvent.ACTION_POINTER_DOWN -> {
                doubleTapFinished()
            }

            MotionEvent.ACTION_UP -> {
                doubleTapFinished()
                transform.setGestureInProgress(false)

                if (scheduledAnimators.isNotEmpty()) {
                    // Start all awaiting velocity animations
                    animationsTimeoutHandler.removeCallbacksAndMessages(null)
                    for (animator in scheduledAnimators) {
                        animator.start()
                    }
                    scheduledAnimators.clear()
                }
            }

            MotionEvent.ACTION_CANCEL -> {
                scheduledAnimators.clear()
                transform.setGestureInProgress(false)
                doubleTapFinished()
            }
        }

        return result
    }

    fun cancelAnimators() {
        animationsTimeoutHandler.removeCallbacksAndMessages(null)
        scheduledAnimators.clear()

        cancelAnimator(scaleAnimator)
        cancelAnimator(rotateAnimator)

        dispatchCameraIdle()
    }

    private fun cancelAnimator(animator: Animator?) {
        if (animator != null && animator.isStarted) {
            animator.cancel()
        }
    }

    /**
     * Schedules a velocity animator to be executed when user lifts fingers,
     * unless canceled by the [cancelAnimatorsRunnable].
     *
     * @param animator animator ot be scheduled
     */
    private fun scheduleAnimator(animator: Animator) {
        scheduledAnimators.add(animator)
        animationsTimeoutHandler.removeCallbacksAndMessages(null)
        animationsTimeoutHandler.postDelayed(cancelAnimatorsRunnable, MapLibreConstants.SCHEDULED_ANIMATION_TIMEOUT)
    }

    /**
     * Called for events that don't fit the other handlers.
     *
     * Examples of such events are mouse scroll events, mouse moves, joystick & trackpad.
     *
     * @param event The MotionEvent occurred
     * @return True is the event is handled
     */
    fun onGenericMotionEvent(event: MotionEvent): Boolean {
        // Mouse events
        // if (event.isFromSource(InputDevice.SOURCE_CLASS_POINTER)) { // this is not available before API 18
        if (event.source and InputDevice.SOURCE_CLASS_POINTER != InputDevice.SOURCE_CLASS_POINTER) {
            // We are not interested in this event
            return false
        }

        // Choose the action
        return when (event.actionMasked) {
            // Mouse scrolls
            MotionEvent.ACTION_SCROLL -> {
                if (!uiSettings.isZoomGesturesEnabled) {
                    return false
                }

                // Cancel any animation
                transform.cancelTransitions()

                // Get the vertical scroll amount, one click = 1
                val scrollDist = event.getAxisValue(MotionEvent.AXIS_VSCROLL)

                // Scale the map by the appropriate power of two factor
                transform.zoomBy(scrollDist.toDouble(), PointF(event.x, event.y))

                true
            }

            // We are not interested in this event
            else -> {
                false
            }
        }
    }

    private inner class StandardGestureListener(
        private val doubleTapMovementThreshold: Float,
    ) : StandardGestureDetector.SimpleStandardOnGestureListener() {
        override fun onDown(motionEvent: MotionEvent): Boolean = true

        override fun onSingleTapUp(motionEvent: MotionEvent): Boolean {
            transform.cancelTransitions()
            return true
        }

        override fun onSingleTapConfirmed(motionEvent: MotionEvent): Boolean {
            val tapPoint = PointF(motionEvent.x, motionEvent.y)
            val tapHandled = annotationManager.onTap(tapPoint)

            if (!tapHandled) {
                if (uiSettings.isDeselectMarkersOnTap) {
                    // deselect any selected marker
                    annotationManager.deselectMarkers()
                }

                notifyOnMapClickListeners(tapPoint)
            }

            return true
        }

        override fun onDoubleTapEvent(motionEvent: MotionEvent): Boolean {
            if (motionEvent.actionMasked == MotionEvent.ACTION_DOWN) {
                doubleTapFocalPoint = PointF(motionEvent.x, motionEvent.y)
                doubleTapStarted()
            }

            if (motionEvent.actionMasked == MotionEvent.ACTION_UP) {
                val diffX = abs(motionEvent.x - doubleTapFocalPoint.x)
                val diffY = abs(motionEvent.y - doubleTapFocalPoint.y)
                if (diffX > doubleTapMovementThreshold || diffY > doubleTapMovementThreshold) {
                    // Ignore double-tap event because we've started the quick-zoom. See #14013.
                    return false
                }

                if (!uiSettings.isZoomGesturesEnabled || !uiSettings.isDoubleTapGesturesEnabled) {
                    return false
                }

                // Single finger double tap
                constantFocalPoint?.let {
                    // User provided focal point
                    doubleTapFocalPoint = it
                }

                zoomInAnimated(doubleTapFocalPoint, false)

                return true
            }

            return super.onDoubleTapEvent(motionEvent)
        }

        override fun onLongPress(motionEvent: MotionEvent) {
            notifyOnMapLongClickListeners(PointF(motionEvent.x, motionEvent.y))
        }

        @Suppress("ReturnCount")
        override fun onFling(
            e1: MotionEvent?,
            e2: MotionEvent,
            velocityX: Float,
            velocityY: Float,
        ): Boolean {
            if (!uiSettings.isScrollGesturesEnabled) {
                // don't allow a fling if scroll is disabled
                return false
            }

            if (!uiSettings.isFlingVelocityAnimationEnabled) {
                return false
            }

            val screenDensity = uiSettings.getPixelRatio()

            // calculate velocity vector for xy dimensions, independent from screen size
            val velocityXY = hypot((velocityX / screenDensity).toDouble(), (velocityY / screenDensity).toDouble())
            if (velocityXY < uiSettings.flingThreshold) {
                // ignore short flings, these can occur when other gestures just have finished executing
                return false
            }

            // tilt results in a bigger translation, limiting input for #5281
            val tilt = transform.getTilt()
            val tiltFactor = 1.5 + (if (tilt != 0.0) tilt / 10 else 0.0)

            // calculate animation time based on displacement
            val animationTime = (velocityXY / 7 / tiltFactor + uiSettings.flingAnimationBaseTime).toLong()

            // screenDensity and influcentcetilt come in here via animationTime
            // factor 1000 because speed is in pixels/s
            // and the factor 0.28 was determined by testing: panning the map and releasing
            //  should result in fling animation starting at same speed as the move before
            var offsetX = velocityX * animationTime * 0.28 / 1000
            val offsetY = velocityY * animationTime * 0.28 / 1000

            if (!uiSettings.isHorizontalScrollGesturesEnabled) {
                // determine if angle of fling is valid for performing a vertical fling
                val angle = abs(Math.toDegrees(atan(offsetX / offsetY)))
                if (angle > MapLibreConstants.ANGLE_THRESHOLD_IGNORE_VERTICAL_FLING) {
                    return false
                }
                offsetX = 0.0
            }

            transform.cancelTransitions()
            notifyOnFlingListeners()
            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)

            // update transformation
            transform.moveBy(offsetX, offsetY, animationTime)

            return true
        }
    }

    private fun doubleTapStarted() {
        // disable the move detector in preparation for the quickzoom,
        // so that we don't move the map's center slightly before the quickzoom is started (see #14227)
        gesturesManager.moveGestureDetector.isEnabled = false
        doubleTapRegistered = true
    }

    private fun doubleTapFinished() {
        if (doubleTapRegistered) {
            // re-enable the move detector in case of double tap
            gesturesManager.moveGestureDetector.isEnabled = true
            doubleTapRegistered = false
        }
    }

    private inner class MoveGestureListener : MoveGestureDetector.SimpleOnMoveGestureListener() {
        override fun onMoveBegin(detector: MoveGestureDetector): Boolean {
            if (!uiSettings.isScrollGesturesEnabled) {
                return false
            }

            cancelTransitionsIfRequired()
            notifyOnMoveBeginListeners(detector)
            return true
        }

        override fun onMove(
            detector: MoveGestureDetector,
            distanceX: Float,
            distanceY: Float,
        ): Boolean {
            if (distanceX.isNaN() || distanceY.isNaN()) {
                Logger.e(TAG, "Could not call onMove with parameters $distanceX,$distanceY")
            } else if (distanceX != 0f || distanceY != 0f) {
                // first move event is often delivered with no displacement
                // dispatching camera start event only when the movement actually occurred
                cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)

                // Disable scrolling horizontal if not allowed
                val moveX = if (uiSettings.isHorizontalScrollGesturesEnabled) distanceX else 0f

                // Scroll the map
                transform.moveBy(-moveX.toDouble(), -distanceY.toDouble(), 0)

                notifyOnMoveListeners(detector)
            }
            return true
        }

        override fun onMoveEnd(
            detector: MoveGestureDetector,
            velocityX: Float,
            velocityY: Float,
        ) {
            dispatchCameraIdle()
            notifyOnMoveEndListeners(detector)
        }
    }

    private inner class ScaleGestureListener(
        densityMultiplier: Double,
        private val minimumGestureSpeed: Float,
        private val minimumAngledGestureSpeed: Float,
        private val minimumVelocity: Float,
    ) : StandardScaleGestureDetector.SimpleStandardOnScaleGestureListener() {
        private val scaleVelocityRatioThreshold = SCALE_VELOCITY_RATIO_THRESHOLD * densityMultiplier
        private var quickZoom = false
        private var spanSinceLast = 0f
        private var screenHeight = 0.0
        private var startZoom = 0.0

        @Suppress("ReturnCount")
        override fun onScaleBegin(detector: StandardScaleGestureDetector): Boolean {
            quickZoom = detector.pointersCount == 1

            if (!uiSettings.isZoomGesturesEnabled) {
                return false
            }

            if (quickZoom) {
                if (!uiSettings.isQuickZoomGesturesEnabled) {
                    return false
                }
                // re-try disabling the move detector in case double tap has been interrupted before quickzoom started
                gesturesManager.moveGestureDetector.isEnabled = false
            } else {
                if (detector.previousSpan <= 0) {
                    return false
                }

                val currSpan = detector.currentSpan
                val prevSpan = detector.previousSpan
                val currTime = detector.currentEvent.eventTime.toDouble()
                val prevTime = detector.previousEvent.eventTime.toDouble()
                if (currTime == prevTime) {
                    return false
                }
                val speed = abs(currSpan - prevSpan) / (currTime - prevTime)
                if (speed < minimumGestureSpeed) {
                    // do not scale if the minimal gesture speed is not met
                    return false
                } else if (!gesturesManager.rotateGestureDetector.isInProgress) {
                    val rotationDeltaSinceLast = gesturesManager.rotateGestureDetector.deltaSinceLast
                    if (abs(rotationDeltaSinceLast) > 0.4 && speed < minimumAngledGestureSpeed) {
                        // do not scale in case we're preferring to start rotation
                        return false
                    }

                    if (uiSettings.isDisableRotateWhenScaling) {
                        // disable rotate gesture when scale is detected first
                        gesturesManager.rotateGestureDetector.isEnabled = false
                    }
                }
            }

            screenHeight =
                Resources
                    .getSystem()
                    .displayMetrics.heightPixels
                    .toDouble()
            startZoom = transform.getRawZoom()

            cancelTransitionsIfRequired()

            notifyOnScaleBeginListeners(detector)

            spanSinceLast = abs(detector.currentSpan - detector.previousSpan)

            return true
        }

        override fun onScale(detector: StandardScaleGestureDetector): Boolean {
            // dispatching camera start event only when the movement actually occurred
            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)

            val focalPoint = getScaleFocalPoint(detector)
            if (quickZoom) {
                val pixelDeltaChange = abs(detector.currentEvent.y - doubleTapFocalPoint.y).toDouble()
                val zoomedOut = detector.currentEvent.y < doubleTapFocalPoint.y

                // normalize the pixel delta change, ranging from 0 to screen height, to a constant zoom change range
                val normalizedDeltaChange =
                    normalize(pixelDeltaChange, 0.0, screenHeight, 0.0, QUICK_ZOOM_MAX_ZOOM_CHANGE)

                // calculate target zoom and adjust for a multiplier
                var targetZoom = if (zoomedOut) startZoom - normalizedDeltaChange else startZoom + normalizedDeltaChange
                targetZoom *= uiSettings.zoomRate

                transform.setZoom(targetZoom, focalPoint)
            } else {
                val zoomBy = ln(detector.scaleFactor.toDouble()) / ln(PI / 2) * ZOOM_RATE * uiSettings.zoomRate
                transform.zoomBy(zoomBy, focalPoint)
            }

            notifyOnScaleListeners(detector)

            spanSinceLast = abs(detector.currentSpan - detector.previousSpan)

            return true
        }

        override fun onScaleEnd(
            detector: StandardScaleGestureDetector,
            velocityX: Float,
            velocityY: Float,
        ) {
            if (quickZoom) {
                // re-enabled the move detector if the quickzoom happened
                gesturesManager.moveGestureDetector.isEnabled = true
            } else {
                // re-enable rotation in case it's been disabled
                gesturesManager.rotateGestureDetector.isEnabled = true
            }

            notifyOnScaleEndListeners(detector)

            val velocityXY = abs(velocityX) + abs(velocityY)

            if (!uiSettings.isScaleVelocityAnimationEnabled ||
                velocityXY < minimumVelocity ||
                spanSinceLast / velocityXY < scaleVelocityRatioThreshold
            ) {
                // notifying listeners that camera is idle only if there is no follow-up animation
                dispatchCameraIdle()
                return
            }

            val zoomAddition = calculateScale(velocityXY.toDouble(), detector.isScalingOut)
            val currentZoom = transform.getRawZoom()
            val focalPoint = getScaleFocalPoint(detector)
            // (log(x + 1 / e^2) + 2) * 150, x=0 to 2.5 (MapLibreConstants#MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE)
            val animationTime =
                ((ln(abs(zoomAddition) + 1 / E.pow(2)) + 2) * SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER).toLong()
            scaleAnimator =
                createScaleAnimator(currentZoom, zoomAddition, focalPoint, animationTime).also {
                    scheduleAnimator(it)
                }
        }

        private fun getScaleFocalPoint(detector: StandardScaleGestureDetector): PointF {
            val constantFocalPoint = this@MapGestureDetector.constantFocalPoint
            return when {
                // around user provided focal point
                constantFocalPoint != null -> constantFocalPoint

                // around center
                quickZoom -> PointF(uiSettings.width / 2, uiSettings.height / 2)

                // around gesture
                else -> detector.focalPoint
            }
        }

        private fun calculateScale(
            velocityXY: Double,
            isScalingOut: Boolean,
        ): Double {
            val zoomAddition =
                MathUtils.clamp(
                    velocityXY * MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE * 1e-4,
                    0.0,
                    MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE,
                )
            return if (isScalingOut) -zoomAddition else zoomAddition
        }
    }

    private inner class RotateGestureListener(
        private val minimumScaleSpanWhenRotating: Float,
        densityMultiplier: Double,
        private val angularVelocityMultiplier: Float,
        private val minimumAngularVelocity: Float,
        private val defaultSpanSinceStartThreshold: Float,
    ) : RotateGestureDetector.SimpleOnRotateGestureListener() {
        private val rotateVelocityRatioThreshold = ROTATE_VELOCITY_RATIO_THRESHOLD * densityMultiplier

        @Suppress("ReturnCount")
        override fun onRotateBegin(detector: RotateGestureDetector): Boolean {
            if (!uiSettings.isRotateGesturesEnabled) {
                return false
            }

            val deltaSinceLast = abs(detector.deltaSinceLast)
            val currTime = detector.currentEvent.eventTime.toDouble()
            val prevTime = detector.previousEvent.eventTime.toDouble()
            if (currTime == prevTime) {
                return false
            }
            val speed = deltaSinceLast / (currTime - prevTime)
            val deltaSinceStart = abs(detector.deltaSinceStart)

            // adjust the responsiveness of a rotation gesture - the higher the speed, the bigger the threshold
            if (speed < 0.04 ||
                (speed > 0.07 && deltaSinceStart < 5) ||
                (speed > 0.15 && deltaSinceStart < 7) ||
                (speed > 0.5 && deltaSinceStart < 15)
            ) {
                return false
            }

            if (uiSettings.isIncreaseScaleThresholdWhenRotating) {
                // when rotation starts, interrupting scale and increasing the threshold
                // to make rotation without scaling easier
                gesturesManager.standardScaleGestureDetector.setSpanSinceStartThreshold(minimumScaleSpanWhenRotating)
                gesturesManager.standardScaleGestureDetector.interrupt()
            }

            cancelTransitionsIfRequired()

            notifyOnRotateBeginListeners(detector)

            return true
        }

        override fun onRotate(
            detector: RotateGestureDetector,
            rotationDegreesSinceLast: Float,
            rotationDegreesSinceFirst: Float,
        ): Boolean {
            // dispatching camera start event only when the movement actually occurred
            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)

            // Calculate map bearing value
            val bearing = transform.getRawBearing() + rotationDegreesSinceLast

            // Rotate the map
            val focalPoint = getRotateFocalPoint(detector)
            transform.setBearing(bearing, focalPoint.x, focalPoint.y)

            notifyOnRotateListeners(detector)

            return true
        }

        override fun onRotateEnd(
            detector: RotateGestureDetector,
            velocityX: Float,
            velocityY: Float,
            angularVelocity: Float,
        ) {
            if (uiSettings.isIncreaseScaleThresholdWhenRotating) {
                // resetting default scale threshold values
                gesturesManager.standardScaleGestureDetector
                    .setSpanSinceStartThreshold(defaultSpanSinceStartThreshold)
            }

            notifyOnRotateEndListeners(detector)

            val clampedVelocity =
                MathUtils.clamp(
                    angularVelocity * angularVelocityMultiplier,
                    -MAXIMUM_ANGULAR_VELOCITY,
                    MAXIMUM_ANGULAR_VELOCITY,
                )

            val velocityXY = abs(velocityX) + abs(velocityY)
            val delta = abs(detector.deltaSinceLast)
            val ratio = (delta / velocityXY).toDouble()

            if (!uiSettings.isRotateVelocityAnimationEnabled ||
                abs(clampedVelocity) < minimumAngularVelocity ||
                (gesturesManager.standardScaleGestureDetector.isInProgress && ratio < rotateVelocityRatioThreshold)
            ) {
                // notifying listeners that camera is idle only if there is no follow-up animation
                dispatchCameraIdle()
                return
            }

            val animationTime =
                ((ln(abs(clampedVelocity) + 1 / E.pow(2)) + 2) * SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER).toLong()

            val focalPoint = getRotateFocalPoint(detector)
            rotateAnimator =
                createRotateAnimator(clampedVelocity, animationTime, focalPoint).also {
                    scheduleAnimator(it)
                }
        }

        private fun getRotateFocalPoint(detector: RotateGestureDetector): PointF =
            // User provided focal point, otherwise around gesture
            this@MapGestureDetector.constantFocalPoint ?: detector.focalPoint

        private fun createRotateAnimator(
            angularVelocity: Float,
            animationTime: Long,
            animationFocalPoint: PointF,
        ): Animator =
            ValueAnimator.ofFloat(angularVelocity, 0f).apply {
                duration = animationTime
                interpolator = DecelerateInterpolator()
                addUpdateListener { animation ->
                    transform.setBearing(
                        transform.getRawBearing() + animation.animatedValue as Float,
                        animationFocalPoint.x,
                        animationFocalPoint.y,
                        0L,
                    )
                }

                addListener(
                    object : AnimatorListenerAdapter() {
                        override fun onAnimationStart(animation: Animator) {
                            transform.cancelTransitions()
                            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)
                        }

                        override fun onAnimationCancel(animation: Animator) {
                            transform.cancelTransitions()
                        }

                        override fun onAnimationEnd(animation: Animator) {
                            dispatchCameraIdle()
                        }
                    },
                )
            }
    }

    private inner class ShoveGestureListener : ShoveGestureDetector.SimpleOnShoveGestureListener() {
        override fun onShoveBegin(detector: ShoveGestureDetector): Boolean {
            if (!uiSettings.isTiltGesturesEnabled) {
                return false
            }

            cancelTransitionsIfRequired()

            // disabling move gesture during shove
            gesturesManager.moveGestureDetector.isEnabled = false

            notifyOnShoveBeginListeners(detector)

            return true
        }

        override fun onShove(
            detector: ShoveGestureDetector,
            deltaPixelsSinceLast: Float,
            deltaPixelsSinceStart: Float,
        ): Boolean {
            // dispatching camera start event only when the movement actually occurred
            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)

            // Get tilt value (scale and clamp)
            val pitch =
                MathUtils.clamp(
                    transform.getTilt() - MapLibreConstants.SHOVE_PIXEL_CHANGE_FACTOR * deltaPixelsSinceLast,
                    MapLibreConstants.MINIMUM_TILT,
                    MapLibreConstants.MAXIMUM_TILT,
                )

            // Tilt the map
            transform.setTilt(pitch)

            notifyOnShoveListeners(detector)

            return true
        }

        override fun onShoveEnd(
            detector: ShoveGestureDetector,
            velocityX: Float,
            velocityY: Float,
        ) {
            dispatchCameraIdle()

            // re-enabling move gesture
            gesturesManager.moveGestureDetector.isEnabled = true

            notifyOnShoveEndListeners(detector)
        }
    }

    private inner class TapGestureListener : MultiFingerTapGestureDetector.OnMultiFingerTapGestureListener {
        override fun onMultiFingerTap(
            detector: MultiFingerTapGestureDetector,
            pointersCount: Int,
        ): Boolean {
            if (!uiSettings.isZoomGesturesEnabled || pointersCount != 2) {
                return false
            }

            transform.cancelTransitions()
            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)

            // Single finger double tap, user provided focal point or zoom in on gesture
            val zoomFocalPoint = constantFocalPoint ?: detector.focalPoint

            zoomOutAnimated(zoomFocalPoint, false)

            return true
        }
    }

    private fun createScaleAnimator(
        currentZoom: Double,
        zoomAddition: Double,
        animationFocalPoint: PointF,
        animationTime: Long,
    ): Animator =
        ValueAnimator.ofFloat(currentZoom.toFloat(), (currentZoom + zoomAddition).toFloat()).apply {
            duration = animationTime
            interpolator = DecelerateInterpolator()
            addUpdateListener { animation ->
                transform.setZoom((animation.animatedValue as Float).toDouble(), animationFocalPoint)
            }

            addListener(
                object : AnimatorListenerAdapter() {
                    override fun onAnimationStart(animation: Animator) {
                        transform.cancelTransitions()
                        cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE)
                    }

                    override fun onAnimationCancel(animation: Animator) {
                        transform.cancelTransitions()
                    }

                    override fun onAnimationEnd(animation: Animator) {
                        dispatchCameraIdle()
                    }
                },
            )
        }

    /**
     * Zoom in by 1.
     *
     * @param zoomFocalPoint focal point of zoom animation
     * @param runImmediately if true, animation will be started right away, otherwise it will wait until
     *                       [MotionEvent.ACTION_UP] is registered.
     */
    fun zoomInAnimated(
        zoomFocalPoint: PointF,
        runImmediately: Boolean,
    ) {
        zoomAnimated(true, zoomFocalPoint, runImmediately)
    }

    /**
     * Zoom out by 1.
     *
     * @param zoomFocalPoint focal point of zoom animation
     * @param runImmediately if true, animation will be started right away, otherwise it will wait until
     *                       [MotionEvent.ACTION_UP] is registered.
     */
    fun zoomOutAnimated(
        zoomFocalPoint: PointF,
        runImmediately: Boolean,
    ) {
        zoomAnimated(false, zoomFocalPoint, runImmediately)
    }

    private fun zoomAnimated(
        zoomIn: Boolean,
        zoomFocalPoint: PointF,
        runImmediately: Boolean,
    ) {
        // canceling here as well, because when using a button it will not be canceled automatically by onDown()
        cancelAnimator(scaleAnimator)

        val currentZoom = transform.getRawZoom()
        val animator =
            createScaleAnimator(
                currentZoom,
                if (zoomIn) 1.0 else -1.0,
                zoomFocalPoint,
                MapLibreConstants.ANIMATION_DURATION.toLong(),
            )
        scaleAnimator = animator
        if (runImmediately) {
            animator.start()
        } else {
            scheduleAnimator(animator)
        }
    }

    private fun dispatchCameraIdle() {
        // we need to dispatch camera idle callback only if there is no other gestures in progress
        if (noGesturesInProgress()) {
            // invalidate the camera position, so that it's valid when fetched from the #onIdle event
            // and doesn't rely on the last frame being rendered
            transform.invalidateCameraPosition()
            cameraChangeDispatcher.onCameraIdle()
        }
    }

    private fun cancelTransitionsIfRequired() {
        // we need to cancel core transitions only if there is no started gesture yet
        if (noGesturesInProgress()) {
            transform.cancelTransitions()
        }
    }

    private fun noGesturesInProgress(): Boolean =
        (!uiSettings.isScrollGesturesEnabled || !gesturesManager.moveGestureDetector.isInProgress) &&
            (!uiSettings.isZoomGesturesEnabled || !gesturesManager.standardScaleGestureDetector.isInProgress) &&
            (!uiSettings.isRotateGesturesEnabled || !gesturesManager.rotateGestureDetector.isInProgress) &&
            (!uiSettings.isTiltGesturesEnabled || !gesturesManager.shoveGestureDetector.isInProgress)

    fun notifyOnMapClickListeners(tapPoint: PointF) {
        for (listener in onMapClickListenerList) {
            if (listener.onMapClick(projection.fromScreenLocation(tapPoint))) {
                return
            }
        }
    }

    fun notifyOnMapLongClickListeners(longClickPoint: PointF) {
        for (listener in onMapLongClickListenerList) {
            if (listener.onMapLongClick(projection.fromScreenLocation(longClickPoint))) {
                return
            }
        }
    }

    fun notifyOnFlingListeners() {
        for (listener in onFlingListenerList) {
            listener.onFling()
        }
    }

    fun notifyOnMoveBeginListeners(detector: MoveGestureDetector) {
        for (listener in onMoveListenerList) {
            listener.onMoveBegin(detector)
        }
    }

    fun notifyOnMoveListeners(detector: MoveGestureDetector) {
        for (listener in onMoveListenerList) {
            listener.onMove(detector)
        }
    }

    fun notifyOnMoveEndListeners(detector: MoveGestureDetector) {
        for (listener in onMoveListenerList) {
            listener.onMoveEnd(detector)
        }
    }

    fun notifyOnRotateBeginListeners(detector: RotateGestureDetector) {
        for (listener in onRotateListenerList) {
            listener.onRotateBegin(detector)
        }
    }

    fun notifyOnRotateListeners(detector: RotateGestureDetector) {
        for (listener in onRotateListenerList) {
            listener.onRotate(detector)
        }
    }

    fun notifyOnRotateEndListeners(detector: RotateGestureDetector) {
        for (listener in onRotateListenerList) {
            listener.onRotateEnd(detector)
        }
    }

    fun notifyOnScaleBeginListeners(detector: StandardScaleGestureDetector) {
        for (listener in onScaleListenerList) {
            listener.onScaleBegin(detector)
        }
    }

    fun notifyOnScaleListeners(detector: StandardScaleGestureDetector) {
        for (listener in onScaleListenerList) {
            listener.onScale(detector)
        }
    }

    fun notifyOnScaleEndListeners(detector: StandardScaleGestureDetector) {
        for (listener in onScaleListenerList) {
            listener.onScaleEnd(detector)
        }
    }

    fun notifyOnShoveBeginListeners(detector: ShoveGestureDetector) {
        for (listener in onShoveListenerList) {
            listener.onShoveBegin(detector)
        }
    }

    fun notifyOnShoveListeners(detector: ShoveGestureDetector) {
        for (listener in onShoveListenerList) {
            listener.onShove(detector)
        }
    }

    fun notifyOnShoveEndListeners(detector: ShoveGestureDetector) {
        for (listener in onShoveListenerList) {
            listener.onShoveEnd(detector)
        }
    }

    fun addOnMapClickListener(onMapClickListener: MapLibreMap.OnMapClickListener) {
        onMapClickListenerList.add(onMapClickListener)
    }

    fun removeOnMapClickListener(onMapClickListener: MapLibreMap.OnMapClickListener) {
        onMapClickListenerList.remove(onMapClickListener)
    }

    fun addOnMapLongClickListener(onMapLongClickListener: MapLibreMap.OnMapLongClickListener) {
        onMapLongClickListenerList.add(onMapLongClickListener)
    }

    fun removeOnMapLongClickListener(onMapLongClickListener: MapLibreMap.OnMapLongClickListener) {
        onMapLongClickListenerList.remove(onMapLongClickListener)
    }

    fun addOnFlingListener(onFlingListener: MapLibreMap.OnFlingListener) {
        onFlingListenerList.add(onFlingListener)
    }

    fun removeOnFlingListener(onFlingListener: MapLibreMap.OnFlingListener) {
        onFlingListenerList.remove(onFlingListener)
    }

    fun addOnMoveListener(listener: MapLibreMap.OnMoveListener) {
        onMoveListenerList.add(listener)
    }

    fun removeOnMoveListener(listener: MapLibreMap.OnMoveListener) {
        onMoveListenerList.remove(listener)
    }

    fun addOnRotateListener(listener: MapLibreMap.OnRotateListener) {
        onRotateListenerList.add(listener)
    }

    fun removeOnRotateListener(listener: MapLibreMap.OnRotateListener) {
        onRotateListenerList.remove(listener)
    }

    fun addOnScaleListener(listener: MapLibreMap.OnScaleListener) {
        onScaleListenerList.add(listener)
    }

    fun removeOnScaleListener(listener: MapLibreMap.OnScaleListener) {
        onScaleListenerList.remove(listener)
    }

    fun addShoveListener(listener: MapLibreMap.OnShoveListener) {
        onShoveListenerList.add(listener)
    }

    fun removeShoveListener(listener: MapLibreMap.OnShoveListener) {
        onShoveListenerList.remove(listener)
    }

    fun getGesturesManager(): AndroidGesturesManager = gesturesManager

    fun setGesturesManager(
        context: Context,
        gesturesManager: AndroidGesturesManager,
        attachDefaultListeners: Boolean,
        setDefaultMutuallyExclusives: Boolean,
    ) {
        initializeGesturesManager(gesturesManager, setDefaultMutuallyExclusives)
        initializeGestureListeners(context, attachDefaultListeners)
    }

    private companion object {
        const val TAG = "MapGestureDetector"
    }
}
