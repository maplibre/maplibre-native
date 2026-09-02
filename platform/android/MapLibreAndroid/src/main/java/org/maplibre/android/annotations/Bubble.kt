package org.maplibre.android.annotations

import android.graphics.Canvas
import android.graphics.ColorFilter
import android.graphics.Paint
import android.graphics.Path
import android.graphics.PixelFormat
import android.graphics.Rect
import android.graphics.RectF
import android.graphics.drawable.Drawable
import kotlin.math.min

@Deprecated("As of 7.0.0")
internal class Bubble(
    private val rect: RectF,
    arrowDirection: ArrowDirection,
    private val arrowWidth: Float,
    private val arrowHeight: Float,
    private val arrowPosition: Float,
    private val cornersRadius: Float,
    bubbleColor: Int,
    private val strokeWidth: Float,
    strokeColor: Int,
) : Drawable() {
    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private var strokePaint: Paint? = null
    private var strokePath: Path? = null
    private val path = Path()

    init {
        paint.color = bubbleColor

        if (strokeWidth > 0) {
            val strokePaint = Paint(Paint.ANTI_ALIAS_FLAG)
            strokePaint.color = strokeColor
            this.strokePaint = strokePaint
            val strokePath = Path()
            this.strokePath = strokePath
            initPath(arrowDirection, path, strokeWidth)
            initPath(arrowDirection, strokePath, 0f)
        } else {
            initPath(arrowDirection, path, 0f)
        }
    }

    override fun onBoundsChange(bounds: Rect) {
        super.onBoundsChange(bounds)
    }

    override fun draw(canvas: Canvas) {
        if (strokeWidth > 0) {
            canvas.drawPath(strokePath!!, strokePaint!!)
        }
        canvas.drawPath(path, paint)
    }

    @Deprecated("Deprecated in Java")
    override fun getOpacity(): Int = PixelFormat.TRANSLUCENT

    override fun setAlpha(alpha: Int) {
        paint.alpha = alpha
    }

    override fun setColorFilter(cf: ColorFilter?) {
        paint.colorFilter = cf
    }

    override fun getIntrinsicWidth(): Int = rect.width().toInt()

    override fun getIntrinsicHeight(): Int = rect.height().toInt()

    private fun initPath(
        arrowDirection: ArrowDirection,
        path: Path,
        strokeWidth: Float,
    ) {
        when (arrowDirection.value) {
            ArrowDirection.LEFT -> {
                if (cornersRadius <= 0 || (strokeWidth > 0 && strokeWidth > cornersRadius)) {
                    initLeftSquarePath(rect, path, strokeWidth)
                } else {
                    initLeftRoundedPath(rect, path, strokeWidth)
                }
            }

            ArrowDirection.TOP -> {
                if (cornersRadius <= 0 || (strokeWidth > 0 && strokeWidth > cornersRadius)) {
                    initTopSquarePath(rect, path, strokeWidth)
                } else {
                    initTopRoundedPath(rect, path, strokeWidth)
                }
            }

            ArrowDirection.RIGHT -> {
                if (cornersRadius <= 0 || (strokeWidth > 0 && strokeWidth > cornersRadius)) {
                    initRightSquarePath(rect, path, strokeWidth)
                } else {
                    initRightRoundedPath(rect, path, strokeWidth)
                }
            }

            ArrowDirection.BOTTOM -> {
                if (cornersRadius <= 0 || (strokeWidth > 0 && strokeWidth > cornersRadius)) {
                    initBottomSquarePath(rect, path, strokeWidth)
                } else {
                    initBottomRoundedPath(rect, path, strokeWidth)
                }
            }
        }
    }

    private fun initLeftSquarePath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(arrowWidth + rect.left + strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.width() - strokeWidth, rect.top + strokeWidth)

        path.lineTo(rect.right - strokeWidth, rect.bottom - strokeWidth)

        path.lineTo(rect.left + arrowWidth + strokeWidth, rect.bottom - strokeWidth)

        path.lineTo(rect.left + arrowWidth + strokeWidth, arrowHeight + arrowPosition - (strokeWidth / 2))
        path.lineTo(rect.left + strokeWidth + strokeWidth, arrowPosition + arrowHeight / 2)
        path.lineTo(rect.left + arrowWidth + strokeWidth, arrowPosition + (strokeWidth / 2))

        path.lineTo(rect.left + arrowWidth + strokeWidth, rect.top + strokeWidth)

        path.close()
    }

    private fun initLeftRoundedPath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(arrowWidth + rect.left + cornersRadius + strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.width() - cornersRadius - strokeWidth, rect.top + strokeWidth)
        path.arcTo(
            RectF(
                rect.right - cornersRadius,
                rect.top + strokeWidth,
                rect.right - strokeWidth,
                cornersRadius + rect.top,
            ),
            270f,
            90f,
        )

        path.lineTo(rect.right - strokeWidth, rect.bottom - cornersRadius - strokeWidth)
        path.arcTo(
            RectF(
                rect.right - cornersRadius,
                rect.bottom - cornersRadius,
                rect.right - strokeWidth,
                rect.bottom - strokeWidth,
            ),
            0f,
            90f,
        )

        path.lineTo(rect.left + arrowWidth + cornersRadius + strokeWidth, rect.bottom - strokeWidth)

        path.arcTo(
            RectF(
                rect.left + arrowWidth + strokeWidth,
                rect.bottom - cornersRadius,
                cornersRadius + rect.left + arrowWidth,
                rect.bottom - strokeWidth,
            ),
            90f,
            90f,
        )

        path.lineTo(rect.left + arrowWidth + strokeWidth, arrowHeight + arrowPosition - (strokeWidth / 2))

        path.lineTo(rect.left + strokeWidth + strokeWidth, arrowPosition + arrowHeight / 2)

        path.lineTo(rect.left + arrowWidth + strokeWidth, arrowPosition + (strokeWidth / 2))

        path.lineTo(rect.left + arrowWidth + strokeWidth, rect.top + cornersRadius + strokeWidth)

        path.arcTo(
            RectF(
                rect.left + arrowWidth + strokeWidth,
                rect.top + strokeWidth,
                cornersRadius + rect.left + arrowWidth,
                cornersRadius + rect.top,
            ),
            180f,
            90f,
        )

        path.close()
    }

    private fun initTopSquarePath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(rect.left + arrowPosition + strokeWidth, rect.top + arrowHeight + strokeWidth)

        path.lineTo(rect.left + arrowPosition + (strokeWidth / 2), rect.top + arrowHeight + strokeWidth)
        path.lineTo(rect.left + arrowWidth / 2 + arrowPosition, rect.top + strokeWidth + strokeWidth)
        path.lineTo(rect.left + arrowWidth + arrowPosition - (strokeWidth / 2), rect.top + arrowHeight + strokeWidth)
        path.lineTo(rect.right - strokeWidth, rect.top + arrowHeight + strokeWidth)

        path.lineTo(rect.right - strokeWidth, rect.bottom - strokeWidth)

        path.lineTo(rect.left + strokeWidth, rect.bottom - strokeWidth)

        path.lineTo(rect.left + strokeWidth, rect.top + arrowHeight + strokeWidth)

        path.lineTo(rect.left + arrowPosition + strokeWidth, rect.top + arrowHeight + strokeWidth)

        path.close()
    }

    private fun initTopRoundedPath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(
            rect.left + min(arrowPosition, cornersRadius) + strokeWidth,
            rect.top + arrowHeight + strokeWidth,
        )
        path.lineTo(rect.left + arrowPosition + (strokeWidth / 2), rect.top + arrowHeight + strokeWidth)
        path.lineTo(rect.left + arrowWidth / 2 + arrowPosition, rect.top + strokeWidth + strokeWidth)
        path.lineTo(rect.left + arrowWidth + arrowPosition - (strokeWidth / 2), rect.top + arrowHeight + strokeWidth)
        path.lineTo(rect.right - cornersRadius - strokeWidth, rect.top + arrowHeight + strokeWidth)

        path.arcTo(
            RectF(
                rect.right - cornersRadius,
                rect.top + arrowHeight + strokeWidth,
                rect.right - strokeWidth,
                cornersRadius + rect.top + arrowHeight,
            ),
            270f,
            90f,
        )
        path.lineTo(rect.right - strokeWidth, rect.bottom - cornersRadius - strokeWidth)

        path.arcTo(
            RectF(
                rect.right - cornersRadius,
                rect.bottom - cornersRadius,
                rect.right - strokeWidth,
                rect.bottom - strokeWidth,
            ),
            0f,
            90f,
        )
        path.lineTo(rect.left + cornersRadius + strokeWidth, rect.bottom - strokeWidth)

        path.arcTo(
            RectF(
                rect.left + strokeWidth,
                rect.bottom - cornersRadius,
                cornersRadius + rect.left,
                rect.bottom - strokeWidth,
            ),
            90f,
            90f,
        )

        path.lineTo(rect.left + strokeWidth, rect.top + arrowHeight + cornersRadius + strokeWidth)

        path.arcTo(
            RectF(
                rect.left + strokeWidth,
                rect.top + arrowHeight + strokeWidth,
                cornersRadius + rect.left,
                cornersRadius + rect.top + arrowHeight,
            ),
            180f,
            90f,
        )

        path.close()
    }

    private fun initRightSquarePath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(rect.left + strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.width() - arrowWidth - strokeWidth, rect.top + strokeWidth)

        path.lineTo(rect.right - arrowWidth - strokeWidth, arrowPosition + (strokeWidth / 2))
        path.lineTo(rect.right - strokeWidth - strokeWidth, arrowPosition + arrowHeight / 2)
        path.lineTo(rect.right - arrowWidth - strokeWidth, arrowPosition + arrowHeight - (strokeWidth / 2))

        path.lineTo(rect.right - arrowWidth - strokeWidth, rect.bottom - strokeWidth)

        path.lineTo(rect.left + strokeWidth, rect.bottom - strokeWidth)
        path.lineTo(rect.left + strokeWidth, rect.top + strokeWidth)

        path.close()
    }

    private fun initRightRoundedPath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(rect.left + cornersRadius + strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.width() - cornersRadius - arrowWidth - strokeWidth, rect.top + strokeWidth)
        path.arcTo(
            RectF(
                rect.right - cornersRadius - arrowWidth,
                rect.top + strokeWidth,
                rect.right - arrowWidth - strokeWidth,
                cornersRadius + rect.top,
            ),
            270f,
            90f,
        )

        path.lineTo(rect.right - arrowWidth - strokeWidth, arrowPosition + (strokeWidth / 2))
        path.lineTo(rect.right - strokeWidth - strokeWidth, arrowPosition + arrowHeight / 2)
        path.lineTo(rect.right - arrowWidth - strokeWidth, arrowPosition + arrowHeight - (strokeWidth / 2))
        path.lineTo(rect.right - arrowWidth - strokeWidth, rect.bottom - cornersRadius - strokeWidth)

        path.arcTo(
            RectF(
                rect.right - cornersRadius - arrowWidth,
                rect.bottom - cornersRadius,
                rect.right - arrowWidth - strokeWidth,
                rect.bottom - strokeWidth,
            ),
            0f,
            90f,
        )
        path.lineTo(rect.left + arrowWidth + strokeWidth, rect.bottom - strokeWidth)

        path.arcTo(
            RectF(
                rect.left + strokeWidth,
                rect.bottom - cornersRadius,
                cornersRadius + rect.left,
                rect.bottom - strokeWidth,
            ),
            90f,
            90f,
        )

        path.arcTo(
            RectF(
                rect.left + strokeWidth,
                rect.top + strokeWidth,
                cornersRadius + rect.left,
                cornersRadius + rect.top,
            ),
            180f,
            90f,
        )
        path.close()
    }

    private fun initBottomSquarePath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(rect.left + strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.right - strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.right - strokeWidth, rect.bottom - arrowHeight - strokeWidth)

        path.lineTo(
            rect.left + arrowWidth + arrowPosition - (strokeWidth / 2),
            rect.bottom - arrowHeight - strokeWidth,
        )
        path.lineTo(rect.left + arrowPosition + arrowWidth / 2, rect.bottom - strokeWidth - strokeWidth)
        path.lineTo(rect.left + arrowPosition + (strokeWidth / 2), rect.bottom - arrowHeight - strokeWidth)
        path.lineTo(rect.left + arrowPosition + strokeWidth, rect.bottom - arrowHeight - strokeWidth)

        path.lineTo(rect.left + strokeWidth, rect.bottom - arrowHeight - strokeWidth)
        path.lineTo(rect.left + strokeWidth, rect.top + strokeWidth)
        path.close()
    }

    private fun initBottomRoundedPath(
        rect: RectF,
        path: Path,
        strokeWidth: Float,
    ) {
        path.moveTo(rect.left + cornersRadius + strokeWidth, rect.top + strokeWidth)
        path.lineTo(rect.width() - cornersRadius - strokeWidth, rect.top + strokeWidth)
        path.arcTo(
            RectF(
                rect.right - cornersRadius,
                rect.top + strokeWidth,
                rect.right - strokeWidth,
                cornersRadius + rect.top,
            ),
            270f,
            90f,
        )

        path.lineTo(rect.right - strokeWidth, rect.bottom - arrowHeight - cornersRadius - strokeWidth)
        path.arcTo(
            RectF(
                rect.right - cornersRadius,
                rect.bottom - cornersRadius - arrowHeight,
                rect.right - strokeWidth,
                rect.bottom - arrowHeight - strokeWidth,
            ),
            0f,
            90f,
        )

        path.lineTo(
            rect.left + arrowWidth + arrowPosition - (strokeWidth / 2),
            rect.bottom - arrowHeight - strokeWidth,
        )
        path.lineTo(rect.left + arrowPosition + arrowWidth / 2, rect.bottom - strokeWidth - strokeWidth)
        path.lineTo(rect.left + arrowPosition + (strokeWidth / 2), rect.bottom - arrowHeight - strokeWidth)
        path.lineTo(
            rect.left + min(cornersRadius, arrowPosition) + strokeWidth,
            rect.bottom - arrowHeight - strokeWidth,
        )

        path.arcTo(
            RectF(
                rect.left + strokeWidth,
                rect.bottom - cornersRadius - arrowHeight,
                cornersRadius + rect.left,
                rect.bottom - arrowHeight - strokeWidth,
            ),
            90f,
            90f,
        )
        path.lineTo(rect.left + strokeWidth, rect.top + cornersRadius + strokeWidth)
        path.arcTo(
            RectF(
                rect.left + strokeWidth,
                rect.top + strokeWidth,
                cornersRadius + rect.left,
                cornersRadius + rect.top,
            ),
            180f,
            90f,
        )
        path.close()
    }
}
