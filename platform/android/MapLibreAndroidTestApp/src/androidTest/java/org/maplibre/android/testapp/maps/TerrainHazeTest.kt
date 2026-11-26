package org.maplibre.android.testapp.maps

import android.graphics.Color
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.AppCenter
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.testapp.activity.espresso.DeviceIndependentTestActivity
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * Tests the distance haze of MapLibreMap, the Android counterpart of iOS terrainHazeColor and
 * terrainHazeRange: what core holds after each call, including its limits on the range.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class TerrainHazeTest : AppCenter() {

    @Rule
    @JvmField
    var rule = ActivityTestRule(DeviceIndependentTestActivity::class.java)

    private lateinit var map: MapLibreMap

    @Before
    fun setup() {
        val latch = CountDownLatch(1)
        rule.runOnUiThread {
            rule.activity.mapView.getMapAsync {
                map = it
                latch.countDown()
            }
        }
        assertTrue(latch.await(30, TimeUnit.SECONDS))
    }

    private fun onUiThread(block: () -> Unit) {
        val latch = CountDownLatch(1)
        rule.runOnUiThread {
            block()
            latch.countDown()
        }
        assertTrue(latch.await(10, TimeUnit.SECONDS))
    }

    @Test
    fun testDefaultsToNoHaze() {
        onUiThread {
            assertEquals(Color.TRANSPARENT, map.terrainHazeColor)
            assertEquals(0f, map.terrainHazeRange, 0f)
        }
    }

    @Test
    fun testColorReadsBackAsSet() {
        onUiThread {
            for (color in intArrayOf(Color.RED, Color.argb(128, 255, 128, 0), Color.argb(51, 12, 200, 99), Color.WHITE)) {
                map.terrainHazeColor = color
                assertEquals(Integer.toHexString(color), Integer.toHexString(color), Integer.toHexString(map.terrainHazeColor))
            }
        }
    }

    @Test
    fun testRangeReadsBackAsSet() {
        onUiThread {
            map.terrainHazeRange = 3.5f
            assertEquals(3.5f, map.terrainHazeRange, 0f)
        }
    }

    @Test
    fun testRangeIsLimited() {
        onUiThread {
            // One or less, and NaN, turn the haze off; above 1000 is held at 1000.
            map.terrainHazeRange = 1f
            assertEquals(0f, map.terrainHazeRange, 0f)
            map.terrainHazeRange = -4f
            assertEquals(0f, map.terrainHazeRange, 0f)
            map.terrainHazeRange = Float.NaN
            assertEquals(0f, map.terrainHazeRange, 0f)
            map.terrainHazeRange = 1.5f
            assertEquals(1.5f, map.terrainHazeRange, 0f)
            map.terrainHazeRange = 5000f
            assertEquals(1000f, map.terrainHazeRange, 0f)
        }
    }

    @Test
    fun testColorAndRangeAreSetIndependently() {
        onUiThread {
            map.terrainHazeColor = Color.argb(128, 255, 128, 0)
            map.terrainHazeRange = 3f
            assertEquals(Integer.toHexString(Color.argb(128, 255, 128, 0)), Integer.toHexString(map.terrainHazeColor))

            map.terrainHazeColor = Color.BLUE
            assertEquals(3f, map.terrainHazeRange, 0f)

            map.terrainHazeRange = 7f
            assertEquals(Integer.toHexString(Color.BLUE), Integer.toHexString(map.terrainHazeColor))
        }
    }
}
