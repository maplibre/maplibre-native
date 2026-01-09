package org.maplibre.android.location.engine

import android.app.PendingIntent
import android.content.Context
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.os.Looper
import junit.framework.TestCase.*
import org.assertj.core.api.Assertions.*
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers.*
import org.mockito.Mock
import org.mockito.Mockito.*
import org.mockito.junit.MockitoJUnitRunner
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference

@RunWith(MockitoJUnitRunner::class)
class MapLibreFusedLocationEngineImplTest : BaseTest() {
    @Mock
    private val locationManagerMock: LocationManager? = null
    private var engine: LocationEngine? = null
    private var maplibreFusedLocationEngineImpl: MapLibreFusedLocationEngineImpl? = null
    @Before
    fun setUp() {
        val context = mock(Context::class.java)
        `when`(context.getSystemService(Context.LOCATION_SERVICE)).thenReturn(locationManagerMock)
        maplibreFusedLocationEngineImpl = MapLibreFusedLocationEngineImpl(context)
        engine = LocationEngineProxy(maplibreFusedLocationEngineImpl)
    }

    // J2K: rewrite test to not use "@get"
    @Test
    fun getLastLocation() {
            val latch = CountDownLatch(1)
            val resultRef = AtomicReference<LocationEngineResult>()
            val callback = getCallback(resultRef, latch)
            val location = getMockLocation(LATITUDE, LONGITUDE)
            val expectedResult = getMockEngineResult(location)
            `when`(locationManagerMock!!.allProviders).thenReturn(mutableListOf("gps", "network"))
            `when`(locationManagerMock.getLastKnownLocation(anyString())).thenReturn(location)
            engine!!.getLastLocation(callback)
            assertTrue(latch.await(5, TimeUnit.SECONDS))
            val result = resultRef.get()
            assertThat(result.lastLocation).isEqualTo(expectedResult.lastLocation)
    }

    @Test
    fun createListener() {
        // J2K: IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val callback: LocationEngineCallback<LocationEngineResult> = mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val locationListener = maplibreFusedLocationEngineImpl!!.createListener(callback)
        val mockLocation = getMockLocation(LATITUDE, LONGITUDE)
        locationListener.onLocationChanged(mockLocation)
        val argument = ArgumentCaptor.forClass(LocationEngineResult::class.java)
        verify(callback).onSuccess(argument.capture())
        val result = argument.value
        assertThat(result.lastLocation).isSameAs(mockLocation)
    }

    @Test
    fun requestLocationUpdatesOutdoors() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        // J2K: IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val callback: LocationEngineCallback<LocationEngineResult> = mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val looper = mock(Looper::class.java)
        `when`(locationManagerMock!!.getBestProvider(any(Criteria::class.java), anyBoolean())).thenReturn("gps")
        engine!!.requestLocationUpdates(request, callback, looper)
        verify(locationManagerMock, times(2)).requestLocationUpdates(anyString(),
                anyLong(), anyFloat(), any(LocationListener::class.java), any(Looper::class.java))
    }

    @Test
    fun requestLocationUpdatesIndoors() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        // J2K: IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val callback: LocationEngineCallback<LocationEngineResult> = mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val looper = mock(Looper::class.java)
        `when`(locationManagerMock!!.getBestProvider(any(Criteria::class.java), anyBoolean())).thenReturn("network")
        engine!!.requestLocationUpdates(request, callback, looper)
        verify(locationManagerMock, times(1)).requestLocationUpdates(anyString(),
                anyLong(), anyFloat(), any(LocationListener::class.java), any(Looper::class.java))
    }

    @Test
    fun requestLocationUpdatesOutdoorsWithPendingIntent() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        val pendingIntent = mock(PendingIntent::class.java)
        `when`(locationManagerMock!!.getBestProvider(any(Criteria::class.java), anyBoolean())).thenReturn("gps")
        engine!!.requestLocationUpdates(request, pendingIntent)
        verify(locationManagerMock, times(2)).requestLocationUpdates(anyString(),
                anyLong(), anyFloat(), any(PendingIntent::class.java))
    }

    @Test
    fun requestLocationUpdatesIndoorsWithPendingIntent() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        val pendingIntent = mock(PendingIntent::class.java)
        `when`(locationManagerMock!!.getBestProvider(any(Criteria::class.java), anyBoolean())).thenReturn("network")
        engine!!.requestLocationUpdates(request, pendingIntent)
        verify(locationManagerMock, times(1)).requestLocationUpdates(anyString(),
                anyLong(), anyFloat(), any(PendingIntent::class.java))
    }

// J2k: // TODO: see https://github.com/maplibre/maplibre-native/issues/1949
/*
    // J2K: rewrite test to not use "@get", rename to "getLastLocationNull"
    @Test(expected = NullPointerException::class)
    fun getLastLocationNull() {
        engine!!.getLastLocation(null!!)
    }

    // J2K: changed "null" to "null!!"
    @Test(expected = NullPointerException::class)
    fun requestLocationUpdatesNullCallback() {
        engine!!.requestLocationUpdates(null!!, null!!, null!!)
    }
*/
    @After
    fun tearDown() {
        reset(locationManagerMock)
        engine = null
    }

    companion object {
        private const val LATITUDE = 37.7749
        private const val LONGITUDE = 122.4194
        private fun getCallback(
                resultRef: AtomicReference<LocationEngineResult>,
                latch: CountDownLatch): LocationEngineCallback<LocationEngineResult> {
            // J2K: remove '?' from LocationEngineResult?
            return object : LocationEngineCallback<LocationEngineResult> {
                override fun onSuccess(result: LocationEngineResult) {
                    resultRef.set(result)
                    latch.countDown()
                }

                override fun onFailure(exception: Exception) {
                    exception.printStackTrace()
                }
            }
        }

        private fun getMockEngineResult(location: Location): LocationEngineResult {
            return LocationEngineResult.create(location)
        }

        private fun getMockLocation(lat: Double, lon: Double): Location {
            val location = mock(Location::class.java)
            location.latitude = lat
            location.longitude = lon
            return location
        }
    }
}
