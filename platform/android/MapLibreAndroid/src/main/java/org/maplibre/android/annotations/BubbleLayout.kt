package org.maplibre.android.annotations

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.RectF
import android.util.AttributeSet
import android.util.DisplayMetrics
import android.widget.LinearLayout
import org.maplibre.android.R

/**
 * Bubble View for Android with custom stroke width and color, arrow size, position and direction.
 * This class has been deprecated as of the 7.0.0 version of the Maps SDK for Android.
 * However, even though the Maps SDK for Android team hasn't continued work on this class,
 * the class is still completely fine to use. This class can be used to create a custom
 * [android.view.View], which is then turned into a [android.graphics.Bitmap].
 * After the bitmap is added to the [org.maplibre.android.maps.Style] object, a
 * [org.maplibre.android.style.layers.SymbolLayer] or the
 * [MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android)
 * can reference the image ID.
 */
@Deprecated("As of 7.0.0")
open class BubbleLayout
/**
     * Creates an instance of bubble layout.
     *
     * @param context      The context used to inflate this bubble layout
     * @param attrs        The attribute set to initialise this bubble layout from
     * @param defStyleAttr The default style to apply this bubble layout with
     */
    @JvmOverloads
    constructor(
        context: Context,
        attrs: AttributeSet? = null,
        defStyleAttr: Int = 0,
    ) : LinearLayout(context, attrs, defStyleAttr) {
        private var arrowDirection: ArrowDirection
        private var arrowWidth: Float
        private var arrowHeight: Float
        private var arrowPosition: Float
        private var cornersRadius: Float
        private var bubble: Bubble? = null
        private var bubbleColor: Int
        private var strokeWidth: Float
        private var strokeColor: Int

        init {
            val a = getContext().obtainStyledAttributes(attrs, R.styleable.maplibre_BubbleLayout)

            @ArrowDirection.Value
            val location =
                a.getInt(
                    R.styleable.maplibre_BubbleLayout_maplibre_bl_arrowDirection,
                    ArrowDirection.LEFT,
                )
            arrowDirection = ArrowDirection(location)
            arrowWidth =
                a.getDimension(
                    R.styleable.maplibre_BubbleLayout_maplibre_bl_arrowWidth,
                    convertDpToPixel(8f, context),
                )
            arrowHeight =
                a.getDimension(
                    R.styleable.maplibre_BubbleLayout_maplibre_bl_arrowHeight,
                    convertDpToPixel(8f, context),
                )
            arrowPosition =
                a.getDimension(
                    R.styleable.maplibre_BubbleLayout_maplibre_bl_arrowPosition,
                    convertDpToPixel(12f, context),
                )
            cornersRadius = a.getDimension(R.styleable.maplibre_BubbleLayout_maplibre_bl_cornersRadius, 0f)
            bubbleColor = a.getColor(R.styleable.maplibre_BubbleLayout_maplibre_bl_bubbleColor, Color.WHITE)
            strokeWidth =
                a.getDimension(R.styleable.maplibre_BubbleLayout_maplibre_bl_strokeWidth, DEFAULT_STROKE_WIDTH)
            strokeColor = a.getColor(R.styleable.maplibre_BubbleLayout_maplibre_bl_strokeColor, Color.GRAY)

            a.recycle()
            initPadding()
        }

        override fun onLayout(
            changed: Boolean,
            left: Int,
            top: Int,
            right: Int,
            bottom: Int,
        ) {
            super.onLayout(changed, left, top, right, bottom)
            initDrawable(0, width, 0, height)
        }

        override fun dispatchDraw(canvas: Canvas) {
            bubble?.draw(canvas)
            super.dispatchDraw(canvas)
        }

        /**
         * Get the arrow direction.
         *
         * @return the arrow direction
         */
        internal fun getArrowDirection(): ArrowDirection = arrowDirection

        /**
         * Set the arrow direction.
         *
         * @param arrowDirection The direction of the arrow
         * @return this
         */
        internal fun setArrowDirection(arrowDirection: ArrowDirection): BubbleLayout {
            resetPadding()
            this.arrowDirection = arrowDirection
            initPadding()
            return this
        }

        /**
         * Get the arrow width.
         *
         * @return the width of the arrow
         */
        fun getArrowWidth(): Float = arrowWidth

        /**
         * Set the arrow width.
         *
         * @param arrowWidth The width of the arrow
         * @return this
         */
        fun setArrowWidth(arrowWidth: Float): BubbleLayout {
            resetPadding()
            this.arrowWidth = arrowWidth
            initPadding()
            return this
        }

        /**
         * Get the arrow height
         *
         * @return the height of the arrow
         */
        fun getArrowHeight(): Float = arrowHeight

        /**
         * Set the arrow height.
         *
         * @param arrowHeight The height of the arrow
         * @return this
         */
        fun setArrowHeight(arrowHeight: Float): BubbleLayout {
            resetPadding()
            this.arrowHeight = arrowHeight
            initPadding()
            return this
        }

        /**
         * Get the arrow position.
         *
         * @return the arrow position
         */
        fun getArrowPosition(): Float = arrowPosition

        /**
         * Get the arrow position.
         *
         * @param arrowPosition The arrow position
         * @return this
         */
        fun setArrowPosition(arrowPosition: Float): BubbleLayout {
            resetPadding()
            this.arrowPosition = arrowPosition
            initPadding()
            return this
        }

        /**
         * Get the corner radius
         *
         * @return the corner radius
         */
        fun getCornersRadius(): Float = cornersRadius

        /**
         * Set the corner radius
         *
         * @param cornersRadius The corner radius
         * @return this
         */
        fun setCornersRadius(cornersRadius: Float): BubbleLayout {
            this.cornersRadius = cornersRadius
            requestLayout()
            return this
        }

        /**
         * Get the bubble color.
         *
         * @return the bubble color
         */
        fun getBubbleColor(): Int = bubbleColor

        /**
         * Set the bubble color.
         *
         * @param bubbleColor The buble color
         * @return this
         */
        fun setBubbleColor(bubbleColor: Int): BubbleLayout {
            this.bubbleColor = bubbleColor
            requestLayout()
            return this
        }

        /**
         * Get stroke width.
         *
         * @return the stroke width
         */
        fun getStrokeWidth(): Float = strokeWidth

        /**
         * Set the stroke width.
         *
         * @param strokeWidth The stroke width
         * @return this
         */
        fun setStrokeWidth(strokeWidth: Float): BubbleLayout {
            resetPadding()
            this.strokeWidth = strokeWidth
            initPadding()
            return this
        }

        /**
         * Get the stroke color.
         *
         * @return the stroke color
         */
        fun getStrokeColor(): Int = strokeColor

        /**
         * Set the stroke color.
         *
         * @param strokeColor The stroke color
         * @return this
         */
        fun setStrokeColor(strokeColor: Int): BubbleLayout {
            this.strokeColor = strokeColor
            requestLayout()
            return this
        }

        private fun initPadding() {
            var paddingLeft = getPaddingLeft()
            var paddingRight = getPaddingRight()
            var paddingTop = getPaddingTop()
            var paddingBottom = getPaddingBottom()
            when (arrowDirection.value) {
                ArrowDirection.LEFT -> paddingLeft = (paddingLeft + arrowWidth).toInt()
                ArrowDirection.RIGHT -> paddingRight = (paddingRight + arrowWidth).toInt()
                ArrowDirection.TOP -> paddingTop = (paddingTop + arrowHeight).toInt()
                ArrowDirection.BOTTOM -> paddingBottom = (paddingBottom + arrowHeight).toInt()
            }
            if (strokeWidth > 0) {
                paddingLeft = (paddingLeft + strokeWidth).toInt()
                paddingRight = (paddingRight + strokeWidth).toInt()
                paddingTop = (paddingTop + strokeWidth).toInt()
                paddingBottom = (paddingBottom + strokeWidth).toInt()
            }
            setPadding(paddingLeft, paddingTop, paddingRight, paddingBottom)
        }

        private fun initDrawable(
            left: Int,
            right: Int,
            top: Int,
            bottom: Int,
        ) {
            if (right < left || bottom < top) {
                return
            }

            val rectF = RectF(left.toFloat(), top.toFloat(), right.toFloat(), bottom.toFloat())
            bubble =
                Bubble(
                    rectF,
                    arrowDirection,
                    arrowWidth,
                    arrowHeight,
                    arrowPosition,
                    cornersRadius,
                    bubbleColor,
                    strokeWidth,
                    strokeColor,
                )
        }

        private fun resetPadding() {
            var paddingLeft = getPaddingLeft()
            var paddingRight = getPaddingRight()
            var paddingTop = getPaddingTop()
            var paddingBottom = getPaddingBottom()
            when (arrowDirection.value) {
                ArrowDirection.LEFT -> paddingLeft = (paddingLeft - arrowWidth).toInt()
                ArrowDirection.RIGHT -> paddingRight = (paddingRight - arrowWidth).toInt()
                ArrowDirection.TOP -> paddingTop = (paddingTop - arrowHeight).toInt()
                ArrowDirection.BOTTOM -> paddingBottom = (paddingBottom - arrowHeight).toInt()
            }
            if (strokeWidth > 0) {
                paddingLeft = (paddingLeft - strokeWidth).toInt()
                paddingRight = (paddingRight - strokeWidth).toInt()
                paddingTop = (paddingTop - strokeWidth).toInt()
                paddingBottom = (paddingBottom - strokeWidth).toInt()
            }
            setPadding(paddingLeft, paddingTop, paddingRight, paddingBottom)
        }

        companion object {
            const val DEFAULT_STROKE_WIDTH = -1f

            @JvmStatic
            internal fun convertDpToPixel(
                dp: Float,
                context: Context,
            ): Float {
                val metrics = context.resources.displayMetrics
                return dp * (metrics.densityDpi / DisplayMetrics.DENSITY_DEFAULT)
            }
        }
    }
