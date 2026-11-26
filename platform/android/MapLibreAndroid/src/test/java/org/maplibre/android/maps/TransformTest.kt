package org.maplibre.android.maps

import android.os.Looper.getMainLooper
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import io.mockk.*
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner
import org.robolectric.Shadows.shadowOf
import org.robolectric.shadows.ShadowLooper

@RunWith(RobolectricTestRunner::class)
class TransformTest : BaseTest() {

    private lateinit var mapView: MapView
    private lateinit var nativeMapView: NativeMap
    private lateinit var transform: Transform
    private lateinit var cameraChangeDispatcher: CameraChangeDispatcher
    private lateinit var mainLooper: ShadowLooper

    @Before
    fun setup() {
        cameraChangeDispatcher = spyk()
        mapView = mockk()
        nativeMapView = mockk()
        mainLooper = shadowOf(getMainLooper())
        transform = Transform(mapView, nativeMapView, cameraChangeDispatcher)
        every { nativeMapView.isDestroyed } returns false
        every { nativeMapView.cameraPosition } returns CameraPosition.DEFAULT
        every { nativeMapView.cancelTransitions() } answers {}
        every { nativeMapView.jumpTo(any(), any(), any(), any(), any()) } answers {}
        every { nativeMapView.easeTo(any(), any(), any(), any(), any(), any(), any()) } answers {}
        every { nativeMapView.flyTo(any(), any(), any(), any(), any(), any()) } answers {}
        every { nativeMapView.minZoom = any() } answers {}
        every { nativeMapView.maxZoom = any() } answers {}
        every { nativeMapView.minPitch = any() } answers {}
        every { nativeMapView.maxPitch = any() } answers {}
    }

    @Test
    fun testMoveCamera() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        val callback = mockk<MapLibreMap.CancelableCallback>()
        every { callback.onFinish() } answers {}

        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()
        val update = CameraUpdateFactory.newCameraPosition(expected)

        transform.cameraPosition
        every { nativeMapView.cameraPosition } returns expected
        transform.moveCamera(maplibreMap, update, callback)

        mainLooper.idle()
        verify { cameraChangeDispatcher.onCameraMove() }
        verify { nativeMapView.jumpTo(target, -1.0, -1.0, -1.0, null) }
        verify { callback.onFinish() }
    }

    @Test
    fun testMoveCameraToSamePosition() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        val callback = mockk<MapLibreMap.CancelableCallback>()
        every { callback.onFinish() } answers {}

        val expected = CameraPosition.DEFAULT
        val update = CameraUpdateFactory.newCameraPosition(expected)

        transform.moveCamera(maplibreMap, update, null) // Initialize camera position
        transform.moveCamera(maplibreMap, update, callback)

        verify(exactly = 1, verifyBlock = { nativeMapView.jumpTo(any(), any(), any(), any(), any()) })
        verify { callback.onFinish() }
    }

    @Test
    fun testEaseCamera() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        every { mapView.addOnCameraDidChangeListener(any()) } answers { transform.onCameraDidChange(true) }
        every { mapView.removeOnCameraDidChangeListener(any()) } answers {}

        val callback = mockk<MapLibreMap.CancelableCallback>()
        every { callback.onFinish() } answers {}

        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()
        val update = CameraUpdateFactory.newCameraPosition(expected)

        transform.easeCamera(maplibreMap, update, 100, false, callback)

        mainLooper.idle()
        verify { nativeMapView.easeTo(target, -1.0, -1.0, -1.0, null, 100, false) }
        verify { callback.onFinish() }
    }

    @Test
    fun testEaseCameraToSamePosition() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        val callback = mockk<MapLibreMap.CancelableCallback>()
        every { callback.onFinish() } answers {}

        val expected = CameraPosition.DEFAULT
        val update = CameraUpdateFactory.newCameraPosition(expected)
        transform.moveCamera(maplibreMap, update, null)

        transform.easeCamera(maplibreMap, update, 100, false, callback)

        verify(exactly = 0, verifyBlock = { nativeMapView.easeTo(any(), any(), any(), any(), any(), any(), any()) })
        verify { callback.onFinish() }
    }

    @Test
    fun testAnimateCamera() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        every { mapView.addOnCameraDidChangeListener(any()) } answers { transform.onCameraDidChange(true) }
        every { mapView.removeOnCameraDidChangeListener(any()) } answers {}

        val callback = mockk<MapLibreMap.CancelableCallback>()
        every { callback.onFinish() } answers {}

        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()
        val update = CameraUpdateFactory.newCameraPosition(expected)

        transform.animateCamera(maplibreMap, update, 100, callback)

        mainLooper.idle()
        verify { nativeMapView.flyTo(target, -1.0, -1.0, -1.0, null, 100) }
        verify { callback.onFinish() }
    }

    @Test
    fun testAnimateCameraToSamePosition() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        val callback = mockk<MapLibreMap.CancelableCallback>()
        every { callback.onFinish() } answers {}

        val expected = CameraPosition.DEFAULT
        val update = CameraUpdateFactory.newCameraPosition(expected)
        transform.moveCamera(maplibreMap, update, null)

        transform.animateCamera(maplibreMap, update, 100, callback)

        verify(exactly = 0, verifyBlock = { nativeMapView.flyTo(any(), any(), any(), any(), any(), any()) })
        verify { callback.onFinish() }
    }

    @Test
    fun testMinZoom() {
        transform.minZoom = 10.0
        verify { nativeMapView.minZoom = 10.0 }
    }

    @Test
    fun testMaxZoom() {
        transform.maxZoom = 10.0
        verify { nativeMapView.maxZoom = 10.0 }
    }

    @Test
    fun testMinPitch() {
        transform.minPitch = 10.0
        verify { nativeMapView.minPitch = 10.0 }
    }

    @Test
    fun testMaxPitch() {
        transform.maxPitch = 10.0
        verify { nativeMapView.maxPitch = 10.0 }
    }

    @Test
    fun testCancelNotInvokedFromOnFinish() {
        val slot = slot<MapView.OnCameraDidChangeListener>()
        every { mapView.addOnCameraDidChangeListener(capture(slot)) } answers { slot.captured.onCameraDidChange(true) }
        every { mapView.removeOnCameraDidChangeListener(any()) } answers {}
        // regression test for https://github.com/mapbox/mapbox-gl-native/issues/13735
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }

        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()

        val callback = object : MapLibreMap.CancelableCallback {
            override fun onCancel() {
                throw IllegalStateException("onCancel shouldn't be called from onFinish")
            }

            override fun onFinish() {
                transform.animateCamera(maplibreMap, CameraUpdateFactory.newCameraPosition(expected), 500, null)
            }
        }
        transform.animateCamera(maplibreMap, CameraUpdateFactory.newCameraPosition(expected), 500, callback)
    }

    @Test
    fun testAnchorCenterOnTerrainRefreshesCameraPosition() {
        transform.cameraPosition // cache the sea-level camera
        // Anchoring moves the centre toward the camera and raises the zoom, keeping the view.
        val anchored = CameraPosition.Builder().target(LatLng(1.0, 2.0)).zoom(13.5).tilt(60.0).build()
        every { nativeMapView.anchorCenterOnTerrain() } answers {
            every { nativeMapView.cameraPosition } returns anchored
        }

        transform.anchorCenterOnTerrain()

        verify(exactly = 1) { nativeMapView.anchorCenterOnTerrain() }
        assertEquals(anchored, transform.cameraPosition)
    }

    @Test
    fun testSetCenterAltitudeKeepingViewRefreshesCameraPositionWhenMoved() {
        transform.cameraPosition // cache the sea-level camera
        val raised = CameraPosition.Builder().target(LatLng(1.0, 2.0)).zoom(13.5).tilt(60.0).build()
        every { nativeMapView.setCenterAltitudeKeepingView(1500.0) } answers {
            every { nativeMapView.cameraPosition } returns raised
            true
        }

        assertTrue(transform.setCenterAltitudeKeepingView(1500.0))
        assertEquals(raised, transform.cameraPosition)
    }

    @Test
    fun testSetCenterAltitudeKeepingViewKeepsCameraPositionWhenNothingMoved() {
        transform.cameraPosition
        every { nativeMapView.setCenterAltitudeKeepingView(any()) } returns false

        assertFalse(transform.setCenterAltitudeKeepingView(0.0))
        verify(exactly = 1) { nativeMapView.cameraPosition }
    }

    @Test
    fun testSetFieldOfViewReportsTheMoveAndRefreshesCameraPosition() {
        transform.cameraPosition // cache the default camera
        val wide = CameraPosition.Builder().target(LatLng(1.0, 2.0)).fov(70.0).build()
        every { nativeMapView.setFieldOfView(70.0) } answers {
            every { nativeMapView.cameraPosition } returns wide
        }

        transform.setFieldOfView(70.0)

        verify(exactly = 1) { nativeMapView.setFieldOfView(70.0) }
        verify(exactly = 1) { nativeMapView.cancelTransitions() }
        verify(exactly = 1) { cameraChangeDispatcher.onCameraMoveStarted(any()) }
        // Two idle notifications: one from cancelTransitions(), one from the end of the move.
        verify(exactly = 2) { cameraChangeDispatcher.onCameraIdle() }
        assertEquals(wide, transform.cameraPosition)
    }

    @Test
    fun testSetFieldOfViewToTheSameValueReportsNothing() {
        every { nativeMapView.setFieldOfView(any()) } answers {}

        transform.setFieldOfView(36.87)

        verify(exactly = 1) { nativeMapView.setFieldOfView(36.87) }
        verify(exactly = 0) { nativeMapView.cancelTransitions() }
        verify(exactly = 0) { cameraChangeDispatcher.onCameraMoveStarted(any()) }
        verify(exactly = 0) { cameraChangeDispatcher.onCameraIdle() }
    }

    @Test
    fun testSetFieldOfViewCancelsAPendingCameraCallback() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }
        every { mapView.addOnCameraDidChangeListener(any()) } answers {} // the animation never ends
        val callback = mockk<MapLibreMap.CancelableCallback>(relaxed = true)
        val update = CameraUpdateFactory.newCameraPosition(CameraPosition.Builder().target(LatLng(1.0, 2.0)).build())
        transform.easeCamera(maplibreMap, update, 1000, false, callback)
        val wide = CameraPosition.Builder().fov(70.0).build()
        every { nativeMapView.setFieldOfView(70.0) } answers {
            every { nativeMapView.cameraPosition } returns wide
        }

        transform.setFieldOfView(70.0)

        mainLooper.idle()
        verify(exactly = 1) { callback.onCancel() }
        verify(exactly = 0) { callback.onFinish() }
    }

    @Test
    fun testGetFieldOfViewAsksTheNativeMap() {
        every { nativeMapView.fieldOfView } returns 55.0
        assertEquals(55.0, transform.fieldOfView, 0.0)
    }

    @Test
    fun testSetCenterFromEyeReportsTheMoveAndRefreshesCameraPosition() {
        transform.cameraPosition // cache the default camera
        val moved = CameraPosition.Builder().target(LatLng(38.72, -120.07)).tilt(48.0).build()
        every { nativeMapView.setCenterFromEye(any(), any(), any(), any(), any(), any()) } answers {
            every { nativeMapView.cameraPosition } returns moved
        }

        transform.setCenterFromEye(LatLng(38.72, -120.07), 1000.0, LatLng(38.70, -120.07), 3000.0)

        verify(exactly = 1) { nativeMapView.setCenterFromEye(38.72, -120.07, 1000.0, 38.70, -120.07, 3000.0) }
        verify(exactly = 1) { nativeMapView.cancelTransitions() }
        verify(exactly = 1) { cameraChangeDispatcher.onCameraMoveStarted(any()) }
        // Two idle notifications: one from cancelTransitions(), one from the end of the move.
        verify(exactly = 2) { cameraChangeDispatcher.onCameraIdle() }
        assertEquals(moved, transform.cameraPosition)
    }

    @Test
    fun testSetCenterFromEyeThatLeavesTheCameraReportsNothing() {
        // Core returns early, as when the eye and the center are the same point: the camera stays.
        every { nativeMapView.setCenterFromEye(any(), any(), any(), any(), any(), any()) } answers {}

        transform.setCenterFromEye(LatLng(38.7, -120.07), 1000.0, LatLng(38.7, -120.07), 1000.0)

        verify(exactly = 1) { nativeMapView.setCenterFromEye(any(), any(), any(), any(), any(), any()) }
        verify(exactly = 0) { nativeMapView.cancelTransitions() }
        verify(exactly = 0) { cameraChangeDispatcher.onCameraMoveStarted(any()) }
        verify(exactly = 0) { cameraChangeDispatcher.onCameraMoveCanceled() }
        verify(exactly = 0) { cameraChangeDispatcher.onCameraIdle() }
    }

    @Test
    fun testSetCenterFromEyeCancelsAPendingCameraCallback() {
        val maplibreMap = mockk<MapLibreMap>()
        every { maplibreMap.cameraPosition } answers { CameraPosition.DEFAULT }
        every { mapView.addOnCameraDidChangeListener(any()) } answers {} // the animation never ends
        val callback = mockk<MapLibreMap.CancelableCallback>(relaxed = true)
        val update = CameraUpdateFactory.newCameraPosition(CameraPosition.Builder().target(LatLng(1.0, 2.0)).build())
        transform.easeCamera(maplibreMap, update, 1000, false, callback)
        val moved = CameraPosition.Builder().target(LatLng(38.72, -120.07)).tilt(48.0).build()
        every { nativeMapView.setCenterFromEye(any(), any(), any(), any(), any(), any()) } answers {
            every { nativeMapView.cameraPosition } returns moved
        }

        transform.setCenterFromEye(LatLng(38.72, -120.07), 1000.0, LatLng(38.70, -120.07), 3000.0)

        mainLooper.idle()
        verify(exactly = 1) { callback.onCancel() }
        verify(exactly = 0) { callback.onFinish() }
    }
}
