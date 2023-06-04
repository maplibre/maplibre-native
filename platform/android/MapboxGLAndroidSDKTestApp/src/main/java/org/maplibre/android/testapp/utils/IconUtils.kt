package org.maplibre.android.testapp.utils

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import androidx.core.content.res.ResourcesCompat
import androidx.core.graphics.drawable.DrawableCompat
import org.maplibre.android.annotations.Icon
import org.maplibre.android.annotations.IconFactory

object IconUtils {
    /**
     * Demonstrates converting any Drawable to an Icon, for use as a marker icon.
     */
    @JvmStatic
    fun drawableToIcon(context: Context, @DrawableRes id: Int, @ColorInt colorRes: Int): Icon {
        val vectorDrawable = ResourcesCompat.getDrawable(context.resources, id, context.theme)
        val bitmap = Bitmap.createBitmap(
            vectorDrawable!!.intrinsicWidth,
            vectorDrawable.intrinsicHeight,
            Bitmap.Config.ARGB_8888
        )
        val canvas = Canvas(bitmap)
        vectorDrawable.setBounds(0, 0, canvas.width, canvas.height)
        DrawableCompat.setTint(vectorDrawable, colorRes)
        vectorDrawable.draw(canvas)
        return IconFactory.getInstance(context).fromBitmap(bitmap)
    }
}
