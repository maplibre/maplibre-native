package org.maplibre.android.text;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.WorkerThread;

import java.util.HashMap;
import java.util.Map;

/**
 * LocalGlyphRasterizer is the Android-specific platform implementation used
 * by the portable local_glyph_rasterizer.hpp
 */
@Keep
public class LocalGlyphRasterizer {
  private static final int TEXTURE_SCALE = 2;
  private static final int BITMAP_SIZE = 60;
  private static final int TEXT_SIZE = 48;
  private static final int RASTER_BUFFER = 3 * TEXTURE_SCALE;
  private static final float TOP_ADJUSTMENT = 26.5f;
  private static final float BASELINE_X = 10;

  private final Bitmap bitmap;
  @NonNull
  private final Paint paint;
  @NonNull
  private final Canvas canvas;
  @NonNull
  private final Rect textBounds;
  @NonNull
  // Tiny cache (font stacks × 2 for bold/regular) so glyphs don't
  // pay Typeface.create's cache lookup and internal lock per call.
  private final Map<String, Typeface> typefaceCache;
  // Updated by drawGlyphBitmap so native code can read the measured top without
  // allocating a per-glyph result object.
  private float glyphTop;

  LocalGlyphRasterizer() {
    /*
      60x60px dimensions are hardwired to match local_glyph_rasterizer.cpp.
      Glyphs are drawn at 2x texture resolution, while logical metrics stay at 1x.
      These dimensions are large enough to draw a 48 px character in the middle
      of the bitmap with some buffer around the edge.
    */
    bitmap = Bitmap.createBitmap(BITMAP_SIZE, BITMAP_SIZE, Bitmap.Config.ARGB_8888);

    paint = new Paint();
    paint.setAntiAlias(true);
    paint.setTextSize(TEXT_SIZE);

    canvas = new Canvas();
    canvas.setBitmap(bitmap);
    textBounds = new Rect();
    typefaceCache = new HashMap<>();
  }

  private void setTypeface(String fontFamily, boolean bold) {
    String key = (bold ? "bold:" : "regular:") + fontFamily;
    Typeface typeface = typefaceCache.get(key);
    if (typeface == null) {
      typeface = Typeface.create(fontFamily, bold ? Typeface.BOLD : Typeface.NORMAL);
      typefaceCache.put(key, typeface);
    }
    paint.setTypeface(typeface);
  }

  private int measureGlyphTop(String glyph) {
    paint.getTextBounds(glyph, 0, glyph.length(), textBounds);
    return Math.max(0, -textBounds.top);
  }

  /***
   * Uses Android-native drawing code to rasterize a single glyph
   * to a square {@link Bitmap} which can be returned to portable
   * code for transformation into a Signed Distance Field glyph.
   *
   * @param fontFamily Font family string to pass to Typeface.create
   * @param bold If true, use Typeface.BOLD option
   * @param glyphID 16-bit Unicode BMP codepoint to draw
   *
   * @return Return a {@link Bitmap} to be displayed in the requested tile.
   */
  @WorkerThread
  protected Bitmap drawGlyphBitmap(String fontFamily, boolean bold, char glyphID) {
    setTypeface(fontFamily, bold);
    String glyph = String.valueOf(glyphID);
    int glyphTopPx = measureGlyphTop(glyph);
    glyphTop = glyphTopPx / (float) TEXTURE_SCALE - TOP_ADJUSTMENT;
    canvas.drawColor(Color.WHITE);
    canvas.drawText(glyph, BASELINE_X, RASTER_BUFFER + glyphTopPx, paint);
    return bitmap;
  }

  @WorkerThread
  protected float getLastGlyphTop() {
    return glyphTop;
  }
}
