package org.maplibre.android.annotations.data

import android.graphics.PointF

data class Translate(
    val offset: PointF,
    val anchor: Translate.Anchor
) {
    enum class Anchor {
        MAP, VIEWPORT
    }
}