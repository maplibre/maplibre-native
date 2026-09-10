package org.maplibre.android.location

import android.animation.TypeEvaluator
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener

internal class MapLibreLatLngAnimator(
    values: Array<LatLng>,
    updateListener: AnimationsValueChangeListener<LatLng>,
    maxAnimationFps: Int,
) : MapLibreAnimator<LatLng>(values, updateListener, maxAnimationFps) {
    override fun provideEvaluator(): TypeEvaluator<*> = LatLngEvaluator()
}
