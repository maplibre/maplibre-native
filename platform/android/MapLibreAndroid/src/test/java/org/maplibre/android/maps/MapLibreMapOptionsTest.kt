package org.maplibre.android.maps

import android.graphics.Color
import android.view.Gravity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner
import org.robolectric.RuntimeEnvironment
import java.util.*

@RunWith(RobolectricTestRunner::class)
class MapLibreMapOptionsTest : BaseTest() {
    @Test
    fun testSanity() {
        Assert.assertNotNull("should not be null",
            MapLibreMapOptions()
        )
    }

    @Test
    fun testDebugEnabled() {
        Assert.assertFalse(MapLibreMapOptions().debugActive)
        Assert.assertTrue(MapLibreMapOptions().debugActive(true).debugActive)
        Assert.assertFalse(MapLibreMapOptions().debugActive(false).debugActive)
    }

    @Test
    fun testCompassEnabled() {
        Assert.assertTrue(MapLibreMapOptions().compassEnabled(true).compassEnabled)
        Assert.assertFalse(MapLibreMapOptions().compassEnabled(false).compassEnabled)
    }

    @Test
    fun testCompassGravity() {
        Assert.assertEquals(
            Gravity.TOP or Gravity.END,
            MapLibreMapOptions().compassGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MapLibreMapOptions().compassGravity(Gravity.BOTTOM).compassGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MapLibreMapOptions().compassGravity(Gravity.BOTTOM).compassGravity.toLong()
        )
    }

    @Test
    fun testCompassMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapLibreMapOptions()
                    .compassMargins(intArrayOf(0, 1, 2, 3)).compassMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapLibreMapOptions()
                    .compassMargins(intArrayOf(0, 0, 0, 0)).compassMargins
            )
        )
    }

    @Test
    fun testLogoEnabled() {
        Assert.assertTrue(MapLibreMapOptions().logoEnabled(true).logoEnabled)
        Assert.assertFalse(MapLibreMapOptions().logoEnabled(false).logoEnabled)
    }

    @Test
    fun testLogoGravity() {
        Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MapLibreMapOptions().logoGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MapLibreMapOptions().logoGravity(Gravity.BOTTOM).logoGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MapLibreMapOptions().logoGravity(Gravity.BOTTOM).logoGravity.toLong()
        )
    }

    @Test
    fun testLogoMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapLibreMapOptions()
                    .logoMargins(intArrayOf(0, 1, 2, 3)).logoMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapLibreMapOptions()
                    .logoMargins(intArrayOf(0, 0, 0, 0)).logoMargins
            )
        )
    }

    @Test
    fun testAttributionTintColor() {
        Assert.assertEquals(-1, MapLibreMapOptions().attributionTintColor)
        Assert.assertEquals(
            Color.RED,
            MapLibreMapOptions().attributionTintColor(Color.RED).attributionTintColor
        )
    }

    @Test
    fun testAttributionEnabled() {
        Assert.assertTrue(MapLibreMapOptions().attributionEnabled(true).attributionEnabled)
        Assert.assertFalse(MapLibreMapOptions().attributionEnabled(false).attributionEnabled)
    }

    @Test
    fun testAttributionGravity() {
        Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MapLibreMapOptions().attributionGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MapLibreMapOptions().attributionGravity(Gravity.BOTTOM).attributionGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MapLibreMapOptions().attributionGravity(Gravity.BOTTOM).attributionGravity.toLong()
        )
    }

    @Test
    fun testAttributionMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapLibreMapOptions()
                    .attributionMargins(intArrayOf(0, 1, 2, 3)).attributionMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapLibreMapOptions()
                    .attributionMargins(intArrayOf(0, 0, 0, 0)).attributionMargins
            )
        )
    }

    @Test
    fun testMinZoom() {
        Assert.assertEquals(
            MapLibreConstants.MINIMUM_ZOOM.toDouble(),
            MapLibreMapOptions().minZoomPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapLibreMapOptions().minZoomPreference(5.0).minZoomPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapLibreMapOptions().minZoomPreference(5.0).minZoomPreference,
            DELTA
        )
    }

    @Test
    fun testMaxZoom() {
        Assert.assertEquals(
            MapLibreConstants.MAXIMUM_ZOOM.toDouble(),
            MapLibreMapOptions().maxZoomPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapLibreMapOptions().maxZoomPreference(5.0).maxZoomPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapLibreMapOptions().maxZoomPreference(5.0).maxZoomPreference,
            DELTA
        )
    }

    @Test
    fun testMinPitch() {
        Assert.assertEquals(
            MapLibreConstants.MINIMUM_PITCH.toDouble(),
            MapLibreMapOptions().minPitchPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapLibreMapOptions().minPitchPreference(5.0).minPitchPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapLibreMapOptions().minPitchPreference(5.0).minPitchPreference,
            DELTA
        )
    }

    @Test
    fun testMaxPitch() {
        Assert.assertEquals(
            MapLibreConstants.MAXIMUM_PITCH.toDouble(),
            MapLibreMapOptions().maxPitchPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapLibreMapOptions().maxPitchPreference(5.0).maxPitchPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapLibreMapOptions().maxPitchPreference(5.0).maxPitchPreference,
            DELTA
        )
    }

    @Test
    fun testTiltGesturesEnabled() {
        Assert.assertTrue(MapLibreMapOptions().tiltGesturesEnabled)
        Assert.assertTrue(MapLibreMapOptions().tiltGesturesEnabled(true).tiltGesturesEnabled)
        Assert.assertFalse(MapLibreMapOptions().tiltGesturesEnabled(false).tiltGesturesEnabled)
    }

    @Test
    fun testScrollGesturesEnabled() {
        Assert.assertTrue(MapLibreMapOptions().scrollGesturesEnabled)
        Assert.assertTrue(MapLibreMapOptions().scrollGesturesEnabled(true).scrollGesturesEnabled)
        Assert.assertFalse(MapLibreMapOptions().scrollGesturesEnabled(false).scrollGesturesEnabled)
    }

    @Test
    fun testHorizontalScrollGesturesEnabled() {
        Assert.assertTrue(MapLibreMapOptions().horizontalScrollGesturesEnabled)
        Assert.assertTrue(MapLibreMapOptions().horizontalScrollGesturesEnabled(true).horizontalScrollGesturesEnabled)
        Assert.assertFalse(MapLibreMapOptions().horizontalScrollGesturesEnabled(false).horizontalScrollGesturesEnabled)
    }

    @Test
    fun testZoomGesturesEnabled() {
        Assert.assertTrue(MapLibreMapOptions().zoomGesturesEnabled)
        Assert.assertTrue(MapLibreMapOptions().zoomGesturesEnabled(true).zoomGesturesEnabled)
        Assert.assertFalse(MapLibreMapOptions().zoomGesturesEnabled(false).zoomGesturesEnabled)
    }

    @Test
    fun testRotateGesturesEnabled() {
        Assert.assertTrue(MapLibreMapOptions().rotateGesturesEnabled)
        Assert.assertTrue(MapLibreMapOptions().rotateGesturesEnabled(true).rotateGesturesEnabled)
        Assert.assertFalse(MapLibreMapOptions().rotateGesturesEnabled(false).rotateGesturesEnabled)
    }

    @Test
    fun testCamera() {
        val position = CameraPosition.Builder().build()
        Assert.assertEquals(
            CameraPosition.Builder(position).build(),
            MapLibreMapOptions().camera(position).camera
        )
        Assert.assertNotEquals(
            CameraPosition.Builder().target(LatLng(1.0, 1.0)),
            MapLibreMapOptions().camera(position)
        )
        Assert.assertNull(MapLibreMapOptions().camera)
    }

    @Test
    fun testPrefetchesTiles() {
        // Default value
        Assert.assertTrue(MapLibreMapOptions().prefetchesTiles)

        // Check mutations
        Assert.assertTrue(MapLibreMapOptions().setPrefetchesTiles(true).prefetchesTiles)
        Assert.assertFalse(MapLibreMapOptions().setPrefetchesTiles(false).prefetchesTiles)
    }

    @Test
    fun testPrefetchZoomDelta() {
        // Default value
        Assert.assertEquals(4, MapLibreMapOptions().prefetchZoomDelta)

        // Check mutations
        Assert.assertEquals(
            5,
            MapLibreMapOptions().setPrefetchZoomDelta(5).prefetchZoomDelta
        )
    }

    @Test
    fun testCrossSourceCollisions() {
        // Default value
        Assert.assertTrue(MapLibreMapOptions().crossSourceCollisions)

        // check mutations
        Assert.assertTrue(MapLibreMapOptions().crossSourceCollisions(true).crossSourceCollisions)
        Assert.assertFalse(MapLibreMapOptions().crossSourceCollisions(false).crossSourceCollisions)
    }

    @Test
    fun testLocalIdeographFontFamily_enabledByDefault() {
        val options = MapLibreMapOptions.createFromAttributes(RuntimeEnvironment.application, null)
        Assert.assertEquals(
            MapLibreConstants.DEFAULT_FONT,
            options.localIdeographFontFamily
        )
    }

    companion object {
        private const val DELTA = 1e-15
    }
}
