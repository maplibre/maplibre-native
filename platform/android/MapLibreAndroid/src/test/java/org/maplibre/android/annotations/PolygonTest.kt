package org.maplibre.android.annotations

import org.maplibre.android.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest

class PolygonTest : BaseTest() {
    @Test
    fun testSanity() {
        val polygonOptions = PolygonOptions()
        Assert.assertNotNull("polygonOptions should not be null", polygonOptions)
    }

    @Test
    fun testPolygon() {
        val polygon = PolygonOptions().polygon
        Assert.assertNotNull("polyline should not be null", polygon)
    }

    @Test
    fun testAlpha() {
        val polygon = PolygonOptions().alpha(0.5f).polygon
        Assert.assertEquals(0.5f, polygon.alpha, 0.0f)
    }

    @Test
    fun testStrokeColor() {
        val polygon = PolygonOptions().strokeColor(1).polygon
        Assert.assertEquals(1, polygon.strokeColor.toLong())
    }

    @Test
    fun testFillColor() {
        val polygon = PolygonOptions().fillColor(1).polygon
        Assert.assertEquals(1, polygon.fillColor.toLong())
    }

    @Test
    fun testLatLng() {
        val polygon = PolygonOptions().add(LatLng(0.0, 0.0)).polygon
        Assert.assertNotNull("points should not be null", polygon.points)
        Assert.assertEquals(LatLng(0.0, 0.0), polygon.points[0])
    }

    @Test
    fun testAddAllLatLng() {
        val coordinates: MutableList<LatLng> = ArrayList()
        coordinates.add(LatLng(0.0, 0.0))
        val polygon = PolygonOptions().addAll(coordinates).polygon
        Assert.assertNotNull(polygon.points)
        Assert.assertEquals(LatLng(0.0, 0.0), polygon.points[0])
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
