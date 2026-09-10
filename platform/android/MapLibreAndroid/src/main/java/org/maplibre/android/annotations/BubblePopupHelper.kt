package org.maplibre.android.annotations

import android.content.Context
import android.view.ViewGroup
import android.widget.PopupWindow
import org.maplibre.android.R

@Deprecated("As of 7.0.0")
internal object BubblePopupHelper {
    @JvmStatic
    fun create(
        context: Context,
        bubbleLayout: BubbleLayout,
    ): PopupWindow {
        val popupWindow = PopupWindow(context)

        popupWindow.contentView = bubbleLayout
        popupWindow.isOutsideTouchable = true
        popupWindow.width = ViewGroup.LayoutParams.WRAP_CONTENT
        popupWindow.height = ViewGroup.LayoutParams.WRAP_CONTENT
        popupWindow.animationStyle = android.R.style.Animation_Dialog
        // change background color to transparent
        popupWindow.setBackgroundDrawable(context.getDrawable(R.drawable.maplibre_popup_window_transparent))

        return popupWindow
    }
}
