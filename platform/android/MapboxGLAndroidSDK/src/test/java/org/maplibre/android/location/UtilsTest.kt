package org.maplibre.android.location

import org.junit.Assert
import org.junit.Test

class UtilsTest {
    @Test
    @Throws(Exception::class)
    fun shortestRotation_doesReturnValueDistanceQuickestToZero() {
        var value = Utils.shortestRotation(0f, 181f)
        Assert.assertEquals(360f, value)
        value = Utils.shortestRotation(0f, 179f)
        Assert.assertEquals(0f, value)
        value = Utils.shortestRotation(0f, 180f)
        Assert.assertEquals(0f, value)
    }

    @Test
    @Throws(Exception::class)
    fun shortestRotation_doesReturnValueDistanceQuickestToFifty() {
        var value = Utils.shortestRotation(50f, 231f)
        Assert.assertEquals(410f, value)
        value = Utils.shortestRotation(50f, 229f)
        Assert.assertEquals(50f, value)
        value = Utils.shortestRotation(50f, 180f)
        Assert.assertEquals(50f, value)
    }
}
