package org.maplibre.android.annotations.data

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.drawable.Drawable
import androidx.annotation.ColorInt
import androidx.annotation.FloatRange
import androidx.core.graphics.drawable.toBitmap
import org.maplibre.android.annotations.Symbol
import org.maplibre.android.annotations.PairWithDefault
import org.maplibre.android.annotations.asColorString
import org.maplibre.android.annotations.default

/**
 * The image provided to the icon will be matched based on reference only for higher
 * reliability and performance reasons. For this reason, generating many different Bitmap
 * objects with the same contents will lead to bad performance.
 *
 * TODO Think this through and discuss this.
 */
open class Icon(
    val image: Bitmap,
    val size: Float = Defaults.ICON_SIZE,
    val rotate: Float = Defaults.ICON_ROTATE,
    val offset: PointF = Defaults.ICON_OFFSET,
    val anchor: Anchor = Defaults.ICON_ANCHOR,
    @FloatRange(from = 0.0, to = 1.0)
    val opacity: Float = Defaults.ICON_OPACITY,
    // TODO val fitText: FitText = Defaults.ICON_FIT_TEXT, // NDD
    // TODO val keepUpright: Boolean = false // (NDD)
    // TODO val pitchAlignment: Alignment? (NDD)
) {

    constructor(
        image: Drawable,
        size: Float = Defaults.ICON_SIZE,
        rotate: Float = Defaults.ICON_ROTATE,
        offset: PointF = Defaults.ICON_OFFSET,
        anchor: Anchor = Defaults.ICON_ANCHOR,
        opacity: Float = Defaults.ICON_OPACITY
    ) : this(image.toBitmap(), size, rotate, offset, anchor, opacity)

    init {
        if (opacity > 1f || opacity < 0f) {
            throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
        }
    }

    data class FitText(
        val width: Boolean,
        val height: Boolean,
        val padding: Padding
    ) {

        data class Padding(
            val top: Float,
            val right: Float,
            val bottom: Float,
            val left: Float
        ) {
            companion object {
                val ZERO = Padding(0f, 0f, 0f, 0f)
            }
        }
    }

    open val flattenedValues: List<PairWithDefault>
        get() = listOf(
            Symbol.PROPERTY_ICON_SIZE to size default Defaults.ICON_SIZE,
            Symbol.PROPERTY_ICON_IMAGE to image default Unit, // todo string representation
            Symbol.PROPERTY_ICON_ROTATE to rotate default Defaults.ICON_ROTATE,
            Symbol.PROPERTY_ICON_OFFSET to offset.toArray() default Defaults.ICON_OFFSET,
            Symbol.PROPERTY_ICON_ANCHOR to anchor.toString() default Defaults.ICON_ANCHOR.toString(),
            Symbol.PROPERTY_ICON_OPACITY to opacity default Defaults.ICON_OPACITY
        )
}

class SdfIcon(
    image: Bitmap,
    @ColorInt val color: Int,
    val halo: Halo? = Defaults.ICON_HALO,
    size: Float = Defaults.ICON_SIZE,
    rotate: Float = Defaults.ICON_ROTATE,
    offset: PointF = Defaults.ICON_OFFSET,
    anchor: Anchor = Defaults.ICON_ANCHOR,
    opacity: Float = Defaults.ICON_OPACITY
) : Icon(image, size, rotate, offset, anchor, opacity) {
    override val flattenedValues: List<PairWithDefault>
        get() = super.flattenedValues + listOf(
            Symbol.PROPERTY_ICON_COLOR to color.asColorString() default Unit,
            Symbol.PROPERTY_ICON_HALO_COLOR to halo?.color default Defaults.ICON_HALO?.color,
            Symbol.PROPERTY_ICON_HALO_WIDTH to halo?.width default Defaults.ICON_HALO?.width,
            Symbol.PROPERTY_ICON_HALO_BLUR to halo?.blur default Defaults.ICON_HALO?.blur
        )
}

fun PointF.toArray() = arrayOf(x, y)
