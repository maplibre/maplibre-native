package org.maplibre.android.geometry

import android.location.Location
import org.maplibre.android.utils.MockParcel
import org.junit.Assert
import org.junit.Rule
import org.junit.Test
import org.junit.rules.ExpectedException
import org.maplibre.android.BaseTest
import org.mockito.Mockito

class LatLngTest : BaseTest() {
    @Test
    fun testSanity() {
        val latLng = LatLng(0.0, 0.0)
        Assert.assertNotNull("latLng  should not be null", latLng)
    }

    @Test
    fun testLatitudeEmptyConstructor() {
        val latLng = LatLng()
        Assert.assertEquals("latitude default value", latLng.latitude, 0.0, DELTA)
    }

    @Test
    fun testLongitudeEmptyConstructor() {
        val latLng = LatLng()
        Assert.assertEquals("longitude default value", latLng.longitude, 0.0, DELTA)
    }

    @Test
    fun testAltitudeEmptyConstructor() {
        val latLng1 = LatLng()
        Assert.assertEquals("altitude default value", latLng1.altitude, 0.0, DELTA)
    }

    @Test
    fun testLatitudeConstructor() {
        val latitude = 1.2
        val latLng = LatLng(latitude, 3.4)
        Assert.assertEquals("latitude should match", latLng.latitude, latitude, DELTA)
    }

    @Test
    fun testLongitudeConstructor() {
        val longitude = 3.4
        val latLng = LatLng(1.2, longitude)
        Assert.assertEquals("longitude should match", latLng.longitude, longitude, DELTA)
    }

    @Test
    fun testAltitudeConstructor() {
        val latLng1 = LatLng(1.2, 3.4)
        Assert.assertEquals("altitude default value", latLng1.altitude, 0.0, DELTA)
        val altitude = 5.6
        val latLng2 = LatLng(1.2, 3.4, altitude)
        Assert.assertEquals("altitude default value", latLng2.altitude, altitude, DELTA)
    }

    @Test
    fun testLatitudeSetter() {
        val latLng = LatLng(1.2, 3.4)
        latLng.latitude = 3.0
        Assert.assertEquals("latitude should match", 3.0, latLng.latitude, DELTA)
    }

    @Test
    fun testLongitudeSetter() {
        val latLng = LatLng(1.2, 3.4)
        latLng.longitude = 3.0
        Assert.assertEquals("longitude should match", 3.0, latLng.longitude, DELTA)
    }

    @Rule @JvmField
    val exception = ExpectedException.none()

    @Test
    fun testConstructorChecksLatitudeNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must not be NaN")
        LatLng(Double.NaN, 0.0)
    }

    @Test
    fun testConstructorChecksLongitudeNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be NaN")
        LatLng(0.0, Double.NaN)
    }

    @Test
    fun testConstructorChecksLatitudeGreaterThan90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        LatLng(95.0, 0.0)
    }

    @Test
    fun testConstructorChecksLatitudeLessThanThanNegative90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        LatLng(-95.0, 0.0)
    }

    @Test
    fun testConstructorChecksLongitudeInfinity() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be infinite")
        LatLng(0.0, Double.POSITIVE_INFINITY)
    }

    @Test
    fun testLatitudeSetterChecksNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must not be NaN")
        LatLng().latitude = Double.NaN
    }

    @Test
    fun testLongitudeSetterChecksNaN() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be NaN")
        LatLng().longitude = Double.NaN
    }

    @Test
    fun testLatitudeSetterChecksGreaterThan90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        LatLng().latitude = 95.0
    }

    @Test
    fun testLatitudeSetterChecksLessThanThanNegative90() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("latitude must be between -90 and 90")
        LatLng().latitude = -95.0
    }

    @Test
    fun testLongitudeSetterChecksInfinity() {
        exception.expect(IllegalArgumentException::class.java)
        exception.expectMessage("longitude must not be infinite")
        LatLng().longitude = Double.NEGATIVE_INFINITY
    }

    @Test
    fun testAltitudeSetter() {
        val latLng = LatLng(1.2, 3.4)
        latLng.altitude = 3.0
        Assert.assertEquals("altitude should match", 3.0, latLng.altitude, DELTA)
    }

    @Test
    fun testLatLngConstructor() {
        val latLng1 = LatLng(1.2, 3.4)
        val latLng2 = LatLng(latLng1)
        Assert.assertEquals("latLng should match", latLng1, latLng2)
    }

    @Test
    fun testDistanceTo() {
        val latLng1 = LatLng(0.0, 0.0)
        val latLng2 = LatLng(1.0, 1.0)
        Assert.assertEquals(
            "distances should match",
            latLng1.distanceTo(latLng2),
            157298.7453847275,
            DELTA
        )
    }

    @Test
    fun testDistanceToSamePoint() {
        val latLng1 = LatLng(40.71199035644531, -74.0081)
        val latLng2 = LatLng(40.71199035644531, -74.0081)
        val distance = latLng1.distanceTo(latLng2)
        Assert.assertEquals("distance should match", 0.0, distance, DELTA)
    }

    // Regression test for #14216
    @Test
    fun testDistanceToClosePointNotNaN() {
        val latLng = LatLng(40.00599, -105.29261)
        val other = LatLng(40.005990000000025, -105.29260999999997)
        val distance = latLng.distanceTo(other)
        Assert.assertNotEquals(distance, Double.NaN)
    }

    @Test
    fun testLocationProvider() {
        val latitude = 1.2
        val longitude = 3.4
        val altitude = 5.6

        // Mock the location class
        val locationMocked = Mockito.mock(
            Location::class.java
        )
        Mockito.`when`(locationMocked.latitude).thenReturn(latitude)
        Mockito.`when`(locationMocked.longitude).thenReturn(longitude)
        Mockito.`when`(locationMocked.altitude).thenReturn(altitude)

        // Test the constructor
        val latLng = LatLng(locationMocked)
        Assert.assertEquals("latitude should match", latLng.latitude, latitude, DELTA)
        Assert.assertEquals("longitude should match", latLng.longitude, longitude, DELTA)
        Assert.assertEquals("altitude should match", latLng.altitude, altitude, DELTA)
    }

    @Test
    fun testHashCode() {
        val latitude = 1.2
        val longitude = 3.4
        val altitude = 5.6
        val latLng = LatLng(latitude, longitude, altitude)
        Assert.assertEquals("hash code should match", latLng.hashCode().toLong(), -151519232)
    }

    @Test
    fun testToString() {
        val latitude = 1.2
        val longitude = 3.4
        val altitude = 5.6
        val latLng = LatLng(latitude, longitude, altitude)
        Assert.assertEquals(
            "string should match",
            latLng.toString(),
            "LatLng [latitude=1.2, longitude=3.4, altitude=5.6]"
        )
    }

    @Test
    fun testEqualsOther() {
        val latitude = 1.2
        val longitude = 3.4
        val altitude = 5.6
        val latLng1 = LatLng(latitude, longitude, altitude)
        val latLng2 = LatLng(latitude, longitude, altitude)
        Assert.assertEquals("LatLng should match", latLng1, latLng2)
    }

    @Test
    fun testEqualsItself() {
        val latLng = LatLng(1.0, 2.0, 3.0)
        Assert.assertEquals("LatLng should match", latLng, latLng)
    }

    @Test
    fun testNotEquals() {
        val latLng = LatLng(1.0, 2.0)
        Assert.assertNotEquals("LatLng should match", latLng, Any())
    }

    @Test
    fun testParcelable() {
        val latLng = LatLng(45.0, -185.0)
        val parcel = MockParcel.obtain(latLng)
        Assert.assertEquals("parcel should match initial object", latLng, parcel)
    }

    @Test
    fun testWrapped() {
        val originalLatLng = LatLng(45.0, -185.0)
        var newLatlng = originalLatLng.wrap()
        Assert.assertNotSame(" new wrapped LatLng is created", originalLatLng, newLatlng)
        Assert.assertEquals("longitude wrapped value", originalLatLng.longitude, -185.0, DELTA)
        Assert.assertEquals("longitude wrapped value", newLatlng.longitude, 175.0, DELTA)
        newLatlng = LatLng(45.0, 180.0).wrap()
        Assert.assertEquals("longitude wrapped max value", newLatlng.longitude, 180.0, DELTA)
        newLatlng = LatLng(45.0, -180.0).wrap()
        Assert.assertEquals("longitude wrapped min value", newLatlng.longitude, -180.0, DELTA)
    }

    @Test
    fun testUnnecessaryWrapped() {
        val latLng = LatLng(45.0, 50.0).wrap()
        Assert.assertEquals("longitude wrapped value", latLng.longitude, 50.0, DELTA)
    }

    companion object {
        private const val DELTA = 1e-15
    }
}
