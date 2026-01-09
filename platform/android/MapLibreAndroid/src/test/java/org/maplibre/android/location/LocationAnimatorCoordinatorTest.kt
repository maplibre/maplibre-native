package org.maplibre.android.location

import android.animation.Animator
import android.animation.ValueAnimator
import android.location.Location
import android.util.SparseArray
import android.view.animation.LinearInterpolator
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_TILT_ANIM_DURATION
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_ZOOM_ANIM_DURATION
import org.maplibre.android.location.MapLibreAnimator.*
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Projection
import io.mockk.*
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_PADDING_ANIM_DURATION
import org.mockito.Mockito
import org.robolectric.RobolectricTestRunner
import org.maplibre.testUtils.Assert as MapLibreAssert

@RunWith(RobolectricTestRunner::class)
class LocationAnimatorCoordinatorTest : BaseTest() {

    private lateinit var locationAnimatorCoordinator: LocationAnimatorCoordinator
    private val cameraPosition: CameraPosition = CameraPosition.DEFAULT

    private val animatorProvider: MapLibreAnimatorProvider = mockk()
    private val animatorSetProvider: MapLibreAnimatorSetProvider = mockk()

    private val projection: Projection = mockk()

    @Before
    fun setUp() {
        locationAnimatorCoordinator = LocationAnimatorCoordinator(
            projection,
            animatorSetProvider,
            animatorProvider
        )
        configureAnimatorProvider()
        every { projection.getMetersPerPixelAtLatitude(any()) } answers { 1.0 }
        val startedAnimatorsSlot = slot<List<Animator>>()
        every { animatorSetProvider.startAnimation(capture(startedAnimatorsSlot), any(), any()) } answers {
            startedAnimatorsSlot.captured.forEach {
                it.start()
            }
        }
        locationAnimatorCoordinator.updateAnimatorListenerHolders(
            getListenerHoldersSet(
                ANIMATOR_LAYER_LATLNG,
                ANIMATOR_CAMERA_LATLNG,
                ANIMATOR_LAYER_GPS_BEARING,
                ANIMATOR_LAYER_COMPASS_BEARING,
                ANIMATOR_CAMERA_GPS_BEARING,
                ANIMATOR_CAMERA_COMPASS_BEARING,
                ANIMATOR_LAYER_ACCURACY,
                ANIMATOR_ZOOM,
                ANIMATOR_TILT,
                ANIMATOR_PADDING
            )
        )
    }

    private fun configureAnimatorProvider() {
        // workaround https://github.com/mockk/mockk/issues/229#issuecomment-457816131
        registerInstanceFactory { AnimationsValueChangeListener<Float> {} }
        registerInstanceFactory { AnimationsValueChangeListener<LatLng> {} }
        val floatsSlot = slot<Array<Float>>()
        val listenerSlot = slot<AnimationsValueChangeListener<*>>()
        val maxFpsSlot = slot<Int>()
        every {
            animatorProvider.floatAnimator(capture(floatsSlot), capture(listenerSlot), capture(maxFpsSlot))
        } answers {
            MapLibreFloatAnimator(
                floatsSlot.captured,
                listenerSlot.captured,
                maxFpsSlot.captured
            )
        }

        val latLngsSlot = slot<Array<LatLng>>()
        every {
            animatorProvider.latLngAnimator(capture(latLngsSlot), capture(listenerSlot), capture(maxFpsSlot))
        } answers {
            MapLibreLatLngAnimator(
                latLngsSlot.captured,
                listenerSlot.captured,
                maxFpsSlot.captured
            )
        }

        val callback = slot<MapLibreMap.CancelableCallback>()
        every {
            animatorProvider.cameraAnimator(capture(floatsSlot), capture(listenerSlot), capture(callback))
        } answers {
            MapLibreCameraAnimatorAdapter(
                floatsSlot.captured,
                listenerSlot.captured,
                callback.captured
            )
        }
        every {
            animatorProvider.cameraAnimator(capture(floatsSlot), capture(listenerSlot), null)
        } answers {
            MapLibreCameraAnimatorAdapter(
                floatsSlot.captured,
                listenerSlot.captured,
                null
            )
        }

        val doubleArraySlot = slot<Array<DoubleArray>>()
        val doubleArrayListenerSlot = slot<AnimationsValueChangeListener<DoubleArray>>()
        every {
            animatorProvider.paddingAnimator(capture(doubleArraySlot), capture(doubleArrayListenerSlot), capture(callback))
        } answers {
            MapLibrePaddingAnimator(doubleArraySlot.captured, doubleArrayListenerSlot.captured, callback.captured)
        }
        every {
            animatorProvider.paddingAnimator(capture(doubleArraySlot), capture(doubleArrayListenerSlot), null)
        } answers {
            MapLibrePaddingAnimator(doubleArraySlot.captured, doubleArrayListenerSlot.captured, null)
        }
    }

    @Test
    fun feedNewLocation_animatorsAreCreated() {
        locationAnimatorCoordinator.feedNewLocation(Location(""), cameraPosition, false)

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING] != null)
    }

    @Test
    fun feedNewLocation_animatorValue() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 35f
        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val cameraLatLngTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG]?.target as LatLng
        MapLibreAssert.assertEquals(location.latitude, cameraLatLngTarget.latitude)
        MapLibreAssert.assertEquals(location.longitude, cameraLatLngTarget.longitude)

        val layerLatLngTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG]?.target as LatLng
        MapLibreAssert.assertEquals(location.latitude, layerLatLngTarget.latitude)
        MapLibreAssert.assertEquals(location.longitude, layerLatLngTarget.longitude)

        val cameraBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(location.bearing, cameraBearingTarget)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(location.bearing, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_animatorValue_multiplePoints() {
        val previousLocation = Location("")
        previousLocation.latitude = 0.0
        previousLocation.longitude = 0.0
        previousLocation.bearing = 0f

        val locationInter = Location("")
        locationInter.latitude = 51.1
        locationInter.longitude = 17.1
        locationInter.bearing = 35f
        val location = Location("")
        location.latitude = 51.2
        location.longitude = 17.2
        location.bearing = 36f
        locationAnimatorCoordinator.feedNewLocation(arrayOf(locationInter, location), cameraPosition, false, false)

        val cameraLatLngTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG]?.target as LatLng
        MapLibreAssert.assertEquals(location.latitude, cameraLatLngTarget.latitude)
        MapLibreAssert.assertEquals(location.longitude, cameraLatLngTarget.longitude)

        val layerLatLngTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG]?.target as LatLng
        MapLibreAssert.assertEquals(location.latitude, layerLatLngTarget.latitude)
        MapLibreAssert.assertEquals(location.longitude, layerLatLngTarget.longitude)

        val cameraBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(location.bearing, cameraBearingTarget)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(location.bearing, layerBearingTarget)

        verify {
            animatorProvider.latLngAnimator(
                arrayOf(
                    LatLng(previousLocation.latitude, previousLocation.longitude),
                    LatLng(locationInter.latitude, locationInter.longitude),
                    LatLng(location.latitude, location.longitude)
                ),
                any(),
                any()
            )
        }
        verify {
            animatorProvider.floatAnimator(
                arrayOf(previousLocation.bearing, locationInter.bearing, location.bearing),
                any(),
                any()
            )
        }
    }

    @Test
    fun feedNewLocation_animatorValue_bearing() {
        val previous = Location("")
        previous.latitude = 51.1
        previous.longitude = 17.1
        previous.bearing = 355f

        val current = Location("")
        current.latitude = 51.2
        current.longitude = 17.2
        current.bearing = 0f

        locationAnimatorCoordinator.feedNewLocation(arrayOf(previous, current), cameraPosition, false, false)

        verify {
            animatorProvider.floatAnimator(
                arrayOf(0f, -5f, 0f),
                any(),
                any()
            )
        }

        locationAnimatorCoordinator.feedNewLocation(arrayOf(previous, current), cameraPosition, true, false)

        verify {
            animatorProvider.floatAnimator(
                arrayOf(0f, 0f),
                any(),
                any()
            )
        }
    }

    @Test
    fun feedNewLocation_animatorValue_multiplePoints_animationDuration() {
        every { projection.getMetersPerPixelAtLatitude(any()) } answers { 10000.0 } // disable snap
        val locationInter = Location("")
        locationInter.latitude = 51.1
        locationInter.longitude = 17.1
        locationInter.bearing = 35f
        val location = Location("")
        location.latitude = 51.2
        location.longitude = 17.2
        location.bearing = 36f
        locationAnimatorCoordinator.feedNewLocation(arrayOf(locationInter, location), cameraPosition, false, false)

        verify {
            animatorSetProvider.startAnimation(
                eq(
                    listOf(
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG],
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING],
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG],
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING]
                    )
                ),
                any<LinearInterpolator>(),
                0
            )
        }
    }

    @Test
    fun feedNewLocation_animatorValue_multiplePoints_animationDuration_lookAhead() {
        every { projection.getMetersPerPixelAtLatitude(any()) } answers { 10000.0 } // disable snap
        val locationInter = Location("")
        locationInter.latitude = 51.1
        locationInter.longitude = 17.1
        locationInter.bearing = 35f
        val location = Location("")
        location.latitude = 51.2
        location.longitude = 17.2
        location.bearing = 36f
        location.time = System.currentTimeMillis() + 2000
        locationAnimatorCoordinator.feedNewLocation(arrayOf(locationInter, location), cameraPosition, false, true)

        verify {
            animatorSetProvider.startAnimation(
                eq(
                    listOf(
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG],
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING],
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG],
                        locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING]
                    )
                ),
                any<LinearInterpolator>(),
                more(1500L)
            )
        }
    }

    @Test
    fun feedNewLocation_animatorValue_correctRotation_1() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 0f

        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        every { animator.animatedValue } returns 270f
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(360f, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_animatorValue_correctRotation_2() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 90f

        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        every { animator.animatedValue } returns 280f
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(450f, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_animatorValue_correctRotation_3() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 300f

        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        every { animator.animatedValue } returns 450f
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(-60f, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_animatorValue_correctRotation_4() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 350f

        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        every { animator.animatedValue } returns 10f
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(-10f, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_animatorValue_correctRotation_5() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 90f

        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        every { animator.animatedValue } returns -280f
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(90f, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_animatorValue_correctRotation_6() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 270f

        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        every { animator.animatedValue } returns -350f
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(-90f, layerBearingTarget)
    }

    @Test
    fun feedNewLocation_isNorth_animatorsAreCreated() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 35f
        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, false)

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING] != null)
    }

    @Test
    fun feedNewLocation_isNorth_animatorValue() {
        val location = Location("")
        location.latitude = 51.0
        location.longitude = 17.0
        location.bearing = 35f
        locationAnimatorCoordinator.feedNewLocation(location, cameraPosition, true)

        val cameraLatLngTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG]?.target as LatLng
        MapLibreAssert.assertEquals(cameraLatLngTarget.latitude, cameraLatLngTarget.latitude)

        val layerLatLngTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_LATLNG]?.target as LatLng
        MapLibreAssert.assertEquals(layerLatLngTarget.latitude, layerLatLngTarget.latitude)

        val cameraBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(0f, cameraBearingTarget)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_GPS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(location.bearing, layerBearingTarget)
    }

    @Test
    fun feedNewCompassBearing_animatorsAreCreated() {
        locationAnimatorCoordinator.feedNewCompassBearing(77f, cameraPosition)

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_COMPASS_BEARING] != null)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_COMPASS_BEARING] != null)
    }

    @Test
    fun feedNewCompassBearing_animatorValue() {
        val bearing = 77f
        locationAnimatorCoordinator.feedNewCompassBearing(bearing, cameraPosition)

        val cameraBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_COMPASS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(bearing, cameraBearingTarget)

        val layerBearingTarget = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_COMPASS_BEARING]?.target as Float
        MapLibreAssert.assertEquals(bearing, layerBearingTarget)
    }

    @Test
    fun feedNewAccuracyRadius_animatorsCreated() {
        locationAnimatorCoordinator.feedNewAccuracyRadius(150f, false)

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_ACCURACY] != null)
    }

    @Test
    fun feedNewAccuracyRadius_animatorValue() {
        val accuracy = 150f
        locationAnimatorCoordinator.feedNewAccuracyRadius(accuracy, false)

        val layerAccuracy = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_ACCURACY]?.target as Float
        MapLibreAssert.assertEquals(layerAccuracy, accuracy)
    }

    @Test
    fun feedNewAccuracyRadius_noAnimation_animatorsCreated() {
        locationAnimatorCoordinator.feedNewAccuracyRadius(150f, true)

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_ACCURACY] != null)
    }

    @Test
    fun feedNewAccuracyRadius_noAnimation_animatorValue() {
        val accuracy = 150f
        locationAnimatorCoordinator.feedNewAccuracyRadius(accuracy, true)

        val layerAccuracy = locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_ACCURACY]?.target as Float
        MapLibreAssert.assertEquals(layerAccuracy, accuracy)
    }

    @Test
    fun feedNewZoomLevel_animatorsCreated() {
        locationAnimatorCoordinator.feedNewZoomLevel(
            15.0,
            cameraPosition,
            DEFAULT_TRACKING_ZOOM_ANIM_DURATION,
            null
        )

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_ZOOM] != null)
    }

    @Test
    fun feedNewZoomLevel_animatorValue() {
        val zoom = 15.0f
        locationAnimatorCoordinator.feedNewZoomLevel(
            zoom.toDouble(),
            cameraPosition,
            DEFAULT_TRACKING_ZOOM_ANIM_DURATION,
            null
        )

        val animator = locationAnimatorCoordinator.animatorArray[ANIMATOR_ZOOM]
        MapLibreAssert.assertEquals(zoom, animator.target as Float)
        verify { animatorSetProvider.startAnimation(eq(listOf(animator)), any<LinearInterpolator>(), DEFAULT_TRACKING_ZOOM_ANIM_DURATION) }
    }

    @Test
    fun feedNewPadding_animatorsCreated() {
        locationAnimatorCoordinator.feedNewPadding(
            doubleArrayOf(100.0, 200.0, 300.0, 400.0),
            cameraPosition,
            DEFAULT_TRACKING_PADDING_ANIM_DURATION,
            null
        )

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_PADDING] != null)
    }

    @Test
    fun feedNewPadding_animatorValue() {
        val padding = doubleArrayOf(100.0, 200.0, 300.0, 400.0)
        locationAnimatorCoordinator.feedNewPadding(
            padding,
            cameraPosition,
            DEFAULT_TRACKING_PADDING_ANIM_DURATION,
            null
        )

        val animator = locationAnimatorCoordinator.animatorArray[ANIMATOR_PADDING]
        assertTrue(padding.contentEquals(animator.target as DoubleArray))
        verify { animatorSetProvider.startAnimation(eq(listOf(animator)), any<LinearInterpolator>(), DEFAULT_TRACKING_PADDING_ANIM_DURATION) }
    }

    @Test
    fun feedNewTiltLevel_animatorsCreated() {
        locationAnimatorCoordinator.feedNewTilt(
            30.0,
            cameraPosition,
            DEFAULT_TRACKING_TILT_ANIM_DURATION,
            null
        )

        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_TILT] != null)
    }

    @Test
    fun feedNewTiltLevel_animatorValue() {
        val tilt = 30.0f
        locationAnimatorCoordinator.feedNewTilt(
            tilt.toDouble(),
            cameraPosition,
            DEFAULT_TRACKING_TILT_ANIM_DURATION,
            null
        )

        val animator = locationAnimatorCoordinator.animatorArray[ANIMATOR_TILT]
        MapLibreAssert.assertEquals(tilt, animator.target as Float)
        verify { animatorSetProvider.startAnimation(eq(listOf(animator)), any<LinearInterpolator>(), DEFAULT_TRACKING_TILT_ANIM_DURATION) }
    }

    @Test
    fun cancelAllAnimators() {
        locationAnimatorCoordinator.feedNewLocation(Location(""), cameraPosition, true)
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG].isStarted)

        locationAnimatorCoordinator.cancelAllAnimations()

        assertFalse(locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_LATLNG].isStarted)
    }

    // regression test for crash https://github.com/maplibre/maplibre-native/issues/3294
    @Test
    fun resetAllCameraAnimations_null_target() {
        locationAnimatorCoordinator.feedNewLocation(Location(""), CameraPosition.DEFAULT, true)

        val cameraPosition = CameraPosition.Builder().build()
        locationAnimatorCoordinator.resetAllCameraAnimations(cameraPosition, false)
    }

    @Test
    fun cancelZoomAnimators() {
        locationAnimatorCoordinator.feedNewZoomLevel(
            15.0,
            cameraPosition,
            DEFAULT_TRACKING_ZOOM_ANIM_DURATION,
            null
        )
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_ZOOM].isStarted)

        locationAnimatorCoordinator.cancelZoomAnimation()

        assertFalse(locationAnimatorCoordinator.animatorArray[ANIMATOR_ZOOM].isStarted)
    }

    @Test
    fun cancelPaddingAnimators() {
        locationAnimatorCoordinator.feedNewPadding(
            doubleArrayOf(100.0, 200.0, 300.0, 400.0),
            cameraPosition,
            DEFAULT_TRACKING_PADDING_ANIM_DURATION,
            null
        )
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_PADDING].isStarted)

        locationAnimatorCoordinator.cancelPaddingAnimation()

        assertFalse(locationAnimatorCoordinator.animatorArray[ANIMATOR_PADDING].isStarted)
    }

    @Test
    fun cancelTiltAnimation() {
        locationAnimatorCoordinator.feedNewTilt(
            30.0,
            cameraPosition,
            DEFAULT_TRACKING_TILT_ANIM_DURATION,
            null
        )
        assertTrue(locationAnimatorCoordinator.animatorArray[ANIMATOR_TILT].isStarted)

        locationAnimatorCoordinator.cancelTiltAnimation()

        assertFalse(locationAnimatorCoordinator.animatorArray[ANIMATOR_TILT].isStarted)
    }

    @Test
    fun resetAllCameraAnimations_empty() {
        locationAnimatorCoordinator.resetAllCameraAnimations(cameraPosition, false)
        assertTrue(locationAnimatorCoordinator.animatorArray.size() == 0)
    }

    @Test
    fun resetAllLayerAnimations_empty() {
        locationAnimatorCoordinator.resetAllLayerAnimations()
        assertTrue(locationAnimatorCoordinator.animatorArray.size() == 0)
    }

    @Test
    fun addNewListener() {
        val listener = Mockito.mock(AnimationsValueChangeListener::class.java)
        val holder = AnimatorListenerHolder(RenderMode.NORMAL, listener)
        val set = HashSet<AnimatorListenerHolder>().also {
            it.add(holder)
        }
        locationAnimatorCoordinator.updateAnimatorListenerHolders(set)

        assertTrue(locationAnimatorCoordinator.listeners.contains(listener))
    }

    @Test
    fun updateListeners() {
        val listener = Mockito.mock(AnimationsValueChangeListener::class.java)
        val holder = AnimatorListenerHolder(ANIMATOR_LAYER_LATLNG, listener)
        val set = HashSet<AnimatorListenerHolder>().also {
            it.add(holder)
        }
        locationAnimatorCoordinator.updateAnimatorListenerHolders(set)

        val listener2 = Mockito.mock(AnimationsValueChangeListener::class.java)
        val holder2 = AnimatorListenerHolder(ANIMATOR_LAYER_LATLNG, listener2)
        val listener3 = Mockito.mock(AnimationsValueChangeListener::class.java)
        val holder3 = AnimatorListenerHolder(ANIMATOR_LAYER_ACCURACY, listener3)
        val set2 = HashSet<AnimatorListenerHolder>().also {
            it.add(holder2)
            it.add(holder3)
        }
        locationAnimatorCoordinator.updateAnimatorListenerHolders(set2)

        assertTrue(locationAnimatorCoordinator.listeners.size() == 2)
        assertTrue(locationAnimatorCoordinator.listeners.contains(listener2))
        assertTrue(locationAnimatorCoordinator.listeners.contains(listener3))
    }

    @Test
    fun updateListeners_listenerRemoved_animtorInvalid() {
        // setup accuracy animator
        val listener: AnimationsValueChangeListener<Float> = mockk(relaxUnitFun = true)
        val holder = AnimatorListenerHolder(ANIMATOR_LAYER_ACCURACY, listener)
        val set = HashSet<AnimatorListenerHolder>().also {
            it.add(holder)
        }
        locationAnimatorCoordinator.updateAnimatorListenerHolders(set)
        locationAnimatorCoordinator.feedNewAccuracyRadius(500f, true)

        // reset animator listeners which should make the previously created animator invalid
        val listener2: AnimationsValueChangeListener<*> = mockk()
        val holder2 = AnimatorListenerHolder(ANIMATOR_LAYER_LATLNG, listener2)
        val listener3: AnimationsValueChangeListener<*> = mockk()
        val holder3 = AnimatorListenerHolder(ANIMATOR_CAMERA_GPS_BEARING, listener3)
        val set2 = HashSet<AnimatorListenerHolder>().also {
            it.add(holder2)
            it.add(holder3)
        }
        locationAnimatorCoordinator.updateAnimatorListenerHolders(set2)

        // try pushing an update to verify it was ignored
        val valueAnimator: ValueAnimator = mockk()
        every { valueAnimator.animatedValue } returns 10f
        val animator = locationAnimatorCoordinator.animatorArray.get(ANIMATOR_LAYER_ACCURACY)
        animator.onAnimationUpdate(valueAnimator)

        verify(exactly = 0) { listener.onNewAnimationValue(10f) }
    }

    @Test
    fun feedNewCompassBearing_withAnimation() {
        locationAnimatorCoordinator.setCompassAnimationEnabled(true)
        locationAnimatorCoordinator.feedNewCompassBearing(77f, cameraPosition)

        val animators = mutableListOf<Animator>(
            locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_COMPASS_BEARING],
            locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_COMPASS_BEARING]
        )

        verify(exactly = 1) { animatorSetProvider.startAnimation(eq(animators), ofType(LinearInterpolator::class), eq(LocationComponentConstants.COMPASS_UPDATE_RATE_MS)) }
    }

    @Test
    fun feedNewCompassBearing_withoutAnimation() {
        locationAnimatorCoordinator.setCompassAnimationEnabled(false)
        locationAnimatorCoordinator.feedNewCompassBearing(77f, cameraPosition)

        val animators = mutableListOf<Animator>(
            locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_COMPASS_BEARING],
            locationAnimatorCoordinator.animatorArray[ANIMATOR_CAMERA_COMPASS_BEARING]
        )

        verify(exactly = 1) { animatorSetProvider.startAnimation(eq(animators), ofType(LinearInterpolator::class), eq(0)) }
    }

    @Test
    fun feedNewAccuracy_withAnimation() {
        locationAnimatorCoordinator.setAccuracyAnimationEnabled(true)
        locationAnimatorCoordinator.feedNewAccuracyRadius(150f, false)

        val animators = mutableListOf<Animator>(
            locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_ACCURACY]
        )

        verify(exactly = 1) { animatorSetProvider.startAnimation(eq(animators), ofType(LinearInterpolator::class), eq(LocationComponentConstants.ACCURACY_RADIUS_ANIMATION_DURATION)) }
    }

    @Test
    fun feedNewAccuracy_withoutAnimation() {
        locationAnimatorCoordinator.setAccuracyAnimationEnabled(false)
        locationAnimatorCoordinator.feedNewAccuracyRadius(150f, false)

        val animators = mutableListOf<Animator>(
            locationAnimatorCoordinator.animatorArray[ANIMATOR_LAYER_ACCURACY]
        )

        verify(exactly = 1) { animatorSetProvider.startAnimation(eq(animators), ofType(LinearInterpolator::class), eq(0)) }
    }

    @Test
    fun maxFps_setter() {
        locationAnimatorCoordinator.setMaxAnimationFps(5)
        assertEquals(5, locationAnimatorCoordinator.maxAnimationFps)
    }

    @Test
    fun maxFps_moreThanZeroRequired() {
        locationAnimatorCoordinator.setMaxAnimationFps(0)
        assertEquals(Int.MAX_VALUE, locationAnimatorCoordinator.maxAnimationFps)
        locationAnimatorCoordinator.setMaxAnimationFps(-1)
        assertEquals(Int.MAX_VALUE, locationAnimatorCoordinator.maxAnimationFps)
    }

    @Test
    fun maxFps_givenToAnimator() {
        locationAnimatorCoordinator.setMaxAnimationFps(5)
        locationAnimatorCoordinator.feedNewLocation(Location(""), cameraPosition, false)
        verify { animatorProvider.latLngAnimator(any(), any(), 5) }
        verify { animatorProvider.floatAnimator(any(), any(), 5) }
    }

    @Test
    fun remove_gps_animator() {
        val animator = mockk<MapLibreFloatAnimator>(relaxed = true)
        locationAnimatorCoordinator.animatorArray.put(ANIMATOR_LAYER_GPS_BEARING, animator)

        locationAnimatorCoordinator.cancelAndRemoveGpsBearingAnimation()
        assertTrue(locationAnimatorCoordinator.animatorArray.get(ANIMATOR_LAYER_GPS_BEARING) == null)
    }

    private fun getListenerHoldersSet(vararg animatorTypes: Int): Set<AnimatorListenerHolder> {
        return HashSet<AnimatorListenerHolder>().also {
            for (type in animatorTypes) {
                it.add(AnimatorListenerHolder(type, mockk(relaxUnitFun = true)))
            }
        }
    }
}

private fun <E> SparseArray<E>.contains(listener: AnimationsValueChangeListener<*>?): Boolean {
    for (i in 0 until this.size()) {
        val element = this.get(this.keyAt(i))
        if (element == listener) {
            return true
        }
    }
    return false
}
