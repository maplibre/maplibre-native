package org.maplibre.android.location

import android.animation.TypeEvaluator
import androidx.annotation.Size

internal class PaddingEvaluator : TypeEvaluator<DoubleArray> {
    private val padding = DoubleArray(4)
    override fun evaluate(
        fraction: Float, @Size(min = 4) startValue: DoubleArray,
        @Size(min = 4) endValue: DoubleArray
    ): DoubleArray {
        padding[0] = startValue[0] + (endValue[0] - startValue[0]) * fraction
        padding[1] = startValue[1] + (endValue[1] - startValue[1]) * fraction
        padding[2] = startValue[2] + (endValue[2] - startValue[2]) * fraction
        padding[3] = startValue[3] + (endValue[3] - startValue[3]) * fraction
        return padding
    }
}