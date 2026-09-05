package org.maplibre.android.text

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import android.graphics.Typeface
import androidx.annotation.Keep
import androidx.annotation.WorkerThread

/**
 * LocalGlyphRasterizer is the Android-specific platform implementation used
 * by the portable local_glyph_rasterizer.hpp
 */
@Keep
open class LocalGlyphRasterizer internal constructor() {
    /*
        60x60px dimensions are hardwired to match local_glyph_rasterizer.cpp.
        Glyphs are drawn at 2x texture resolution, while logical metrics stay at 1x.
        These dimensions are large enough to draw a 48 px character in the middle
        of the bitmap with some buffer around the edge.
     */
    private val bitmap: Bitmap = Bitmap.createBitmap(BITMAP_SIZE, BITMAP_SIZE, Bitmap.Config.ARGB_8888)

    private val paint =
        Paint().apply {
            isAntiAlias = true
            textSize = TEXT_SIZE.toFloat()
        }

    private val canvas = Canvas().apply { setBitmap(bitmap) }

    private val textBounds = Rect()

    // Tiny cache (font stacks × 2 for bold/regular) so glyphs don't
    // pay Typeface.create's cache lookup and internal lock per call.
    private val typefaceCache = HashMap<String, Typeface>()

    // Updated by drawGlyphBitmap so native code can read the measured top without
    // allocating a per-glyph result object.
    private var glyphTop = 0f

    private fun setTypeface(
        fontFamily: String,
        bold: Boolean,
    ) {
        val key = (if (bold) "bold:" else "regular:") + fontFamily
        var typeface = typefaceCache[key]
        if (typeface == null) {
            typeface = Typeface.create(fontFamily, if (bold) Typeface.BOLD else Typeface.NORMAL)
            typefaceCache[key] = typeface
        }
        paint.typeface = typeface
    }

    private fun measureGlyphTop(glyph: String): Int {
        paint.getTextBounds(glyph, 0, glyph.length, textBounds)
        return Math.max(0, -textBounds.top)
    }

    /**
     * Uses Android-native drawing code to rasterize a single glyph
     * to a square [Bitmap] which can be returned to portable
     * code for transformation into a Signed Distance Field glyph.
     *
     * @param fontFamily Font family string to pass to Typeface.create
     * @param bold If true, use Typeface.BOLD option
     * @param glyphID 16-bit Unicode BMP codepoint to draw
     *
     * @return Return a [Bitmap] to be displayed in the requested tile.
     */
    @WorkerThread
    protected fun drawGlyphBitmap(
        fontFamily: String,
        bold: Boolean,
        glyphID: Char,
    ): Bitmap {
        setTypeface(fontFamily, bold)
        val glyph = glyphID.toString()
        val glyphTopPx = measureGlyphTop(glyph)
        glyphTop = glyphTopPx / TEXTURE_SCALE.toFloat() - TOP_ADJUSTMENT
        canvas.drawColor(Color.WHITE)
        canvas.drawText(glyph, BASELINE_X, (RASTER_BUFFER + glyphTopPx).toFloat(), paint)
        return bitmap
    }

    @WorkerThread
    protected fun getLastGlyphTop(): Float = glyphTop

    private companion object {
        private const val TEXTURE_SCALE = 2
        private const val BITMAP_SIZE = 60
        private const val TEXT_SIZE = 48
        private const val RASTER_BUFFER = 3 * TEXTURE_SCALE
        private const val TOP_ADJUSTMENT = 26.5f
        private const val BASELINE_X = 10f
    }
}
