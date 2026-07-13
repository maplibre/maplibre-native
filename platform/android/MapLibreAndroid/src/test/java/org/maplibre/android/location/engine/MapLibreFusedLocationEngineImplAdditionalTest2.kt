package org.maplibre.android.location.engine

import android.content.Context
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.location.LocationProvider
import android.os.Looper
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.ArgumentMatchers
import org.mockito.Mockito.*
import org.mockito.junit.MockitoJUnitRunner
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

@RunWith(MockitoJUnitRunner::class)
class MapLibreFusedLocationEngineImplAdditionalTest2 : BaseTest() {
    private val engines = ArrayList<LocationEngineProxy<*>>()
    private var mockLocationManager: LocationManager? = null
    private var location = Location(PROVIDER)
    @Before
    fun setUp() {
        location = mock(Location::class.java)
        `when`(location.latitude).thenReturn(1.0)
        `when`(location.longitude).thenReturn(2.0)
        val mockContext = mock(Context::class.java)
        mockLocationManager = mock(LocationManager::class.java)
        `when`(mockContext.getSystemService(anyString())).thenReturn(mockLocationManager)
        val providers: MutableList<String> = ArrayList()
        providers.add(PROVIDER)
        // J2K: add ?
        `when`(mockLocationManager?.getAllProviders()).thenReturn(providers)
        `when`(mockLocationManager?.getBestProvider(any(Criteria::class.java), anyBoolean()))
                .thenReturn(LocationManager.GPS_PROVIDER)
        doAnswer { invocation ->
            val listener = invocation.arguments[3] as LocationListener
            listener.onProviderEnabled(PROVIDER)
            listener.onStatusChanged(PROVIDER, LocationProvider.AVAILABLE, null)
            listener.onLocationChanged(location)
            listener.onProviderDisabled(PROVIDER)
            null
        }.`when`(mockLocationManager)
                // J2K: add ?
                ?.requestLocationUpdates(ArgumentMatchers.anyString(), ArgumentMatchers.anyLong(), ArgumentMatchers.anyFloat(), ArgumentMatchers.any(LocationListener::class.java), ArgumentMatchers.any(Looper::class.java))
        engines.add(LocationEngineProxy(MapLibreFusedLocationEngineImpl(mockContext)))
        engines.add(LocationEngineProxy(AndroidLocationEngineImpl(mockContext)))
    }

    @Test
    @Throws(InterruptedException::class)
    fun checkGetLastLocation() {
        val latch = CountDownLatch(engines.size)
        for (engineProxy in engines) {
            // J2K: remove ? from LocationEngineResult
            engineProxy.getLastLocation(object : LocationEngineCallback<LocationEngineResult> {
                override fun onSuccess(result: LocationEngineResult) {}
                override fun onFailure(exception: Exception) {
                    assertEquals("Last location unavailable", exception.localizedMessage)
                    latch.countDown()
                }
            })
        }
        assertTrue(latch.await(1, TimeUnit.SECONDS))
        `when`(mockLocationManager!!.getLastKnownLocation(anyString())).thenReturn(location)
        val latch1 = CountDownLatch(engines.size)
        for (engineProxy in engines) {
            // J2K: remove ? from LocationEngineResult
            engineProxy.getLastLocation(object : LocationEngineCallback<LocationEngineResult> {
                override fun onSuccess(result: LocationEngineResult) {
                    val list = result.locations
                    assertEquals(1, list.size.toLong())
                    assertEquals(1.0, list[0].latitude, 0.0)
                    assertEquals(2.0, list[0].longitude, 0.0)
                    latch1.countDown()
                }

                override fun onFailure(exception: Exception) {}
            })
        }
        assertTrue(latch1.await(1, TimeUnit.SECONDS))
    }

    @Test
    @Throws(InterruptedException::class)
    fun checkRequestAndRemoveLocationUpdates() {
        val latch = CountDownLatch(engines.size)
        // J2K: remove ? from LocationEngineResult
        val engineCallback: LocationEngineCallback<LocationEngineResult> = object : LocationEngineCallback<LocationEngineResult> {
            override fun onSuccess(result: LocationEngineResult) {
                val list = result.locations
                assertEquals(1, list.size.toLong())
                assertEquals(1.0, list[0].latitude, 0.0)
                assertEquals(2.0, list[0].longitude, 0.0)
                latch.countDown()
            }

            override fun onFailure(exception: Exception) {}
        }
        for (engineProxy in engines) {
            engineProxy.requestLocationUpdates(getRequest(INTERVAL, LocationEngineRequest.PRIORITY_HIGH_ACCURACY),
                    engineCallback, mock(Looper::class.java))
            assertTrue(latch.await(1, TimeUnit.SECONDS))
            assertNotNull(engineProxy.removeListener(engineCallback))
        }
    }

    companion object {
        private const val INTERVAL = 1000L
        private const val PROVIDER = "test_provider"
        private fun getRequest(interval: Long, priority: Int): LocationEngineRequest {
            return LocationEngineRequest.Builder(interval).setPriority(priority).build()
        }
    }
}
