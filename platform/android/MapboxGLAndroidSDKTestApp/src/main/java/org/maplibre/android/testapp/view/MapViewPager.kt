package org.maplibre.android.testapp.view

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceView
import android.view.View
import androidx.viewpager.widget.PagerTabStrip
import androidx.viewpager.widget.ViewPager

class MapViewPager(context: Context?, attrs: AttributeSet?) : ViewPager(
    context!!,
    attrs
) {
    override fun canScroll(v: View, checkV: Boolean, dx: Int, x: Int, y: Int): Boolean {
        return v is SurfaceView || v is PagerTabStrip || super.canScroll(v, checkV, dx, x, y)
    }
}
