package org.maplibre.android.integration

import android.os.SystemClock
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.testapp.activity.fragment.ReusedMapViewActivity
import java.util.concurrent.atomic.AtomicInteger

/**
 * Regression tests for a single activity app that reuses one MapView for all of its screens, see
 * [ReusedMapViewActivity]. Navigating away from the map screen detaches the MapView and navigating
 * back attaches it again, while the activity stays resumed.
 *
 * With a SurfaceView backed MapView, leaving the map screen a second time used to block the UI
 * thread until the app was reported as not responding. Every navigation step is therefore bounded,
 * see [SurfaceViewDetachTestBase].
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class ReusedMapViewTest : SurfaceViewDetachTestBase() {

    override fun getActivityClass(): Class<*> = ReusedMapViewActivity::class.java

    private val activity: ReusedMapViewActivity
        get() = rule.activity as ReusedMapViewActivity

    private val surfaces = SurfaceCounter()

    /** Show the map, open a screen without it, return, and open a screen without it again. */
    @Test
    @LargeTest
    fun leaveAndReturnToMapScreen() {
        validateSurfaceViewSetup()
        assertMapRendersAFrame("on the map screen")

        for (i in 0 until CYCLES) {
            openScreenWithoutMap("cycle $i")
            val created = surfaces.created.get()
            returnToMapScreen("cycle $i")
            assertMapRendersAFrame("after returning to the map screen, cycle $i")
            Assert.assertTrue(
                "Returning to the map screen did not create a new surface (cycle $i)",
                surfaces.created.get() > created
            )
        }

        openScreenWithoutMap("after $CYCLES cycles")
        returnToMapScreen("after $CYCLES cycles")
    }

    /**
     * Return to the map screen and leave it again before the map had a chance to draw. The MapView
     * never gets a surface on these returns, so only the first leave destroys one.
     */
    @Test
    @LargeTest
    fun returnAndLeaveBeforeTheMapIsDrawn() {
        validateSurfaceViewSetup()
        openScreenWithoutMap("start")

        for (i in 0 until CYCLES) {
            returnAndLeaveInOneMessage("cycle $i")
        }

        returnToMapScreen("end")
        assertMapRendersAFrame("after returning to the map screen")
    }

    /**
     * A snapshot requested on the screen without map is scheduled for the renderer while no render
     * thread exists. Returning and leaving again before the map is drawn used to block the UI thread
     * forever. That detach happens before the MapView gets a surface again, so it is not preceded by
     * surfaceDestroyed, unlike leaving a map screen that is showing the map.
     */
    @Test
    @LargeTest
    fun snapshotOnScreenWithoutMap() {
        validateSurfaceViewSetup()
        openScreenWithoutMap("start")

        runOnUiThreadWithin(QUERY_TIMEOUT_MS, "requesting a snapshot on the screen without map", Runnable {
            maplibreMap.snapshot(MapLibreMap.SnapshotReadyCallback { })
        })
        returnAndLeaveInOneMessage("after the snapshot")

        returnToMapScreen("end")
        assertMapRendersAFrame("after returning to the map screen")
    }

    private fun validateSurfaceViewSetup() {
        validateTestSetup()
        Assert.assertTrue(
            "ReusedMapViewActivity has to render with a SurfaceView, see textureMode",
            mapView.renderView is SurfaceView
        )
        runOnUiThreadWithin(QUERY_TIMEOUT_MS, "watching the map's surface", Runnable {
            (mapView.renderView as SurfaceView).holder.addCallback(surfaces)
        })
    }

    /** Leaves the map screen while the MapView has a surface, which destroys that surface. */
    private fun openScreenWithoutMap(context: String) {
        val destroyed = surfaces.destroyed.get()
        runOnUiThreadWithin(LIFECYCLE_TIMEOUT_MS, "opening the screen without map ($context)", Runnable {
            activity.openScreenWithoutMap()
            activity.supportFragmentManager.executePendingTransactions()
        })
        awaitMapViewAttached(false, "opening the screen without map ($context)")
        Assert.assertTrue(
            "Leaving the map screen did not destroy the map's surface ($context)",
            surfaces.destroyed.get() > destroyed
        )
    }

    private fun returnToMapScreen(context: String) {
        runOnUiThreadWithin(LIFECYCLE_TIMEOUT_MS, "returning to the map screen ($context)", Runnable {
            Assert.assertTrue("Not on the screen without map", activity.supportFragmentManager.popBackStackImmediate())
        })
        awaitMapViewAttached(true, "returning to the map screen ($context)")
    }

    /** In a single message, so no traversal runs in between and the MapView never gets a surface. */
    private fun returnAndLeaveInOneMessage(context: String) {
        val created = surfaces.created.get()
        runOnUiThreadWithin(LIFECYCLE_TIMEOUT_MS, "returning and leaving again in one message ($context)", Runnable {
            Assert.assertTrue("Not on the screen without map", activity.supportFragmentManager.popBackStackImmediate())
            Assert.assertTrue("Returning did not attach the MapView", mapView.isAttachedToWindow)
            activity.openScreenWithoutMap()
            activity.supportFragmentManager.executePendingTransactions()
        })
        awaitMapViewAttached(false, "returning and leaving again in one message ($context)")
        Assert.assertEquals(
            "The MapView got a surface although it was attached and detached in one message ($context)",
            created,
            surfaces.created.get()
        )
    }

    private fun awaitMapViewAttached(attached: Boolean, context: String) {
        val deadline = SystemClock.elapsedRealtime() + LIFECYCLE_TIMEOUT_MS
        while (true) {
            var isAttached = !attached
            runOnUiThreadWithin(LIFECYCLE_TIMEOUT_MS, "checking whether the MapView is attached ($context)", Runnable {
                isAttached = mapView.isAttachedToWindow
            })
            if (isAttached == attached) {
                return
            }
            if (SystemClock.elapsedRealtime() > deadline) {
                Assert.fail("The MapView was not ${if (attached) "attached" else "detached"} ($context)")
            }
            SystemClock.sleep(POLL_INTERVAL_MS)
        }
    }

    /** Counts the surfaces the MapView gets and loses, and logs them. */
    private class SurfaceCounter : SurfaceHolder.Callback {
        val created = AtomicInteger()
        val destroyed = AtomicInteger()

        override fun surfaceCreated(holder: SurfaceHolder) {
            Log.i(TAG, "Map surface created (${created.incrementAndGet()})")
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            Log.i(TAG, "Map surface destroyed (${destroyed.incrementAndGet()})")
        }
    }

    companion object {
        private const val TAG = "ReusedMapViewTest"

        private const val CYCLES = 10

        private const val POLL_INTERVAL_MS = 20L
    }
}
