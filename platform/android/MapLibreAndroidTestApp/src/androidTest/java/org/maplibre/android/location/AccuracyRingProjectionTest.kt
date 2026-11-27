package org.maplibre.android.location

import android.location.Location
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.activity.BaseTest
import org.maplibre.android.testapp.activity.projection.NationalGridActivity
import org.maplibre.android.testapp.projection.NationalGrid
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * The accuracy ring of the symbol location layer measures at the location, which a transverse Mercator's scale depends
 * on through the longitude too.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class AccuracyRingProjectionTest : BaseTest() {

    override fun getActivityClass(): Class<*> = NationalGridActivity::class.java

    @Test
    fun ringInMercatorDependsOnTheLatitudeAlone() {
        loadStyle("mercator")
        val location = location(51.0, 3.0)
        val (radius, metersPerPixel) = onMain {
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(50.0, 0.0), 5.0))
            Utils.calculateZoomLevelRadius(maplibreMap, location) to
                maplibreMap.projection.getMetersPerPixelAtLatitude(location.latitude)
        }
        assertEquals((location.accuracy * (1 / metersPerPixel)).toFloat(), radius)
    }

    @Test
    fun ringInATransverseMercatorMeasuresAtTheLocation() {
        loadStyle(NationalGrid.NAME)
        val location = location(54.0, 2.0)
        val zoom = 8.0
        val metersPerPixelAt = { longitude: Double ->
            onMain {
                maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(location.latitude, longitude), zoom))
                maplibreMap.projection.getMetersPerPixelAtLatitude(location.latitude)
            }
        }
        val atLocation = metersPerPixelAt(location.longitude)
        val onCentralMeridian = metersPerPixelAt(-2.0)
        assertNotEquals(atLocation, onCentralMeridian, atLocation * 1e-4)

        // The camera is on the central meridian, and the ring is measured four degrees east of it.
        val radius = onMain { Utils.calculateZoomLevelRadius(maplibreMap, location) }
        assertEquals(location.accuracy / atLocation, radius.toDouble(), location.accuracy / atLocation * 1e-6)
    }

    private fun location(latitude: Double, longitude: Double) = Location("test").apply {
        this.latitude = latitude
        this.longitude = longitude
        accuracy = 150f
    }

    private fun loadStyle(projection: String) {
        val latch = CountDownLatch(1)
        val json = """{"version": 8, "projection": {"type": "$projection"}, "sources": {}, "layers": []}"""
        rule.runOnUiThread { maplibreMap.setStyle(Style.Builder().fromJson(json)) { latch.countDown() } }
        assertTrue("The style loads", latch.await(30, TimeUnit.SECONDS))
    }

    private fun <T> onMain(block: () -> T): T {
        var result: Result<T>? = null
        rule.runOnUiThread { result = runCatching(block) }
        return result!!.getOrThrow()
    }
}
