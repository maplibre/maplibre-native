package org.maplibre.android.attribution

import android.graphics.Bitmap
import android.graphics.PointF

class AttributionLayout(
    val logo: Bitmap?,
    val anchorPoint: PointF?,
    val isShortText: Boolean,
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as AttributionLayout

        return logo == other.logo && anchorPoint == other.anchorPoint
    }

    override fun hashCode(): Int {
        var result = logo?.hashCode() ?: 0
        result = 31 * result + (anchorPoint?.hashCode() ?: 0)
        return result
    }

    override fun toString(): String =
        "AttributionLayout{" +
            "logo=" + logo +
            ", anchorPoint=" + anchorPoint +
            '}'
}
