package org.maplibre.android.maps

import android.graphics.PointF
import org.maplibre.android.gestures.MoveGestureDetector
import org.maplibre.android.gestures.RotateGestureDetector
import org.maplibre.android.gestures.ShoveGestureDetector
import org.maplibre.android.gestures.StandardScaleGestureDetector
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap.*
import org.junit.Before
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.Mockito

class MapTouchListenersTest : BaseTest() {
    private var mapGestureDetector: MapGestureDetector? = null
    private var latLng: LatLng? = null
    private var pointF: PointF? = null

    @Before
    @Throws(Exception::class)
    fun setUp() {
        latLng = LatLng()
        pointF = PointF()
        val projection = Mockito.mock(
            Projection::class.java
        )
        Mockito.`when`(projection.fromScreenLocation(pointF!!)).thenReturn(latLng)
        mapGestureDetector = MapGestureDetector(
            null,
            null,
            projection,
            null,
            null,
            null
        )
    }

    @Test
    @Throws(Exception::class)
    fun onMapClickListenerTest() {
        val listener = Mockito.mock(
            OnMapClickListener::class.java
        )
        mapGestureDetector!!.addOnMapClickListener(listener)
        mapGestureDetector!!.notifyOnMapClickListeners(pointF!!)
        Mockito.verify(listener, Mockito.times(1)).onMapClick(
            latLng!!
        )
        mapGestureDetector!!.removeOnMapClickListener(listener)
        mapGestureDetector!!.notifyOnMapClickListeners(pointF!!)
        Mockito.verify(listener, Mockito.times(1)).onMapClick(
            latLng!!
        )
    }

    @Test
    @Throws(Exception::class)
    fun onMapLongClickListenerTest() {
        val listener = Mockito.mock(
            OnMapLongClickListener::class.java
        )
        mapGestureDetector!!.addOnMapLongClickListener(listener)
        mapGestureDetector!!.notifyOnMapLongClickListeners(pointF!!)
        Mockito.verify(listener, Mockito.times(1)).onMapLongClick(
            latLng!!
        )
        mapGestureDetector!!.removeOnMapLongClickListener(listener)
        mapGestureDetector!!.notifyOnMapLongClickListeners(pointF!!)
        Mockito.verify(listener, Mockito.times(1)).onMapLongClick(
            latLng!!
        )
    }

    @Test
    @Throws(Exception::class)
    fun onFlingListenerTest() {
        val listener = Mockito.mock(
            OnFlingListener::class.java
        )
        mapGestureDetector!!.addOnFlingListener(listener)
        mapGestureDetector!!.notifyOnFlingListeners()
        Mockito.verify(listener, Mockito.times(1)).onFling()
        mapGestureDetector!!.removeOnFlingListener(listener)
        mapGestureDetector!!.notifyOnFlingListeners()
        Mockito.verify(listener, Mockito.times(1)).onFling()
    }

    @Test
    @Throws(Exception::class)
    fun onMoveListenerTest() {
        val listener = Mockito.mock(OnMoveListener::class.java)
        val detector = Mockito.mock(
            MoveGestureDetector::class.java
        )
        mapGestureDetector!!.addOnMoveListener(listener)
        mapGestureDetector!!.notifyOnMoveBeginListeners(detector)
        mapGestureDetector!!.notifyOnMoveListeners(detector)
        mapGestureDetector!!.notifyOnMoveEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onMoveBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onMove(detector)
        Mockito.verify(listener, Mockito.times(1)).onMoveEnd(detector)
        mapGestureDetector!!.removeOnMoveListener(listener)
        mapGestureDetector!!.notifyOnMoveBeginListeners(detector)
        mapGestureDetector!!.notifyOnMoveListeners(detector)
        mapGestureDetector!!.notifyOnMoveEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onMoveBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onMove(detector)
        Mockito.verify(listener, Mockito.times(1)).onMoveEnd(detector)
    }

    @Test
    @Throws(Exception::class)
    fun onRotateListenerTest() {
        val listener = Mockito.mock(
            OnRotateListener::class.java
        )
        val detector = Mockito.mock(
            RotateGestureDetector::class.java
        )
        mapGestureDetector!!.addOnRotateListener(listener)
        mapGestureDetector!!.notifyOnRotateBeginListeners(detector)
        mapGestureDetector!!.notifyOnRotateListeners(detector)
        mapGestureDetector!!.notifyOnRotateEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onRotateBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onRotate(detector)
        Mockito.verify(listener, Mockito.times(1)).onRotateEnd(detector)
        mapGestureDetector!!.removeOnRotateListener(listener)
        mapGestureDetector!!.notifyOnRotateBeginListeners(detector)
        mapGestureDetector!!.notifyOnRotateListeners(detector)
        mapGestureDetector!!.notifyOnRotateEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onRotateBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onRotate(detector)
        Mockito.verify(listener, Mockito.times(1)).onRotateEnd(detector)
    }

    @Test
    @Throws(Exception::class)
    fun onScaleListenerTest() {
        val listener = Mockito.mock(OnScaleListener::class.java)
        val detector = Mockito.mock(
            StandardScaleGestureDetector::class.java
        )
        mapGestureDetector!!.addOnScaleListener(listener)
        mapGestureDetector!!.notifyOnScaleBeginListeners(detector)
        mapGestureDetector!!.notifyOnScaleListeners(detector)
        mapGestureDetector!!.notifyOnScaleEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onScaleBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onScale(detector)
        Mockito.verify(listener, Mockito.times(1)).onScaleEnd(detector)
        mapGestureDetector!!.removeOnScaleListener(listener)
        mapGestureDetector!!.notifyOnScaleBeginListeners(detector)
        mapGestureDetector!!.notifyOnScaleListeners(detector)
        mapGestureDetector!!.notifyOnScaleEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onScaleBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onScale(detector)
        Mockito.verify(listener, Mockito.times(1)).onScaleEnd(detector)
    }

    @Test
    @Throws(Exception::class)
    fun onShoveListenerTest() {
        val listener = Mockito.mock(OnShoveListener::class.java)
        val detector = Mockito.mock(
            ShoveGestureDetector::class.java
        )
        mapGestureDetector!!.addShoveListener(listener)
        mapGestureDetector!!.notifyOnShoveBeginListeners(detector)
        mapGestureDetector!!.notifyOnShoveListeners(detector)
        mapGestureDetector!!.notifyOnShoveEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onShoveBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onShove(detector)
        Mockito.verify(listener, Mockito.times(1)).onShoveEnd(detector)
        mapGestureDetector!!.removeShoveListener(listener)
        mapGestureDetector!!.notifyOnShoveBeginListeners(detector)
        mapGestureDetector!!.notifyOnShoveListeners(detector)
        mapGestureDetector!!.notifyOnShoveEndListeners(detector)
        Mockito.verify(listener, Mockito.times(1)).onShoveBegin(detector)
        Mockito.verify(listener, Mockito.times(1)).onShove(detector)
        Mockito.verify(listener, Mockito.times(1)).onShoveEnd(detector)
    }
}
