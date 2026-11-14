package org.maplibre.android.annotations

import android.graphics.Bitmap
import android.util.DisplayMetrics
import java.nio.ByteBuffer

/**
 * Icon is the visual representation of a Marker on a MapView.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class Icon internal constructor(
    /**
     * String identifier for this icon.
     */
    val id: String?,
    bitmap: Bitmap?,
) {
    private var iconBitmap: Bitmap? = bitmap

    /**
     * The bitmap being used for this icon.
     */
    val bitmap: Bitmap?
        get() {
            val current = iconBitmap
            if (current != null && current.config != Bitmap.Config.ARGB_8888) {
                iconBitmap = current.copy(Bitmap.Config.ARGB_8888, false)
            }
            return iconBitmap
        }

    /**
     * The icon bitmap scale.
     *
     * Requires the bitmap to be set before reading this property.
     */
    val scale: Float
        get() {
            val current =
                iconBitmap
                    ?: throw IllegalStateException("Required to set a Icon before calling getScale")
            var density = current.density.toFloat()
            if (density == Bitmap.DENSITY_NONE.toFloat()) {
                density = DisplayMetrics.DENSITY_DEFAULT.toFloat()
            }
            return density / DisplayMetrics.DENSITY_DEFAULT
        }

    /**
     * Get the icon bitmap bytes.
     *
     * Requires the bitmap to be set before calling this method.
     *
     * @return the bytes of the bitmap
     */
    fun toBytes(): ByteArray {
        val current =
            iconBitmap
                ?: throw IllegalStateException("Required to set a Icon before calling toBytes")
        val buffer = ByteBuffer.allocate(current.rowBytes * current.height)
        current.copyPixelsToBuffer(buffer)
        return buffer.array()
    }

    /**
     * Compares this icon object with another icon and determines if they match.
     *
     * @param other Another icon to compare with this object.
     * @return True if the icon being passed in matches this icon object. Else, false.
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        other as Icon
        return iconBitmap == other.iconBitmap && id == other.id
    }

    /**
     * Gives an integer which can be used as the bucket number for storing elements of the set/map.
     * This bucket number is the address of the element inside the set/map. There's no guarantee
     * that this hash value will be consistent between different Java implementations, or even
     * between different execution runs of the same program.
     *
     * @return integer value you can use for storing element.
     */
    override fun hashCode(): Int {
        var result = 0
        iconBitmap?.let { result = it.hashCode() }
        id?.let { result = 31 * result + it.hashCode() }
        return result
    }
}
