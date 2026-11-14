package org.maplibre.android.maps

/**
 * Describes the image content, e.g. where text can be fit into an image.
 *
 * When sizing icons with icon-text-fit,
 * the icon size will be adjusted so that the this content box fits exactly around the text.
 */
class ImageContent(
    private val left: Float,
    private val top: Float,
    private val right: Float,
    private val bottom: Float,
) {
    /**
     * Get the array for this content, sorted by left, top, right, bottom.
     *
     * @return the content array.
     */
    val contentArray: FloatArray
        get() = floatArrayOf(left, top, right, bottom)

    override fun equals(other: Any?): Boolean {
        if (other !is ImageContent) {
            return false
        }
        return left == other.left && top == other.top && right == other.right && bottom == other.bottom
    }

    override fun hashCode(): Int {
        var result = if (left != +0.0f) left.toBits() else 0
        result = 31 * result + if (top != +0.0f) top.toBits() else 0
        result = 31 * result + if (right != +0.0f) right.toBits() else 0
        result = 31 * result + if (bottom != +0.0f) bottom.toBits() else 0
        return result
    }

    override fun toString(): String = "[ left: $left, top: $top, right: $right, bottom: $bottom ]"
}
