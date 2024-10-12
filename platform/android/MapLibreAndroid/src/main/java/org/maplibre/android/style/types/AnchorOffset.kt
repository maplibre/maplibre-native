package org.maplibre.android.style.types

import androidx.annotation.Keep

/**
 * Data class representing an anchor offset in the MapLibre Android SDK.
 *
 * @property anchorType The type of anchor.
 * @property offset An array of floats representing the offset values.
 */
@Keep
data class AnchorOffset(
    val anchorType: String,
    val offset: Array<Float>
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as AnchorOffset

        if (anchorType != other.anchorType) return false
        if (!offset.contentEquals(other.offset)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = anchorType.hashCode()
        result = 31 * result + offset.contentHashCode()
        return result
    }
}
