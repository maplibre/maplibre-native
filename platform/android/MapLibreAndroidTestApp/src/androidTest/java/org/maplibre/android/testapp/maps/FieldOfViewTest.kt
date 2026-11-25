package org.maplibre.android.testapp.maps

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.AppCenter
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.testapp.activity.espresso.DeviceIndependentTestActivity
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import kotlin.math.ln
import kotlin.math.tan

/**
 * Tests MapLibreMap.getFieldOfView, setFieldOfView and setCameraFromEye, the Android counterparts
 * of iOS MLNMapView.fieldOfView and setCenterCoordinate:centerAltitude:fromEyeCoordinate:eyeAltitude:.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class FieldOfViewTest : AppCenter() {

    @Rule
    @JvmField
    var rule = ActivityTestRule(DeviceIndependentTestActivity::class.java)

    private lateinit var map: MapLibreMap

    @Before
    fun setup() {
        val latch = CountDownLatch(1)
        rule.runOnUiThread {
            rule.activity.mapView.getMapAsync {
                map = it
                latch.countDown()
            }
        }
        assertTrue(latch.await(30, TimeUnit.SECONDS))
    }

    private fun onUiThread(block: () -> Unit) {
        val latch = CountDownLatch(1)
        var failure: Throwable? = null
        rule.runOnUiThread {
            try {
                block()
            } catch (t: Throwable) {
                failure = t
            }
            latch.countDown()
        }
        assertTrue(latch.await(10, TimeUnit.SECONDS))
        failure?.let { throw it }
    }

    // The height of the map in the units core lays the view out in: logical pixels.
    private fun mapHeight(): Double {
        val view = rule.activity.mapView
        return Math.ceil(view.height / rule.activity.resources.displayMetrics.density.toDouble())
    }

    // Projection.getMetersPerPixelAtLatitude is per physical pixel, so pair it with the height in physical pixels.
    private fun mapHeightInPixels(): Double = rule.activity.mapView.height.toDouble()

    @Test
    fun testDefaultFieldOfView() {
        onUiThread {
            // 2 * atan((height / 2) / (height * 1.5))
            assertEquals(36.8699, map.fieldOfView, 0.001)
        }
    }

    @Test
    fun testFieldOfViewKeepsTheRestOfTheCamera() {
        onUiThread {
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(38.7, -120.07)).zoom(12.0).tilt(50.0).bearing(30.0).build()
            val before = map.cameraPosition

            map.fieldOfView = 70.0

            assertEquals(70.0, map.fieldOfView, 0.001)
            val after = map.cameraPosition
            assertEquals(70.0, after.fov, 0.001)
            assertEquals(before.zoom, after.zoom, 0.0001)
            assertEquals(before.target!!.latitude, after.target!!.latitude, 0.000001)
            assertEquals(before.target!!.longitude, after.target!!.longitude, 0.000001)
            assertEquals(50.0, after.tilt, 0.001)
            assertEquals(30.0, after.bearing, 0.001)
        }
    }

    @Test
    fun testFieldOfViewIsLimited() {
        onUiThread {
            map.fieldOfView = 200.0
            assertEquals(150.0, map.fieldOfView, 0.001)
            map.fieldOfView = 0.0
            assertEquals(0.1, map.fieldOfView, 0.001)
        }
    }

    @Test
    fun testAWiderFieldOfViewShowsMoreOfTheDistance() {
        onUiThread {
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(38.7, -120.07)).zoom(12.0).tilt(50.0).build()
            // A point a quarter of the way down from the top edge, on the center line.
            val point = android.graphics.PointF(rule.activity.mapView.width / 2f, rule.activity.mapView.height / 4f)
            val narrow = map.projection.fromScreenLocation(point)

            map.fieldOfView = 70.0
            val wide = map.projection.fromScreenLocation(point)

            // The scale at the center is unchanged, so the same screen point is farther from the
            // center on the ground with the wider field of view.
            val center = map.cameraPosition.target!!
            assertTrue(narrow.latitude > center.latitude)
            assertTrue(wide.latitude - center.latitude > (narrow.latitude - center.latitude) * 1.2)
        }
    }

    @Test
    fun testCameraFromAnEyeLooksDownAtTheCenter() {
        onUiThread {
            // 0.02 degrees of latitude is 2,224 m along the ground, and the eye is 2,000 m up.
            val eye = LatLng(38.70, -120.07)
            val center = LatLng(38.72, -120.07)
            map.setMaxPitchPreference(85.0)

            map.setCameraFromEye(center, 1000.0, eye, 3000.0)

            val position = map.cameraPosition
            assertEquals(center.latitude, position.target!!.latitude, 0.000001)
            assertEquals(center.longitude, position.target!!.longitude, 0.000001)
            assertEquals(1000.0, map.centerAltitude, 0.001)
            assertEquals(0.0, position.bearing, 0.001)
            assertEquals(48.03, position.tilt, 0.1)
            // The camera is 1.5 times the map height from the center at the default field of view,
            // and that is the 2,991 m line of sight.
            val metersPerPixel = map.projection.getMetersPerPixelAtLatitude(center.latitude)
            assertEquals(2991.0, metersPerPixel * 1.5 * mapHeightInPixels(), 15.0)
        }
    }

    @Test
    fun testCameraFromAnEyeFollowsTheFieldOfView() {
        onUiThread {
            val eye = LatLng(38.70, -120.07)
            val center = LatLng(38.72, -120.07)
            map.setMaxPitchPreference(85.0)
            map.fieldOfView = 90.0

            map.setCameraFromEye(center, 1000.0, eye, 3000.0)

            // At 90 degrees the camera is half the map height from the center.
            val position = map.cameraPosition
            val metersPerPixel = map.projection.getMetersPerPixelAtLatitude(center.latitude)
            assertEquals(2991.0, metersPerPixel * 0.5 * mapHeightInPixels(), 15.0)
            assertEquals(48.03, position.tilt, 0.1)
            // The same eye with the default field of view needs a zoom level 1.585 higher.
            assertEquals(-ln(tan(Math.toRadians(90.0 / 2)) / tan(Math.toRadians(36.8699 / 2))) / ln(2.0), position.zoom - zoomAtDefaultFieldOfView(eye, center), 0.01)
        }
    }

    private fun zoomAtDefaultFieldOfView(eye: LatLng, center: LatLng): Double {
        map.fieldOfView = 36.8699
        map.setCameraFromEye(center, 1000.0, eye, 3000.0)
        val zoom = map.cameraPosition.zoom
        map.fieldOfView = 90.0
        map.setCameraFromEye(center, 1000.0, eye, 3000.0)
        return zoom
    }

    @Test
    fun testCameraFromAnEyeHoldsTheTiltAtTheMaximumPitch() {
        onUiThread {
            val eye = LatLng(38.70, -120.07)
            val center = LatLng(38.70, -120.05)
            map.setMaxPitchPreference(85.0)

            map.setCameraFromEye(center, 3000.0, eye, 1000.0)

            val position = map.cameraPosition
            assertEquals(3000.0, map.centerAltitude, 0.001)
            assertEquals(90.0, position.bearing, 0.001)
            // The center is 1,736 m east of the eye and 2,000 m above it: that needs a tilt of
            // 90 + 49 degrees, and the maximum pitch holds it at 85.
            assertEquals(85.0, position.tilt, 0.001)
        }
    }

    @Test
    fun testCameraFromAnEyeAtTheCenterLeavesTheCamera() {
        onUiThread {
            map.cameraPosition = CameraPosition.Builder()
                .target(LatLng(38.7, -120.07)).zoom(12.0).tilt(30.0).bearing(10.0).build()
            val before = map.cameraPosition
            var moves = 0
            map.addOnCameraMoveStartedListener { moves++ }

            map.setCameraFromEye(LatLng(38.7, -120.07), 1000.0, LatLng(38.7, -120.07), 1000.0)

            assertEquals(before, map.cameraPosition)
            assertEquals(0, moves)
        }
    }
}
