package com.mapbox.mapboxsdk.testapp.view

import android.content.Context
import android.util.AttributeSet
import androidx.viewpager.widget.ViewPager
import android.view.SurfaceView
import android.view.View
import androidx.viewpager.widget.PagerTabStrip

class MapViewPager(context: Context?, attrs: AttributeSet?) : ViewPager(
    context!!, attrs
) {
    override fun canScroll(v: View, checkV: Boolean, dx: Int, x: Int, y: Int): Boolean {
        return v is SurfaceView || v is PagerTabStrip || super.canScroll(v, checkV, dx, x, y)
    }
}