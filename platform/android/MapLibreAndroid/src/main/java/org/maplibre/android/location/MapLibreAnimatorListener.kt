package org.maplibre.android.location

import android.animation.Animator
import android.animation.AnimatorListenerAdapter
import org.maplibre.android.maps.MapLibreMap

internal class MapLibreAnimatorListener(cancelableCallback: MapLibreMap.CancelableCallback?) :
    AnimatorListenerAdapter() {
    private val cancelableCallback: MapLibreMap.CancelableCallback?

    init {
        this.cancelableCallback = cancelableCallback
    }

    override fun onAnimationCancel(animation: Animator) {
        cancelableCallback?.onCancel()
    }

    override fun onAnimationEnd(animation: Animator) {
        cancelableCallback?.onFinish()
    }
}