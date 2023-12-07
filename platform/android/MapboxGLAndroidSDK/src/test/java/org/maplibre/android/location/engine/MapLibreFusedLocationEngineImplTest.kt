package org.maplibre.android.location.engine

import android.app.PendingIntent
import android.content.Context
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.os.Looper
import junit.framework.TestCase
import org.assertj.core.api.Assertions
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.junit.MockitoJUnitRunner
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference

@RunWith(MockitoJUnitRunner::class)
class MapLibreFusedLocationEngineImplTest {
    @Mock
    private val locationManagerMock: LocationManager? = null
    private var engine: LocationEngine? = null
    private var maplibreFusedLocationEngineImpl: MapLibreFusedLocationEngineImpl? = null
    @Before
    fun setUp() {
        val context = Mockito.mock(Context::class.java)
        Mockito.`when`(context.getSystemService(Context.LOCATION_SERVICE)).thenReturn(locationManagerMock)
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
            Mockito.`when`(locationManagerMock!!.allProviders).thenReturn(mutableListOf("gps", "network"))
            Mockito.`when`(locationManagerMock.getLastKnownLocation(ArgumentMatchers.anyString())).thenReturn(location)
            engine!!.getLastLocation(callback)
            TestCase.assertTrue(latch.await(5, TimeUnit.SECONDS))
            val result = resultRef.get()
            Assertions.assertThat(result.lastLocation).isEqualTo(expectedResult.lastLocation)
    }

    @Test
    fun createListener() {
        // J2K: IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val callback: LocationEngineCallback<LocationEngineResult> = Mockito.mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val locationListener = maplibreFusedLocationEngineImpl!!.createListener(callback)
        val mockLocation = getMockLocation(LATITUDE, LONGITUDE)
        locationListener.onLocationChanged(mockLocation)
        val argument = ArgumentCaptor.forClass(LocationEngineResult::class.java)
        Mockito.verify(callback).onSuccess(argument.capture())
        val result = argument.value
        Assertions.assertThat(result.lastLocation).isSameAs(mockLocation)
    }

    @Test
    fun requestLocationUpdatesOutdoors() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        // J2K: IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val callback: LocationEngineCallback<LocationEngineResult> = Mockito.mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val looper = Mockito.mock(Looper::class.java)
        Mockito.`when`(locationManagerMock!!.getBestProvider(ArgumentMatchers.any(Criteria::class.java), ArgumentMatchers.anyBoolean())).thenReturn("gps")
        engine!!.requestLocationUpdates(request, callback, looper)
        Mockito.verify(locationManagerMock, Mockito.times(2)).requestLocationUpdates(ArgumentMatchers.anyString(),
                ArgumentMatchers.anyLong(), ArgumentMatchers.anyFloat(), ArgumentMatchers.any(LocationListener::class.java), ArgumentMatchers.any(Looper::class.java))
    }

    @Test
    fun requestLocationUpdatesIndoors() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        // J2K: IDE suggestion "as LocationEngineCallback<LocationEngineResult>"
        val callback: LocationEngineCallback<LocationEngineResult> = Mockito.mock(LocationEngineCallback::class.java) as LocationEngineCallback<LocationEngineResult>
        val looper = Mockito.mock(Looper::class.java)
        Mockito.`when`(locationManagerMock!!.getBestProvider(ArgumentMatchers.any(Criteria::class.java), ArgumentMatchers.anyBoolean())).thenReturn("network")
        engine!!.requestLocationUpdates(request, callback, looper)
        Mockito.verify(locationManagerMock, Mockito.times(1)).requestLocationUpdates(ArgumentMatchers.anyString(),
                ArgumentMatchers.anyLong(), ArgumentMatchers.anyFloat(), ArgumentMatchers.any(LocationListener::class.java), ArgumentMatchers.any(Looper::class.java))
    }

    @Test
    fun requestLocationUpdatesOutdoorsWithPendingIntent() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        val pendingIntent = Mockito.mock(PendingIntent::class.java)
        Mockito.`when`(locationManagerMock!!.getBestProvider(ArgumentMatchers.any(Criteria::class.java), ArgumentMatchers.anyBoolean())).thenReturn("gps")
        engine!!.requestLocationUpdates(request, pendingIntent)
        Mockito.verify(locationManagerMock, Mockito.times(2)).requestLocationUpdates(ArgumentMatchers.anyString(),
                ArgumentMatchers.anyLong(), ArgumentMatchers.anyFloat(), ArgumentMatchers.any(PendingIntent::class.java))
    }

    @Test
    fun requestLocationUpdatesIndoorsWithPendingIntent() {
        val request = LocationEngineRequest.Builder(10)
                .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY).build()
        val pendingIntent = Mockito.mock(PendingIntent::class.java)
        Mockito.`when`(locationManagerMock!!.getBestProvider(ArgumentMatchers.any(Criteria::class.java), ArgumentMatchers.anyBoolean())).thenReturn("network")
        engine!!.requestLocationUpdates(request, pendingIntent)
        Mockito.verify(locationManagerMock, Mockito.times(1)).requestLocationUpdates(ArgumentMatchers.anyString(),
                ArgumentMatchers.anyLong(), ArgumentMatchers.anyFloat(), ArgumentMatchers.any(PendingIntent::class.java))
    }

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

    @After
    fun tearDown() {
        Mockito.reset(locationManagerMock)
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
            val location = Mockito.mock(Location::class.java)
            location.latitude = lat
            location.longitude = lon
            return location
        }
    }
}
