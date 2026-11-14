package org.maplibre.android.location

import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener

/**
 * Manages the logic of the interpolated animation which is applied to the LocationComponent's pulsing circle
 *
 * @param updateListener  the [AnimationsValueChangeListener] associated with this animator.
 * @param maxAnimationFps the maximum frames per second that the animator should use. Default
 *                        is the [LocationAnimatorCoordinator.maxAnimationFps] variable.
 */
internal class PulsingLocationCircleAnimator(
    updateListener: AnimationsValueChangeListener<Float>,
    maxAnimationFps: Int,
    circleMaxRadius: Float,
) : MapLibreFloatAnimator(arrayOf(0f, circleMaxRadius), updateListener, maxAnimationFps)
