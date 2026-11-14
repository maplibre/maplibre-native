package org.maplibre.android.annotations

import androidx.annotation.IntDef

@Deprecated("As of 7.0.0")
internal class ArrowDirection(
    @param:Value @get:Value val value: Int,
) {
    @IntDef(LEFT, RIGHT, TOP, BOTTOM)
    @Retention(AnnotationRetention.SOURCE)
    annotation class Value

    companion object {
        const val LEFT = 0
        const val RIGHT = 1
        const val TOP = 2
        const val BOTTOM = 3
    }
}
