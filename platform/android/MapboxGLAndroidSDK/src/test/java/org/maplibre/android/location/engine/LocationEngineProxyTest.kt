package org.maplibre.android.location.engine

import android.location.LocationListener
import org.assertj.core.api.Assertions
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.location.engine.AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.junit.MockitoJUnitRunner

@RunWith(MockitoJUnitRunner::class)
class LocationEngineProxyTest {
    @Mock
    private val callback: LocationEngineCallback<LocationEngineResult>? = null

    @Mock
    private val engineImpl: LocationEngineImpl<LocationListener>? = null
    private var locationEngineProxy: LocationEngineProxy<LocationListener>? = null
    @Before
    fun setUp() {
        locationEngineProxy = LocationEngineProxy(engineImpl)
    }

    @Test
    fun testAddListener() {
        val transport = AndroidLocationEngineCallbackTransport(callback)
        Mockito.`when`(engineImpl!!.createListener(callback)).thenReturn(transport)
        val locationListener = locationEngineProxy!!.getListener(callback!!)
        Assertions.assertThat(locationListener).isSameAs(transport)
        Assertions.assertThat(locationEngineProxy!!.listenersCount).isEqualTo(1)
    }

    @Test
    fun testAddListenerTwice() {
        val transport = AndroidLocationEngineCallbackTransport(callback)
        Mockito.`when`(engineImpl!!.createListener(callback)).thenReturn(transport)
        locationEngineProxy!!.getListener(callback!!)
        locationEngineProxy!!.getListener(callback)
        Assertions.assertThat(locationEngineProxy!!.listenersCount).isEqualTo(1)
    }

    @Test
    fun testAddTwoListeners() {
        val transport = AndroidLocationEngineCallbackTransport(callback)
        Mockito.`when`(engineImpl!!.createListener(callback)).thenReturn(transport)
        locationEngineProxy!!.getListener(callback!!)
        // J2K: using IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val anotherCallback: LocationEngineCallback<LocationEngineResult> = Mockito.mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val anotherTransport = AndroidLocationEngineCallbackTransport(anotherCallback)
        Mockito.`when`(engineImpl.createListener(anotherCallback)).thenReturn(anotherTransport)
        locationEngineProxy!!.getListener(anotherCallback)
        Assertions.assertThat(locationEngineProxy!!.listenersCount).isEqualTo(2)
    }

    @Test
    fun testRemoveListener() {
        val transport = AndroidLocationEngineCallbackTransport(callback)
        Mockito.`when`(engineImpl!!.createListener(callback)).thenReturn(transport)
        locationEngineProxy!!.getListener(callback!!)
        locationEngineProxy!!.removeListener(callback)
        Assertions.assertThat(locationEngineProxy!!.listenersCount).isEqualTo(0)
    }

    @Test
    fun testCheckRemovedListener() {
        val transport = AndroidLocationEngineCallbackTransport(callback)
        Mockito.`when`(engineImpl!!.createListener(callback)).thenReturn(transport)
        locationEngineProxy!!.getListener(callback!!)
        // J2K: using IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val anotherCallback: LocationEngineCallback<LocationEngineResult> = Mockito.mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val anotherTransport = AndroidLocationEngineCallbackTransport(anotherCallback)
        Mockito.`when`(engineImpl.createListener(anotherCallback)).thenReturn(anotherTransport)
        locationEngineProxy!!.getListener(anotherCallback)
        Assertions.assertThat(locationEngineProxy!!.removeListener(callback)).isSameAs(transport)
        Assertions.assertThat(locationEngineProxy!!.removeListener(anotherCallback)).isSameAs(anotherTransport)
    }

    @Test
    fun testRemoveListenerTwice() {
        val transport = AndroidLocationEngineCallbackTransport(callback)
        Mockito.`when`(engineImpl!!.createListener(callback)).thenReturn(transport)
        locationEngineProxy!!.getListener(callback!!)
        Assertions.assertThat(locationEngineProxy!!.removeListener(callback)).isSameAs(transport)
        Assertions.assertThat(locationEngineProxy!!.removeListener(callback)).isNull()
    }
}
