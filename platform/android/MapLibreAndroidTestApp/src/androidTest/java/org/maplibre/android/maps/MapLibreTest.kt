package org.maplibre.android.maps

import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.AppCenter
import org.maplibre.android.MapLibre.*
import org.maplibre.android.exceptions.MapLibreConfigurationException

@RunWith(AndroidJUnit4ClassRunner::class)
class MapLibreTest : AppCenter() {
    private var realToken: String? = null
    @Before
    fun setup() {
        realToken = getApiKey()
    }

    @Test
    @UiThreadTest
    fun testConnected() {
        assertTrue(isConnected())

        // test manual connectivity
        setConnected(true)
        assertTrue(isConnected())
        setConnected(false)
        assertFalse(isConnected())

        // reset to Android connectivity
        setConnected(null)
        assertTrue(isConnected())
    }

    @Test
    @UiThreadTest
    fun setApiKey() {
        setApiKey(API_KEY)
        assertSame(API_KEY, getApiKey())
        setApiKey(API_KEY_2)
        assertSame(API_KEY_2, getApiKey())
    }

    @Test
    @UiThreadTest
    fun setNullApiKey() {
        assertThrows(MapLibreConfigurationException::class.java) { setApiKey(null) }
    }

    @After
    fun tearDown() {
        setApiKey(realToken)
    }

    companion object {
        private const val API_KEY = "pk.0000000001"
        private const val API_KEY_2 = "pk.0000000002"
    }
}