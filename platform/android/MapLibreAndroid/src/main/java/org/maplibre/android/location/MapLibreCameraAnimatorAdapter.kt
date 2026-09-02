package org.maplibre.android.location

import androidx.annotation.Size
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener
import org.maplibre.android.maps.MapLibreMap

internal class MapLibreCameraAnimatorAdapter(
    @Size(min = 2) values: Array<Float>,
    updateListener: AnimationsValueChangeListener<Float>,
    cancelableCallback: MapLibreMap.CancelableCallback?,
) : MapLibreFloatAnimator(values, updateListener, Int.MAX_VALUE) {
    init {
        addListener(MapLibreAnimatorListener(cancelableCallback))
    }
}
