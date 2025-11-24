package org.maplibre.android.annotations

import android.graphics.Bitmap
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.MockitoAnnotations

class IconTest : BaseTest() {
    @Mock
    var bitmap: Bitmap? = null

    @Before
    fun beforeTest() {
        MockitoAnnotations.initMocks(this)
        Mockito.`when`(bitmap!!.config).thenReturn(Bitmap.Config.ARGB_8888)
    }

    @Test
    fun testId() {
        val id = "test"
        val icon = IconFactory.recreate(id, Bitmap.createBitmap(0, 0, Bitmap.Config.ALPHA_8))
        Assert.assertEquals("id should match", id, icon.id)
    }

    @Test
    fun testBitmap() {
        val icon = IconFactory.recreate("test", bitmap!!)
        Assert.assertEquals("bitmap should match", bitmap, icon.bitmap)
    }

    @Test
    fun testEquals() {
        val icon1 = IconFactory.recreate("test", bitmap!!)
        val icon2 = IconFactory.recreate("test", bitmap!!)
        Assert.assertEquals("icons should not match", icon1, icon2)
    }

    @Test
    fun testEqualsObject() {
        val icon = IconFactory.recreate("test", Bitmap.createBitmap(0, 0, Bitmap.Config.ALPHA_8))
        Assert.assertNotSame("icon should not match", Any(), icon)
    }

    @Test
    fun testHashcode() {
        val icon = IconFactory.recreate("test", bitmap!!)
        val expectedHashcode = (31 * bitmap.hashCode() + "test".hashCode()).toLong()
        Assert.assertEquals("hashcode should match", expectedHashcode, icon.hashCode().toLong())
    }
}
