package org.maplibre.android.geometry

import org.maplibre.android.utils.MockParcel
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest

class VisibleRegionTest : BaseTest() {
    @Test
    fun testSanity() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertNotNull("region should not be null", region)
    }

    @Test
    fun testEquality() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals(
            "visibleRegion is not equal to a LatLng",
            region.equals(FAR_LEFT),
            false
        )
        Assert.assertEquals("visibleRegion is equal to itself", region.equals(region), true)
    }

    @Test
    fun testFarLeftConstructor() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("LatLng should match", region.farLeft, FAR_LEFT)
    }

    @Test
    fun testNearLeftConstructor() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("LatLng should match", region.nearLeft, NEAR_LEFT)
    }

    @Test
    fun testFarRightConstructor() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("LatLng should match", region.farRight, FAR_RIGHT)
    }

    @Test
    fun testNearRightConstructor() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("LatLng should match", region.nearRight, NEAR_RIGHT)
    }

    @Test
    fun testLatLngBoundsConstructor() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("LatLngBounds should match", region.latLngBounds, BOUNDS)
    }

    @Test
    fun testEquals() {
        val regionLeft = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        val regionRight = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("VisibleRegions should match", regionLeft, regionRight)
    }

    @Test
    fun testHashcode() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals("hashcode should match", -923534102, region.hashCode().toLong())
    }

    @Test
    fun testToString() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        Assert.assertEquals(
            "string should match",
            "[farLeft [LatLng [latitude=52.0, longitude=-12.0, altitude=0.0]], " +
                "farRight [LatLng [latitude=52.0, longitude=26.0, altitude=0.0]], " +
                "nearLeft [LatLng [latitude=34.0, longitude=-12.0, altitude=0.0]], " +
                "nearRight [LatLng [latitude=34.0, longitude=26.0, altitude=0.0]], " +
                "latLngBounds [N:52.0; E:26.0; S:34.0; W:-12.0]]",
            region.toString()
        )
    }

    @Test
    fun testParcelable() {
        val region = VisibleRegion(FAR_LEFT, FAR_RIGHT, NEAR_LEFT, NEAR_RIGHT, BOUNDS)
        val parcel = MockParcel.obtain(region)
        Assert.assertEquals("parcel should match initial object", region, parcel)
    }

    companion object {
        private val FAR_LEFT = LatLng(52.0, -12.0)
        private val NEAR_LEFT = LatLng(34.0, -12.0)
        private val FAR_RIGHT = LatLng(52.0, 26.0)
        private val NEAR_RIGHT = LatLng(34.0, 26.0)
        private val BOUNDS = LatLngBounds.Builder().include(FAR_LEFT).include(FAR_RIGHT).include(
            NEAR_LEFT
        ).include(NEAR_RIGHT).build()
    }
}
