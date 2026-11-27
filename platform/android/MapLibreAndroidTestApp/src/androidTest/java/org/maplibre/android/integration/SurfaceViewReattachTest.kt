package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * Regression tests for what has to survive a SurfaceView backed MapView being detached and
 * attached again: work queued for the render thread, and the map itself.
 *
 * Every detach stops the render thread and every attach starts a new one, so there are stretches
 * in which no render thread exists. The renderer reset of a detach used to be posted to the render
 * thread while the UI thread waited for it. Once a request was lost in such a stretch, the reset
 * of a later detach queued up behind it, and the UI thread waited forever: an ANR when leaving a
 * screen with a map that had been detached and attached before.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class SurfaceViewReattachTest : SurfaceViewDetachTestBase() {

    /**
     * A snapshot requested while detached is scheduled through the renderer's mailbox while no
     * render thread exists. Detaching again before the next render thread had built a new renderer,
     * and with it a new mailbox, then blocked the UI thread forever.
     */
    @Test
    @LargeTest
    fun detachBeforeSurfaceIsCreatedAfterSnapshotWhileDetached() {
        validateTestSetup()
        detachMapView()

        runOnUiThreadWithin(QUERY_TIMEOUT_MS, "requesting a snapshot while detached", Runnable {
            maplibreMap.snapshot(MapLibreMap.SnapshotReadyCallback { })
        })

        // In a single message, so no traversal runs in between and the new render thread never
        // gets a surface to build a renderer for.
        runOnUiThreadWithin(
            LIFECYCLE_TIMEOUT_MS,
            "attaching and detaching before the surface is created",
            Runnable {
                attachNow()
                detachNow()
            }
        )

        attachMapView()
        assertMapRendersAFrame("after detaching before the surface was created")
    }

    @Test
    @LargeTest
    fun eventQueuedWhileDetachedRunsAfterReattach() {
        validateTestSetup()
        detachMapView()

        val ran = CountDownLatch(1)
        runOnUiThreadWithin(QUERY_TIMEOUT_MS, "queueing an event while detached", Runnable {
            mapView.queueEvent { ran.countDown() }
        })

        attachMapView()
        Assert.assertTrue(
            "An event queued while detached never ran on the render thread",
            ran.await(FRAME_TIMEOUT_MS, TimeUnit.MILLISECONDS)
        )
    }

    /**
     * Detaching releases the native renderer and attaching builds a new one. The new renderer has
     * to draw the map as it is, not wait for the next change of the map.
     */
    @Test
    @LargeTest
    fun reattachedMapRendersWithoutAMapChange() {
        validateTestSetup()
        awaitIdle()
        detachMapView()

        val frameRendered = CountDownLatch(1)
        val listener = MapView.OnDidFinishRenderingFrameListener { _, _, _ -> frameRendered.countDown() }
        runOnUiThreadWithin(QUERY_TIMEOUT_MS, "adding the frame listener", Runnable {
            mapView.addOnDidFinishRenderingFrameListener(listener)
        })

        try {
            attachMapView()
            Assert.assertTrue(
                "The re-attached map did not render a frame until the map changed",
                frameRendered.await(FRAME_TIMEOUT_MS, TimeUnit.MILLISECONDS)
            )
        } finally {
            postOnUiThread(Runnable { mapView.removeOnDidFinishRenderingFrameListener(listener) })
                .awaitWithin(QUERY_TIMEOUT_MS, "removing the frame listener")
        }
    }

    /** Waits until the map has nothing left to load or animate, so nothing else asks for a frame. */
    private fun awaitIdle() {
        val idle = CountDownLatch(1)
        val listener = MapView.OnDidBecomeIdleListener { idle.countDown() }
        runOnUiThreadWithin(QUERY_TIMEOUT_MS, "adding the idle listener", Runnable {
            mapView.addOnDidBecomeIdleListener(listener)
            maplibreMap.triggerRepaint()
        })

        try {
            Assert.assertTrue(
                "The map did not become idle",
                idle.await(FRAME_TIMEOUT_MS, TimeUnit.MILLISECONDS)
            )
        } finally {
            postOnUiThread(Runnable { mapView.removeOnDidBecomeIdleListener(listener) })
                .awaitWithin(QUERY_TIMEOUT_MS, "removing the idle listener")
        }
    }
}
