package org.maplibre.android.location.engine

import android.location.Location
import android.location.LocationListener
import android.os.Looper
import org.assertj.core.api.Assertions.*
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.maplibre.android.location.engine.AndroidLocationEngineImpl.AndroidLocationEngineCallbackTransport
import org.mockito.Mock
import org.mockito.Mockito.*
import org.mockito.junit.MockitoJUnitRunner
import org.mockito.stubbing.Stubber
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference

@RunWith(MockitoJUnitRunner::class)
class LocationEngineTest : BaseTest() {
    @Mock
    private val locationEngineImpl: LocationEngineImpl<LocationListener>? = null
    private var engine: LocationEngine? = null
    @Before
    fun setUp() {
        engine = LocationEngineProxy(locationEngineImpl)
    }

    //@get:Throws(InterruptedException::class)
    @Test
    fun getLastLocation ()
    {
        val latch = CountDownLatch(1)
        val resultRef = AtomicReference<LocationEngineResult>()
        val callback = getCallback(resultRef, latch)
        val location = getMockLocation(LATITUDE, LONGITUDE)
        val expectedResult = getMockEngineResult(location)
        setupDoAnswer(expectedResult).`when`(locationEngineImpl)?.getLastLocation(callback)
        engine!!.getLastLocation(callback)
        assertTrue(latch.await(5, TimeUnit.SECONDS))
        val result = resultRef.get()
        assertThat(result).isSameAs(expectedResult)
        assertThat(result.lastLocation).isEqualTo(expectedResult.lastLocation)
    }

    // J2K: // TODO: this becomes unreachable, validate necessity of test
    @Test(expected = NullPointerException::class)
    fun getLastLocationNullCallback () {
        engine!!.getLastLocation(null!!)
    }

    @Test
    @Throws(InterruptedException::class)
    fun requestLocationUpdates() {
        val latch = CountDownLatch(1)
        val resultRef = AtomicReference<LocationEngineResult>()
        val callback = getCallback(resultRef, latch)
        val location = getMockLocation(LATITUDE, LONGITUDE)
        val expectedResult = getMockEngineResult(location)
        val looper = mock(Looper::class.java)
        val transport = AndroidLocationEngineCallbackTransport(callback)
        `when`(locationEngineImpl!!.createListener(callback)).thenReturn(transport)
        engine!!.requestLocationUpdates(getRequest(INTERVAL), callback, looper)
        transport.onLocationChanged(location)
        assertTrue(latch.await(5, TimeUnit.SECONDS))
        val result = resultRef.get()
        assertThat(result.lastLocation).isEqualTo(expectedResult.lastLocation)
    }

    // J2K: // TODO: this becomes unreachable, validate necessity of test
    @Test(expected = NullPointerException::class)
    fun requestLocationUpdatesNullCallback() {
        engine!!.requestLocationUpdates(null!!, null!!, null!!)
    }

    @After
    fun tearDown() {
        reset(locationEngineImpl)
        engine = null
    }

    companion object {
        private const val LATITUDE = 37.7749
        private const val LONGITUDE = 122.4194
        private const val INTERVAL = 1000L
        private fun setupDoAnswer(expectedResult: LocationEngineResult): Stubber {
            return doAnswer { invocation ->
                val callback = invocation.getArgument<LocationEngineCallback<LocationEngineResult>>(0)
                callback.onSuccess(expectedResult)
                null
            }
        }

        private fun getRequest(interval: Long): LocationEngineRequest {
            return LocationEngineRequest.Builder(interval).build()
        }

        private fun getCallback(
                resultRef: AtomicReference<LocationEngineResult>,
                latch: CountDownLatch): LocationEngineCallback<LocationEngineResult> {
            // J2K: remove ?
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
