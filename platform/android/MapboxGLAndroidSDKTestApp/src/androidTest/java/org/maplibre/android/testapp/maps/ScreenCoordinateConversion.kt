package org.maplibre.android.testapp.maps

import android.graphics.PointF
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.junit.Assert.assertArrayEquals
import org.maplibre.android.AppCenter
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.espresso.DeviceIndependentTestActivity
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

@RunWith(AndroidJUnit4ClassRunner::class)
class ScreenCoordinateConversion : AppCenter() {

    @Rule
    @JvmField
    var rule = ActivityTestRule(DeviceIndependentTestActivity::class.java)

    private lateinit var mapView: MapView
    private lateinit var latch: CountDownLatch

    @Before
    fun setup() {
        latch = CountDownLatch(1)
    }

    @Test
    fun testToScreenCoordinates() {
        rule.runOnUiThread {
            mapView = rule.activity.findViewById(R.id.mapView)
            val expected = doubleArrayOf(261.6888889445351, 253.15541110551123, 267.37777788907033, 247.4627648056528)
            val actual = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
            mapView.getMapAsync {
                val projection = it.projection
                projection.toScreenLocations(doubleArrayOf(1.0, 2.0, 3.0, 4.0), actual)
                // check all values
                assertTrue(expected.contentEquals(actual))
                // check first coordinate value
                val pointFirst = PointF(expected[0].toFloat(), expected[1].toFloat())
                val convertedFirst = projection.toScreenLocation(LatLng(1.0, 2.0))
                assertEquals(
                    pointFirst.x,
                    convertedFirst.x,
                    DELTA_FLOAT
                )
                assertEquals(
                    pointFirst.y,
                    convertedFirst.y,
                    DELTA_FLOAT
                )

                // check second latlng value
                val pointSecond = PointF(expected[2].toFloat(), expected[3].toFloat())
                val convertedSecond = projection.toScreenLocation(LatLng(3.0, 4.0))
                assertEquals(
                    pointSecond.x,
                    convertedSecond.x,
                    DELTA_FLOAT
                )
                assertEquals(
                    pointSecond.y,
                    convertedSecond.y,
                    DELTA_FLOAT
                )
                latch.countDown()
            }
        }
        latch.await(10, TimeUnit.SECONDS)
    }

    @Test
    fun testFromScreenCoordinates() {
        rule.runOnUiThread {
            mapView = rule.activity.findViewById(R.id.mapView)
            val expected = doubleArrayOf(66.2314571265913, -89.64843662311537, 65.94647142295494, -88.94531162999371)
            val actual = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
            mapView.getMapAsync {
                val projection = it.projection
                projection.fromScreenLocations(doubleArrayOf(1.0, 2.0, 3.0, 4.0), actual)
                // check all values at once
                assertArrayEquals(expected, actual, DELTA_DOUBLE)

                // check first latlng value
                val latLngFirst = LatLng(expected[0], expected[1])
                val convertedFirst = projection.fromScreenLocation(PointF(1.0f, 2.0f))
                assertEquals(
                    latLngFirst.latitude,
                    convertedFirst.latitude,
                    DELTA_DOUBLE
                )
                assertEquals(
                    latLngFirst.longitude,
                    convertedFirst.longitude,
                    DELTA_DOUBLE
                )

                // check second latlng value
                val latLngSecond = LatLng(expected[2], expected[3])
                val convertedSecond = projection.fromScreenLocation(PointF(3.0f, 4.0f))
                assertEquals(
                    latLngSecond.latitude,
                    convertedSecond.latitude,
                    DELTA_DOUBLE
                )
                assertEquals(
                    latLngSecond.longitude,
                    convertedSecond.longitude,
                    DELTA_DOUBLE
                )
                latch.countDown()
            }
        }
        latch.await(10, TimeUnit.SECONDS)
    }

    companion object {
        const val DELTA_DOUBLE = 0.000001
        const val DELTA_FLOAT = 0.000001f
    }
}
