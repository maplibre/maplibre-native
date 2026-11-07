package org.maplibre.android.integration

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import com.google.gson.JsonObject
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.style.sources.GeoJsonSource

/**
 * Regression tests for renderer queries issued after a SurfaceView backed MapView has been
 * detached from its window.
 *
 *
 * Detaching runs `MapLibreSurfaceView#onDetachedFromWindow`, which calls `nativeReset()`
 * and destroys the native `Renderer` on the render thread before the thread itself exits.
 * Every entry point below then reaches `AndroidRendererFrontend`, which historically used
 * `MapRenderer::actor()` without checking whether the renderer still existed.
 *
 *
 * See:
 *
 *  * [#4549](https://github.com/maplibre/maplibre-native/issues/4549) -
 * `reset()` left `rendererRef` dangling, so these calls ran against a destroyed
 * `Renderer`.
 *  * [#4108](https://github.com/maplibre/maplibre-native/issues/4108) - the
 * not-yet-initialised variant of the same dereference.
 *
 *
 *
 * These tests deliberately assert only that each call returns, returns within a deadline, and does
 * not throw. The values are not specified: with the renderer gone an empty result is correct, but
 * pinning that down would freeze an implementation detail. A native dereference of a destroyed
 * renderer kills the process (reported as a crashed run), and a wedged mailbox shows up as the
 * deadline being missed.
 *
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class SurfaceViewDetachQueryTest : SurfaceViewDetachTestBase() {
    @Test
    @LargeTest
    fun queryRenderedFeaturesAfterDetach() {
        validateTestSetup()
        detachMapView()

        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
            "queryRenderedFeatures after detach",
            Runnable {
                Assert.assertNotNull(maplibreMap.queryRenderedFeatures(PointF(10f, 10f)))
                Assert.assertNotNull(maplibreMap.queryRenderedFeatures(RectF(0f, 0f, 50f, 50f)))
            })
    }

    @Test
    @LargeTest
    fun tileCacheApiAfterDetach() {
        validateTestSetup()
        detachMapView()

        // MapLibreMap#getTileCacheEnabled is a blocking ask() on the render thread, and
        // #setTileCacheEnabled an invoke(); both need the renderer's absence handled natively.
        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
            "tile cache API after detach",
            Runnable {
                maplibreMap.setTileCacheEnabled(false)
                maplibreMap.getTileCacheEnabled()
                maplibreMap.setTileCacheEnabled(true)
            })
    }

    @Test
    @LargeTest
    fun lowMemoryAfterDetach() {
        validateTestSetup()
        detachMapView()

        // Reaches Renderer::reduceMemoryUse through the frontend actor. Listed in #4549 as one of the
        // reachable call sites, and the one an app hits without any user interaction at all.
        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS, "onLowMemory after detach",
            Runnable { mapView.onLowMemory() })
    }

    @Test
    @LargeTest
    fun sourceQueriesAfterDetach() {
        validateTestSetup()

        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
            "adding the test source",
            Runnable {
                val source: GeoJsonSource = GeoJsonSource(SOURCE_ID, FEATURE_COLLECTION)
                maplibreMap.getStyle()!!.addSource(source)
            })

        detachMapView()

        // querySourceFeatures, setFeatureState and getFeatureState each route straight through
        // AndroidRendererFrontend's actor, same as the rendered feature queries above.
        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
            "source queries after detach",
            Runnable {
                val source = maplibreMap.getStyle()!!.getSource(SOURCE_ID) as GeoJsonSource?
                Assert.assertNotNull("The test source disappeared from the style", source)

                Assert.assertNotNull(source!!.querySourceFeatures(null))

                val state = JsonObject()
                state.addProperty("hovered", true)
                source.setFeatureState(FEATURE_ID, state)
                source.getFeatureState(FEATURE_ID)
            })
    }

    @Test
    @LargeTest
    fun snapshotAfterDetach() {
        validateTestSetup()
        detachMapView()

        // requestSnapshot posts through the same mailbox. The callback is not expected to fire with no
        // renderer, so only the call itself is bounded here.
        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
            "requesting a snapshot after detach",
            Runnable { maplibreMap.snapshot(MapLibreMap.SnapshotReadyCallback { snapshot: Bitmap? -> }) })
    }

    @Test
    @LargeTest
    fun queriesWorkAgainAfterReattach() {
        validateTestSetup()

        detachMapView()
        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS, "querying while detached",
            Runnable { Assert.assertNotNull(maplibreMap.queryRenderedFeatures(PointF(10f, 10f))) })

        attachMapView()

        // The renderer is rebuilt on a fresh render thread with a fresh mailbox. If the mailbox from
        // before the detach was left closed or wedged, the blocking query below never returns and the
        // repaint below never produces a frame - both failures are silent without these assertions.
        assertMapRendersAFrame("after re-attaching the MapView")

        runOnUiThreadWithin(
            SurfaceViewDetachTestBase.Companion.QUERY_TIMEOUT_MS,
            "querying after re-attach",
            Runnable {
                Assert.assertNotNull(maplibreMap.queryRenderedFeatures(PointF(10f, 10f)))
                maplibreMap.getTileCacheEnabled()
            })
    }

    companion object {
        private const val SOURCE_ID = "detach-test-source"
        private const val FEATURE_ID = "detach-test-feature"
        private val FEATURE_COLLECTION = ("{\"type\":\"FeatureCollection\",\"features\":["
                + "{\"type\":\"Feature\",\"id\":\"" + FEATURE_ID + "\",\"properties\":{},"
                + "\"geometry\":{\"type\":\"Point\",\"coordinates\":[0.0,0.0]}}]}")
    }
}
