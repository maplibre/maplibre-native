package org.maplibre.android.maps

/**
 * Describes the image stretch areas.
 *
 * @param first  the first stretchable part in pixel units
 * @param second the second stretchable part in pixel units
 */
class ImageStretches(
    val first: Float,
    val second: Float,
) {
    override fun equals(other: Any?): Boolean {
        if (other !is ImageStretches) {
            return false
        }
        return first == other.first && second == other.second
    }

    override fun hashCode(): Int {
        var result = if (first != +0.0f) first.toBits() else 0
        result = 31 * result + if (second != +0.0f) second.toBits() else 0
        return result
    }

    override fun toString(): String = "[ first: $first, second: $second ]"
}
