package org.maplibre.android.maps

import android.graphics.PointF
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.AppCenter
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.espresso.EspressoTestActivity
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeoutException

@RunWith(AndroidJUnit4ClassRunner::class)
class GlobeVisibleRegionTest : AppCenter() {

    @Rule
    @JvmField
    var rule = ActivityTestRule(EspressoTestActivity::class.java)

    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap

    @Before
    fun setup() {
        val styleLoaded = CountDownLatch(1)
        rule.runOnUiThread {
            mapView = rule.activity.findViewById(R.id.mapView)
            mapView.getMapAsync {
                maplibreMap = it
                maplibreMap.setStyle(Style.Builder().fromJson(GLOBE)) { styleLoaded.countDown() }
            }
        }
        if (!styleLoaded.await(30, TimeUnit.SECONDS)) {
            throw TimeoutException()
        }
    }

    @Test
    fun visibleRegionContainsTheMiddlesOfTheEdges() {
        rule.runOnUiThread {
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 1.0))
            val width = mapView.width.toFloat()
            val height = mapView.height.toFloat()
            val bounds = maplibreMap.projection.visibleRegion.latLngBounds
            for (point in listOf(
                PointF(0f, 0f),
                PointF(width / 2, 0f),
                PointF(width, 0f),
                PointF(width, height / 2),
                PointF(width, height),
                PointF(width / 2, height),
                PointF(0f, height),
                PointF(0f, height / 2)
            )) {
                val latLng = maplibreMap.projection.fromScreenLocation(point)
                assertTrue("$point at $latLng outside $bounds", bounds.contains(latLng))
            }
        }
    }

    @Test
    fun visibleRegionWithThePoleSpansEveryLongitude() {
        rule.runOnUiThread {
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(75.0, 0.0), 1.0))
            val bounds = maplibreMap.projection.visibleRegion.latLngBounds
            assertEquals(90.0, bounds.latitudeNorth, 1e-9)
            assertEquals(-180.0, bounds.longitudeWest, 1e-9)
            assertEquals(180.0, bounds.longitudeEast, 1e-9)
        }
    }

    companion object {
        private const val GLOBE = """{"version":8,"projection":{"type":"globe"},"sources":{},"layers":[]}"""
    }
}
