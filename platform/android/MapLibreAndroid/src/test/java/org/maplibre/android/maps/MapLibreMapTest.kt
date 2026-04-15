package org.maplibre.android.maps

import android.content.Context
import android.graphics.PointF
import org.maplibre.android.MapLibreInjector
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ConfigUtils
import io.mockk.*
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.Mock
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class MapLibreMapTest : BaseTest() {

    private lateinit var maplibreMap: MapLibreMap

    private lateinit var nativeMapView: NativeMap

    private lateinit var transform: Transform

    private lateinit var cameraChangeDispatcher: CameraChangeDispatcher

    private lateinit var developerAnimationListener: MapLibreMap.OnDeveloperAnimationListener

    @Mock
    private lateinit var context: Context

    @Mock
    private lateinit var appContext: Context

    @Before
    fun setup() {
        MockitoAnnotations.initMocks(this)
        MapLibreInjector.inject(context, "abcdef", ConfigUtils.getMockedOptions())
        cameraChangeDispatcher = spyk()
        developerAnimationListener = mockk(relaxed = true)
        nativeMapView = mockk(relaxed = true)
        transform = mockk(relaxed = true)
        maplibreMap = MapLibreMap(
            nativeMapView,
            transform,
            mockk(relaxed = true),
            null,
            null,
            cameraChangeDispatcher,
            listOf(developerAnimationListener)
        )
        every { nativeMapView.isDestroyed } returns false
        every { nativeMapView.nativePtr } returns 5
        maplibreMap.injectLocationComponent(spyk())
        maplibreMap.setStyle(Style.getPredefinedStyle("Streets"))
        maplibreMap.onFinishLoadingStyle()
    }

    @Test
    fun testTransitionOptions() {
        val expected = TransitionOptions(100, 200)
        maplibreMap.style?.transition = expected
        verify { nativeMapView.transitionOptions = expected }
    }

    @Test
    fun testMoveCamera() {
        val callback = mockk<MapLibreMap.CancelableCallback>()
        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()
        val update = CameraUpdateFactory.newCameraPosition(expected)
        maplibreMap.moveCamera(update, callback)
        verify { transform.moveCamera(maplibreMap, update, callback) }
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
    }

    @Test
    fun testEaseCamera() {
        val callback = mockk<MapLibreMap.CancelableCallback>()
        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()
        val update = CameraUpdateFactory.newCameraPosition(expected)
        maplibreMap.easeCamera(update, callback)
        verify { transform.easeCamera(maplibreMap, update, MapLibreConstants.ANIMATION_DURATION, true, callback) }
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
    }

    @Test
    fun testAnimateCamera() {
        val callback = mockk<MapLibreMap.CancelableCallback>()
        val target = LatLng(1.0, 2.0)
        val expected = CameraPosition.Builder().target(target).build()
        val update = CameraUpdateFactory.newCameraPosition(expected)
        maplibreMap.animateCamera(update, callback)
        verify { transform.animateCamera(maplibreMap, update, MapLibreConstants.ANIMATION_DURATION, callback) }
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
    }

    @Test
    fun testScrollBy() {
        maplibreMap.scrollBy(100f, 200f)
        verify { nativeMapView.moveBy(100.0, 200.0, 0) }
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
    }

    @Test
    fun testResetNorth() {
        maplibreMap.resetNorth()
        verify { transform.resetNorth() }
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
    }

    @Test
    fun testFocalBearing() {
        maplibreMap.setFocalBearing(35.0, 100f, 200f, 1000)
        verify { transform.setBearing(35.0, 100f, 200f, 1000) }
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
    }

    @Test
    fun testMinZoom() {
        maplibreMap.setMinZoomPreference(10.0)
        verify { transform.minZoom = 10.0 }
    }

    @Test
    fun testMaxZoom() {
        maplibreMap.setMaxZoomPreference(10.0)
        verify { transform.maxZoom = 10.0 }
    }

    @Test
    fun testMinPitch() {
        maplibreMap.setMinPitchPreference(10.0)
        verify { transform.minPitch = 10.0 }
    }

    @Test
    fun testMaxPitch() {
        maplibreMap.setMaxPitchPreference(10.0)
        verify { transform.maxPitch = 10.0 }
    }

    @Test
    fun testFpsListener() {
        val fpsChangedListener = mockk<MapLibreMap.OnFpsChangedListener>()
        maplibreMap.onFpsChangedListener = fpsChangedListener
        assertEquals("Listener should match", fpsChangedListener, maplibreMap.onFpsChangedListener)
    }

    @Test
    fun testTilePrefetch() {
        maplibreMap.prefetchesTiles = true
        verify { nativeMapView.prefetchTiles = true }
    }

    @Test
    fun testGetPrefetchZoomDelta() {
        every { nativeMapView.prefetchZoomDelta } answers { 3 }
        assertEquals(3, maplibreMap.prefetchZoomDelta)
    }

    @Test
    fun testSetPrefetchZoomDelta() {
        maplibreMap.prefetchZoomDelta = 2
        verify { nativeMapView.prefetchZoomDelta = 2 }
    }

    @Test
    fun testCameraForLatLngBounds() {
        val bounds = LatLngBounds.Builder().include(LatLng()).include(LatLng(1.0, 1.0)).build()
        maplibreMap.setLatLngBoundsForCameraTarget(bounds)
        verify { nativeMapView.setLatLngBounds(bounds) }
    }

    @Test(expected = IllegalArgumentException::class)
    fun testAnimateCameraChecksDurationPositive() {
        maplibreMap.animateCamera(CameraUpdateFactory.newLatLng(LatLng(30.0, 30.0)), 0, null)
    }

    @Test(expected = IllegalArgumentException::class)
    fun testEaseCameraChecksDurationPositive() {
        maplibreMap.easeCamera(CameraUpdateFactory.newLatLng(LatLng(30.0, 30.0)), 0, null)
    }

    @Test
    fun testGetNativeMapPtr() {
        assertEquals(5, maplibreMap.nativeMapPtr)
    }

    @Test
    fun testNativeMapIsNotCalledOnStateSave() {
        clearMocks(nativeMapView)
        maplibreMap.onSaveInstanceState(mockk(relaxed = true))
        verify { nativeMapView wasNot Called }
    }

    @Test
    fun testCameraChangeDispatcherCleared() {
        maplibreMap.onDestroy()
        verify { cameraChangeDispatcher.onDestroy() }
    }

    @Test
    fun testStyleClearedOnDestroy() {
        val style = mockk<Style>(relaxed = true)
        val builder = mockk<Style.Builder>(relaxed = true)
        every { builder.build(nativeMapView) } returns style
        maplibreMap.setStyle(builder)

        maplibreMap.onDestroy()
        verify(exactly = 1) { style.clear() }
    }

    @Test
    fun testStyleCallbackNotCalledWhenPreviousFailed() {
        val style = mockk<Style>(relaxed = true)
        val builder = mockk<Style.Builder>(relaxed = true)
        every { builder.build(nativeMapView) } returns style
        val onStyleLoadedListener = mockk<Style.OnStyleLoaded>(relaxed = true)

        maplibreMap.setStyle(builder, onStyleLoadedListener)
        maplibreMap.onFailLoadingStyle()
        maplibreMap.setStyle(builder, onStyleLoadedListener)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { onStyleLoadedListener.onStyleLoaded(style) }
    }

    @Test
    fun testStyleCallbackNotCalledWhenPreviousNotFinished() {
        // regression test for #14337
        val style = mockk<Style>(relaxed = true)
        val builder = mockk<Style.Builder>(relaxed = true)
        every { builder.build(nativeMapView) } returns style
        val onStyleLoadedListener = mockk<Style.OnStyleLoaded>(relaxed = true)

        maplibreMap.setStyle(builder, onStyleLoadedListener)
        maplibreMap.setStyle(builder, onStyleLoadedListener)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { onStyleLoadedListener.onStyleLoaded(style) }
    }

    @Test
    fun testGetZoom() {
        maplibreMap.zoom
        verify { nativeMapView.zoom }
        assertEquals(maplibreMap.zoom, 0.0, 0.0)
    }

    @Test
    fun testSetZoom() {
        val target = PointF(100f, 100f)
        maplibreMap.setZoom(2.0, target, 0)
        verify { developerAnimationListener.onDeveloperAnimationStarted() }
        verify { nativeMapView.setZoom(2.0, target, 0) }
    }
}
