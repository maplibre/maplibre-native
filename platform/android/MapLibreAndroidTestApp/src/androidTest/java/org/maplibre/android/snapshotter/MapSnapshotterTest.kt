package org.maplibre.android.snapshotter

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.BackgroundLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.snapshotter.buildings.BuildingExtrusionSnapshot
import org.maplibre.android.snapshotter.buildings.GeoJsonViewportFilter
import org.maplibre.android.testapp.activity.FeatureOverviewActivity
import org.junit.Assert
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.geojson.Polygon
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeoutException

/**
 * Integration test that validates if a snapshotter creation
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class MapSnapshotterTest {

    @Rule
    @JvmField
    var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

    private val countDownLatch = CountDownLatch(1)

    @Test
    fun mapSnapshotter() {
        var mapSnapshotter: MapSnapshotter?
        rule.activity.runOnUiThread {
            val bg = BackgroundLayer("rand_tint")
            bg.setProperties(PropertyFactory.backgroundColor("rgba(255,128,0,0.7)"))
            val options = MapSnapshotter.Options(512, 512)
                .withPixelRatio(1.0f)
                .withStyleBuilder(
                    Style.Builder().fromUri(TestStyles.OPENFREEMAP_BRIGHT)
                        .withLayerAbove(bg, "country-label")
                )
                .withCameraPosition(
                    CameraPosition.Builder()
                        .zoom(12.0)
                        .target(LatLng(51.145495, 5.742234))
                        .build()
                )
            mapSnapshotter = MapSnapshotter(rule.activity, options)
            mapSnapshotter!!.start(
                {
                    assertNotNull(it)
                    assertNotNull(it.bitmap)
                    countDownLatch.countDown()
                },
                {
                    Assert.fail(it)
                }
            )
        }
        if (!countDownLatch.await(30, TimeUnit.SECONDS)) {
            throw TimeoutException()
        }
    }

    @Test
    fun mapSnapshotter_withBuildingExtrusions() {
        val countDown = CountDownLatch(1)
        val centerLat = 51.145495
        val centerLng = 5.742234
        val d = 0.0004
        fun boxFeature(height: Double, east: Double, north: Double): Feature {
            val ring = listOf(
                Point.fromLngLat(centerLng - d + east, centerLat - d + north),
                Point.fromLngLat(centerLng + d + east, centerLat - d + north),
                Point.fromLngLat(centerLng + d + east, centerLat + d + north),
                Point.fromLngLat(centerLng - d + east, centerLat + d + north),
                Point.fromLngLat(centerLng - d + east, centerLat - d + north)
            )
            val feature = Feature.fromGeometry(Polygon.fromLngLats(listOf(ring)))
            feature.addNumberProperty("height", height)
            return feature
        }
        val buildings = FeatureCollection.fromFeatures(
            listOf(
                boxFeature(40.0, 0.0, 0.0),
                boxFeature(55.0, d * 4, 0.0),
                boxFeature(30.0, 0.0, d * 4)
            )
        )
        val viewport = LatLngBounds.from(
            centerLat + d * 6,
            centerLng + d * 6,
            centerLat - d * 6,
            centerLng - d * 6
        )
        val filtered = GeoJsonViewportFilter.filterFeaturesInBounds(buildings, viewport, 20.0)
        assertTrue((filtered.features()?.size ?: 0) >= 1)

        rule.activity.runOnUiThread {
            val styleBuilder = BuildingExtrusionSnapshot.applyToStyleBuilder(
                Style.Builder().fromUri(TestStyles.OPENFREEMAP_BRIGHT),
                filtered,
                "country-label",
                minZoom = 14f,
                maxZoom = 24f
            )
            val options = MapSnapshotter.Options(512, 512)
                .withPixelRatio(1.0f)
                .withStyleBuilder(styleBuilder)
                .withCameraPosition(
                    CameraPosition.Builder()
                        .zoom(15.0)
                        .target(LatLng(centerLat, centerLng))
                        .build()
                )
            val snapshotter = MapSnapshotter(rule.activity, options)
            snapshotter.start(
                {
                    assertNotNull(it)
                    assertNotNull(it.bitmap)
                    assertTrue(it.bitmap.width > 0)
                    countDown.countDown()
                },
                {
                    Assert.fail(it)
                }
            )
        }
        if (!countDown.await(60, TimeUnit.SECONDS)) {
            throw TimeoutException()
        }
    }
}
