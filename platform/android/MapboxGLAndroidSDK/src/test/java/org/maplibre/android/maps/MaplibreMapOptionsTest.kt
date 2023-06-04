package org.maplibre.android.maps

import android.graphics.Color
import android.view.Gravity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MaplibreConstants
import org.maplibre.android.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.RuntimeEnvironment
import java.util.*

@RunWith(RobolectricTestRunner::class)
class MaplibreMapOptionsTest {
    @Test
    fun testSanity() {
        Assert.assertNotNull("should not be null",
            MaplibreMapOptions()
        )
    }

    @Test
    fun testDebugEnabled() {
        Assert.assertFalse(MaplibreMapOptions().debugActive)
        Assert.assertTrue(MaplibreMapOptions().debugActive(true).debugActive)
        Assert.assertFalse(MaplibreMapOptions().debugActive(false).debugActive)
    }

    @Test
    fun testCompassEnabled() {
        Assert.assertTrue(MaplibreMapOptions().compassEnabled(true).compassEnabled)
        Assert.assertFalse(MaplibreMapOptions().compassEnabled(false).compassEnabled)
    }

    @Test
    fun testCompassGravity() {
        Assert.assertEquals(
            Gravity.TOP or Gravity.END,
            MaplibreMapOptions().compassGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MaplibreMapOptions().compassGravity(Gravity.BOTTOM).compassGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MaplibreMapOptions().compassGravity(Gravity.BOTTOM).compassGravity.toLong()
        )
    }

    @Test
    fun testCompassMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MaplibreMapOptions()
                    .compassMargins(intArrayOf(0, 1, 2, 3)).compassMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MaplibreMapOptions()
                    .compassMargins(intArrayOf(0, 0, 0, 0)).compassMargins
            )
        )
    }

    @Test
    fun testLogoEnabled() {
        Assert.assertTrue(MaplibreMapOptions().logoEnabled(true).logoEnabled)
        Assert.assertFalse(MaplibreMapOptions().logoEnabled(false).logoEnabled)
    }

    @Test
    fun testLogoGravity() {
        Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MaplibreMapOptions().logoGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MaplibreMapOptions().logoGravity(Gravity.BOTTOM).logoGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MaplibreMapOptions().logoGravity(Gravity.BOTTOM).logoGravity.toLong()
        )
    }

    @Test
    fun testLogoMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MaplibreMapOptions()
                    .logoMargins(intArrayOf(0, 1, 2, 3)).logoMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MaplibreMapOptions()
                    .logoMargins(intArrayOf(0, 0, 0, 0)).logoMargins
            )
        )
    }

    @Test
    fun testAttributionTintColor() {
        Assert.assertEquals(-1, MaplibreMapOptions().attributionTintColor)
        Assert.assertEquals(
            Color.RED,
            MaplibreMapOptions().attributionTintColor(Color.RED).attributionTintColor
        )
    }

    @Test
    fun testAttributionEnabled() {
        Assert.assertTrue(MaplibreMapOptions().attributionEnabled(true).attributionEnabled)
        Assert.assertFalse(MaplibreMapOptions().attributionEnabled(false).attributionEnabled)
    }

    @Test
    fun testAttributionGravity() {
        Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MaplibreMapOptions().attributionGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MaplibreMapOptions().attributionGravity(Gravity.BOTTOM).attributionGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MaplibreMapOptions().attributionGravity(Gravity.BOTTOM).attributionGravity.toLong()
        )
    }

    @Test
    fun testAttributionMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MaplibreMapOptions()
                    .attributionMargins(intArrayOf(0, 1, 2, 3)).attributionMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MaplibreMapOptions()
                    .attributionMargins(intArrayOf(0, 0, 0, 0)).attributionMargins
            )
        )
    }

    @Test
    fun testMinZoom() {
        Assert.assertEquals(
            MaplibreConstants.MINIMUM_ZOOM.toDouble(),
            MaplibreMapOptions().minZoomPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MaplibreMapOptions().minZoomPreference(5.0).minZoomPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MaplibreMapOptions().minZoomPreference(5.0).minZoomPreference,
            DELTA
        )
    }

    @Test
    fun testMaxZoom() {
        Assert.assertEquals(
            MaplibreConstants.MAXIMUM_ZOOM.toDouble(),
            MaplibreMapOptions().maxZoomPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MaplibreMapOptions().maxZoomPreference(5.0).maxZoomPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MaplibreMapOptions().maxZoomPreference(5.0).maxZoomPreference,
            DELTA
        )
    }

    @Test
    fun testMinPitch() {
        Assert.assertEquals(
            MaplibreConstants.MINIMUM_PITCH.toDouble(),
            MaplibreMapOptions().minPitchPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MaplibreMapOptions().minPitchPreference(5.0).minPitchPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MaplibreMapOptions().minPitchPreference(5.0).minPitchPreference,
            DELTA
        )
    }

    @Test
    fun testMaxPitch() {
        Assert.assertEquals(
            MaplibreConstants.MAXIMUM_PITCH.toDouble(),
            MaplibreMapOptions().maxPitchPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MaplibreMapOptions().maxPitchPreference(5.0).maxPitchPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MaplibreMapOptions().maxPitchPreference(5.0).maxPitchPreference,
            DELTA
        )
    }

    @Test
    fun testTiltGesturesEnabled() {
        Assert.assertTrue(MaplibreMapOptions().tiltGesturesEnabled)
        Assert.assertTrue(MaplibreMapOptions().tiltGesturesEnabled(true).tiltGesturesEnabled)
        Assert.assertFalse(MaplibreMapOptions().tiltGesturesEnabled(false).tiltGesturesEnabled)
    }

    @Test
    fun testScrollGesturesEnabled() {
        Assert.assertTrue(MaplibreMapOptions().scrollGesturesEnabled)
        Assert.assertTrue(MaplibreMapOptions().scrollGesturesEnabled(true).scrollGesturesEnabled)
        Assert.assertFalse(MaplibreMapOptions().scrollGesturesEnabled(false).scrollGesturesEnabled)
    }

    @Test
    fun testHorizontalScrollGesturesEnabled() {
        Assert.assertTrue(MaplibreMapOptions().horizontalScrollGesturesEnabled)
        Assert.assertTrue(MaplibreMapOptions().horizontalScrollGesturesEnabled(true).horizontalScrollGesturesEnabled)
        Assert.assertFalse(MaplibreMapOptions().horizontalScrollGesturesEnabled(false).horizontalScrollGesturesEnabled)
    }

    @Test
    fun testZoomGesturesEnabled() {
        Assert.assertTrue(MaplibreMapOptions().zoomGesturesEnabled)
        Assert.assertTrue(MaplibreMapOptions().zoomGesturesEnabled(true).zoomGesturesEnabled)
        Assert.assertFalse(MaplibreMapOptions().zoomGesturesEnabled(false).zoomGesturesEnabled)
    }

    @Test
    fun testRotateGesturesEnabled() {
        Assert.assertTrue(MaplibreMapOptions().rotateGesturesEnabled)
        Assert.assertTrue(MaplibreMapOptions().rotateGesturesEnabled(true).rotateGesturesEnabled)
        Assert.assertFalse(MaplibreMapOptions().rotateGesturesEnabled(false).rotateGesturesEnabled)
    }

    @Test
    fun testCamera() {
        val position = CameraPosition.Builder().build()
        Assert.assertEquals(
            CameraPosition.Builder(position).build(),
            MaplibreMapOptions().camera(position).camera
        )
        Assert.assertNotEquals(
            CameraPosition.Builder().target(LatLng(1.0, 1.0)),
            MaplibreMapOptions().camera(position)
        )
        Assert.assertNull(MaplibreMapOptions().camera)
    }

    @Test
    fun testPrefetchesTiles() {
        // Default value
        Assert.assertTrue(MaplibreMapOptions().prefetchesTiles)

        // Check mutations
        Assert.assertTrue(MaplibreMapOptions().setPrefetchesTiles(true).prefetchesTiles)
        Assert.assertFalse(MaplibreMapOptions().setPrefetchesTiles(false).prefetchesTiles)
    }

    @Test
    fun testPrefetchZoomDelta() {
        // Default value
        Assert.assertEquals(4, MaplibreMapOptions().prefetchZoomDelta)

        // Check mutations
        Assert.assertEquals(
            5,
            MaplibreMapOptions().setPrefetchZoomDelta(5).prefetchZoomDelta
        )
    }

    @Test
    fun testCrossSourceCollisions() {
        // Default value
        Assert.assertTrue(MaplibreMapOptions().crossSourceCollisions)

        // check mutations
        Assert.assertTrue(MaplibreMapOptions().crossSourceCollisions(true).crossSourceCollisions)
        Assert.assertFalse(MaplibreMapOptions().crossSourceCollisions(false).crossSourceCollisions)
    }

    @Test
    fun testLocalIdeographFontFamily_enabledByDefault() {
        val options = MaplibreMapOptions.createFromAttributes(RuntimeEnvironment.application, null)
        Assert.assertEquals(
            MaplibreConstants.DEFAULT_FONT,
            options.localIdeographFontFamily
        )
    }

    companion object {
        private const val DELTA = 1e-15
    }
}
