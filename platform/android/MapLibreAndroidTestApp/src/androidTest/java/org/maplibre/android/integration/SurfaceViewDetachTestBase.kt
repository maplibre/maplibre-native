package org.maplibre.android.integration

import android.os.Handler
import android.os.Looper
import android.view.ViewGroup
import androidx.annotation.UiThread
import org.junit.After
import org.junit.Assert
import org.maplibre.android.maps.MapView.OnDidFinishRenderingFrameListener
import org.maplibre.android.testapp.activity.EspressoTest
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference

/**
 * Shared scaffolding for the SurfaceView detach regression tests.
 *
 *
 * Detaching a [MapView] from its window tears down the render thread and, through
 * `MapLibreSurfaceView#onDetachedFromWindow`, calls `nativeReset()` on the native
 * `MapRenderer`. Everything in this package exercises that boundary.
 *
 *
 * The important property of this base class is that it *never* blocks the instrumentation
 * thread on the UI thread without a timeout. A regression in the native teardown path typically
 * shows up as a blocked UI thread (a blocking `ask().wait()` that nobody services, or a
 * mutex held across a render thread round trip), and a plain `runOnMainSync` would hang the
 * whole test run instead of failing a single test. Work is therefore posted to the main looper and
 * awaited with a deadline.
 *
 */
abstract class SurfaceViewDetachTestBase : EspressoTest() {
    private val uiHandler = Handler(Looper.getMainLooper())

    private var mapParent: ViewGroup? = null

    /**
     * Removes the MapView from its parent, which detaches it from the window and triggers the
     * native renderer reset. Must complete within [.LIFECYCLE_TIMEOUT_MS]: a detach that
     * blocks is an ANR in a real app.
     */
    protected fun detachMapView() {
        postOnUiThread(Runnable { this.detachNow() }).awaitWithin(
            LIFECYCLE_TIMEOUT_MS,
            "detaching the MapView"
        )
    }

    /**
     * Re-adds the MapView to the parent it was detached from, which starts a fresh render thread.
     */
    protected fun attachMapView() {
        postOnUiThread(Runnable { this.attachNow() }).awaitWithin(
            LIFECYCLE_TIMEOUT_MS,
            "re-attaching the MapView"
        )
    }

    @UiThread
    protected fun detachNow() {
        val parent = mapView.getParent() as ViewGroup?
        if (parent == null) {
            // Already detached, nothing to do.
            return
        }
        mapParent = parent
        mapParent!!.removeView(mapView)
    }

    @UiThread
    protected fun attachNow() {
        if (mapView.getParent() != null) {
            return
        }
        Assert.assertNotNull("attachNow() called before the MapView was ever detached", mapParent)
        mapParent!!.addView(mapView, 0)
    }

    /**
     * Fails unless the UI thread picks up and runs a no-op within the given deadline. Used to tell a
     * deadlocked UI thread apart from a test that is merely slow.
     */
    protected fun assertUiThreadResponsive(timeoutMs: Long, context: String) {
        postOnUiThread(Runnable {}).awaitWithin(
            timeoutMs,
            "a no-op posted to the UI thread (" + context + ")"
        )
    }

    /**
     * Asks the map to repaint and fails unless a frame is actually rendered.
     *
     *
     * This is the assertion that catches a silently broken renderer: if the native mailbox is
     * wedged or the renderer was never rebuilt after a re-attach, no frame ever arrives even though
     * every call still returns successfully.
     *
     */
    protected fun assertMapRendersAFrame(context: String) {
        val frameRendered = CountDownLatch(1)
        val listener =
            OnDidFinishRenderingFrameListener { fully: Boolean, frameEncodingTime: Double, frameRenderingTime: Double -> frameRendered.countDown() }

        postOnUiThread(Runnable {
            mapView.addOnDidFinishRenderingFrameListener(listener)
            maplibreMap.triggerRepaint()
        }).awaitWithin(QUERY_TIMEOUT_MS, "requesting a repaint (" + context + ")")

        try {
            val rendered = frameRendered.await(FRAME_TIMEOUT_MS, TimeUnit.MILLISECONDS)
            if (!rendered) {
                Assert.fail(
                    ("The map did not render a frame within " + FRAME_TIMEOUT_MS + " ms (" + context
                            + "). The renderer was not rebuilt, or the native mailbox is no longer being drained.")
                )
            }
        } catch (interrupted: InterruptedException) {
            Thread.currentThread().interrupt()
            Assert.fail("Interrupted while waiting for a frame (" + context + ")")
        } finally {
            postOnUiThread(Runnable { mapView.removeOnDidFinishRenderingFrameListener(listener) })
                .awaitWithin(QUERY_TIMEOUT_MS, "removing the frame listener (" + context + ")")
        }
    }

    /**
     * Posts work to the UI thread without waiting for it. Await the returned handle with
     * [UiTask.awaitWithin].
     *
     *
     * Posting and awaiting separately matters: it lets a test queue renderer queries directly behind
     * a re-attach, so they run while the new render thread is still bringing its surface up. That is
     * the interleaving in which a lock held across a render thread round trip deadlocks.
     *
     */
    protected fun postOnUiThread(action: Runnable): UiTask {
        val task = UiTask()
        Assert.assertTrue("Could not post to the main looper", uiHandler.post(Runnable {
            try {
                action.run()
            } catch (throwable: Throwable) {
                task.failure.set(throwable)
            } finally {
                task.done.countDown()
            }
        }))
        return task
    }

    /** Posts work to the UI thread and waits for it to finish, or fails.  */
    protected fun runOnUiThreadWithin(timeoutMs: Long, what: String, action: Runnable) {
        postOnUiThread(action).awaitWithin(timeoutMs, what)
    }

    /**
     * Best effort re-attach so the Activity teardown sees the view hierarchy it expects. Failures
     * are ignored on purpose: if the UI thread is wedged, the test body has already reported why,
     * and a second failure here would only obscure it.
     */
    @After
    fun reattachMapView() {
        val reattach = postOnUiThread(Runnable { this.attachNow() })
        try {
            reattach.done.await(LIFECYCLE_TIMEOUT_MS, TimeUnit.MILLISECONDS)
        } catch (interrupted: InterruptedException) {
            Thread.currentThread().interrupt()
        }
    }

    /** Handle for work posted to the UI thread.  */
    protected class UiTask {
        internal val done = CountDownLatch(1)
        internal val failure = AtomicReference<Throwable?>()

        /**
         * Fails if the work did not finish within the deadline, or rethrows whatever it threw.
         */
        fun awaitWithin(timeoutMs: Long, what: String) {
            val completed: Boolean
            try {
                completed = done.await(timeoutMs, TimeUnit.MILLISECONDS)
            } catch (interrupted: InterruptedException) {
                Thread.currentThread().interrupt()
                throw AssertionError("Interrupted while waiting for " + what)
            }

            if (!completed) {
                Assert.fail(
                    (what + " did not complete within " + timeoutMs + " ms. The UI thread is blocked; the"
                            + " usual causes are a native ask() whose render thread never services it, or a mutex"
                            + " held across a render thread round trip.")
                )
            }

            val throwable = failure.get()
            if (throwable != null) {
                throw AssertionError(what + " failed", throwable)
            }
        }
    }

    companion object {
        /** Budget for a single detach or re-attach. Both are expected to be non-blocking.  */
        const val LIFECYCLE_TIMEOUT_MS: Long = 5000L

        /** Budget for a batch of renderer queries issued from the UI thread.  */
        const val QUERY_TIMEOUT_MS: Long = 5000L

        /** Budget for the map to produce a frame once it has been asked to repaint.  */
        const val FRAME_TIMEOUT_MS: Long = 15000L
    }
}
