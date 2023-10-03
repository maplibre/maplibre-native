package org.maplibre.android.annotations.data

import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.PointF
import androidx.annotation.ColorInt
import com.google.gson.JsonElement
import org.maplibre.android.annotations.Circle
import org.maplibre.android.annotations.Line

class Defaults {
    companion object {

        const val Z_LAYER: Int = 0
        const val DRAGGABLE: Boolean = false
        val JSON_ELEMENT: JsonElement? = null

        val SYMBOL_ICON: Icon? = null
        val SYMBOL_TEXT: Text? = null

        const val FIT_TEXT_WIDTH: Boolean = false
        const val FIT_TEXT_HEIGHT: Boolean = false
        private val FIT_TEXT_PADDING: Icon.FitText.Padding = Icon.FitText.Padding.ZERO

        const val ICON_SIZE: Float = 1f
        const val ICON_ROTATE: Float = 0f
        val ICON_OFFSET: PointF = PointF(0f, 0f)
        val ICON_ANCHOR: Anchor = Anchor.CENTER
        const val ICON_OPACITY: Float = 1f
        val ICON_FIT_TEXT: Icon.FitText = Icon.FitText(
            width = FIT_TEXT_WIDTH,
            height = FIT_TEXT_HEIGHT,
            padding = FIT_TEXT_PADDING
        )
        val ICON_HALO: Halo? = null

        val HALO_BLUR: Float? = null

        val TEXT_FONT: List<String>? = null
        const val TEXT_SIZE: Float = 16f
        const val TEXT_MAX_WIDTH: Float = 10f
        const val TEXT_LETTER_SPACING: Float = 0f
        val TEXT_JUSTIFY: Text.Justify = Text.Justify.CENTER
        val TEXT_ANCHOR: Anchor = Anchor.CENTER
        const val TEXT_ROTATE: Float = 0f
        val TEXT_TRANSFORM: Text.Transform? = null
        val TEXT_OFFSET: Text.Offset? = null
        const val TEXT_OPACITY: Float = 1f
        @ColorInt
        val TEXT_COLOR: Int = Color.BLACK
        val TEXT_HALO: Halo? = null

        val CIRCLE_RADIUS: Float = 5f
        @ColorInt
        val CIRCLE_COLOR: Int = Color.BLACK
        val CIRCLE_BLUR: Float? = null
        val CIRCLE_OPACITY: Float = 1f
        val CIRCLE_STROKE: Circle.Stroke? = null

        val CIRCLE_STROKE_COLOR: Int = Color.BLACK
        val CIRCLE_STROKE_OPACITY: Float = 1f

        val LINE_JOIN: Line.Join = Line.Join.MITER
        val LINE_OPACITY: Float = 1f
        @ColorInt
        val LINE_COLOR: Int = Color.BLACK
        val LINE_WIDTH: Float = 1f
        val LINE_GAP: Float? = null
        val LINE_OFFSET: Float = 0f
        val LINE_BLUR: Float? = null
        val LINE_PATTERN: Bitmap? = null
    }
}
