package org.maplibre.android.geometry

import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.exceptions.InvalidLatLngBoundsException
import org.maplibre.android.geometry.LatLngBounds.Companion.from
import org.maplibre.android.geometry.LatLngBounds.Companion.world
import org.maplibre.android.utils.MockParcel
import org.junit.Assert
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.rules.ExpectedException
import org.maplibre.android.BaseTest

class LatLngBoundsTest : BaseTest() {
    private var latLngBounds: LatLngBounds? = null

    @Before
    fun beforeTest() {
        latLngBounds = LatLngBounds.Builder()
            .include(LAT_LNG_NULL_ISLAND)
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .build()
    }

    @Test
    fun testSanity() {
        val latLngBoundsBuilder = LatLngBounds.Builder()
        latLngBoundsBuilder.include(LAT_LNG_NULL_ISLAND).include(LAT_LNG_NOT_NULL_ISLAND)
        Assert.assertNotNull("latLng  should not be null", latLngBoundsBuilder.build())
    }

    @Test(expected = InvalidLatLngBoundsException::class)
    fun noLatLngs() {
        LatLngBounds.Builder().build()
    }

    @Test(expected = InvalidLatLngBoundsException::class)
    fun oneLatLngs() {
        LatLngBounds.Builder().include(LAT_LNG_NULL_ISLAND).build()
    }

    @Test
    fun latitiudeSpan() {
        Assert.assertEquals(
            "Span should be the same",
            2.0,
            latLngBounds!!.latitudeSpan,
            DELTA
        )
    }

    @Test
    fun longitudeSpan() {
        Assert.assertEquals(
            "Span should be the same",
            2.0,
            latLngBounds!!.longitudeSpan,
            DELTA
        )
    }

    @Test
    fun coordinateSpan() {
        val latLngSpan = latLngBounds!!.span
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(2.0, 2.0),
            latLngSpan
        )
    }

    @Test
    fun dateLineSpanBuilder1() {
        latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, -170.0))
            .include(LatLng(-10.0, 170.0))
            .build()
        val latLngSpan = latLngBounds!!.span
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 340.0),
            latLngSpan
        )
    }

    @Test
    fun dateLineSpanBuilder2() {
        latLngBounds = LatLngBounds.Builder()
            .include(LatLng(-10.0, -170.0))
            .include(LatLng(10.0, 170.0))
            .build()
        val latLngSpan = latLngBounds!!.span
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 340.0),
            latLngSpan
        )
    }

    @Test
    fun dateLineSpanFrom1() {
        latLngBounds = from(10.0, -170.0, -10.0, -190.0)
        val latLngSpan = latLngBounds!!.span
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 20.0),
            latLngSpan
        )
    }

    @Test
    fun dateLineSpanFrom2() {
        latLngBounds = from(10.0, 170.0, -10.0, -170.0)
        val latLngSpan = latLngBounds!!.span
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 340.0),
            latLngSpan
        )
    }

    @Test
    fun zeroLongitudeSpan() {
        latLngBounds = from(10.0, 10.0, -10.0, 10.0)
        val latLngSpan = latLngBounds!!.span
        Assert.assertEquals(
            "LatLngSpan should be shortest distance",
            LatLngSpan(20.0, 0.0),
            latLngSpan
        )
    }

    @Test
    fun nearDateLineCenter1() {
        latLngBounds = from(10.0, -175.0, -10.0, -195.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(0.0, -185.0), center)
    }

    @Test
    fun nearDateLineCenter2() {
        latLngBounds = from(10.0, 195.0, -10.0, 175.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(0.0, 185.0), center)
    }

    @Test
    fun nearDateLineCenter3() {
        latLngBounds = from(10.0, -170.0, -10.0, -190.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(0.0, -180.0), center)
    }

    @Test
    fun nearDateLineCenter4() {
        latLngBounds = from(10.0, 180.0, -10.0, 0.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(0.0, 90.0), center)
    }

    @Test
    fun nearDateLineCenter5() {
        latLngBounds = from(10.0, 180.0, -10.0, 0.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(0.0, 90.0), center)
    }

    @Test
    fun centerForBoundsWithSameLongitude() {
        latLngBounds = from(10.0, 10.0, -10.0, 10.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(0.0, 10.0), center)
    }

    @Test
    fun centerForBoundsWithSameLatitude() {
        latLngBounds = from(10.0, 10.0, 10.0, -10.0)
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(10.0, 0.0), center)
    }

    @Test
    fun center() {
        val center = latLngBounds!!.center
        Assert.assertEquals("Center should match", LatLng(1.0, 1.0), center)
    }

    @Test
    fun notEmptySpan() {
        latLngBounds = LatLngBounds.Builder()
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .include(LAT_LNG_NULL_ISLAND)
            .build()
        Assert.assertFalse("Should not be empty", latLngBounds!!.isEmptySpan)
    }

    @Test
    fun includeSameLatLngs() {
        latLngBounds = LatLngBounds.Builder()
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .include(LAT_LNG_NULL_ISLAND)
            .include(LAT_LNG_NULL_ISLAND)
            .build()
        Assert.assertEquals(latLngBounds!!.northEast, LAT_LNG_NOT_NULL_ISLAND)
        Assert.assertEquals(latLngBounds!!.southWest, LAT_LNG_NULL_ISLAND)
    }

    @Test
    fun toLatLngs() {
        latLngBounds = LatLngBounds.Builder()
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .include(LAT_LNG_NULL_ISLAND)
            .build()
        Assert.assertArrayEquals(
            "LatLngs should match",
            arrayOf(LAT_LNG_NOT_NULL_ISLAND, LAT_LNG_NULL_ISLAND),
            latLngBounds!!.toLatLngs()
        )
    }

    @Test
    fun include() {
        Assert.assertTrue(
            "LatLng should be included",
            latLngBounds!!.contains(LatLng(1.0, 1.0))
        )
    }

    @Test
    fun includes() {
        val points: MutableList<LatLng> = ArrayList()
        points.add(LAT_LNG_NULL_ISLAND)
        points.add(LAT_LNG_NOT_NULL_ISLAND)
        val latLngBounds1 = LatLngBounds.Builder()
            .includes(points)
            .build()
        val latLngBounds2 = LatLngBounds.Builder()
            .include(LAT_LNG_NULL_ISLAND)
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .build()
        Assert.assertEquals("LatLngBounds should match", latLngBounds1, latLngBounds2)
    }

    @Test
    fun includesOrderDoesNotMatter() {
        val sameLongitudeFirst = LatLngBounds.Builder()
            .include(LatLng(50.0, 10.0)) // southWest
            .include(LatLng(60.0, 10.0))
            .include(LatLng(60.0, 20.0)) // northEast
            .include(LatLng(50.0, 20.0))
            .include(LatLng(50.0, 10.0)) // southWest again
            .build()
        val sameLatitudeFirst = LatLngBounds.Builder()
            .include(LatLng(50.0, 20.0))
            .include(LatLng(50.0, 10.0)) // southWest
            .include(LatLng(60.0, 10.0))
            .include(LatLng(60.0, 20.0)) // northEast
            .include(LatLng(50.0, 20.0))
            .build()
        Assert.assertEquals(sameLatitudeFirst, sameLongitudeFirst)
    }

    @Test
    fun includesOverDateline1() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, -170.0))
            .include(LatLng(-10.0, -175.0))
            .include(LatLng(0.0, -190.0))
            .build()
        val latLngSpan = latLngBounds.span
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 20.0),
            latLngSpan
        )
    }

    @Test
    fun includesOverDateline2() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, 170.0))
            .include(LatLng(-10.0, 175.0))
            .include(LatLng(0.0, 190.0))
            .build()
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 20.0),
            latLngBounds.span
        )
    }

    @Test
    fun includesOverDateline3() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, -190.0))
            .include(LatLng(-10.0, -170.0))
            .include(LatLng(0.0, -180.0))
            .include(LatLng(5.0, -180.0))
            .build()
        Assert.assertEquals(
            "LatLngSpan should be the same",
            LatLngSpan(20.0, 20.0),
            latLngBounds.span
        )
    }

    @Test
    fun containsNot() {
        Assert.assertFalse(
            "LatLng should not be included",
            latLngBounds!!.contains(LatLng(3.0, 1.0))
        )
    }

    @Test
    fun containsBoundsInWorld() {
        Assert.assertTrue(
            "LatLngBounds should be contained in the world",
            world().contains(latLngBounds!!)
        )
    }

    @Test
    fun worldSpan() {
        Assert.assertEquals(
            "LatLngBounds world span should be 180, 360",
            GeometryConstants.LATITUDE_SPAN,
            world().latitudeSpan,
            DELTA
        )
        Assert.assertEquals(
            "LatLngBounds world span should be 180, 360",
            GeometryConstants.LONGITUDE_SPAN,
            world().longitudeSpan,
            DELTA
        )
    }

    @Test
    fun emptySpan() {
        val latLngBounds = from(
            GeometryConstants.MIN_LATITUDE,
            GeometryConstants.MAX_LONGITUDE,
            GeometryConstants.MIN_LATITUDE,
            GeometryConstants.MAX_LONGITUDE
        )
        Assert.assertTrue("LatLngBounds empty span", latLngBounds.isEmptySpan)
    }

    @Test
    fun containsBounds() {
        val inner = LatLngBounds.Builder()
            .include(LatLng(-5.0, -5.0))
            .include(LatLng(5.0, 5.0))
            .build()
        val outer = LatLngBounds.Builder()
            .include(LatLng(-10.0, -10.0))
            .include(LatLng(10.0, 10.0))
            .build()
        Assert.assertTrue(outer.contains(inner))
        Assert.assertFalse(inner.contains(outer))
    }

    @Test
    fun testHashCode() {
        Assert.assertEquals(2147483647f, latLngBounds.hashCode().toFloat(), -1946419200f)
    }

    @Test
    fun equality() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LAT_LNG_NULL_ISLAND)
            .include(LAT_LNG_NOT_NULL_ISLAND)
            .build()
        Assert.assertEquals("equality should match", this.latLngBounds, latLngBounds)
        Assert.assertEquals(
            "not equal to a different object type",
            this.latLngBounds!!.equals(LAT_LNG_NOT_NULL_ISLAND),
            false
        )
    }

    @Test
    fun testToString() {
        Assert.assertEquals(latLngBounds.toString(), "N:2.0; E:2.0; S:0.0; W:0.0")
    }

    @Test
    fun intersect() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(1.0, 1.0))
            .include(LAT_LNG_NULL_ISLAND)
            .build()
        Assert.assertEquals(
            "intersect should match",
            latLngBounds,
            latLngBounds.intersect(
                this.latLngBounds!!.getLatNorth(),
                this.latLngBounds!!.getLonEast(),
                this.latLngBounds!!.getLatSouth(),
                this.latLngBounds!!.getLonWest()
            )
        )
    }

    @Test
    fun intersectNot() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, 10.0))
            .include(LatLng(9.0, 8.0))
            .build()
        Assert.assertNull(latLngBounds.intersect(this.latLngBounds!!))
    }

    @Test
    fun intersectNorthCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        val intersectLatLngBounds = from(10.0, 10.0, 0.0, 0.0)
            .intersect(200.0, 200.0, 0.0, 0.0)
    }

    @Test
    fun intersectSouthCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        val intersectLatLngBounds = from(0.0, 0.0, -10.0, -10.0)
            .intersect(0.0, 0.0, -200.0, -200.0)
    }

    @Test
    fun intersectSouthLessThanNorthCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latNorth cannot be less than latSouth")
        val intersectLatLngBounds = from(10.0, 10.0, 0.0, 0.0)
            .intersect(0.0, 200.0, 20.0, 0.0)
    }

    @Test
    fun intersectEastLessThanWestCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("lonEast cannot be less than lonWest")
        val intersectLatLngBounds = from(10.0, -10.0, 0.0, 0.0)
            .intersect(0.0, 200.0, 20.0, 0.0)
    }

    fun intersectEastDoesNotWrapCheck() {
        val latLngBounds1 = from(10.0, 210.0, 0.0, 0.0)
        val latLngBounds2 = from(90.0, 200.0, 0.0, 0.0)
        val intersectLatLngBounds = from(10.0, 200.0, 0.0, 0.0)
        Assert.assertEquals(latLngBounds1.intersect(latLngBounds2), intersectLatLngBounds)
        Assert.assertEquals(latLngBounds2.intersect(latLngBounds1), intersectLatLngBounds)
    }

    @Test
    fun intersectDoesNotWestWrapCheck() {
        val latLngBounds1 = from(0.0, 0.0, -10.0, -210.0)
        val latLngBounds2 = from(0.0, 0.0, -90.0, -200.0)
        val intersectLatLngBounds = from(0.0, 0.0, -10.0, -200.0)
        Assert.assertEquals(latLngBounds1.intersect(latLngBounds2), intersectLatLngBounds)
        Assert.assertEquals(latLngBounds2.intersect(latLngBounds1), intersectLatLngBounds)
    }

    @Test
    fun innerUnion() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(1.0, 1.0))
            .include(LAT_LNG_NULL_ISLAND)
            .build()
        Assert.assertEquals(
            "union should match",
            latLngBounds,
            latLngBounds.intersect(
                this.latLngBounds!!
            )
        )
    }

    @Test
    fun outerUnion() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, 10.0))
            .include(LatLng(9.0, 8.0))
            .build()
        Assert.assertEquals(
            "outer union should match",
            latLngBounds.union(this.latLngBounds!!),
            LatLngBounds.Builder()
                .include(LatLng(10.0, 10.0))
                .include(LAT_LNG_NULL_ISLAND)
                .build()
        )
    }

    @Test
    fun unionOverDateLine() {
        val latLngBounds1 = LatLngBounds.Builder()
            .include(LatLng(10.0, 170.0))
            .include(LatLng(0.0, 160.0))
            .build()
        val latLngBounds2 = LatLngBounds.Builder()
            .include(LatLng(0.0, 190.0))
            .include(LatLng(-10.0, 200.0))
            .build()
        val union1 = latLngBounds1.union(latLngBounds2)
        val union2 = latLngBounds2.union(latLngBounds1)
        Assert.assertEquals(
            union1,
            LatLngBounds.Builder()
                .include(LatLng(10.0, 160.0))
                .include(LatLng(-10.0, 200.0))
                .build()
        )
        Assert.assertEquals(union1, union2)
    }

    @Test
    fun unionOverDateLine2() {
        val latLngBounds1 = LatLngBounds.Builder()
            .include(LatLng(10.0, 170.0))
            .include(LatLng(0.0, 160.0))
            .build()
        val latLngBounds2 = LatLngBounds.Builder()
            .include(LatLng(0.0, 165.0))
            .include(LatLng(-10.0, 200.0))
            .build()
        val union1 = latLngBounds1.union(latLngBounds2)
        val union2 = latLngBounds2.union(latLngBounds1)
        Assert.assertEquals(
            union1,
            LatLngBounds.Builder()
                .include(LatLng(10.0, 160.0))
                .include(LatLng(-10.0, 200.0))
                .build()
        )
        Assert.assertEquals(union1, union2)
    }

    @Test
    fun unionOverDateLine3() {
        val latLngBounds1 = LatLngBounds.Builder()
            .include(LatLng(10.0, 195.0))
            .include(LatLng(0.0, 160.0))
            .build()
        val latLngBounds2 = LatLngBounds.Builder()
            .include(LatLng(0.0, 190.0))
            .include(LatLng(-10.0, 200.0))
            .build()
        val union1 = latLngBounds1.union(latLngBounds2)
        val union2 = latLngBounds2.union(latLngBounds1)
        Assert.assertEquals(
            union1,
            LatLngBounds.Builder()
                .include(LatLng(10.0, 160.0))
                .include(LatLng(-10.0, 200.0))
                .build()
        )
        Assert.assertEquals(union1, union2)
    }

    @Test
    fun unionOverDateLine4() {
        val latLngBounds1 = LatLngBounds.Builder()
            .include(LatLng(10.0, -160.0))
            .include(LatLng(0.0, -200.0))
            .build()
        val latLngBounds2 = LatLngBounds.Builder()
            .include(LatLng(0.0, -170.0))
            .include(LatLng(-10.0, -175.0))
            .build()
        val union1 = latLngBounds1.union(latLngBounds2)
        val union2 = latLngBounds2.union(latLngBounds1)
        Assert.assertEquals(
            union1,
            LatLngBounds.Builder()
                .include(LatLng(10.0, -200.0))
                .include(LatLng(-10.0, -160.0))
                .build()
        )
        Assert.assertEquals(union1, union2)
    }

    @Test
    fun unionOverDateLine5() {
        val latLngBounds1 = LatLngBounds.Builder()
            .include(LatLng(10.0, 200.0))
            .include(LatLng(0.0, 160.0))
            .build()
        val latLngBounds2 = LatLngBounds.Builder()
            .include(LatLng(0.0, 170.0))
            .include(LatLng(-10.0, 175.0))
            .build()
        val union1 = latLngBounds1.union(latLngBounds2)
        val union2 = latLngBounds2.union(latLngBounds1)
        Assert.assertEquals(
            union1,
            LatLngBounds.Builder()
                .include(LatLng(10.0, 160.0))
                .include(LatLng(-10.0, 200.0))
                .build()
        )
        Assert.assertEquals(union1, union2)
    }

    @Test
    fun unionOverDateLineReturnLongerThanWorldLonSpan() {
        val latLngBounds1 = from(10.0, 200.0, -10.0, -10.0)
        val latLngBounds2 = from(10.0, 10.0, -10.0, -200.0)
        val union1 = latLngBounds1.union(latLngBounds2)
        val union2 = latLngBounds2.union(latLngBounds1)
        Assert.assertEquals(union1, union2)
        Assert.assertEquals(union1, from(10.0, 200.0, -10.0, -200.0))
    }

    @Test
    fun unionNorthCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        val unionLatLngBounds = from(10.0, 10.0, 0.0, 0.0)
            .union(200.0, 200.0, 0.0, 0.0)
    }

    @Test
    fun unionSouthCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        val unionLatLngBounds = from(0.0, 0.0, -10.0, -10.0)
            .union(0.0, 0.0, -200.0, -200.0)
    }

    @Test
    fun unionSouthLessThanNorthCheck() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latNorth cannot be less than latSouth")
        val unionLatLngBounds = from(10.0, 10.0, 0.0, 0.0)
            .union(0.0, 200.0, 20.0, 0.0)
    }

    @Test
    fun unionEastDoesNotWrapCheck() {
        val latLngBounds1 = from(10.0, 10.0, 0.0, 0.0)
        val latLngBounds2 = from(90.0, 200.0, 0.0, 0.0)
        val unionLatLngBounds = from(90.0, 200.0, 0.0, 0.0)
        Assert.assertEquals(latLngBounds1.union(latLngBounds2), unionLatLngBounds)
        Assert.assertEquals(latLngBounds2.union(latLngBounds1), unionLatLngBounds)
    }

    @Test
    fun unionWestDoesNotWrapCheck() {
        val latLngBounds1 = from(0.0, 0.0, -10.0, -10.0)
        val latLngBounds2 = from(0.0, 0.0, -90.0, -200.0)
        val unionLatLngBounds = from(0.0, 0.0, -90.0, -200.0)
        Assert.assertEquals(latLngBounds1.union(latLngBounds2), unionLatLngBounds)
        Assert.assertEquals(latLngBounds2.union(latLngBounds1), unionLatLngBounds)
    }

    @Test
    fun northWest() {
        val minLat = 5.0
        val minLon = 6.0
        val maxLat = 20.0
        val maxLon = 21.0
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(minLat, minLon))
            .include(LatLng(maxLat, maxLon))
            .build()
        Assert.assertEquals(
            "NorthWest should match",
            latLngBounds.northWest,
            LatLng(maxLat, minLon)
        )
    }

    @Test
    fun southWest() {
        val minLat = 5.0
        val minLon = 6.0
        val maxLat = 20.0
        val maxLon = 21.0
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(minLat, minLon))
            .include(LatLng(maxLat, maxLon))
            .build()
        Assert.assertEquals(
            "SouthWest should match",
            latLngBounds.southWest,
            LatLng(minLat, minLon)
        )
    }

    @Test
    fun northEast() {
        val minLat = 5.0
        val minLon = 6.0
        val maxLat = 20.0
        val maxLon = 21.0
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(minLat, minLon))
            .include(LatLng(maxLat, maxLon))
            .build()
        Assert.assertEquals(
            "NorthEast should match",
            latLngBounds.northEast,
            LatLng(maxLat, maxLon)
        )
    }

    @Test
    fun southEast() {
        val minLat = 5.0
        val minLon = 6.0
        val maxLat = 20.0
        val maxLon = 21.0
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(minLat, minLon))
            .include(LatLng(maxLat, maxLon))
            .build()
        Assert.assertEquals(
            "SouthEast should match",
            latLngBounds.southEast,
            LatLng(minLat, maxLon)
        )
    }

    @Test
    fun testParcelable() {
        val latLngBounds = LatLngBounds.Builder()
            .include(LatLng(10.0, 10.0))
            .include(LatLng(9.0, 8.0))
            .build()
        val parcel = MockParcel.obtain(latLngBounds)
        Assert.assertEquals("Parcel should match original object", parcel, latLngBounds)
    }

    @Test
    fun fromTileID() {
        var bounds = from(0, 0, 0)
        Assert.assertEquals(
            GeometryConstants.MIN_WRAP_LONGITUDE,
            bounds.getLonWest(),
            DELTA
        )
        Assert.assertEquals(
            GeometryConstants.MIN_MERCATOR_LATITUDE,
            bounds.getLatSouth(),
            DELTA
        )
        Assert.assertEquals(
            GeometryConstants.MAX_WRAP_LONGITUDE,
            bounds.getLonEast(),
            DELTA
        )
        Assert.assertEquals(
            GeometryConstants.MAX_MERCATOR_LATITUDE,
            bounds.getLatNorth(),
            DELTA
        )
        bounds = from(10, 288, 385)
        Assert.assertEquals(-78.75, bounds.getLonWest(), DELTA)
        Assert.assertEquals(40.446947059600497, bounds.getLatSouth(), DELTA)
        Assert.assertEquals(-78.3984375, bounds.getLonEast(), DELTA)
        Assert.assertEquals(40.713955826286039, bounds.getLatNorth(), DELTA)
    }

    @Rule @JvmField
    val exception = ExpectedException.none()

    @Test
    fun testConstructorChecksNorthLatitudeNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must not be NaN")
        from(Double.NaN, 0.0, -20.0, -20.0)
    }

    @Test
    fun testConstructorChecksEastLongitudeNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be NaN")
        from(0.0, Double.NaN, -20.0, -20.0)
    }

    @Test
    fun testConstructorChecksNorthLatitudeGreaterThan90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        from(95.0, 0.0, -20.0, -20.0)
    }

    @Test
    fun testConstructorChecksNorthLatitudeLessThanThanNegative90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        from(-95.0, 0.0, -20.0, -20.0)
    }

    @Test
    fun testConstructorChecksEastLongitudeInfinity() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be infinite")
        from(0.0, Double.POSITIVE_INFINITY, -20.0, -20.0)
    }

    @Test
    fun testConstructorChecksSouthLatitudeNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must not be NaN")
        from(20.0, 20.0, Double.NaN, 0.0)
    }

    @Test
    fun testConstructorChecksWesttLongitudeNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be NaN")
        from(20.0, 20.0, 0.0, Double.NaN)
    }

    @Test
    fun testConstructorChecksSouthLatitudeGreaterThan90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        from(20.0, 20.0, 95.0, 0.0)
    }

    @Test
    fun testConstructorChecksSouthLatitudeLessThanThanNegative90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        from(20.0, 20.0, -95.0, 0.0)
    }

    @Test
    fun testConstructorChecksWestLongitudeInfinity() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be infinite")
        from(20.0, 20.0, 0.0, Double.POSITIVE_INFINITY)
    }

    @Test
    fun testConstructorCheckLatSouthGreaterLatNorth() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latNorth cannot be less than latSouth")
        from(0.0, 20.0, 20.0, 0.0)
    }

    @Test
    fun testConstructorCheckLonWestGreaterLonEast() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("lonEast cannot be less than lonWest")
        from(20.0, 0.0, 0.0, 20.0)
    }

    companion object {
        private const val DELTA = 1e-13
        private val LAT_LNG_NULL_ISLAND = LatLng(0.0, 0.0)
        private val LAT_LNG_NOT_NULL_ISLAND = LatLng(2.0, 2.0)
    }
}
