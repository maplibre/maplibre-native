package org.maplibre.android.testapp.projection

import android.graphics.Bitmap
import android.graphics.Color
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
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.cos
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
        // The core test projection's answers for the same crosshairs, where the render test
        // `projection/national-grid/crosshairs` puts its rings, and its forward of the guide's Helmert example at sea
        // level, 422297.7979 E 412878.7399 N.
        val crosshairs = listOf(
            LatLng(53.368860650404621, -3.347702452627864),
            LatLng(52.407393940926873, -1.2239561059133772),
            LatLng(50.711902262780129, 0.9941609476954022),
            LatLng(51.622594999864035, -3.8130780437971459)
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
    fun inverseUndoesForward() {
        // A camera move takes its anchor to lat/lng and back on every step, so any gap between the two adds up over a
        // pinch.
        val positions = mutableListOf<Double>()
        for (easting in 0..700000 step 50000) {
            for (northing in 0..1250000 step 50000) {
                positions += easting.toDouble()
                positions += northing.toDouble()
            }
        }
        val count = positions.size / 2
        val projection = NationalGrid.coordinateReferenceSystem.projection
        val lngLats = DoubleArray(2 * count)
        val again = DoubleArray(2 * count)
        val lngLatsAgain = DoubleArray(2 * count)
        projection.inverse(positions.toDoubleArray(), lngLats, count)
        projection.forward(lngLats, again, count)
        projection.inverse(again, lngLatsAgain, count)
        for (i in 0 until count) {
            val (x, y) = 2 * i to 2 * i + 1
            val where = "${positions[x]} E ${positions[y]} N"
            val missed = hypot(again[x] - positions[x], again[y] - positions[y])
            assertTrue("forward(inverse(p)) is $missed m from p at $where", missed < MICROMETER)
            val latitude = lngLats[y] * PI / 180
            val moved = hypot((lngLatsAgain[x] - lngLats[x]) * cos(latitude), lngLatsAgain[y] - lngLats[y]) *
                METERS_PER_DEGREE
            assertTrue("inverse(forward(q)) is $moved m from q at $where", moved < MICROMETER)
        }
    }

    @Test
    fun ringSetFromKotlinLandsOnItsCrosshair() {
        val pixelsPerDp = rule.activity.resources.displayMetrics.density
        val window = (WINDOW_DP * pixelsPerDp).toInt()
        val armGap = ARM_GAP_DP * pixelsPerDp
        var worst = 0.0
        for ((easting, northing) in CROSSHAIRS) {
            val (latitude, longitude) = NationalGrid.wgs84FromEastingNorthing(easting, northing)
            val crosshair = LatLng(latitude, longitude)
            awaitRingAt(crosshair)
            val image = snapshot()
            val expected = onMain { maplibreMap.projection.toScreenLocation(crosshair) }
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
     * Centers the map on a crosshair at [ZOOM], so the square measured around it is on screen whatever the map's size,
     * and waits until the ring the activity sets from Kotlin there is drawn.
     */
    private fun awaitRingAt(crosshair: LatLng) {
        val latch = CountDownLatch(1)
        val onIdle = MapView.OnDidBecomeIdleListener {
            val where = maplibreMap.projection.toScreenLocation(crosshair)
            if (maplibreMap.queryRenderedFeatures(where, RINGS_LAYER).isNotEmpty()) {
                latch.countDown()
            }
        }
        rule.runOnUiThread {
            mapView.addOnDidBecomeIdleListener(onIdle)
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(crosshair, ZOOM))
            maplibreMap.triggerRepaint()
        }
        val drawn = latch.await(WAIT_SECONDS, TimeUnit.SECONDS)
        rule.runOnUiThread { mapView.removeOnDidBecomeIdleListener(onIdle) }
        assertTrue("The ring at $crosshair is drawn", drawn)
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

        /** The core fixture's zoom, where a tile's pixel is a point on the screen. */
        const val ZOOM = 2.0

        const val MICROMETER = 1e-6

        const val METERS_PER_DEGREE = 6371008.8 * PI / 180

        /** Half the side of the square around a crosshair that is measured, and the crosshair's arms are 20 dp. */
        const val WINDOW_DP = 30

        /** How far from the crosshair's center its arms count, so each arm measures one axis. */
        const val ARM_GAP_DP = 4f
    }
}
