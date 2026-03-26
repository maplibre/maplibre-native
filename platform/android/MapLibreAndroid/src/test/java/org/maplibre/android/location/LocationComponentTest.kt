package org.maplibre.android.location

import android.content.Context
import android.location.Location
import android.os.Looper
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.location.LocationComponentConstants.TRANSITION_ANIMATION_DURATION_MS
import org.maplibre.android.location.engine.LocationEngine
import org.maplibre.android.location.engine.LocationEngineRequest
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Projection
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.Transform
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.ArgumentCaptor
import org.mockito.Mock
import org.mockito.Mockito.*
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class LocationComponentTest : BaseTest() {
    private lateinit var locationComponent: LocationComponent

    @Mock
    private lateinit var locationComponentOptions: LocationComponentOptions

    @Mock
    private lateinit var maplibreMap: MapLibreMap

    @Mock
    private lateinit var transform: Transform

    @Mock
    private lateinit var context: Context

    @Mock
    private lateinit var locationEngine: LocationEngine

    @Mock
    private lateinit var locationEngineRequest: LocationEngineRequest

    @Mock
    private lateinit var currentListener: LocationComponent.CurrentLocationEngineCallback

    @Mock
    private lateinit var lastListener: LocationComponent.LastLocationEngineCallback

    @Mock
    private lateinit var compassEngine: CompassEngine

    @Mock
    private lateinit var locationLayerController: LocationLayerController

    @Mock
    private lateinit var locationCameraController: LocationCameraController

    @Mock
    private lateinit var locationAnimatorCoordinator: LocationAnimatorCoordinator

    @Mock
    private lateinit var staleStateManager: StaleStateManager

    @Mock
    private lateinit var style: Style

    private lateinit var developerAnimationListeners: List<MapLibreMap.OnDeveloperAnimationListener>

    private val defaultOptions: LocationComponentActivationOptions
        get() = LocationComponentActivationOptions.builder(context, style).locationEngine(locationEngine).locationEngineRequest(locationEngineRequest).locationComponentOptions(locationComponentOptions).build()

    @Before
    fun before() {
        MockitoAnnotations.initMocks(this)
        developerAnimationListeners = mutableListOf()
        locationComponent = LocationComponent(maplibreMap, transform, developerAnimationListeners, currentListener, lastListener, locationLayerController, locationCameraController, locationAnimatorCoordinator, staleStateManager, compassEngine, false)
        doReturn(style).`when`(maplibreMap).style
        `when`(style.isFullyLoaded).thenReturn(true)
    }

    @Test
    fun activateWithDefaultLocationEngineRequestAndOptionsTestDefaultLocationEngine() {
        val options = LocationComponentActivationOptions.builder(context, style).locationEngine(locationEngine).locationEngineRequest(locationEngineRequest).locationComponentOptions(locationComponentOptions).build()
        locationComponent.activateLocationComponent(options)

        Assert.assertEquals(locationEngineRequest, locationComponent.locationEngineRequest)
        Assert.assertNotNull(locationComponent.locationEngine)
    }

    @Test
    fun activateWithDefaultLocationEngineRequestAndOptionsTestCustomLocationEngine() {
        val options = LocationComponentActivationOptions.builder(context, style).useDefaultLocationEngine(false).locationEngineRequest(locationEngineRequest).locationComponentOptions(locationComponentOptions).build()
        locationComponent.activateLocationComponent(options)

        Assert.assertEquals(locationEngineRequest, locationComponent.locationEngineRequest)
        Assert.assertNull(locationComponent.locationEngine)
    }

    @Test
    fun locationUpdatesWhenEnabledDisableTest() {
        locationComponent.activateLocationComponent(defaultOptions)

        verify(locationEngine, times(0)).removeLocationUpdates(currentListener)
        verify(locationEngine, times(0)).requestLocationUpdates(eq(locationEngineRequest), eq(currentListener), any(Looper::class.java))

        locationComponent.onStart()
        verify(locationEngine, times(0)).removeLocationUpdates(currentListener)
        verify(locationEngine, times(0)).requestLocationUpdates(eq(locationEngineRequest), eq(currentListener), any(Looper::class.java))

        locationComponent.isLocationComponentEnabled = true
        verify(locationEngine).requestLocationUpdates(eq(locationEngineRequest), eq(currentListener), any(Looper::class.java))

        locationComponent.isLocationComponentEnabled = false
        verify(locationEngine).requestLocationUpdates(eq(locationEngineRequest), eq(currentListener), any(Looper::class.java))
        verify(locationEngine).removeLocationUpdates(currentListener)
    }

    @Test
    fun locationUpdatesWhenStartedStoppedTest() {
        locationComponent.activateLocationComponent(defaultOptions)

        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true

        locationComponent.onStop()
        verify(locationEngine).removeLocationUpdates(currentListener)

        locationComponent.onStart()
        verify(locationEngine, times(2)).requestLocationUpdates(eq(locationEngineRequest), eq(currentListener), any(Looper::class.java))
    }

    @Test
    fun locationUpdatesWhenNewRequestTest() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true

        val newRequest = mock(LocationEngineRequest::class.java)
        locationComponent.locationEngineRequest = newRequest
        verify(locationEngine).removeLocationUpdates(currentListener)
        verify(locationEngine).requestLocationUpdates(eq(newRequest), eq(currentListener), any(Looper::class.java))
    }

    @Test
    fun lastLocationUpdateOnStartTest() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true

        verify(locationEngine).getLastLocation(lastListener)
    }

    @Test
    fun transitionCallbackFinishedTest() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        val listener = mock(OnLocationCameraTransitionListener::class.java)

        val callback = ArgumentCaptor.forClass(OnLocationCameraTransitionListener::class.java)
        locationComponent.setCameraMode(CameraMode.TRACKING, listener)
        verify(locationCameraController).setCameraMode(eq(CameraMode.TRACKING), any(), eq(TRANSITION_ANIMATION_DURATION_MS), isNull(), isNull(), isNull(), callback.capture())
        callback.value.onLocationCameraTransitionFinished(CameraMode.TRACKING)

        verify(listener).onLocationCameraTransitionFinished(CameraMode.TRACKING)
        verify(locationAnimatorCoordinator).resetAllCameraAnimations(CameraPosition.DEFAULT, false)
    }

    @Test
    fun transitionCallbackCanceledTest() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        val listener = mock(OnLocationCameraTransitionListener::class.java)

        val callback = ArgumentCaptor.forClass(OnLocationCameraTransitionListener::class.java)
        locationComponent.setCameraMode(CameraMode.TRACKING, listener)
        verify(locationCameraController).setCameraMode(eq(CameraMode.TRACKING), any(), eq(TRANSITION_ANIMATION_DURATION_MS), isNull(), isNull(), isNull(), callback.capture())
        callback.value.onLocationCameraTransitionCanceled(CameraMode.TRACKING)

        verify(listener).onLocationCameraTransitionCanceled(CameraMode.TRACKING)
        verify(locationAnimatorCoordinator).resetAllCameraAnimations(CameraPosition.DEFAULT, false)
    }

    @Test
    fun transitionCustomFinishedTest() {
        locationComponent.activateLocationComponent(defaultOptions)

        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        val listener = mock(OnLocationCameraTransitionListener::class.java)

        val callback = ArgumentCaptor.forClass(OnLocationCameraTransitionListener::class.java)
        locationComponent.setCameraMode(CameraMode.TRACKING, 1200, 14.0, 13.0, 45.0, listener)
        verify(locationCameraController).setCameraMode(eq(CameraMode.TRACKING), any(), eq(1200L), eq(14.0), eq(13.0), eq(45.0), callback.capture())
        callback.value.onLocationCameraTransitionFinished(CameraMode.TRACKING)

        verify(listener).onLocationCameraTransitionFinished(CameraMode.TRACKING)
        verify(locationAnimatorCoordinator).resetAllCameraAnimations(CameraPosition.DEFAULT, false)
    }

    @Test
    fun compass_listenWhenConsumedByNoneCamera() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationCameraController.isConsumingCompass).thenReturn(true)
        locationComponent.cameraMode = CameraMode.NONE_COMPASS
        verify(compassEngine).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_listenWhenConsumedByTrackingCamera() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationCameraController.isConsumingCompass).thenReturn(true)
        locationComponent.cameraMode = CameraMode.TRACKING_COMPASS
        verify(compassEngine).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_listenWhenConsumedByLayer() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        verify(compassEngine).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_notListenWhenNotConsumed() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(false)
        `when`(locationCameraController.isConsumingCompass).thenReturn(false)
        locationComponent.renderMode = RenderMode.GPS
        locationComponent.renderMode = RenderMode.NORMAL
        locationComponent.cameraMode = CameraMode.TRACKING
        locationComponent.cameraMode = CameraMode.NONE
        locationComponent.cameraMode = CameraMode.NONE_GPS
        locationComponent.cameraMode = CameraMode.TRACKING_GPS
        locationComponent.cameraMode = CameraMode.TRACKING_GPS_NORTH
        verify(compassEngine, never()).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_removeListenerOnChange() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        `when`(locationLayerController.isConsumingCompass).thenReturn(false)
        locationComponent.renderMode = RenderMode.NORMAL
        verify(compassEngine).removeCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_removeListenerOnStop() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        locationComponent.onStop()
        verify(compassEngine).removeCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_reAddListenerOnStart() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        locationComponent.onStop()
        locationComponent.onStart()
        verify(compassEngine, times(2)).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_removeListenerOnStyleStartLoad() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        locationComponent.onStartLoadingMap()
        verify(compassEngine).removeCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_reAddListenerOnStyleLoadFinished() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        locationComponent.onStartLoadingMap()
        locationComponent.onFinishLoadingStyle()
        verify(compassEngine, times(2)).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_reAddListenerOnlyWhenEnabled() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        locationComponent.isLocationComponentEnabled = false

        locationComponent.onStartLoadingMap()
        locationComponent.onFinishLoadingStyle()
        verify(compassEngine).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_notAdListenerWhenDisabled() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        verify(compassEngine, never()).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_notAdListenerWhenStopped() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS
        verify(compassEngine, never()).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun compass_notAddListenerWhenLayerNotReady() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.onStart()
        locationComponent.isLocationComponentEnabled = true
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationLayerController.isConsumingCompass).thenReturn(true)
        locationComponent.renderMode = RenderMode.COMPASS

        verify(compassEngine, times(1)).addCompassListener(any(CompassListener::class.java))

        locationComponent.onStartLoadingMap()
        // Layer should be disabled at this point
        locationComponent.setCameraMode(CameraMode.TRACKING_COMPASS)
        verify(compassEngine, times(1)).addCompassListener(any(CompassListener::class.java))
    }

    @Test
    fun developerAnimationCalled() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        for (listener in developerAnimationListeners) {
            listener.onDeveloperAnimationStarted()
        }
        verify(locationCameraController).setCameraMode(eq(CameraMode.NONE), isNull<Location>(), eq(TRANSITION_ANIMATION_DURATION_MS), isNull<Double>(), isNull<Double>(), isNull<Double>(), any())
    }

    @Test
    fun internal_cameraTrackingChangedListener_onCameraTrackingDismissed() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true

        val cameraChangeListener: OnCameraTrackingChangedListener = mock(OnCameraTrackingChangedListener::class.java)
        locationComponent.addOnCameraTrackingChangedListener(cameraChangeListener)

        locationComponent.cameraTrackingChangedListener.onCameraTrackingDismissed()

        verify(cameraChangeListener).onCameraTrackingDismissed()
    }

    @Test
    fun internal_cameraTrackingChangedListener_onCameraTrackingChanged() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true

        val cameraValueListener: AnimatorListenerHolder = mock(AnimatorListenerHolder::class.java)
        val layerValueListener: AnimatorListenerHolder = mock(AnimatorListenerHolder::class.java)
        `when`(locationCameraController.animationListeners).thenReturn(setOf(cameraValueListener))
        `when`(locationLayerController.animationListeners).thenReturn(setOf(layerValueListener))
        val cameraChangeListener: OnCameraTrackingChangedListener = mock(OnCameraTrackingChangedListener::class.java)
        locationComponent.addOnCameraTrackingChangedListener(cameraChangeListener)

        locationComponent.cameraTrackingChangedListener.onCameraTrackingChanged(CameraMode.TRACKING_GPS)

        verify(locationAnimatorCoordinator).cancelZoomAnimation()
        verify(locationAnimatorCoordinator).cancelTiltAnimation()
        verify(locationAnimatorCoordinator).updateAnimatorListenerHolders(eq(setOf(cameraValueListener, layerValueListener)))
        verify(locationAnimatorCoordinator).resetAllCameraAnimations(any(), anyBoolean())
        verify(locationAnimatorCoordinator).resetAllLayerAnimations()
        verify(cameraChangeListener).onCameraTrackingChanged(CameraMode.TRACKING_GPS)
    }

    @Test
    fun internal_renderModeChangedListener_onRenderModeChanged() {
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true

        val cameraListener: AnimatorListenerHolder = mock(AnimatorListenerHolder::class.java)
        val layerListener: AnimatorListenerHolder = mock(AnimatorListenerHolder::class.java)
        `when`(locationCameraController.animationListeners).thenReturn(setOf(cameraListener))
        `when`(locationLayerController.animationListeners).thenReturn(setOf(layerListener))
        val renderChangeListener: OnRenderModeChangedListener = mock(OnRenderModeChangedListener::class.java)
        locationComponent.addOnRenderModeChangedListener(renderChangeListener)

        locationComponent.renderModeChangedListener.onRenderModeChanged(RenderMode.NORMAL)

        verify(locationAnimatorCoordinator).updateAnimatorListenerHolders(eq(setOf(cameraListener, layerListener)))
        verify(locationAnimatorCoordinator).resetAllCameraAnimations(any(), anyBoolean())
        verify(locationAnimatorCoordinator).resetAllLayerAnimations()
        verify(renderChangeListener).onRenderModeChanged(RenderMode.NORMAL)
    }

    @Test
    fun change_to_gps_mode_symbolLayerBearingValue() {
        val location = Location("test")
        location.bearing = 50f
        val projection: Projection = mock(Projection::class.java)
        `when`(projection.getMetersPerPixelAtLatitude(location.latitude)).thenReturn(10.0)
        `when`(maplibreMap.projection).thenReturn(projection)
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)

        locationComponent.activateLocationComponent(
            LocationComponentActivationOptions.builder(context, style)
                .locationComponentOptions(locationComponentOptions)
                .useDefaultLocationEngine(false)
                .build()
        )
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()
        locationComponent.renderMode = RenderMode.NORMAL
        locationComponent.forceLocationUpdate(location)

        verify(locationLayerController, times(0)).setGpsBearing(50f)

        locationComponent.renderMode = RenderMode.GPS
        verify(locationLayerController, times(1)).setGpsBearing(50f)
        verify(locationAnimatorCoordinator).cancelAndRemoveGpsBearingAnimation()
    }

    @Test
    fun tiltWhileTracking_notReady() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.tiltWhileTracking(30.0, 500L, callback)
        verify(callback).onCancel()
        verify(locationAnimatorCoordinator, times(0)).feedNewTilt(anyDouble(), any(), anyLong(), any())
    }

    @Test
    fun tiltWhileTracking_notTracking() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationCameraController.cameraMode).thenReturn(CameraMode.NONE)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.tiltWhileTracking(30.0, 500L, callback)
        verify(callback).onCancel()
        verify(locationAnimatorCoordinator, times(0)).feedNewTilt(anyDouble(), any(), anyLong(), any())
    }

    @Test
    fun tiltWhileTracking_transitioning() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationCameraController.cameraMode).thenReturn(CameraMode.TRACKING)
        `when`(locationCameraController.isTransitioning).thenReturn(true)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.tiltWhileTracking(30.0, 500L, callback)
        verify(callback).onCancel()
        verify(locationAnimatorCoordinator, times(0)).feedNewTilt(anyDouble(), any(), anyLong(), any())
    }

    @Test
    fun tiltWhileTracking_sucessful() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationCameraController.cameraMode).thenReturn(CameraMode.TRACKING)
        `when`(locationCameraController.isTransitioning).thenReturn(false)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.tiltWhileTracking(30.0, 500L, callback)
        verify(callback, times(0)).onCancel()
        verify(locationAnimatorCoordinator).feedNewTilt(30.0, CameraPosition.DEFAULT, 500L, callback)
    }

    @Test
    fun zoomWhileTracking_notReady() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.zoomWhileTracking(14.0, 500L, callback)
        verify(callback).onCancel()
        verify(locationAnimatorCoordinator, times(0)).feedNewZoomLevel(anyDouble(), any(), anyLong(), any())
    }

    @Test
    fun zoomWhileTracking_notTracking() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationCameraController.cameraMode).thenReturn(CameraMode.NONE)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.zoomWhileTracking(14.0, 500L, callback)
        verify(callback).onCancel()
        verify(locationAnimatorCoordinator, times(0)).feedNewZoomLevel(anyDouble(), any(), anyLong(), any())
    }

    @Test
    fun zoomWhileTracking_transitioning() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationCameraController.cameraMode).thenReturn(CameraMode.TRACKING)
        `when`(locationCameraController.isTransitioning).thenReturn(true)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.zoomWhileTracking(14.0, 500L, callback)
        verify(callback).onCancel()
        verify(locationAnimatorCoordinator, times(0)).feedNewZoomLevel(anyDouble(), any(), anyLong(), any())
    }

    @Test
    fun zoomWhileTracking_successful() {
        `when`(maplibreMap.cameraPosition).thenReturn(CameraPosition.DEFAULT)
        `when`(locationCameraController.cameraMode).thenReturn(CameraMode.TRACKING)
        `when`(locationCameraController.isTransitioning).thenReturn(false)
        locationComponent.activateLocationComponent(defaultOptions)
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()

        val callback = mock(MapLibreMap.CancelableCallback::class.java)

        locationComponent.zoomWhileTracking(14.0, 500L, callback)
        verify(callback, times(0)).onCancel()
        verify(locationAnimatorCoordinator).feedNewZoomLevel(14.0, CameraPosition.DEFAULT, 500L, callback)
    }

    @Test
    fun newLocation_accuracy_symbolLayerRadiusValue() {
        val location = Location("test")
        location.accuracy = 50f
        val projection: Projection = mock(Projection::class.java)
        `when`(projection.getMetersPerPixelAtLatitude(location.latitude)).thenReturn(10.0)
        `when`(maplibreMap.projection).thenReturn(projection)
        locationComponent.activateLocationComponent(
            LocationComponentActivationOptions.builder(context, style)
                .locationComponentOptions(locationComponentOptions)
                .useDefaultLocationEngine(false)
                .build()
        )
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()
        locationComponent.forceLocationUpdate(location)

        val radius = (location.accuracy * (1 / 10.0)).toFloat()
        verify(locationAnimatorCoordinator).feedNewAccuracyRadius(radius, false)
    }

    @Test
    fun newLocation_accuracy_indicatorLayerRadiusValue() {
        val location = Location("test")
        location.accuracy = 50f
        locationComponent = LocationComponent(maplibreMap, transform, developerAnimationListeners, currentListener, lastListener, locationLayerController, locationCameraController, locationAnimatorCoordinator, staleStateManager, compassEngine, true)
        locationComponent.activateLocationComponent(
            LocationComponentActivationOptions.builder(context, style)
                .locationComponentOptions(locationComponentOptions)
                .useSpecializedLocationLayer(true)
                .useDefaultLocationEngine(false)
                .build()
        )
        locationComponent.isLocationComponentEnabled = true
        locationComponent.onStart()
        locationComponent.forceLocationUpdate(location)

        verify(locationAnimatorCoordinator).feedNewAccuracyRadius(location.accuracy, false)
    }
}
