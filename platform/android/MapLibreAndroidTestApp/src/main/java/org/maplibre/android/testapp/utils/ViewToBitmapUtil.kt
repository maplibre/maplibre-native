package org.maplibre.android.testapp.utils

import android.graphics.Bitmap
import android.graphics.Canvas
import android.view.View

/**
 * Converts a View to a Bitmap so we can use an Android SDK View as a Symbol.
 */
object ViewToBitmapUtil {
    fun convertToBitmap(view: View): Bitmap {
        view.measure(
            View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED),
            View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
        )
        view.layout(0, 0, view.measuredWidth, view.measuredHeight)
        val bitmap =
            Bitmap.createBitmap(view.measuredWidth, view.measuredHeight, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        view.draw(canvas)
        return bitmap
    }
}
