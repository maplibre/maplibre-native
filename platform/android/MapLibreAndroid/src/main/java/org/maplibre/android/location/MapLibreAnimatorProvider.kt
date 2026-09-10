package org.maplibre.android.location

import android.animation.ValueAnimator
import android.view.animation.Interpolator
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener
import org.maplibre.android.maps.MapLibreMap

internal class MapLibreAnimatorProvider private constructor() {
    fun latLngAnimator(
        values: Array<LatLng>,
        updateListener: AnimationsValueChangeListener<LatLng>,
        maxAnimationFps: Int,
    ): MapLibreLatLngAnimator = MapLibreLatLngAnimator(values, updateListener, maxAnimationFps)

    fun floatAnimator(
        values: Array<Float>,
        updateListener: AnimationsValueChangeListener<Float>,
        maxAnimationFps: Int,
    ): MapLibreFloatAnimator = MapLibreFloatAnimator(values, updateListener, maxAnimationFps)

    fun cameraAnimator(
        values: Array<Float>,
        updateListener: AnimationsValueChangeListener<Float>,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ): MapLibreCameraAnimatorAdapter = MapLibreCameraAnimatorAdapter(values, updateListener, cancelableCallback)

    fun paddingAnimator(
        values: Array<DoubleArray>,
        updateListener: AnimationsValueChangeListener<DoubleArray>,
        cancelableCallback: MapLibreMap.CancelableCallback?,
    ): MapLibrePaddingAnimator = MapLibrePaddingAnimator(values, updateListener, cancelableCallback)

    /**
     * This animator is for the LocationComponent pulsing circle.
     *
     * @param updateListener the listener that is found in the [LocationAnimatorCoordinator]'s
     *                       listener array.
     * @param maxAnimationFps the max frames per second of the pulsing animation
     * @param pulseSingleDuration the number of milliseconds it takes for the animator to create
     *                            a single pulse.
     * @param pulseMaxRadius the max radius when the circle is finished with a single pulse.
     * @param pulseInterpolator the type of Android-system interpolator to use for
     *                                       the pulsing animation (linear, accelerate, bounce, etc.)
     * @return a built [PulsingLocationCircleAnimator] object.
     */
    fun pulsingCircleAnimator(
        updateListener: AnimationsValueChangeListener<Float>,
        maxAnimationFps: Int,
        pulseSingleDuration: Float,
        pulseMaxRadius: Float,
        pulseInterpolator: Interpolator,
    ): PulsingLocationCircleAnimator {
        val pulsingLocationCircleAnimator =
            PulsingLocationCircleAnimator(updateListener, maxAnimationFps, pulseMaxRadius)
        pulsingLocationCircleAnimator.duration = pulseSingleDuration.toLong()
        pulsingLocationCircleAnimator.repeatMode = ValueAnimator.RESTART
        pulsingLocationCircleAnimator.repeatCount = ValueAnimator.INFINITE
        pulsingLocationCircleAnimator.interpolator = pulseInterpolator
        return pulsingLocationCircleAnimator
    }

    companion object {
        @Suppress("ktlint:standard:property-naming")
        private var INSTANCE: MapLibreAnimatorProvider? = null

        @JvmStatic
        fun getInstance(): MapLibreAnimatorProvider = INSTANCE ?: MapLibreAnimatorProvider().also { INSTANCE = it }
    }
}
