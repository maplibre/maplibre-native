package org.maplibre.android.geometry

import org.maplibre.android.utils.MockParcel
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest

class LatLngSpanTest : BaseTest() {
    @Test
    fun testSanity() {
        val latLngSpan = LatLngSpan(0.0, 0.0)
        Assert.assertNotNull("latLngSpan should not be null", latLngSpan)
    }

    @Test
    fun testEquality() {
        val latLngSpan = LatLngSpan(0.0, 0.0)
        Assert.assertEquals(
            "latLngSpan is not equal to a LatLng",
            latLngSpan.equals(
                LAT_LNG_NULL_ISLAND
            ),
            false
        )
    }

    @Test
    fun testLatitudeConstructor() {
        val latitude = 1.23
        val latLngSpan = LatLngSpan(latitude, 0.0)
        Assert.assertEquals("latitude in constructor", latLngSpan.latitudeSpan, latitude, DELTA)
    }

    @Test
    fun testLongitudeConstructor() {
        val longitude = 1.23
        val latLngSpan = LatLngSpan(0.0, longitude)
        Assert.assertEquals("latitude in constructor", latLngSpan.longitudeSpan, longitude, DELTA)
    }

    @Test
    fun testLatitudeMethod() {
        val latitude = 1.23
        val latLngSpan = LatLngSpan(0.0, 0.0)
        latLngSpan.latitudeSpan = latitude
        Assert.assertEquals("latitude in constructor", latLngSpan.latitudeSpan, latitude, DELTA)
    }

    @Test
    fun testLongitudeMethod() {
        val longitude = 1.23
        val latLngSpan = LatLngSpan(0.0, 0.0)
        latLngSpan.longitudeSpan = longitude
        Assert.assertEquals("latitude in constructor", latLngSpan.longitudeSpan, longitude, DELTA)
    }

    @Test
    fun testParcelable() {
        val `object` = LatLngSpan(1.0, 2.0)
        val parcel = MockParcel.obtain(`object`)
        Assert.assertEquals("parcel should match initial object", `object`, parcel)
    }

    companion object {
        private const val DELTA = 1e-15
        private val LAT_LNG_NULL_ISLAND = LatLng(0.0, 0.0)
    }
}
