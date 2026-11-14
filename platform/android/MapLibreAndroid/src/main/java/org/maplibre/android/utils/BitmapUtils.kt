package org.maplibre.android.utils

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.view.View
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import androidx.annotation.VisibleForTesting
import androidx.core.content.ContextCompat
import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer
import java.util.Arrays

/**
 * Utility class for creating bitmaps
 */
object BitmapUtils {
    /**
     * Convert a view to a bitmap.
     *
     * @param view the view to convert
     * @return the converted bitmap
     */
    @JvmStatic
    @Suppress("DEPRECATION")
    fun createBitmapFromView(view: View): Bitmap? {
        view.isDrawingCacheEnabled = true
        view.drawingCacheQuality = View.DRAWING_CACHE_QUALITY_LOW
        view.buildDrawingCache()

        val drawingCache = view.drawingCache ?: return null

        val snapshot = Bitmap.createBitmap(drawingCache)
        view.isDrawingCacheEnabled = false
        view.destroyDrawingCache()
        return snapshot
    }

    /**
     * Create a bitmap from a background and a foreground bitmap. The foreground bitmap
     * will be shifted 10px to the right and 10px to the bottom relative to the background.
     *
     * @param background The bitmap placed in the background
     * @param foreground The bitmap placed in the foreground
     * @return the merged bitmap
     */
    @Deprecated(
        "mergeBitmaps should be used instead, as it does not shift the input by 10px to the " +
            "right and bottom.",
        ReplaceWith("mergeBitmaps(background, foreground)"),
    )
    @JvmStatic
    fun mergeBitmap(
        background: Bitmap,
        foreground: Bitmap,
    ): Bitmap = mergeBitmap(background, foreground, 10f, 10f)

    /**
     * Create a bitmap from a background and a foreground bitmap.
     *
     * @param background The bitmap placed in the background
     * @param foreground The bitmap placed in the foreground
     * @return the merged bitmap
     */
    @JvmStatic
    fun mergeBitmaps(
        background: Bitmap,
        foreground: Bitmap,
    ): Bitmap = mergeBitmap(background, foreground, 0f, 0f)

    /**
     * Create a bitmap from a background and a foreground bitmap
     *
     * @param background The bitmap placed in the background
     * @param foreground The bitmap placed in the foreground
     * @param left position of the left side of the foreground bitmap
     * @param top position of the top side of the foreground bitmap
     * @return the merged bitmap
     */
    @JvmStatic
    fun mergeBitmap(
        background: Bitmap,
        foreground: Bitmap,
        left: Float,
        top: Float,
    ): Bitmap {
        val result = Bitmap.createBitmap(background.width, background.height, background.config)
        val canvas = Canvas(result)
        canvas.drawBitmap(background, 0f, 0f, null)
        canvas.drawBitmap(foreground, left, top, null)
        return result
    }

    /**
     * Extract an underlying bitmap from a drawable
     *
     * @param sourceDrawable The source drawable
     * @return The underlying bitmap
     */
    @JvmStatic
    fun getBitmapFromDrawable(sourceDrawable: Drawable?): Bitmap? {
        if (sourceDrawable == null) {
            return null
        }

        if (sourceDrawable is BitmapDrawable) {
            return sourceDrawable.bitmap
        } else {
            // copying drawable object to not manipulate on the same reference
            val constantState = sourceDrawable.constantState ?: return null
            val drawable = constantState.newDrawable().mutate()

            val bitmap =
                Bitmap.createBitmap(
                    drawable.intrinsicWidth,
                    drawable.intrinsicHeight,
                    Bitmap.Config.ARGB_8888,
                )
            val canvas = Canvas(bitmap)
            drawable.setBounds(0, 0, canvas.width, canvas.height)
            drawable.draw(canvas)
            return bitmap
        }
    }

    /**
     * Create a byte array out of drawable
     *
     * @param drawable The source drawable
     * @return The byte array of source drawable
     */
    @JvmStatic
    fun getByteArrayFromDrawable(drawable: Drawable?): ByteArray? {
        if (drawable == null) {
            return null
        }

        val bitmap = getBitmapFromDrawable(drawable) ?: return null

        try {
            val stream = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream)
            return stream.toByteArray()
        } finally {
            // If drawable is not a BitMapDrawable, getBitmapFromDrawable() allocated
            // a Bitmap. Release early as recommended by
            // https://developer.android.com/reference/android/graphics/Bitmap#recycle()
            // see also https://github.com/maplibre/maplibre-native/issues/3864
            if (drawable !is BitmapDrawable) {
                bitmap.recycle()
            }
        }
    }

    /**
     * Decode byte array to drawable object
     *
     * @param context Context to obtain [android.content.res.Resources]
     * @param array   The source byte array
     * @return The drawable created from source byte array
     */
    @JvmStatic
    fun getDrawableFromByteArray(
        context: Context,
        array: ByteArray?,
    ): Drawable? {
        if (array == null) {
            return null
        }
        val compass = BitmapFactory.decodeByteArray(array, 0, array.size)
        return BitmapDrawable(context.resources, compass)
    }

    /**
     * Get a drawable from a resource.
     *
     * @param context     Context to obtain [android.content.res.Resources]
     * @param drawableRes Drawable resource
     * @return The drawable created from the resource
     */
    @JvmStatic
    fun getDrawableFromRes(
        context: Context,
        @DrawableRes drawableRes: Int,
    ): Drawable? = getDrawableFromRes(context, drawableRes, null)

    /**
     * Get a tinted drawable from a resource.
     *
     * @param context     Context to obtain [android.content.res.Resources]
     * @param drawableRes Drawable resource
     * @param tintColor   Tint color
     * @return The drawable created from the resource
     */
    @JvmStatic
    fun getDrawableFromRes(
        context: Context,
        @DrawableRes drawableRes: Int,
        @ColorInt tintColor: Int?,
    ): Drawable? {
        val drawable = ContextCompat.getDrawable(context, drawableRes) ?: return null

        if (tintColor == null) {
            return drawable
        }

        drawable.setTint(tintColor)
        return drawable
    }

    /**
     * Validates if the bytes of a bitmap matches another
     *
     * @param bitmap the bitmap to be compared against
     * @param other  the bitmap to compare with
     * @return true if equal
     */
    @VisibleForTesting
    @JvmStatic
    fun equals(
        bitmap: Bitmap,
        other: Bitmap,
    ): Boolean {
        val buffer = ByteBuffer.allocate(bitmap.height * bitmap.rowBytes)
        bitmap.copyPixelsToBuffer(buffer)

        val bufferOther = ByteBuffer.allocate(other.height * other.rowBytes)
        other.copyPixelsToBuffer(bufferOther)

        return Arrays.equals(buffer.array(), bufferOther.array())
    }
}
