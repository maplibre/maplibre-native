package org.maplibre.android.geometry

import org.maplibre.android.utils.MockParcel
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest

class ProjectedMetersTest : BaseTest() {
    @Test
    fun testSanity() {
        val projectedMeters = ProjectedMeters(0.0, 0.0)
        Assert.assertNotNull("projectedMeters should not be null", projectedMeters)
    }

    @Test
    fun testEquality() {
        val projectedMeters = ProjectedMeters(0.0, 0.0)
        Assert.assertEquals(
            "projectedMeters is not equal to a LatLng",
            projectedMeters.equals(
                LAT_LNG_NULL_ISLAND
            ),
            false
        )
        Assert.assertEquals(
            "projectedMeters is equal to itself",
            projectedMeters.equals(projectedMeters),
            true
        )
    }

    @Test
    fun testNorthing() {
        val projectedMeters = ProjectedMeters(1.0, 0.0)
        Assert.assertEquals("northing should be 1", 1.0, projectedMeters.northing, 0.0)
    }

    @Test
    fun testEasting() {
        val projectedMeters = ProjectedMeters(0.0, 1.0)
        Assert.assertEquals("easting should be 1", 1.0, projectedMeters.easting, 0.0)
    }

    @Test
    fun testConstructor() {
        val projectedMeters1 = ProjectedMeters(1.0, 2.0)
        val projectedMeters2 = ProjectedMeters(projectedMeters1)
        Assert.assertEquals("projectedmeters should match", projectedMeters1, projectedMeters2)
    }

    @Test
    fun testHashcode() {
        val meters = ProjectedMeters(1.0, 2.0)
        Assert.assertEquals("hashcode should match", -1048576, meters.hashCode().toLong())
    }

    @Test
    fun testToString() {
        val meters = ProjectedMeters(1.0, 1.0)
        Assert.assertEquals(
            "toString should match",
            "ProjectedMeters [northing=1.0, easting=1.0]",
            meters.toString()
        )
    }

    @Test
    fun testParcelable() {
        val meters = ProjectedMeters(1.0, 1.0)
        val parcel = MockParcel.obtain(meters)
        Assert.assertEquals("parcel should match initial object", meters, parcel)
    }

    companion object {
        private val LAT_LNG_NULL_ISLAND = LatLng(0.0, 0.0)
    }
}
