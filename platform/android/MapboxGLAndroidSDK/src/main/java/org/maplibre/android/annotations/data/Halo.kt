package org.maplibre.android.annotations.data

import androidx.annotation.ColorInt
import androidx.annotation.FloatRange

class Halo(
    @FloatRange(from = 0.0, fromInclusive = false)
    val width: Float,
    @ColorInt val color: Int,
    @FloatRange(from = 0.0, fromInclusive = false)
    val blur: Float? = Defaults.HALO_BLUR
) {

    init {
        if (width <= 0f) {
            throw IllegalArgumentException(
                "A width of $width has been provided for a Halo object. It must be greater than zero, else the " +
                    "Halo would not show. Please use `null` instead of creating a Halo object that has no effect."
            )
        }

        if (blur != null && blur <= 0) {
            throw IllegalArgumentException(
                "A blur of $blur has been provided for a Halo object. This means that no blur is to be used. " +
                    "Please use `null` to indicate that `blur` is not used."
            )
        }
    }
}
