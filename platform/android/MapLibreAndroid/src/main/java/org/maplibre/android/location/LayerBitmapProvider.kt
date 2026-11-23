package org.maplibre.android.location

import android.content.Context
import android.graphics.Bitmap
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import org.maplibre.android.R
import org.maplibre.android.utils.BitmapUtils

internal class LayerBitmapProvider(
    private val context: Context,
) {
    fun generateBitmap(
        @DrawableRes drawableRes: Int,
        @ColorInt tintColor: Int?,
    ): Bitmap? {
        val drawable = BitmapUtils.getDrawableFromRes(context, drawableRes, tintColor)
        return BitmapUtils.getBitmapFromDrawable(drawable)
    }

    fun generateShadowBitmap(options: LocationComponentOptions): Bitmap {
        val shadowDrawable = BitmapUtils.getDrawableFromRes(context, R.drawable.maplibre_user_icon_shadow)
        return Utils.generateShadow(shadowDrawable!!, options.elevation())
    }
}
