package org.maplibre.android.utils

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.maplibre.android.constants.MapLibreConstants
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4ClassRunner::class)
class FontUtilsTest {
    @Test
    fun testExtractedFontShouldMatchDefault() {
        val fonts = arrayOf("foo", "bar")
        val actual = FontUtils.extractValidFont(*fonts)
        Assert.assertEquals("Selected font should match", MapLibreConstants.DEFAULT_FONT, actual)
    }

    @Test
    fun testExtractedFontShouldMatchMonospace() {
        val expected = "monospace"
        val fonts = arrayOf("foo", expected)
        val actual = FontUtils.extractValidFont(*fonts)
        Assert.assertEquals("Selected font should match", expected, actual)
    }

    @Test
    fun testExtractedFontShouldBeNull() {
        val actual = FontUtils.extractValidFont()
        Assert.assertNull(actual)
    }
}
