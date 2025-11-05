package org.maplibre.android.camera

import android.content.res.TypedArray
import android.os.Parcel
import org.maplibre.android.R
import org.maplibre.android.camera.CameraUpdateFactory.ZoomUpdate
import org.maplibre.android.camera.CameraUpdateFactory.zoomTo
import org.maplibre.android.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.Mockito
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class CameraPositionTest : BaseTest() {
    @Test
    fun testSanity() {
        val latLng = LatLng(1.0, 2.0)
        val cameraPosition =
            CameraPosition(latLng, 3.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        Assert.assertNotNull("cameraPosition should not be null", cameraPosition)
    }

    @Test
    fun testDefaultTypedArrayBuilder() {
        val typedArray: TypedArray? = null
        val cameraPosition = CameraPosition.Builder(typedArray).build()
        Assert.assertEquals("bearing should match", -1.0, cameraPosition.bearing, DELTA)
        Assert.assertNull("latlng should be null", cameraPosition.target)
        Assert.assertEquals("tilt should match", -1.0, cameraPosition.tilt, DELTA)
        Assert.assertEquals("zoom should match", -1.0, cameraPosition.zoom, DELTA)
        Assert.assertNull("padding should be null", cameraPosition.padding)
    }

    @Test
    fun testTypedArrayBuilder() {
        val bearing = 180f
        val zoom = 12f
        val latitude = 10f
        val longitude = 11f
        val tilt = 44f
        val typedArray = Mockito.mock(TypedArray::class.java)
        Mockito.`when`(
            typedArray.getFloat(
                R.styleable.maplibre_MapView_maplibre_cameraBearing,
                0.0f
            )
        ).thenReturn(bearing)
        Mockito.`when`(
            typedArray.getFloat(
                R.styleable.maplibre_MapView_maplibre_cameraTargetLat,
                0.0f
            )
        ).thenReturn(latitude)
        Mockito.`when`(
            typedArray.getFloat(
                R.styleable.maplibre_MapView_maplibre_cameraTargetLng,
                0.0f
            )
        ).thenReturn(longitude)
        Mockito.`when`(typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraZoom, 0.0f))
            .thenReturn(zoom)
        Mockito.`when`(typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraTilt, 0.0f))
            .thenReturn(tilt)
        Mockito.doNothing().`when`(typedArray).recycle()
        val cameraPosition = CameraPosition.Builder(typedArray).build()
        Assert.assertEquals(
            "bearing should match",
            bearing.toDouble(),
            cameraPosition.bearing,
            DELTA
        )
        Assert.assertEquals(
            "latlng should match",
            LatLng(latitude.toDouble(), longitude.toDouble()),
            cameraPosition.target
        )
        Assert.assertEquals("tilt should match", tilt.toDouble(), cameraPosition.tilt, DELTA)
        Assert.assertEquals("zoom should match", zoom.toDouble(), cameraPosition.zoom, DELTA)
    }

    @Test
    fun testToString() {
        val latLng = LatLng(1.0, 2.0)
        val cameraPosition =
            CameraPosition(latLng, 3.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        Assert.assertEquals(
            "toString should match",
            "Target: LatLng [latitude=1.0, longitude=2.0, altitude=0.0], Zoom:3.0, " +
                "Bearing:5.0, Tilt:4.0, Padding:[0.0, 500.0, 0.0, 0.0]",
            cameraPosition.toString()
        )
    }

    @Test
    fun testHashcode() {
        val latLng = LatLng(1.0, 2.0)
        val cameraPosition =
            CameraPosition(latLng, 3.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        Assert.assertEquals("hashCode should match", -420915327, cameraPosition.hashCode().toLong())
    }

    @Test
    fun testZoomUpdateBuilder() {
        val zoomLevel = 5f
        val builder = CameraPosition.Builder(
            zoomTo(zoomLevel.toDouble()) as ZoomUpdate
        )
        Assert.assertEquals("zoom should match", zoomLevel.toDouble(), builder.build().zoom, 0.0)
    }

    @Test
    fun testEquals() {
        val latLng = LatLng(1.0, 2.0)
        val cameraPosition =
            CameraPosition(latLng, 3.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        val cameraPositionBearing =
            CameraPosition(latLng, 3.0, 4.0, 9.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        val cameraPositionTilt =
            CameraPosition(latLng, 3.0, 9.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        val cameraPositionZoom =
            CameraPosition(latLng, 9.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        val cameraPositionTarget =
            CameraPosition(LatLng(), 3.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        val cameraPositionPadding =
            CameraPosition(LatLng(), 3.0, 4.0, 5.0, doubleArrayOf(0.0, 501.0, 0.0, 0.0))
        Assert.assertEquals("cameraPosition should match itself", cameraPosition, cameraPosition)
        Assert.assertNotEquals("cameraPosition should not match null", null, cameraPosition)
        Assert.assertNotEquals("cameraPosition should not match object", Any(), cameraPosition)
        Assert.assertNotEquals(
            "cameraPosition should not match for bearing",
            cameraPositionBearing,
            cameraPosition
        )
        Assert.assertNotEquals(
            "cameraPosition should not match for tilt",
            cameraPositionTilt,
            cameraPosition
        )
        Assert.assertNotEquals(
            "cameraPosition should not match for zoom",
            cameraPositionZoom,
            cameraPosition
        )
        Assert.assertNotEquals(
            "cameraPosition should not match for target",
            cameraPositionTarget,
            cameraPosition
        )
        Assert.assertNotEquals(
            "cameraPosition should not match for padding",
            cameraPositionPadding,
            cameraPosition
        )
    }

    @Test
    fun testParcelable() {
        val cameraPosition1 =
            CameraPosition(LatLng(1.0, 2.0), 3.0, 4.0, 5.0, doubleArrayOf(0.0, 500.0, 0.0, 0.0))
        val parcel = Parcel.obtain()
        cameraPosition1.writeToParcel(parcel, 0)
        parcel.setDataPosition(0)
        val cameraPosition2 = CameraPosition.CREATOR.createFromParcel(parcel)
        Assert.assertEquals("Parcel should match original object", cameraPosition1, cameraPosition2)
    }

    @Test
    fun testParcelableNulls() {
        val cameraPosition1 = CameraPosition(null, 3.0, 4.0, 5.0, null)
        val parcel = Parcel.obtain()
        cameraPosition1.writeToParcel(parcel, 0)
        parcel.setDataPosition(0)
        val cameraPosition2 = CameraPosition.CREATOR.createFromParcel(parcel)
        Assert.assertEquals("Parcel should match original object", cameraPosition1, cameraPosition2)
    }

    companion object {
        private const val DELTA = 1e-15
    }
}
