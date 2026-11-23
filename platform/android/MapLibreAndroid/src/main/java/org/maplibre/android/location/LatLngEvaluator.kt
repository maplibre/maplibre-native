package org.maplibre.android.location

import android.animation.TypeEvaluator
import org.maplibre.android.geometry.LatLng

internal class LatLngEvaluator : TypeEvaluator<LatLng> {
    private val latLng = LatLng()

    override fun evaluate(
        fraction: Float,
        startValue: LatLng,
        endValue: LatLng,
    ): LatLng {
        latLng.latitude = startValue.latitude + ((endValue.latitude - startValue.latitude) * fraction)
        latLng.longitude = startValue.longitude + ((endValue.longitude - startValue.longitude) * fraction)
        return latLng
    }
}
