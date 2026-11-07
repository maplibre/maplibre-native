package org.maplibre.android.testapp.activity.stability

import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.hardware.display.DisplayManager
import android.util.AttributeSet
import android.view.Display
import android.view.View
import kotlin.math.min

/**
 * Scrolling bar graph of per-frame encoding time.
 * Bar width, exaggeration, and colors follow the iOS frame-time graph.
 */
class FrameTimeGraphView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
) : View(context, attrs) {

    private val exaggeration = 10f
    private val barWidthPx = 4f * resources.displayMetrics.density
    private val bars = ArrayDeque<Double>()
    private val barPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val thresholdPaint = Paint().apply { color = 0xFF444444.toInt() }
    private val safeColor = rgb(0, 190, 123)
    private val warningColor = rgb(255, 154, 82)
    private val dangerColor = rgb(255, 91, 86)

    init {
        setBackgroundColor(0x40000000)
    }

    fun addFrameDuration(frameDurationSeconds: Double) {
        bars.addLast(frameDurationSeconds)
        val maxBars = ((width / barWidthPx) * 3).toInt().coerceAtLeast(8)
        while (bars.size > maxBars) {
            bars.removeFirst()
        }
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (bars.isEmpty() || width == 0 || height == 0) {
            return
        }

        val target = renderDurationTargetSeconds()
        val thresholdY = height - height * exaggeration * target
        canvas.drawRect(0f, thresholdY, width.toFloat(), thresholdY + 1f, thresholdPaint)

        val visible = (width / barWidthPx).toInt().coerceAtLeast(1)
        val start = (bars.size - visible).coerceAtLeast(0)
        var x = 0f
        for (index in start until bars.size) {
            val duration = bars[index]
            val barHeight = min(duration.toFloat() * exaggeration * height, height.toFloat())
            barPaint.color = color(duration, target)
            canvas.drawRect(x, height - barHeight, x + barWidthPx, height.toFloat(), barPaint)
            x += barWidthPx
        }
    }

    private fun renderDurationTargetSeconds(): Float {
        val displayManager = context.getSystemService(DisplayManager::class.java)
        val refreshRate = displayManager?.getDisplay(Display.DEFAULT_DISPLAY)?.refreshRate ?: 60f
        val fps = if (refreshRate > 1f) refreshRate else 60f
        return 1f / fps
    }

    private fun color(frameDuration: Double, target: Float): Int {
        val greenLevel = target.toDouble()
        val yellowLevel = greenLevel * 3
        return when {
            frameDuration <= greenLevel -> safeColor
            frameDuration <= yellowLevel -> warningColor
            else -> dangerColor
        }
    }

    private fun rgb(red: Int, green: Int, blue: Int): Int {
        return android.graphics.Color.rgb(red, green, blue)
    }
}
