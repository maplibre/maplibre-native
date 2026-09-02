package org.maplibre.android.location

import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener

internal class AnimatorListenerHolder(
    @param:MapLibreAnimator.Type @get:MapLibreAnimator.Type val animatorType: Int,
    val listener: AnimationsValueChangeListener<*>,
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as AnimatorListenerHolder

        if (animatorType != other.animatorType) {
            return false
        }
        return listener == other.listener
    }

    override fun hashCode(): Int {
        var result = animatorType
        result = 31 * result + listener.hashCode()
        return result
    }
}
