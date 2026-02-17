package org.maplibre.android.maps

import androidx.test.rule.ActivityTestRule
import org.junit.Assert.*
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.espresso.EspressoTestActivity
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * Test for verifying that calling setTileCacheEnabled and setPrefetchZoomDelta
 * before the renderer is initialized does not crash.
 * 
 * This test addresses issue where these methods caused a SIGSEGV crash when
 * called in getMapAsync callback before the style was fully loaded.
 */
class EarlyInitializationTest {

    @Rule
    @JvmField
    val rule = ActivityTestRule(EspressoTestActivity::class.java)

    private var mapView: MapView? = null
    private lateinit var latch: CountDownLatch

    @Before
    fun setUp() {
        latch = CountDownLatch(1)
    }

    /**
     * Test that setTileCacheEnabled can be called immediately in getMapAsync
     * without waiting for style to load.
     */
    @Test
    fun testSetTileCacheEnabledBeforeStyleLoad() {
        var exceptionThrown = false
        var mapLibreMap: MapLibreMap? = null

        rule.runOnUiThread {
            mapView = rule.activity.findViewById(R.id.mapView)
            mapView?.getMapAsync { map ->
                try {
                    // Call setTileCacheEnabled immediately, before style is loaded
                    // This should not crash
                    map.tileCacheEnabled = false
                    map.tileCacheEnabled = true
                    mapLibreMap = map
                } catch (e: Exception) {
                    exceptionThrown = true
                } finally {
                    latch.countDown()
                }
            }
        }

        // Wait for the callback to complete
        assertTrue("Test timed out", latch.await(10, TimeUnit.SECONDS))
        assertFalse("Exception was thrown when calling setTileCacheEnabled early", exceptionThrown)
        assertNotNull("MapLibreMap should be initialized", mapLibreMap)
    }

    /**
     * Test that setPrefetchZoomDelta can be called immediately in getMapAsync
     * without waiting for style to load.
     */
    @Test
    fun testSetPrefetchZoomDeltaBeforeStyleLoad() {
        var exceptionThrown = false
        var mapLibreMap: MapLibreMap? = null

        rule.runOnUiThread {
            mapView = rule.activity.findViewById(R.id.mapView)
            mapView?.getMapAsync { map ->
                try {
                    // Call setPrefetchZoomDelta immediately, before style is loaded
                    // This should not crash
                    map.prefetchZoomDelta = 0
                    map.prefetchZoomDelta = 2
                    mapLibreMap = map
                } catch (e: Exception) {
                    exceptionThrown = true
                } finally {
                    latch.countDown()
                }
            }
        }

        // Wait for the callback to complete
        assertTrue("Test timed out", latch.await(10, TimeUnit.SECONDS))
        assertFalse("Exception was thrown when calling setPrefetchZoomDelta early", exceptionThrown)
        assertNotNull("MapLibreMap should be initialized", mapLibreMap)
    }

    /**
     * Test that both methods can be called together immediately in getMapAsync
     * without waiting for style to load.
     */
    @Test
    fun testMultipleEarlyInitializationCalls() {
        var exceptionThrown = false
        var mapLibreMap: MapLibreMap? = null

        rule.runOnUiThread {
            mapView = rule.activity.findViewById(R.id.mapView)
            mapView?.getMapAsync { map ->
                try {
                    // Call both methods immediately, before style is loaded
                    // This should not crash
                    map.tileCacheEnabled = false
                    map.prefetchZoomDelta = 0
                    mapLibreMap = map
                } catch (e: Exception) {
                    exceptionThrown = true
                } finally {
                    latch.countDown()
                }
            }
        }

        // Wait for the callback to complete
        assertTrue("Test timed out", latch.await(10, TimeUnit.SECONDS))
        assertFalse("Exception was thrown when calling early initialization methods", exceptionThrown)
        assertNotNull("MapLibreMap should be initialized", mapLibreMap)
    }
}
