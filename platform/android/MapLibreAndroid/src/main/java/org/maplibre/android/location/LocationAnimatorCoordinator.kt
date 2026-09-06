package org.maplibre.android.location

import android.animation.Animator
import android.location.Location
import android.os.SystemClock
import android.util.SparseArray
import android.view.animation.DecelerateInterpolator
import android.view.animation.LinearInterpolator
import androidx.annotation.Size
import androidx.annotation.VisibleForTesting
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.ACCURACY_RADIUS_ANIMATION_DURATION
import org.maplibre.android.location.LocationComponentConstants.COMPASS_UPDATE_RATE_MS
import org.maplibre.android.location.LocationComponentConstants.MAX_ANIMATION_DURATION_MS
import org.maplibre.android.location.LocationComponentConstants.TRANSITION_ANIMATION_DURATION_MS
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener
import org.maplibre.android.location.Utils.immediateAnimation
import org.maplibre.android.location.Utils.normalize
import org.maplibre.android.location.Utils.shortestRotation
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Projection
import kotlin.math.min

internal class LocationAnimatorCoordinator(
    private val projection: Projection,
    private val animatorSetProvider: MapLibreAnimatorSetProvider,
    private val animatorProvider: MapLibreAnimatorProvider,
) {
    @VisibleForTesting
    val animatorArray = SparseArray<MapLibreAnimator<*>>()

    @VisibleForTesting
    val listeners = SparseArray<AnimationsValueChangeListener<*>>()

    private var previousLocation: Location? = null
    private var previousAccuracyRadius = -1f
    private var previousCompassBearing = -1f
    private var locationUpdateTimestamp = -1L
    private var durationMultiplier = 0f
    private var compassAnimationEnabled = false
    private var accuracyAnimationEnabled = false

    @VisibleForTesting
    var maxAnimationFps = Int.MAX_VALUE
        set(value) {
            if (value <= 0) {
                Logger.e(TAG, "Max animation FPS cannot be less or equal to 0.")
                return
            }
            field = value
        }

    fun updateAnimatorListenerHolders(listenerHolders: Set<AnimatorListenerHolder>) {
        listeners.clear()
        for (holder in listenerHolders) {
            listeners.append(holder.animatorType, holder.listener)
        }

        for (i in 0 until animatorArray.size()) {
            @MapLibreAnimator.Type val animatorType = animatorArray.keyAt(i)
            if (listeners.get(animatorType) == null) {
                animatorArray.get(animatorType)?.makeInvalid()
            }
        }
    }

    fun feedNewLocation(
        newLocation: Location,
        currentCameraPosition: CameraPosition,
        isGpsNorth: Boolean,
    ) {
        feedNewLocation(arrayOf(newLocation), currentCameraPosition, isGpsNorth, false)
    }

    fun feedNewLocation(
        @Size(min = 1) newLocations: Array<Location>,
        currentCameraPosition: CameraPosition,
        isGpsNorth: Boolean,
        lookAheadUpdate: Boolean,
    ) {
        val newLocation = newLocations[newLocations.size - 1]
        if (previousLocation == null) {
            previousLocation = newLocation
            locationUpdateTimestamp = SystemClock.elapsedRealtime() - TRANSITION_ANIMATION_DURATION_MS
        }

        val previousLayerLatLng = getPreviousLayerLatLng()
        val previousLayerBearing = getPreviousLayerGpsBearing()
        val previousCameraLatLng = currentCameraPosition.target ?: previousLayerLatLng
        val previousCameraBearing = normalize(currentCameraPosition.bearing.toFloat())

        // generate targets for layer
        val latLngValues = getLatLngValues(previousLayerLatLng, newLocations)
        var bearingValues = getBearingValues(previousLayerBearing, newLocations)
        updateLayerAnimators(latLngValues, bearingValues)

        // replace the animation start with the camera's previous value
        latLngValues[0] = previousCameraLatLng
        bearingValues =
            if (isGpsNorth) {
                arrayOf(previousCameraBearing, shortestRotation(0f, previousCameraBearing))
            } else {
                getBearingValues(previousCameraBearing, newLocations)
            }
        updateCameraAnimators(latLngValues, bearingValues)

        val targetLatLng = LatLng(newLocation)
        val snap =
            immediateAnimation(projection, previousCameraLatLng, targetLatLng) ||
                immediateAnimation(projection, previousLayerLatLng, targetLatLng)

        var animationDuration = 0L
        if (!snap) {
            val previousUpdateTimeStamp = locationUpdateTimestamp
            locationUpdateTimestamp = SystemClock.elapsedRealtime()

            animationDuration =
                if (previousUpdateTimeStamp == 0L) {
                    0L
                } else if (lookAheadUpdate) {
                    val currentTimestamp = System.currentTimeMillis()
                    if (currentTimestamp > newLocation.time) {
                        Logger.e(
                            "LocationAnimatorCoordinator",
                            "Lookahead enabled, but the target location's timestamp is smaller than current timestamp",
                        )
                        0L
                    } else {
                        newLocation.time - currentTimestamp
                    }
                } else {
                    // make animation slightly longer with durationMultiplier, defaults to 1.1f
                    ((locationUpdateTimestamp - previousUpdateTimeStamp) * durationMultiplier).toLong()
                }

            animationDuration = min(animationDuration, MAX_ANIMATION_DURATION_MS)
        }

        playAnimators(
            animationDuration,
            MapLibreAnimator.ANIMATOR_LAYER_LATLNG,
            MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING,
            MapLibreAnimator.ANIMATOR_CAMERA_LATLNG,
            MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING,
        )

        previousLocation = newLocation
    }

    fun feedNewCompassBearing(
        targetCompassBearing: Float,
        currentCameraPosition: CameraPosition,
    ) {
        if (previousCompassBearing < 0) {
            previousCompassBearing = targetCompassBearing
        }

        val previousLayerBearing = getPreviousLayerCompassBearing()
        val previousCameraBearing = currentCameraPosition.bearing.toFloat()

        updateCompassAnimators(targetCompassBearing, previousLayerBearing, previousCameraBearing)
        playAnimators(
            if (compassAnimationEnabled) COMPASS_UPDATE_RATE_MS else 0L,
            MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
            MapLibreAnimator.ANIMATOR_CAMERA_COMPASS_BEARING,
        )

        previousCompassBearing = targetCompassBearing
    }

    fun feedNewAccuracyRadius(
        targetAccuracyRadius: Float,
        noAnimation: Boolean,
    ) {
        if (previousAccuracyRadius < 0) {
            previousAccuracyRadius = targetAccuracyRadius
        }

        updateAccuracyAnimators(targetAccuracyRadius, getPreviousAccuracyRadius())
        playAnimators(
            if (noAnimation || !accuracyAnimationEnabled) 0L else ACCURACY_RADIUS_ANIMATION_DURATION,
            MapLibreAnimator.ANIMATOR_LAYER_ACCURACY,
        )

        previousAccuracyRadius = targetAccuracyRadius
    }

    /**
     * Initializes the [PulsingLocationCircleAnimator], which is a type of [MapLibreAnimator].
     * This method also adds the animator to this class' animator array.
     *
     * @param options the [LocationComponentOptions] passed to this class upstream from the
     *                [LocationComponent].
     */
    fun startLocationComponentCirclePulsing(options: LocationComponentOptions) {
        cancelAnimator(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE)
        val listener = listeners.get(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE)
        if (listener != null) {
            @Suppress("UNCHECKED_CAST")
            val pulsingLocationCircleAnimator =
                animatorProvider.pulsingCircleAnimator(
                    listener as AnimationsValueChangeListener<Float>,
                    maxAnimationFps,
                    options.pulseSingleDuration(),
                    options.pulseMaxRadius(),
                    options.pulseInterpolator() ?: DecelerateInterpolator(),
                )
            animatorArray.put(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE, pulsingLocationCircleAnimator)
            playPulsingAnimator()
        }
    }

    fun feedNewZoomLevel(
        targetZoomLevel: Double,
        currentCameraPosition: CameraPosition,
        animationDuration: Long,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        updateZoomAnimator(targetZoomLevel.toFloat(), currentCameraPosition.zoom.toFloat(), callback)
        playAnimators(animationDuration, MapLibreAnimator.ANIMATOR_ZOOM)
    }

    fun feedNewPadding(
        padding: DoubleArray,
        currentCameraPosition: CameraPosition,
        animationDuration: Long,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        updatePaddingAnimator(padding, currentCameraPosition.padding!!, callback)
        playAnimators(animationDuration, MapLibreAnimator.ANIMATOR_PADDING)
    }

    fun feedNewTilt(
        targetTilt: Double,
        currentCameraPosition: CameraPosition,
        animationDuration: Long,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        updateTiltAnimator(targetTilt.toFloat(), currentCameraPosition.tilt.toFloat(), callback)
        playAnimators(animationDuration, MapLibreAnimator.ANIMATOR_TILT)
    }

    private fun getPreviousLayerLatLng(): LatLng {
        val latLngAnimator = animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_LATLNG)
        return if (latLngAnimator != null) {
            latLngAnimator.animatedValue as LatLng
        } else {
            LatLng(previousLocation!!)
        }
    }

    private fun getPreviousLayerGpsBearing(): Float {
        val animator = animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING) as MapLibreFloatAnimator?
        return if (animator != null) {
            animator.animatedValue as Float
        } else {
            previousLocation!!.bearing
        }
    }

    private fun getPreviousLayerCompassBearing(): Float {
        val animator = animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING) as MapLibreFloatAnimator?
        return if (animator != null) {
            animator.animatedValue as Float
        } else {
            previousCompassBearing
        }
    }

    private fun getPreviousAccuracyRadius(): Float {
        val animator = animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_ACCURACY)
        return if (animator != null) {
            animator.animatedValue as Float
        } else {
            previousAccuracyRadius
        }
    }

    private fun getLatLngValues(
        previousLatLng: LatLng,
        targetLocations: Array<Location>,
    ): Array<LatLng> =
        Array(targetLocations.size + 1) { i ->
            if (i == 0) previousLatLng else LatLng(targetLocations[i - 1])
        }

    private fun getBearingValues(
        previousBearing: Float,
        targetLocations: Array<Location>,
    ): Array<Float> {
        val bearings = Array(targetLocations.size + 1) { 0f }

        // Because Location bearing values are normalized to [0, 360]
        // we need to do the same for the previous bearing value to determine the shortest path
        bearings[0] = normalize(previousBearing)
        for (i in 1 until bearings.size) {
            bearings[i] = shortestRotation(targetLocations[i - 1].bearing, bearings[i - 1])
        }
        return bearings
    }

    private fun updateLayerAnimators(
        latLngValues: Array<LatLng>,
        bearingValues: Array<Float>,
    ) {
        createNewLatLngAnimator(MapLibreAnimator.ANIMATOR_LAYER_LATLNG, latLngValues)
        createNewFloatAnimator(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING, bearingValues)
    }

    private fun updateCameraAnimators(
        latLngValues: Array<LatLng>,
        bearingValues: Array<Float>,
    ) {
        createNewLatLngAnimator(MapLibreAnimator.ANIMATOR_CAMERA_LATLNG, latLngValues)
        createNewFloatAnimator(MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING, bearingValues)
    }

    private fun updateCompassAnimators(
        targetCompassBearing: Float,
        previousLayerBearing: Float,
        previousCameraBearing: Float,
    ) {
        val normalizedLayerBearing = shortestRotation(targetCompassBearing, previousLayerBearing)
        createNewFloatAnimator(
            MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
            previousLayerBearing,
            normalizedLayerBearing,
        )

        val normalizedCameraBearing = shortestRotation(targetCompassBearing, previousCameraBearing)
        createNewFloatAnimator(
            MapLibreAnimator.ANIMATOR_CAMERA_COMPASS_BEARING,
            previousCameraBearing,
            normalizedCameraBearing,
        )
    }

    private fun updateAccuracyAnimators(
        targetAccuracyRadius: Float,
        previousAccuracyRadius: Float,
    ) {
        createNewFloatAnimator(MapLibreAnimator.ANIMATOR_LAYER_ACCURACY, previousAccuracyRadius, targetAccuracyRadius)
    }

    private fun updateZoomAnimator(
        targetZoomLevel: Float,
        previousZoomLevel: Float,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ) {
        createNewCameraAdapterAnimator(
            MapLibreAnimator.ANIMATOR_ZOOM,
            arrayOf(previousZoomLevel, targetZoomLevel),
            cancelableCallback,
        )
    }

    private fun updatePaddingAnimator(
        targetPadding: DoubleArray,
        previousPadding: DoubleArray,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ) {
        createNewPaddingAnimator(
            MapLibreAnimator.ANIMATOR_PADDING,
            arrayOf(previousPadding, targetPadding),
            cancelableCallback,
        )
    }

    private fun updateTiltAnimator(
        targetTilt: Float,
        previousTiltLevel: Float,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ) {
        createNewCameraAdapterAnimator(
            MapLibreAnimator.ANIMATOR_TILT,
            arrayOf(previousTiltLevel, targetTilt),
            cancelableCallback,
        )
    }

    private fun createNewLatLngAnimator(
        @MapLibreAnimator.Type animatorType: Int,
        previous: LatLng,
        target: LatLng,
    ) {
        createNewLatLngAnimator(animatorType, arrayOf(previous, target))
    }

    @Suppress("UNCHECKED_CAST")
    private fun createNewLatLngAnimator(
        @MapLibreAnimator.Type animatorType: Int,
        values: Array<LatLng>,
    ) {
        cancelAnimator(animatorType)
        val listener = listeners.get(animatorType) as AnimationsValueChangeListener<LatLng>?
        if (listener != null) {
            animatorArray.put(animatorType, animatorProvider.latLngAnimator(values, listener, maxAnimationFps))
        }
    }

    private fun createNewFloatAnimator(
        @MapLibreAnimator.Type animatorType: Int,
        previous: Float,
        target: Float,
    ) {
        createNewFloatAnimator(animatorType, arrayOf(previous, target))
    }

    @Suppress("UNCHECKED_CAST")
    private fun createNewFloatAnimator(
        @MapLibreAnimator.Type animatorType: Int,
        @Size(min = 2) values: Array<Float>,
    ) {
        cancelAnimator(animatorType)
        val listener = listeners.get(animatorType) as AnimationsValueChangeListener<Float>?
        if (listener != null) {
            animatorArray.put(animatorType, animatorProvider.floatAnimator(values, listener, maxAnimationFps))
        }
    }

    @Suppress("UNCHECKED_CAST")
    private fun createNewCameraAdapterAnimator(
        @MapLibreAnimator.Type animatorType: Int,
        @Size(min = 2) values: Array<Float>,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ) {
        cancelAnimator(animatorType)
        val listener = listeners.get(animatorType) as AnimationsValueChangeListener<Float>?
        if (listener != null) {
            animatorArray.put(animatorType, animatorProvider.cameraAnimator(values, listener, cancelableCallback))
        }
    }

    @Suppress("UNCHECKED_CAST")
    private fun createNewPaddingAnimator(
        @MapLibreAnimator.Type animatorType: Int,
        @Size(min = 2) values: Array<DoubleArray>,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ) {
        cancelAnimator(animatorType)
        val listener = listeners.get(animatorType) as AnimationsValueChangeListener<DoubleArray>?
        if (listener != null) {
            animatorArray.put(animatorType, animatorProvider.paddingAnimator(values, listener, cancelableCallback))
        }
    }

    private fun checkGpsNorth(
        isGpsNorth: Boolean,
        targetCameraBearing: Float,
    ): Float = if (isGpsNorth) 0f else targetCameraBearing

    private fun playAnimators(
        duration: Long,
        @MapLibreAnimator.Type vararg animatorTypes: Int,
    ) {
        val animators = mutableListOf<Animator>()
        for (animatorType in animatorTypes) {
            animatorArray.get(animatorType)?.let { animators.add(it) }
        }
        animatorSetProvider.startAnimation(animators, LinearInterpolator(), duration)
    }

    /**
     * Starts the [PulsingLocationCircleAnimator] in the animator array. This method is separate
     * from [playAnimators] because the MapboxAnimatorSetProvider has many more
     * customizable animation parameters than the other [MapLibreAnimator]s.
     */
    private fun playPulsingAnimator() {
        animatorArray.get(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE)?.start()
    }

    fun resetAllCameraAnimations(
        currentCameraPosition: CameraPosition,
        isGpsNorth: Boolean,
    ) {
        resetCameraCompassAnimation(currentCameraPosition)
        val snap = resetCameraLocationAnimations(currentCameraPosition, isGpsNorth)
        playAnimators(
            if (snap) 0L else TRANSITION_ANIMATION_DURATION_MS,
            MapLibreAnimator.ANIMATOR_CAMERA_LATLNG,
            MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING,
        )
    }

    private fun resetCameraLocationAnimations(
        currentCameraPosition: CameraPosition,
        isGpsNorth: Boolean,
    ): Boolean {
        resetCameraGpsBearingAnimation(currentCameraPosition, isGpsNorth)
        return resetCameraLatLngAnimation(currentCameraPosition)
    }

    private fun resetCameraLatLngAnimation(currentCameraPosition: CameraPosition): Boolean {
        val animator =
            animatorArray.get(MapLibreAnimator.ANIMATOR_CAMERA_LATLNG) as MapLibreLatLngAnimator?
                ?: return false

        val currentTarget = animator.target
        val previousCameraTarget = currentCameraPosition.target ?: return false

        createNewLatLngAnimator(MapLibreAnimator.ANIMATOR_CAMERA_LATLNG, previousCameraTarget, currentTarget)

        return immediateAnimation(projection, previousCameraTarget, currentTarget)
    }

    private fun resetCameraGpsBearingAnimation(
        currentCameraPosition: CameraPosition,
        isGpsNorth: Boolean,
    ) {
        val animator =
            animatorArray.get(MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING) as MapLibreFloatAnimator?
                ?: return

        val currentTargetBearing = checkGpsNorth(isGpsNorth, animator.target)
        val previousCameraBearing = currentCameraPosition.bearing.toFloat()
        val normalizedCameraBearing = shortestRotation(currentTargetBearing, previousCameraBearing)
        createNewFloatAnimator(
            MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING,
            previousCameraBearing,
            normalizedCameraBearing,
        )
    }

    private fun resetCameraCompassAnimation(currentCameraPosition: CameraPosition) {
        val animator =
            animatorArray.get(MapLibreAnimator.ANIMATOR_CAMERA_COMPASS_BEARING) as MapLibreFloatAnimator?
                ?: return

        val currentTargetBearing = animator.target
        val previousCameraBearing = currentCameraPosition.bearing.toFloat()
        val normalizedCameraBearing = shortestRotation(currentTargetBearing, previousCameraBearing)
        createNewFloatAnimator(
            MapLibreAnimator.ANIMATOR_CAMERA_COMPASS_BEARING,
            previousCameraBearing,
            normalizedCameraBearing,
        )
    }

    fun resetAllLayerAnimations() {
        val latLngAnimator = animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_LATLNG) as MapLibreLatLngAnimator?
        val gpsBearingAnimator =
            animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING) as MapLibreFloatAnimator?
        val compassBearingAnimator =
            animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING) as MapLibreFloatAnimator?
        val accuracyAnimator = animatorArray.get(MapLibreAnimator.ANIMATOR_LAYER_ACCURACY) as MapLibreFloatAnimator?

        if (latLngAnimator != null && gpsBearingAnimator != null) {
            val currentLatLng = latLngAnimator.animatedValue as LatLng
            val currentLatLngTarget = latLngAnimator.target
            createNewLatLngAnimator(MapLibreAnimator.ANIMATOR_LAYER_LATLNG, currentLatLng, currentLatLngTarget)

            val currentGpsBearing = gpsBearingAnimator.animatedValue as Float
            val currentGpsBearingTarget = gpsBearingAnimator.target
            createNewFloatAnimator(
                MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING,
                currentGpsBearing,
                currentGpsBearingTarget,
            )

            val duration = latLngAnimator.duration - latLngAnimator.currentPlayTime

            playAnimators(
                duration,
                MapLibreAnimator.ANIMATOR_LAYER_LATLNG,
                MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING,
            )
        }

        if (compassBearingAnimator != null) {
            val currentLayerBearing = getPreviousLayerCompassBearing()
            val currentLayerBearingTarget = compassBearingAnimator.target
            createNewFloatAnimator(
                MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
                currentLayerBearing,
                currentLayerBearingTarget,
            )
            playAnimators(
                if (compassAnimationEnabled) COMPASS_UPDATE_RATE_MS else 0L,
                MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
            )
        }

        if (accuracyAnimator != null) {
            feedNewAccuracyRadius(previousAccuracyRadius, false)
        }
    }

    fun cancelZoomAnimation() {
        cancelAnimator(MapLibreAnimator.ANIMATOR_ZOOM)
    }

    fun cancelPaddingAnimation() {
        cancelAnimator(MapLibreAnimator.ANIMATOR_PADDING)
    }

    fun cancelTiltAnimation() {
        cancelAnimator(MapLibreAnimator.ANIMATOR_TILT)
    }

    fun cancelAndRemoveGpsBearingAnimation() {
        cancelAnimator(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING)
        animatorArray.remove(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING)
    }

    /**
     * Cancel the pulsing circle location animator.
     */
    fun stopPulsingCircleAnimation() {
        cancelAnimator(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE)
    }

    fun cancelAllAnimations() {
        for (i in 0 until animatorArray.size()) {
            @MapLibreAnimator.Type val animatorType = animatorArray.keyAt(i)
            cancelAnimator(animatorType)
        }
    }

    private fun cancelAnimator(
        @MapLibreAnimator.Type animatorType: Int,
    ) {
        animatorArray.get(animatorType)?.apply {
            cancel()
            removeAllUpdateListeners()
            removeAllListeners()
        }
    }

    fun setTrackingAnimationDurationMultiplier(trackingAnimationDurationMultiplier: Float) {
        durationMultiplier = trackingAnimationDurationMultiplier
    }

    fun setCompassAnimationEnabled(compassAnimationEnabled: Boolean) {
        this.compassAnimationEnabled = compassAnimationEnabled
    }

    fun setAccuracyAnimationEnabled(accuracyAnimationEnabled: Boolean) {
        this.accuracyAnimationEnabled = accuracyAnimationEnabled
    }

    private companion object {
        private const val TAG = "Mbgl-LocationAnimatorCoordinator"
    }
}
