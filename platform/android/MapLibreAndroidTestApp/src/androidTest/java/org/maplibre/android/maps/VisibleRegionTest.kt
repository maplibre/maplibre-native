package org.maplibre.android.maps

import android.graphics.PointF
import androidx.test.espresso.UiController
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.testapp.action.MapLibreMapAction.invoke
import org.maplibre.android.testapp.activity.BaseTest
import org.maplibre.android.testapp.activity.espresso.PixelTestActivity
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Ignore
import org.junit.Test

@Ignore("https://github.com/maplibre/maplibre-native/issues/2468")
class VisibleRegionTest : BaseTest() {

    override fun getActivityClass(): Class<*> {
        return PixelTestActivity::class.java
    }

    override
    fun beforeTest() {
        super.beforeTest()
        mapView = (rule.activity as PixelTestActivity).mapView
    }

    @Test
    fun visibleRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )
            val visibleRegion = maplibreMap.projection.visibleRegion
            assertTrue(latLngs.all { visibleRegion.latLngBounds.contains(it) })
        }
    }

    @Test
    fun paddedVisibleRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(
                mapView.width / 4,
                mapView.height / 4,
                mapView.width / 4,
                mapView.height / 4
            )

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 1)
            assertTrue(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)))
        }
    }

    @Test
    fun paddedLeftVisibleRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(mapView.width / 4, 0, 0, 0)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f)))
        }
    }

    @Test
    fun paddedTopVisibleRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(0, mapView.height / 4, 0, 0)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f)))
        }
    }

    @Test
    fun paddedRightVisibleRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(0, 0, mapView.width / 4, 0)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)))
        }
    }

    @Test
    fun paddedBottomVisibleRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(0, 0, 0, mapView.height / 4)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat())))
        }
    }

    @Test
    fun visibleRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat())
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )
            val visibleRegion = maplibreMap.projection.visibleRegion
            assertTrue(latLngs.all { visibleRegion.latLngBounds.contains(it) })
        }
    }

    @Test
    fun paddedVisibleRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(
                mapView.width / 4,
                mapView.height / 4,
                mapView.width / 4,
                mapView.height / 4
            )

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 1)
            assertTrue(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)))
        }
    }

    @Test
    fun paddedLeftVisibleRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat())
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(mapView.width / 4, 0, 0, 0)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f)))
        }
    }

    @Test
    fun paddedTopVisibleRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { ui: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            ui.loopMainThreadForAtLeast(5000)
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat())
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(0, mapView.height / 4, 0, 0)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f)))
        }
    }

    @Test
    fun paddedRightVisibleRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat())
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(0, 0, mapView.width / 4, 0)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)))
        }
    }

    @Test
    fun paddedBottomVisibleRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat())
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )

            maplibreMap.setPadding(0, 0, 0, mapView.height / 4)

            val visibleRegion = maplibreMap.projection.getVisibleRegion(false)
            val filtered = latLngs.filter { visibleRegion.latLngBounds.contains(it) }
            assertTrue(filtered.size == 6)
            assertFalse(filtered.contains(maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat())))
        }
    }

    @Test
    fun visibleRotatedRegionTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val d = Math.min(maplibreMap.width, maplibreMap.height) / 4
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f - d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f + d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f - d / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f + d / 2f)
            )

            for (bearing in 45 until 360 step 45) {
                maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(bearing.toDouble()))
                val visibleRegion = maplibreMap.projection.visibleRegion
                assertTrue(latLngs.all { visibleRegion.latLngBounds.contains(it) })
            }
        }
    }

    @Test
    fun visibleRotatedRegionOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val d = Math.min(maplibreMap.width, maplibreMap.height) / 4
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f - d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f + d / 2f, mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f - d / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f + d / 2f)
            )

            for (bearing in 45 until 360 step 45) {
                maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(bearing.toDouble()))
                val visibleRegion = maplibreMap.projection.visibleRegion
                assertTrue(latLngs.all { visibleRegion.latLngBounds.contains(it) })
            }
        }
    }

    @Test
    fun visibleRegionWithBoundsEqualTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )
            val bounds = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
            maplibreMap.projection.getVisibleCoordinateBounds(bounds)
            val latLngBounds = LatLngBounds.from(bounds[0], bounds[1], bounds[2], bounds[3])
            val visibleRegion = maplibreMap.projection.visibleRegion
            assertTrue(latLngBounds == visibleRegion.latLngBounds)
            assertTrue(latLngs.all { latLngBounds.contains(it) })
        }
    }

    @Test
    fun visibleRegionBoundsOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(0f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, 0f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), 0f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height / 2f)
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width.toFloat(), mapView.height.toFloat())
                    .also { it.longitude += 360 },
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height.toFloat()),
                maplibreMap.getLatLngFromScreenCoords(0f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f)
            )
            val bounds = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
            maplibreMap.projection.getVisibleCoordinateBounds(bounds)
            val latLngBounds = LatLngBounds.from(bounds[0], bounds[1], bounds[2], bounds[3])
            assertTrue(latLngs.all { latLngBounds.contains(it) })
        }
    }

    @Test
    fun visibleRegionBoundsOverDatelineLatitudeZeroTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 180.0), 8.0))
            val shift = maplibreMap.getLatLngFromScreenCoords(0f, 0f)
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLng(LatLng(0.0, 180.0 - shift.longitude)))

            val bounds = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
            maplibreMap.projection.getVisibleCoordinateBounds(bounds)
            val latLngBounds = LatLngBounds.from(bounds[0], bounds[1], bounds[2], bounds[3])
            val visibleRegion = maplibreMap.projection.visibleRegion
            assertTrue(latLngBounds == visibleRegion.latLngBounds)
        }
    }

    @Test
    fun visibleRotatedRegionBoundEqualTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 0.0), 8.0))
            val d = Math.min(maplibreMap.width, maplibreMap.height) / 4
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f - d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f + d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f - d / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f + d / 2f)
            )

            for (bearing in 45 until 360 step 45) {
                maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(bearing.toDouble()))
                val bounds = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
                maplibreMap.projection.getVisibleCoordinateBounds(bounds)
                val latLngBounds = LatLngBounds.from(bounds[0], bounds[1], bounds[2], bounds[3])
                val visibleRegion = maplibreMap.projection.visibleRegion
                assertTrue(latLngBounds == visibleRegion.latLngBounds)
                assertTrue(latLngs.all { latLngBounds.contains(it) })
            }
        }
    }

    @Test
    fun visibleRotatedRegionBoundsOverDatelineTest() {
        validateTestSetup()
        invoke(maplibreMap) { _: UiController, maplibreMap: MapLibreMap ->
            maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(0.0, 179.0), 8.0))
            val d = Math.min(maplibreMap.width, maplibreMap.height) / 4
            val latLngs = listOf(
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f - d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f + d / 2f, mapView.height / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f - d / 2f),
                maplibreMap.getLatLngFromScreenCoords(mapView.width / 2f, mapView.height / 2f + d / 2f)
            )

            for (bearing in 45 until 360 step 45) {
                maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(bearing.toDouble()))
                val bounds = doubleArrayOf(0.0, 0.0, 0.0, 0.0)
                maplibreMap.projection.getVisibleCoordinateBounds(bounds)
                val latLngBounds = LatLngBounds.from(bounds[0], bounds[1], bounds[2], bounds[3])
                assertTrue(latLngs.all { latLngBounds.contains(it) })
            }
        }
    }

    private fun MapLibreMap.getLatLngFromScreenCoords(x: Float, y: Float): LatLng {
        return this.projection.fromScreenLocation(PointF(x, y))
    }
}
