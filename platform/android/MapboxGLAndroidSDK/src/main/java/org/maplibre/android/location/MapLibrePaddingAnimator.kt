package org.maplibre.android.location

import android.animation.TypeEvaluator
import androidx.annotation.Size
import org.maplibre.android.maps.MapLibreMap.CancelableCallback

class MapLibrePaddingAnimator internal constructor(
    @Size(min = 2) values: Array<DoubleArray>,
    updateListener: AnimationsValueChangeListener<DoubleArray>,
    cancelableCallback: CancelableCallback?
) :
    MapLibreAnimator<DoubleArray>(values, updateListener, Int.MAX_VALUE) {
    init {
        addListener(MapLibreAnimatorListener(cancelableCallback))
    }

    public override fun provideEvaluator(): TypeEvaluator<DoubleArray> {
        return PaddingEvaluator()
    }
}