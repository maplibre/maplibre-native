package org.maplibre.android.style.sources

import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Test
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.geojson.FeatureCollection
import java.util.concurrent.CopyOnWriteArrayList
import java.util.concurrent.CountDownLatch
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger

class CustomGeometrySourceTileRequestsTest {
    private val pools = CopyOnWriteArrayList<ThreadPoolExecutor>()
    private val gates = CopyOnWriteArrayList<CountDownLatch>()
    private val failures = CopyOnWriteArrayList<Throwable>()
    private val submitted = CopyOnWriteArrayList<FeatureCollection>()
    private val data = FeatureCollection.fromFeatures(emptyList())
    private lateinit var requests: CustomGeometrySourceTileRequests

    @After
    fun tearDown() {
        gates.forEach { it.countDown() }
        if (::requests.isInitialized) requests.release()
        pools.forEach {
            it.shutdownNow()
            assertTrue("Tile worker did not stop", it.awaitTermination(5, TimeUnit.SECONDS))
        }
        assertTrue("Unexpected worker failures: $failures", failures.isEmpty())
    }

    @Test
    fun releaseDropsResultEvenWhenProviderIgnoresInterruption() {
        val entered = gate()
        val resume = gate()
        createRequests {
            entered.countDown()
            awaitIgnoringInterrupts(resume)
            data
        }
        requests.start()
        requests.fetchTile(0, 0, 0)
        await(entered)

        requests.release()
        assertFalse("Release must not wait for geometry generation", pools.single().isTerminated)
        resume.countDown()
        assertTrue(pools.single().awaitTermination(5, TimeUnit.SECONDS))
        assertTrue(submitted.isEmpty())
    }

    @Test
    fun releaseWaitsForNativeSubmissionAlreadyInProgress() {
        val submitting = gate()
        val resume = gate()
        val releasing = gate()
        val released = gate()
        createRequests(submit = { collection ->
            submitting.countDown()
            awaitIgnoringInterrupts(resume)
            submitted.add(collection)
        }) { data }
        requests.start()
        requests.fetchTile(0, 0, 0)
        await(submitting)
        val releaseThread =
            Thread {
                releasing.countDown()
                requests.release()
                released.countDown()
            }
        releaseThread.start()
        try {
            await(releasing)
            assertFalse("Native submission must hold off teardown", released.await(100, TimeUnit.MILLISECONDS))
            resume.countDown()
            await(released)
            assertEquals(listOf(data), submitted)
        } finally {
            resume.countDown()
            releaseThread.join(5_000)
        }
    }

    @Test
    fun releaseDiscardsCoalescedRequestWithoutResubmittingToStoppedExecutor() {
        val entered = gate()
        val resume = gate()
        val calls = AtomicInteger()
        createRequests {
            calls.incrementAndGet()
            entered.countDown()
            awaitIgnoringInterrupts(resume)
            data
        }
        requests.start()
        requests.fetchTile(0, 0, 0)
        await(entered)
        requests.fetchTile(0, 0, 0)
        requests.release()
        resume.countDown()

        assertTrue(pools.single().awaitTermination(5, TimeUnit.SECONDS))
        assertEquals(1, calls.get())
        assertTrue(submitted.isEmpty())
    }

    @Test
    fun oldRequestCannotSubmitOrRemoveNewRequestAfterRestart() {
        val oldEntered = gate()
        val newEntered = gate()
        val oldResume = gate()
        val newResume = gate()
        val delivered = gate()
        val calls = AtomicInteger()
        createRequests(submit = { collection ->
            assertFalse(requests.isCancelled(0, 0, 0))
            submitted.add(collection)
            delivered.countDown()
        }) {
            if (calls.getAndIncrement() == 0) {
                oldEntered.countDown()
                awaitIgnoringInterrupts(oldResume)
            } else {
                newEntered.countDown()
                awaitIgnoringInterrupts(newResume)
            }
            data
        }
        requests.start()
        requests.fetchTile(0, 0, 0)
        await(oldEntered)
        requests.release()
        requests.start()
        requests.fetchTile(0, 0, 0)
        await(newEntered)

        oldResume.countDown()
        assertTrue(pools.first().awaitTermination(5, TimeUnit.SECONDS))
        assertTrue(submitted.isEmpty())
        assertFalse(requests.isCancelled(0, 0, 0))

        newResume.countDown()
        await(delivered)
        assertEquals(listOf(data), submitted)
    }

    @Test
    fun cancellingActiveRequestStillAllowsLatestCoalescedRequest() {
        val entered = gate()
        val resume = gate()
        val delivered = gate()
        val calls = AtomicInteger()
        createRequests(submit = { collection ->
            submitted.add(collection)
            delivered.countDown()
        }) {
            if (calls.incrementAndGet() == 1) {
                entered.countDown()
                awaitIgnoringInterrupts(resume)
            }
            data
        }
        requests.start()
        requests.fetchTile(0, 0, 0)
        await(entered)
        requests.fetchTile(0, 0, 0)
        requests.fetchTile(0, 0, 0)
        requests.cancelTile(0, 0, 0)
        resume.countDown()

        await(delivered)
        assertEquals(2, calls.get())
        assertEquals(listOf(data), submitted)
    }

    @Test
    fun callsBeforeStartAndAfterReleaseAreHarmless() {
        createRequests {
            fail("Provider should not run")
            data
        }
        requests.release()
        requests.fetchTile(0, 0, 0)
        requests.cancelTile(0, 0, 0)
        requests.setTileData(0, 0, 0, data)
        assertTrue(requests.isCancelled(0, 0, 0))
        requests.start()
        requests.release()
        requests.release()
        requests.fetchTile(0, 0, 0)
        requests.setTileData(0, 0, 0, data)
        assertTrue(submitted.isEmpty())
    }

    private fun createRequests(
        submit: (FeatureCollection) -> Unit = { submitted.add(it) },
        provide: () -> FeatureCollection,
    ) {
        requests =
            CustomGeometrySourceTileRequests(
                provider =
                    object : GeometryTileProvider {
                        override fun getFeaturesForBounds(
                            bounds: LatLngBounds,
                            zoomLevel: Int,
                        ) = provide()
                    },
                submit = { _, _, _, collection -> submit(collection) },
                createExecutor = {
                    ThreadPoolExecutor(1, 1, 0, TimeUnit.MILLISECONDS, LinkedBlockingQueue()) { runnable ->
                        Thread(runnable).apply {
                            uncaughtExceptionHandler = Thread.UncaughtExceptionHandler { _, failure -> failures.add(failure) }
                        }
                    }.also { pools.add(it) }
                },
            )
    }

    private fun gate() = CountDownLatch(1).also { gates.add(it) }

    private fun await(latch: CountDownLatch) {
        assertTrue("Timed out waiting for worker", latch.await(5, TimeUnit.SECONDS))
    }

    private fun awaitIgnoringInterrupts(latch: CountDownLatch) {
        val deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(5)
        while (true) {
            try {
                assertTrue("Timed out waiting for test", latch.await(deadline - System.nanoTime(), TimeUnit.NANOSECONDS))
                return
            } catch (_: InterruptedException) {
                // A provider may finish CPU work despite shutdownNow(). Exercise that case explicitly.
            }
        }
    }
}
