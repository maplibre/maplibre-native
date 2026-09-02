package org.maplibre.android.location

import android.animation.Animator
import android.animation.AnimatorSet
import android.view.animation.Interpolator

internal class MapLibreAnimatorSetProvider private constructor() {
    fun startAnimation(
        animators: List<Animator>,
        interpolator: Interpolator,
        duration: Long,
    ) {
        val locationAnimatorSet = AnimatorSet()
        locationAnimatorSet.playTogether(animators)
        locationAnimatorSet.interpolator = interpolator
        locationAnimatorSet.duration = duration
        locationAnimatorSet.start()
    }

    companion object {
        private var instance: MapLibreAnimatorSetProvider? = null

        @JvmStatic
        fun getInstance(): MapLibreAnimatorSetProvider = instance ?: MapLibreAnimatorSetProvider().also { instance = it }
    }
}
