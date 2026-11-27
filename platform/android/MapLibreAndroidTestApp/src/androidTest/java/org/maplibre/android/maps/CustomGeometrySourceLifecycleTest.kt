package org.maplibre.android.maps

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.MapLibre
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.style.sources.CustomGeometrySource
import org.maplibre.android.style.sources.GeometryTileProvider
import org.maplibre.geojson.FeatureCollection
import java.util.concurrent.CopyOnWriteArrayList
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/** Exercises native peer teardown with a provider that finishes after shutdownNow(). */
@RunWith(AndroidJUnit4ClassRunner::class)
class CustomGeometrySourceLifecycleTest {
    private val instrumentation = InstrumentationRegistry.getInstrumentation()
    private val entered = CountDownLatch(1)
    private val resume = CountDownLatch(1)
    private val failures = CopyOnWriteArrayList<Throwable>()
    private lateinit var nativeMap: NativeMapView
    private lateinit var style: Style
    private lateinit var source: CustomGeometrySource
    private lateinit var worker: Thread
    private var destroyed = false

    @Before
    fun setUp() {
        instrumentation.runOnMainSync {
            val context = instrumentation.targetContext
            MapLibre.getInstance(context)
            nativeMap = NativeMapView(context, NativeMapOptions(1.0f, false), null, null, NativeMapViewTest.DummyRenderer(context))
            nativeMap.styleJson = Style.EMPTY_JSON
            style = Style.Builder().build(nativeMap)
            style.onDidFinishLoadingStyle()
            source =
                CustomGeometrySource(
                    "teardown-test",
                    object : GeometryTileProvider {
                        override fun getFeaturesForBounds(
                            bounds: LatLngBounds,
                            zoomLevel: Int,
                        ): FeatureCollection {
                            worker = Thread.currentThread()
                            worker.uncaughtExceptionHandler = Thread.UncaughtExceptionHandler { _, failure -> failures.add(failure) }
                            entered.countDown()
                            val deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(10)
                            while (true) {
                                try {
                                    check(resume.await(deadline - System.nanoTime(), TimeUnit.NANOSECONDS))
                                    break
                                } catch (_: InterruptedException) {
                                    // CPU-bound application providers can finish despite interruption.
                                }
                            }
                            return FeatureCollection.fromFeatures(emptyList())
                        }
                    },
                )
            style.addSource(source)
            // Trigger the real tile request without requiring a rendered frame or network style.
            CustomGeometrySource::class.java
                .getDeclaredMethod(
                    "fetchTile",
                    Int::class.javaPrimitiveType,
                    Int::class.javaPrimitiveType,
                    Int::class.javaPrimitiveType,
                ).apply { isAccessible = true }
                .invoke(source, 0, 0, 0)
        }
        assertTrue("Provider did not start", entered.await(5, TimeUnit.SECONDS))
    }

    @After
    fun tearDown() {
        resume.countDown()
        instrumentation.runOnMainSync {
            if (!destroyed && ::nativeMap.isInitialized) {
                style.clear()
                nativeMap.destroy()
                destroyed = true
            }
        }
        if (::worker.isInitialized) {
            worker.join(5_000)
            assertFalse("Tile worker leaked after teardown", worker.isAlive)
        }
        assertTrue("Late tile request failed: $failures", failures.isEmpty())
    }

    @Test
    fun providerCanFinishAfterSourceRemoval() {
        instrumentation.runOnMainSync { assertTrue(style.removeSource(source)) }
        finishProvider()
    }

    @Test
    fun providerCanFinishAfterStyleReplacement() {
        instrumentation.runOnMainSync {
            style.clear()
            nativeMap.styleJson = Style.EMPTY_JSON
        }
        finishProvider()
    }

    @Test
    fun providerCanFinishAfterMapDestruction() {
        instrumentation.runOnMainSync {
            // MapLibreMap.onDestroy() clears the style before NativeMapView.destroy().
            style.clear()
            nativeMap.destroy()
            destroyed = true
        }
        finishProvider()
    }

    @Test
    fun oldProviderCanFinishAfterSourceReattachment() {
        instrumentation.runOnMainSync {
            assertTrue(style.removeSource(source))
            style.addSource(source)
        }
        finishProvider()
        instrumentation.runOnMainSync { assertEquals("teardown-test", source.id) }
    }

    private fun finishProvider() {
        resume.countDown()
        worker.join(5_000)
        assertFalse("Retired tile worker did not stop", worker.isAlive)
        assertTrue("Late tile request failed: $failures", failures.isEmpty())
    }
}
