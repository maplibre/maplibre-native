package org.maplibre.android.integration

import android.graphics.PointF
import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.maps.renderer.MapRenderer

/**
 * Stress tests for the render thread teardown boundary: they drive detach/re-attach cycles while
 * renderer queries are in flight, which is the interleaving in which the native side either
 * deadlocks or services a query against a half-destroyed renderer.
 *
 * **These tests are probabilistic, not deterministic.** The dangerous window is between the
 * render thread starting to tear down and the native `MapRenderer` state being reset, and it
 * cannot be opened on demand through the public API: every map call has to come from the UI thread,
 * which is the same thread that drives attach and detach, and
 * `setPreserveEGLContextOnPause(true)` means the native surface-created path is entered about
 * once per attach. The cycles below widen the window by repetition rather than by control. A pass is
 * therefore weak evidence and a failure is strong evidence - treat a flake here as a real bug, not
 * as test noise.
 *
 * Deliberately absent: `waitForIdleSync()` and `UiDevice#waitForIdle()`. Both block the
 * instrumentation thread on the UI thread with no deadline, so on the very deadlock these tests
 * look for they would hang the whole run instead of failing one test. Liveness is checked with
 * [.assertUiThreadResponsive] instead.
 *
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class SurfaceViewDetachStressTest : SurfaceViewDetachTestBase() {
    @Test
    @LargeTest
    fun rapidDetachReattachStaysResponsive() {
        validateTestSetup()

        // Each detach destroys the renderer on the render thread and each re-attach builds a new one on
        // a new render thread. Both are bounded, so a teardown that blocks the UI thread waiting on a
        // render thread that has already exited fails here instead of ANR-ing silently.
        for (i in 0..<CYCLES) {
            detachMapView()
            attachMapView()
        }

        assertUiThreadResponsive(
            SurfaceViewDetachTestBase.Companion.LIFECYCLE_TIMEOUT_MS,
            "after " + CYCLES + " detach/attach cycles"
        )
        assertMapRendersAFrame("after " + CYCLES + " detach/attach cycles")
    }

    @Test
    @LargeTest
    fun queriesRacingSurfaceRecreationDoNotDeadlock() {
        validateTestSetup()

        for (i in 0..<CYCLES) {
            detachMapView()

            // Post the re-attach and the queries as two separate messages and await only the second one.
            // The queries therefore run on the UI thread immediately behind the re-attach, while the new
            // render thread is still bringing its surface up and the native renderer may not exist yet.
            // A lock held across a render thread round trip deadlocks precisely here.
            postOnUiThread(Runnable { this.attachNow() })
            val queries = postOnUiThread(Runnable {
                Assert.assertNotNull(maplibreMap.queryRenderedFeatures(PointF(10f, 10f)))
                maplibreMap.getTileCacheEnabled()
                mapView.onLowMemory()
            })

            queries.awaitWithin(
                SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
                "queries racing surface recreation on cycle " + i
            )
        }

        assertMapRendersAFrame("after queries raced surface recreation")
    }

    @Test
    @LargeTest
    fun swapBehaviorFlushRacingDetachReattach() {
        validateTestSetup()

        // setSwapBehaviorFlush touches the native backend without going through the renderer actor, so
        // it races the backend being reset during teardown from a different angle than the queries do.
        for (i in 0..<CYCLES) {
            val flush = (i % 2 == 0)

            detachMapView()
            postOnUiThread(Runnable { this.attachNow() })
            postOnUiThread(Runnable { maplibreMap.setSwapBehaviorFlush(flush) })
                .awaitWithin(
                    SurfaceViewDetachTestBase.Companion.LIFECYCLE_TIMEOUT_MS,
                    "setSwapBehaviorFlush racing re-attach on cycle " + i
                )
        }

        assertUiThreadResponsive(
            SurfaceViewDetachTestBase.Companion.LIFECYCLE_TIMEOUT_MS,
            "after swap behavior races"
        )
        assertMapRendersAFrame("after swap behavior races")
    }

    @Test
    @LargeTest
    fun continuousRenderingAcrossDetachReattach() {
        validateTestSetup()

        // Continuous mode keeps the render thread drawing right up to the detach, so teardown happens
        // with work already queued in the render thread's event queue - the queue that
        // MapLibreGLSurfaceView#guardedRun drops on exit without draining.
        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.LIFECYCLE_TIMEOUT_MS,
            "switching to continuous rendering",
            Runnable { mapView.setRenderingRefreshMode(MapRenderer.RenderingRefreshMode.CONTINUOUS) })

        try {
            for (i in 0..<CYCLES) {
                detachMapView()
                attachMapView()
            }

            assertUiThreadResponsive(
                SurfaceViewDetachTestBase.Companion.LIFECYCLE_TIMEOUT_MS,
                "after continuous rendering cycles"
            )
            assertMapRendersAFrame("after continuous rendering cycles")
        } finally {
            runOnUiThreadWithin(
                SurfaceViewDetachTestBase.Companion.LIFECYCLE_TIMEOUT_MS,
                "restoring when-dirty rendering",
                Runnable { mapView.setRenderingRefreshMode(MapRenderer.RenderingRefreshMode.WHEN_DIRTY) })
        }
    }

    companion object {
        private const val CYCLES = 25
    }
}
