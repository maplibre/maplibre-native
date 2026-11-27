package org.maplibre.android.testapp.projection

import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.RectF
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.activity.BaseTest
import org.maplibre.android.testapp.activity.projection.NationalGridActivity
import org.maplibre.android.testapp.activity.projection.NationalGridActivity.Companion.CROSSHAIRS
import org.maplibre.android.testapp.activity.projection.NationalGridActivity.Companion.RINGS_LAYER
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import kotlin.math.abs
import kotlin.math.hypot
import kotlin.math.max
import kotlin.math.min

/**
 * The British National Grid registered from Kotlin: GeoJSON set from Kotlin at each crosshair's WGS 84 position lands
 * on the crosshair the tiles carry, as in the core render test `projection/national-grid/crosshairs`.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class NationalGridTest : BaseTest() {

    override fun getActivityClass(): Class<*> = NationalGridActivity::class.java

    @Test
    fun convertsAsTheCoreTestProjectionDoes() {
        // The core test projection's answers for the same crosshairs (`w11/fixture-points.json` in the core fixture's
        // notes), and its forward of the guide's Helmert example at sea level, 422297.7979 E 412878.7399 N.
        val crosshairs = listOf(
            LatLng(53.368860648334653, -3.3477024864059799),
            LatLng(52.407393941121825, -1.2239561421480576),
            LatLng(50.71190226649788, 0.99416090786370237),
            LatLng(51.62259499994537, -3.8130780753904916)
        )
        for ((crosshair, expected) in CROSSHAIRS.zip(crosshairs)) {
            val (latitude, longitude) = NationalGrid.wgs84FromEastingNorthing(crosshair.first, crosshair.second)
            assertEquals(expected.latitude, latitude, 1e-9)
            assertEquals(expected.longitude, longitude, 1e-9)
        }
        val (easting, northing) = NationalGrid.eastingNorthingFromWgs84(
            53.0 + 36.0 / 60 + 43.1653 / 3600,
            -(1.0 + 39.0 / 60 + 51.9920 / 3600)
        )
        assertEquals(422297.7979, easting, 1e-3)
        assertEquals(412878.7399, northing, 1e-3)
    }

    @Test
    fun ringSetFromKotlinLandsOnItsCrosshair() {
        awaitRings()
        val image = snapshot()
        val pixelsPerDp = rule.activity.resources.displayMetrics.density
        val window = (WINDOW_DP * pixelsPerDp).toInt()
        val armGap = ARM_GAP_DP * pixelsPerDp
        var worst = 0.0
        for ((easting, northing) in CROSSHAIRS) {
            val (latitude, longitude) = NationalGrid.wgs84FromEastingNorthing(easting, northing)
            val expected = onMain { maplibreMap.projection.toScreenLocation(LatLng(latitude, longitude)) }
            val ex = expected.x.toDouble()
            val ey = expected.y.toDouble()
            var vertical = Centroid()
            var horizontal = Centroid()
            var ring = Centroid()
            for (y in max(0, ey.toInt() - window)..min(image.height - 1, ey.toInt() + window)) {
                for (x in max(0, ex.toInt() - window)..min(image.width - 1, ex.toInt() + window)) {
                    val color = image.getPixel(x, y)
                    val r = Color.red(color)
                    val g = Color.green(color)
                    val b = Color.blue(color)
                    val red = max(0, r - max(g, b)).toDouble()
                    val blue = max(0, b - max(r, g)).toDouble()
                    val cx = x + 0.5
                    val cy = y + 0.5
                    if (abs(cy - ey) > armGap) vertical = vertical.add(cx, cy, red)
                    if (abs(cx - ex) > armGap) horizontal = horizontal.add(cx, cy, red)
                    ring = ring.add(cx, cy, blue)
                }
            }
            assertTrue("The crosshair at $easting E $northing N is drawn", vertical.weight > 0 && horizontal.weight > 0)
            assertTrue("The ring at $easting E $northing N is drawn", ring.weight > 0)
            worst = max(worst, hypot(ring.x - vertical.x, ring.y - horizontal.y) / pixelsPerDp)
        }
        // In logical pixels, as the core test measures: raster tiles snap to their grid, which can move them by half
        // of one.
        assertTrue("Each ring is within 1 logical pixel of its crosshair, the farthest $worst", worst <= 1)
    }

    private class Centroid(val sumX: Double = 0.0, val sumY: Double = 0.0, val weight: Double = 0.0) {
        val x get() = sumX / weight
        val y get() = sumY / weight
        fun add(x: Double, y: Double, w: Double) = Centroid(sumX + x * w, sumY + y * w, weight + w)
    }

    /**
     * Centers the map on the crosshairs at the core fixture's zoom, where a tile's pixel is a point on the screen,
     * and waits until the rings the activity sets from Kotlin are drawn.
     */
    private fun awaitRings() {
        val latch = CountDownLatch(1)
        val onIdle = MapView.OnDidBecomeIdleListener {
            val view = RectF(0f, 0f, mapView.width.toFloat(), mapView.height.toFloat())
            if (maplibreMap.queryRenderedFeatures(view, RINGS_LAYER).size >= CROSSHAIRS.size) {
                latch.countDown()
            }
        }
        val eastings = CROSSHAIRS.map { it.first }
        val northings = CROSSHAIRS.map { it.second }
        val (latitude, longitude) = NationalGrid.wgs84FromEastingNorthing(
            (eastings.min() + eastings.max()) / 2,
            (northings.min() + northings.max()) / 2
        )
        rule.runOnUiThread {
            mapView.addOnDidBecomeIdleListener(onIdle)
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(latitude, longitude), 2.0))
            maplibreMap.triggerRepaint()
        }
        val drawn = latch.await(WAIT_SECONDS, TimeUnit.SECONDS)
        rule.runOnUiThread { mapView.removeOnDidBecomeIdleListener(onIdle) }
        assertTrue("The rings are drawn", drawn)
    }

    private fun snapshot(): Bitmap {
        val latch = CountDownLatch(1)
        var image: Bitmap? = null
        rule.runOnUiThread {
            maplibreMap.snapshot {
                image = it
                latch.countDown()
            }
        }
        assertTrue("The snapshot is taken", latch.await(WAIT_SECONDS, TimeUnit.SECONDS))
        return image!!
    }

    private fun <T> onMain(block: () -> T): T {
        var result: Result<T>? = null
        rule.runOnUiThread { result = runCatching(block) }
        return result!!.getOrThrow()
    }

    private companion object {
        const val WAIT_SECONDS = 30L

        /** Half the side of the square around a crosshair that is measured, and the crosshair's arms are 20 dp. */
        const val WINDOW_DP = 30

        /** How far from the crosshair's center its arms count, so each arm measures one axis. */
        const val ARM_GAP_DP = 4f
    }
}
