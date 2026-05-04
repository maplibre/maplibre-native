package org.maplibre.android.utils

import android.graphics.Color
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest

class ColorUtilsTest : BaseTest() {

    @Test
    fun rgbaToColor_decimalComponent() {
        val input = "rgba(255,128.0000952303,0,0.7)"
        val result = ColorUtils.rgbaToColor(input)
        Assert.assertEquals(Color.argb(255, 128, 0, (0.7 * 255).toInt()), result)
    }

    @Test
    fun rgbaToColor_decimalComponent_floor() {
        val input = "rgba(255,128.70123,0,0.7)"
        val result = ColorUtils.rgbaToColor(input)
        Assert.assertEquals(Color.argb(255, 128, 0, (0.7 * 255).toInt()), result)
    }
}
