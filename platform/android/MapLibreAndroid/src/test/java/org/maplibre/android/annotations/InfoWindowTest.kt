package org.maplibre.android.annotations

import android.graphics.PointF
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Projection
import org.junit.Assert
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.InjectMocks
import org.mockito.Mockito

class InfoWindowTest : BaseTest() {
    @InjectMocks
    var mMapView = Mockito.mock(MapView::class.java)

    @InjectMocks
    var mMapLibreMap = Mockito.mock(MapLibreMap::class.java)

    @Test
    fun testSanity() {
        val infoWindow = InfoWindow(mMapView, mMapLibreMap)
        Assert.assertNotNull("infoWindow should exist", infoWindow)
    }

    @Test
    fun testBoundMarker() {
        val markerOptions = MarkerOptions()
        val marker = markerOptions.position(LatLng()).marker
        val infoWindow = InfoWindow(mMapView, mMapLibreMap).setBoundMarker(marker)
        Assert.assertEquals("marker should match", marker, infoWindow.boundMarker)
    }

    @Test
    fun testClose() {
        val infoWindow = InfoWindow(mMapView, mMapLibreMap)
        infoWindow.close()
        Assert.assertEquals("infowindow should not be visible", false, infoWindow.isVisible)
    }

    @Test
    fun testOpen() {
        val latLng = LatLng(0.0, 0.0)
        val projection = Mockito.mock(
            Projection::class.java
        )
        Mockito.`when`(mMapLibreMap.projection).thenReturn(projection)
        Mockito.`when`(projection.toScreenLocation(latLng)).thenReturn(PointF(0f, 0f))
        val infoWindow = InfoWindow(mMapView, mMapLibreMap)
        infoWindow.open(mMapView, MarkerOptions().position(LatLng()).marker, latLng, 0, 0)
        Assert.assertEquals("infowindow should not be visible", true, infoWindow.isVisible)
    }

    @Test
    fun testOpenClose() {
        val latLng = LatLng(0.0, 0.0)
        val projection = Mockito.mock(
            Projection::class.java
        )
        Mockito.`when`(mMapLibreMap.projection).thenReturn(projection)
        Mockito.`when`(projection.toScreenLocation(latLng)).thenReturn(PointF(0f, 0f))
        val infoWindow = InfoWindow(mMapView, mMapLibreMap)
        infoWindow.open(mMapView, MarkerOptions().position(LatLng()).marker, latLng, 0, 0)
        infoWindow.close()
        Assert.assertEquals("infowindow should not be visible", false, infoWindow.isVisible)
    }

    @Test
    fun testUpdate() {
        val latLng = LatLng(0.0, 0.0)
        val projection = Mockito.mock(
            Projection::class.java
        )
        Mockito.`when`(mMapLibreMap.projection).thenReturn(projection)
        Mockito.`when`(projection.toScreenLocation(latLng)).thenReturn(PointF(0f, 0f))
        val infoWindow = InfoWindow(mMapView, mMapLibreMap)
        infoWindow.open(mMapView, MarkerOptions().position(latLng).marker, latLng, 0, 0)
        infoWindow.update()
    }
}
