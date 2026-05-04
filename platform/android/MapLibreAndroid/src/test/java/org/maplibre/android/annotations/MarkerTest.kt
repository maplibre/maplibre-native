package org.maplibre.android.annotations

import android.graphics.Bitmap
import org.maplibre.android.exceptions.InvalidMarkerPositionException
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.utils.MockParcel
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.Mockito

class MarkerTest : BaseTest() {
    @Test
    fun testSanity() {
        val markerOptions = MarkerOptions()
        Assert.assertNotNull("markerOptions should not be null", markerOptions)
    }

    @Test
    fun testMarker() {
        val markerOptions = MarkerOptions().position(LatLng())
        Assert.assertNotNull("marker should not be null", markerOptions.marker)
    }

    @Test(expected = InvalidMarkerPositionException::class)
    fun testInvalidMarker() {
        MarkerOptions().marker
    }

    @Test
    fun testPosition() {
        val markerOptions = MarkerOptions().position(LatLng(10.0, 12.0))
        val marker = markerOptions.marker
        Assert.assertEquals(marker.position, LatLng(10.0, 12.0))
        Assert.assertEquals(markerOptions.getPosition(), LatLng(10.0, 12.0))
    }

    @Test
    fun testTitle() {
        val markerOptions = MarkerOptions().title("MapLibre").position(LatLng())
        val marker = markerOptions.marker
        Assert.assertEquals(marker.title, "MapLibre")
        Assert.assertEquals(markerOptions.getTitle(), "MapLibre")
    }

    @Test
    fun testSnippet() {
        val markerOptions = MarkerOptions().snippet("MapLibre").position(LatLng())
        val marker = markerOptions.marker
        Assert.assertEquals(marker.snippet, "MapLibre")
    }

    @Test
    fun testBuilder() {
        val marker =
            MarkerOptions().title("title").snippet("snippet").position(LatLng(10.0, 12.0)).marker
        Assert.assertEquals(marker.snippet, "snippet")
        Assert.assertEquals(marker.position, LatLng(10.0, 12.0))
    }

    @Test
    fun testIcon() {
        val bitmap = Mockito.mock(Bitmap::class.java)
        val icon = IconFactory.recreate("test", bitmap)
        val markerOptions = MarkerOptions().position(LatLng()).icon(icon)
        val marker = markerOptions.marker
        Assert.assertEquals("Icon should match", icon, marker.icon)
        Assert.assertEquals("Icon should match", icon, markerOptions.getIcon())
    }

    @Test
    fun testHashCode() {
        val marker = MarkerOptions().position(LatLng()).marker
        Assert.assertEquals("hash code should match", marker.hashCode().toLong(), 0)
    }

    @Test
    fun testHashCodeBuilder() {
        val markerOptions = MarkerOptions().position(LatLng(10.0, 12.0))
        Assert.assertEquals("hash code should match", markerOptions.hashCode().toLong(), 579999617)
    }

    @Test
    fun testEquals() {
        val markerOne = MarkerOptions().position(LatLng(0.0, 0.0)).marker
        val markerTwo = MarkerOptions().position(LatLng(0.0, 0.0)).marker
        Assert.assertEquals(markerOne, markerTwo)
    }

    @Test
    fun testEqualityDifferentLocation() {
        val marker = MarkerOptions().position(LatLng(0.0, 0.0))
        val other = MarkerOptions().position(LatLng(1.0, 0.0))
        Assert.assertNotEquals("Should not match", other, marker)
    }

    @Test
    fun testEqualityDifferentSnippet() {
        val marker = MarkerOptions().snippet("s")
        val other = MarkerOptions()
        Assert.assertNotEquals("Should not match", other, marker)
    }

    @Test
    fun testEqualityDifferentIcon() {
        val marker = MarkerOptions().icon(
            Mockito.mock(
                Icon::class.java
            )
        )
        val other = MarkerOptions()
        Assert.assertNotEquals("Should not match", other, marker)
    }

    @Test
    fun testEqualityDifferentTitle() {
        val marker = MarkerOptions().title("t")
        val other = MarkerOptions()
        Assert.assertNotEquals("Should not match", other, marker)
    }

    @Test
    fun testEqualsItself() {
        val markerOptions = MarkerOptions().position(LatLng(0.0, 0.0))
        val marker = markerOptions.marker
        Assert.assertEquals("Marker should match", marker, marker)
        Assert.assertEquals("MarkerOptions should match", markerOptions, markerOptions)
    }

    @Test
    fun testNotEquals() {
        val markerOptions = MarkerOptions().position(LatLng(0.0, 0.0))
        val marker = markerOptions.marker
        Assert.assertNotEquals("MarkerOptions should match", markerOptions, Any())
        Assert.assertNotEquals("Marker should match", marker, Any())
    }

    @Test
    fun testEqualityBuilder() {
        val markerOne = MarkerOptions().position(LatLng(0.0, 0.0))
        val markerTwo = MarkerOptions().position(LatLng(0.0, 0.0))
        Assert.assertEquals(markerOne, markerTwo)
    }

    @Test
    fun testToString() {
        val marker = MarkerOptions().position(LatLng(0.0, 0.0)).marker
        Assert.assertEquals(
            marker.toString(),
            "Marker [position[" + "LatLng [latitude=0.0, longitude=0.0, altitude=0.0]" + "]]"
        )
    }

    @Test
    fun testParcelable() {
        val markerOptions = MarkerOptions().position(LatLng()).title("t").snippet("s")
        val parcelable = MockParcel.obtain(markerOptions)
        Assert.assertEquals("Parcel should match original object", parcelable, markerOptions)
    }
}
