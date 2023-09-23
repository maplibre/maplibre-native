package org.maplibre.android.annotations.data

import android.graphics.PointF
import androidx.annotation.ColorInt
import androidx.annotation.FloatRange
import org.maplibre.android.annotations.KSymbol
import org.maplibre.android.annotations.PairWithDefault
import org.maplibre.android.annotations.asColorString
import org.maplibre.android.annotations.default
import org.maplibre.android.style.layers.Property

class Text(
    val string: String,
    val font: List<String>? = Defaults.TEXT_FONT,
    val size: Float = Defaults.TEXT_SIZE,
    val maxWidth: Float = Defaults.TEXT_MAX_WIDTH,
    val letterSpacing: Float = Defaults.TEXT_LETTER_SPACING,
    val justify: Justify = Defaults.TEXT_JUSTIFY,
    val anchor: Anchor = Defaults.TEXT_ANCHOR,
    val rotate: Float = Defaults.TEXT_ROTATE,
    val transform: Transform? = Defaults.TEXT_TRANSFORM,
    val offset: Offset? = Defaults.TEXT_OFFSET,
    @FloatRange(from = 0.0, to = 1.0)
    val opacity: Float = Defaults.TEXT_OPACITY,
    @ColorInt val color: Int = Defaults.TEXT_COLOR,
    val halo: Halo? = Defaults.TEXT_HALO,
    // val pitchAlignment: Alignment?, (NDD)
    // val lineHeight: Float = 1.2f (NDD)
) {

    init {
        if (opacity > 1f || opacity < 0f) {
            throw IllegalArgumentException("Opacity must be between 0 and 1 (inclusive)")
        }
    }

    enum class Justify {
        AUTO, LEFT, CENTER, RIGHT;

        override fun toString(): String {
            return super.toString().lowercase()
        }
    }

    enum class Transform {
        UPPERCASE, LOWERCASE
    }

    fun Transform?.toString() = if (this == null) {
        Property.TEXT_TRANSFORM_NONE
    } else {
        this.toString().lowercase()
    }

    sealed class Offset {
        abstract val offset: Any
    }

    class RadialOffset(override val offset: Float) : Offset()

    class AbsoluteOffset(override val offset: PointF) : Offset()

    val flattenedValues: List<PairWithDefault>
        get() = listOf(
            KSymbol.PROPERTY_TEXT_FIELD to string default Unit,
            KSymbol.PROPERTY_TEXT_FONT to font default Defaults.TEXT_FONT,
            KSymbol.PROPERTY_TEXT_SIZE to size default Defaults.TEXT_SIZE,
            KSymbol.PROPERTY_TEXT_MAX_WIDTH to maxWidth default Defaults.TEXT_MAX_WIDTH,
            KSymbol.PROPERTY_TEXT_LETTER_SPACING to letterSpacing default Defaults.TEXT_LETTER_SPACING,
            KSymbol.PROPERTY_TEXT_JUSTIFY to justify.toString() default Defaults.TEXT_JUSTIFY.toString(),
            KSymbol.PROPERTY_TEXT_RADIAL_OFFSET to offset?.let {
                if (it is RadialOffset) {
                    it.offset
                } else {
                    null
                }
            } default Unit,
            KSymbol.PROPERTY_TEXT_ANCHOR to anchor.toString() default Defaults.TEXT_ANCHOR.toString(),
            KSymbol.PROPERTY_TEXT_ROTATE to rotate default Defaults.TEXT_ROTATE,
            KSymbol.PROPERTY_TEXT_TRANSFORM to transform.toString() default Defaults.TEXT_TRANSFORM.toString(),
            KSymbol.PROPERTY_TEXT_OFFSET to offset?.let {
                if (it is AbsoluteOffset) {
                    it.offset.toArray()
                } else {
                    null
                }
            } default Unit,
            KSymbol.PROPERTY_TEXT_OPACITY to opacity default Defaults.TEXT_OPACITY,
            KSymbol.PROPERTY_TEXT_COLOR to color.asColorString() default Defaults.TEXT_COLOR.asColorString(),
            KSymbol.PROPERTY_TEXT_HALO_COLOR to halo?.color default Defaults.TEXT_HALO?.color,
            KSymbol.PROPERTY_TEXT_HALO_WIDTH to halo?.width default Defaults.TEXT_HALO?.width,
            KSymbol.PROPERTY_TEXT_HALO_BLUR to halo?.blur default Defaults.TEXT_HALO?.blur
        )
}
