package org.maplibre.android.camera

import android.graphics.PointF
import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.testapp.activity.BaseTest
import org.maplibre.android.testapp.activity.espresso.DeviceIndependentTestActivity
import org.junit.Assert
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4ClassRunner::class)
class CameraUpdateFactoryTest : BaseTest() {

    override fun getActivityClass(): Class<*> {
        return DeviceIndependentTestActivity::class.java
    }

    @Test
    @UiThreadTest
    fun testLatLngBoundsUntiltedUnrotated() {
        maplibreMap.cameraPosition = CameraPosition.Builder()
            .target(LatLng(60.0, 24.0))
            .bearing(0.0)
            .tilt(0.0)
            .build()

        val bounds: LatLngBounds = LatLngBounds.Builder()
            .include(LatLng(62.0, 26.0))
            .include(LatLng(58.0, 22.0))
            .build()

        maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0))

        val cameraPosition = maplibreMap.cameraPosition
        assertEquals("latitude should match:", 60.0, cameraPosition.target!!.latitude, 0.1)
        assertEquals("longitude should match:", 24.0, cameraPosition.target!!.longitude, 0.1)
        assertEquals("bearing should match:", 0.0, cameraPosition.bearing, 0.1)
        assertEquals("zoom should match", 5.5, cameraPosition.zoom, 0.1)
        assertEquals("tilt should match:", 0.0, cameraPosition.tilt, 0.1)
    }

    @Test
    @UiThreadTest
    fun testLatLngBoundsTilted() {
        maplibreMap.cameraPosition = CameraPosition.Builder()
            .target(LatLng(60.0, 24.0))
            .bearing(0.0)
            .tilt(45.0)
            .build()

        val bounds: LatLngBounds = LatLngBounds.Builder()
            .include(LatLng(62.0, 26.0))
            .include(LatLng(58.0, 22.0))
            .build()

        maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0))

        val cameraPosition = maplibreMap.cameraPosition
        assertEquals("latitude should match:", 60.0, cameraPosition.target!!.latitude, 0.1)
        assertEquals("longitude should match:", 24.0, cameraPosition.target!!.longitude, 0.1)
        assertEquals("bearing should match:", 0.0, cameraPosition.bearing, 0.1)
        assertEquals("zoom should match", 6.0, cameraPosition.zoom, 0.1)
        assertEquals("tilt should match:", 45.0, cameraPosition.tilt, 0.1)
    }

    @Test
    @UiThreadTest
    fun testLatLngBoundsRotated() {
        maplibreMap.cameraPosition = CameraPosition.Builder()
            .target(LatLng(60.0, 24.0))
            .bearing(30.0)
            .tilt(0.0)
            .build()

        val bounds: LatLngBounds = LatLngBounds.Builder()
            .include(LatLng(62.0, 26.0))
            .include(LatLng(58.0, 22.0))
            .build()

        maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0))

        val cameraPosition = maplibreMap.cameraPosition
        assertEquals("latitude should match:", 60.0, cameraPosition.target!!.latitude, 0.1)
        assertEquals("longitude should match:", 24.0, cameraPosition.target!!.longitude, 0.1)
        assertEquals("bearing should match:", 30.0, cameraPosition.bearing, 0.1)
        assertEquals("zoom should match", 5.3, cameraPosition.zoom, 0.1)
        assertEquals("tilt should match:", 0.0, cameraPosition.tilt, 0.1)
    }

    @Test
    @UiThreadTest
    fun testLatLngBoundsTiltedRotated() {
        maplibreMap.cameraPosition = CameraPosition.Builder()
            .target(LatLng(60.0, 24.0))
            .bearing(30.0)
            .tilt(45.0)
            .build()

        val bounds: LatLngBounds = LatLngBounds.Builder()
            .include(LatLng(62.0, 26.0))
            .include(LatLng(58.0, 22.0))
            .build()

        maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0))

        val cameraPosition = maplibreMap.cameraPosition
        assertEquals("latitude should match:", 60.0, cameraPosition.target!!.latitude, 0.1)
        assertEquals("longitude should match:", 24.0, cameraPosition.target!!.longitude, 0.1)
        assertEquals("bearing should match:", 30.0, cameraPosition.bearing, 0.1)
        assertEquals("zoom should match", 5.6, cameraPosition.zoom, 0.1)
        assertEquals("tilt should match:", 45.0, cameraPosition.tilt, 0.1)
    }

    @Test
    @UiThreadTest
    fun testLatLngBoundsWithProvidedTiltAndRotation() {
        maplibreMap.cameraPosition = CameraPosition.Builder()
            .target(LatLng(60.0, 24.0))
            .bearing(0.0)
            .tilt(0.0)
            .build()

        val bounds: LatLngBounds = LatLngBounds.Builder()
            .include(LatLng(62.0, 26.0))
            .include(LatLng(58.0, 22.0))
            .build()

        maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 30.0, 40.0, 0))

        val cameraPosition = maplibreMap.cameraPosition
        assertEquals("latitude should match:", 60.0, cameraPosition.target!!.latitude, 0.1)
        assertEquals("longitude should match:", 24.0, cameraPosition.target!!.longitude, 0.1)
        assertEquals("bearing should match:", 30.0, cameraPosition.bearing, 0.1)
        assertEquals("zoom should match", 5.6, cameraPosition.zoom, 0.1)
        assertEquals("tilt should match:", 40.0, cameraPosition.tilt, 0.1)
    }

    @Test
    @UiThreadTest
    fun withPadding_cameraInvalidated_paddingPersisting() {
        val initialCameraPosition = maplibreMap.cameraPosition
        val initialPoint = maplibreMap.projection.toScreenLocation(initialCameraPosition.target!!)

        val bottomPadding = mapView.height / 4
        val leftPadding = mapView.width / 4
        val padding = doubleArrayOf(leftPadding.toDouble(), 0.0, 0.0, bottomPadding.toDouble())
        maplibreMap.moveCamera(CameraUpdateFactory.paddingTo(leftPadding.toDouble(), 0.0, 0.0, bottomPadding.toDouble()))

        Assert.assertArrayEquals(intArrayOf(leftPadding, 0, 0, bottomPadding), maplibreMap.padding)

        val resultingCameraPosition = maplibreMap.cameraPosition
        assertEquals(initialCameraPosition.target!!, resultingCameraPosition.target!!)
        assertEquals(
            PointF(initialPoint.x + leftPadding / 2, initialPoint.y - bottomPadding / 2),
            maplibreMap.projection.toScreenLocation(resultingCameraPosition.target!!)
        )
        Assert.assertArrayEquals(padding, resultingCameraPosition.padding, 0.0001)
    }

    @Test
    @UiThreadTest
    fun withLatLngPadding_cameraInvalidated_paddingPersisting() {
        val expectedTarget = LatLng(2.0, 2.0)

        val topPadding = mapView.height / 4
        val rightPadding = mapView.width / 4
        val padding = doubleArrayOf(0.0, topPadding.toDouble(), rightPadding.toDouble(), 0.0)
        maplibreMap.moveCamera(CameraUpdateFactory.newLatLngPadding(expectedTarget, 0.0, topPadding.toDouble(), rightPadding.toDouble(), 0.0))

        Assert.assertArrayEquals(intArrayOf(0, topPadding, rightPadding, 0), maplibreMap.padding)

        val resultingCameraPosition = maplibreMap.cameraPosition
        assertEquals(expectedTarget.latitude, resultingCameraPosition.target!!.latitude, 0.1)
        assertEquals(expectedTarget!!.longitude, resultingCameraPosition.target!!.longitude, 0.1)

        val centerLatLng = maplibreMap.projection.fromScreenLocation(PointF((mapView.width / 2).toFloat(), (mapView.height / 2).toFloat()))
        assertTrue(centerLatLng.latitude > resultingCameraPosition.target!!.latitude)
        assertTrue(centerLatLng.longitude > resultingCameraPosition.target!!.longitude)
        Assert.assertArrayEquals(padding, resultingCameraPosition.padding, 0.0001)
    }
}
