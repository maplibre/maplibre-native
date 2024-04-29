package org.maplibre.android.maps

import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.After
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.AppCenter
import org.maplibre.android.MapLibre
import org.maplibre.android.exceptions.MapLibreConfigurationException

@RunWith(AndroidJUnit4ClassRunner::class)
class MapLibreTest : AppCenter() {
    private var realToken: String? = null
    @Before
    fun setup() {
        realToken = MapLibre.getApiKey()
    }

    @Test
    @UiThreadTest
    fun testConnected() {
        Assert.assertTrue(MapLibre.isConnected())

        // test manual connectivity
        MapLibre.setConnected(true)
        Assert.assertTrue(MapLibre.isConnected())
        MapLibre.setConnected(false)
        Assert.assertFalse(MapLibre.isConnected())

        // reset to Android connectivity
        MapLibre.setConnected(null)
        Assert.assertTrue(MapLibre.isConnected())
    }

    @Test
    @UiThreadTest
    fun setApiKey() {
        MapLibre.setApiKey(API_KEY)
        Assert.assertSame(API_KEY, MapLibre.getApiKey())
        MapLibre.setApiKey(API_KEY_2)
        Assert.assertSame(API_KEY_2, MapLibre.getApiKey())
    }

    @Test
    @UiThreadTest
    fun setNullApiKey() {
        Assert.assertThrows(
            MapLibreConfigurationException::class.java
        ) { MapLibre.setApiKey(null) }
    }

    @After
    fun tearDown() {
        if (realToken?.isNotEmpty() == true) {
            MapLibre.setApiKey(realToken)
        }

    }

    companion object {
        private const val API_KEY = "pk.0000000001"
        private const val API_KEY_2 = "pk.0000000002"
    }
}