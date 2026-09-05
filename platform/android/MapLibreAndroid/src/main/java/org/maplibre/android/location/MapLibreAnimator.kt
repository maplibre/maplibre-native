package org.maplibre.android.location

import android.animation.Animator
import android.animation.AnimatorListenerAdapter
import android.animation.TypeEvaluator
import android.animation.ValueAnimator
import androidx.annotation.IntDef
import androidx.annotation.Size

/**
 * Abstract class for all of the location component animators.
 *
 * @param K Data type that will be animated.
 */
abstract class MapLibreAnimator<K> :
    ValueAnimator,
    ValueAnimator.AnimatorUpdateListener {
    @Retention(AnnotationRetention.SOURCE)
    @IntDef(
        ANIMATOR_LAYER_LATLNG,
        ANIMATOR_CAMERA_LATLNG,
        ANIMATOR_LAYER_GPS_BEARING,
        ANIMATOR_LAYER_COMPASS_BEARING,
        ANIMATOR_CAMERA_GPS_BEARING,
        ANIMATOR_CAMERA_COMPASS_BEARING,
        ANIMATOR_LAYER_ACCURACY,
        ANIMATOR_ZOOM,
        ANIMATOR_TILT,
        ANIMATOR_PULSING_CIRCLE,
        ANIMATOR_PADDING,
    )
    internal annotation class Type

    private val updateListener: AnimationsValueChangeListener<K>

    internal val target: K

    private var animatedValue: K? = null

    private val minUpdateInterval: Double
    private var timeElapsed: Long

    /**
     * Makes this animator invalid and prevents it from pushing any more updates to the listener.
     *
     * This can be used to prevent propagating final updates when the animator should be immediately canceled.
     */
    private var invalid = false

    constructor(
        @Size(min = 2) values: Array<K>,
        updateListener: AnimationsValueChangeListener<K>,
        maxAnimationFps: Int,
    ) : super() {
        minUpdateInterval = 1E9 / maxAnimationFps
        setObjectValues(*values)
        setEvaluator(provideEvaluator())
        this.updateListener = updateListener
        this.target = values[values.size - 1]
        this.timeElapsed = 0
        addUpdateListener(this)
        addListener(AnimatorListener())
    }

    constructor(
        updateListener: AnimationsValueChangeListener<K>,
        target: K,
        animatedValue: K,
        minUpdateInterval: Double,
        timeElapsed: Long,
    ) : super() {
        this.updateListener = updateListener
        this.target = target
        this.animatedValue = animatedValue
        this.minUpdateInterval = minUpdateInterval
        this.timeElapsed = timeElapsed
    }

    @Suppress("UNCHECKED_CAST")
    override fun onAnimationUpdate(animation: ValueAnimator) {
        animatedValue = animation.animatedValue as K

        val currentTime = System.nanoTime()
        if ((currentTime - timeElapsed) < minUpdateInterval) {
            return
        }

        postUpdates()
        timeElapsed = currentTime
    }

    private inner class AnimatorListener : AnimatorListenerAdapter() {
        override fun onAnimationEnd(animation: Animator) {
            postUpdates()
        }
    }

    @Suppress("UNCHECKED_CAST")
    private fun postUpdates() {
        if (!invalid) {
            updateListener.onNewAnimationValue(animatedValue as K)
        }
    }

    protected abstract fun provideEvaluator(): TypeEvaluator<*>

    fun interface AnimationsValueChangeListener<K> {
        fun onNewAnimationValue(value: K)
    }

    fun makeInvalid() {
        invalid = true
    }

    companion object {
        internal const val ANIMATOR_LAYER_LATLNG = 0
        internal const val ANIMATOR_CAMERA_LATLNG = 1
        internal const val ANIMATOR_LAYER_GPS_BEARING = 2
        internal const val ANIMATOR_LAYER_COMPASS_BEARING = 3
        internal const val ANIMATOR_CAMERA_GPS_BEARING = 4
        internal const val ANIMATOR_CAMERA_COMPASS_BEARING = 5
        internal const val ANIMATOR_LAYER_ACCURACY = 6
        internal const val ANIMATOR_ZOOM = 7
        internal const val ANIMATOR_TILT = 8
        internal const val ANIMATOR_PULSING_CIRCLE = 9
        internal const val ANIMATOR_PADDING = 10
    }
}
