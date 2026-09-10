package org.maplibre.android.location

import android.animation.FloatEvaluator
import android.animation.TypeEvaluator
import androidx.annotation.Size
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener

internal open class MapLibreFloatAnimator(
    @Size(min = 2) values: Array<Float>,
    updateListener: AnimationsValueChangeListener<Float>,
    maxAnimationFps: Int,
) : MapLibreAnimator<Float>(values, updateListener, maxAnimationFps) {
    override fun provideEvaluator(): TypeEvaluator<*> = FloatEvaluator()
}
