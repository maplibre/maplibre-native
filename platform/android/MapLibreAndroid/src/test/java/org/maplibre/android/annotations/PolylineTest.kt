package org.maplibre.android.annotations

import org.maplibre.android.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest

class PolylineTest : BaseTest() {
    @Test
    fun testSanity() {
        val polylineOptions = PolylineOptions()
        Assert.assertNotNull("polylineOptions should not be null", polylineOptions)
    }

    @Test
    fun testPolyline() {
        val polyline = PolylineOptions().polyline
        Assert.assertNotNull("polyline should not be null", polyline)
    }

    @Test
    fun testAlpha() {
        val polyline = PolylineOptions().alpha(0.2f).polyline
        Assert.assertEquals(0.2f, polyline.alpha, 0.0f)
    }

    @Test
    fun testWidth() {
        val polyline = PolylineOptions().width(1f).polyline
        Assert.assertEquals(1.0f, polyline.width, 0f)
    }

    @Test
    fun testColor() {
        val polyline = PolylineOptions().color(1).polyline
        Assert.assertEquals(1, polyline.color.toLong())
    }

    @Test
    fun testAddLatLng() {
        val polyline = PolylineOptions().add(LatLng(0.0, 0.0)).polyline
        Assert.assertNotNull("Points should not be null", polyline.points)
        Assert.assertEquals(LatLng(0.0, 0.0), polyline.points[0])
    }

    @Test
    fun testAddAllLatLng() {
        val coordinates: MutableList<LatLng> = ArrayList()
        coordinates.add(LatLng(0.0, 0.0))
        val polyline = PolylineOptions().addAll(coordinates).polyline
        Assert.assertNotNull(polyline.points)
        Assert.assertEquals(LatLng(0.0, 0.0), polyline.points[0])
    }

    @Test
    fun testBuilder() {
        val polylineOptions = PolylineOptions()
        polylineOptions.width(1.0f)
        polylineOptions.color(2)
        polylineOptions.add(LatLng(0.0, 0.0))
        val polyline = polylineOptions.polyline
        Assert.assertEquals(1.0f, polyline.width, 0f)
        Assert.assertEquals(2, polyline.color.toLong())
        Assert.assertNotNull("Points should not be null", polyline.points)
        Assert.assertEquals(LatLng(0.0, 0.0), polyline.points[0])
    }
}
