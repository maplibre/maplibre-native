package com.mapbox.mapboxsdk.maps

import android.graphics.Color
import android.view.Gravity
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.constants.MapboxConstants
import com.mapbox.mapboxsdk.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.RuntimeEnvironment
import java.util.*

@RunWith(RobolectricTestRunner::class)
class MapboxMapOptionsTest {
    @Test
    fun testSanity() {
        Assert.assertNotNull("should not be null", MapboxMapOptions())
    }

    @Test
    fun testDebugEnabled() {
        Assert.assertFalse(MapboxMapOptions().debugActive)
        Assert.assertTrue(MapboxMapOptions().debugActive(true).debugActive)
        Assert.assertFalse(MapboxMapOptions().debugActive(false).debugActive)
    }

    @Test
    fun testCompassEnabled() {
        Assert.assertTrue(MapboxMapOptions().compassEnabled(true).compassEnabled)
        Assert.assertFalse(MapboxMapOptions().compassEnabled(false).compassEnabled)
    }

    @Test
    fun testCompassGravity() {
        Assert.assertEquals(
            Gravity.TOP or Gravity.END,
            MapboxMapOptions().compassGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MapboxMapOptions().compassGravity(Gravity.BOTTOM).compassGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MapboxMapOptions().compassGravity(Gravity.BOTTOM).compassGravity.toLong()
        )
    }

    @Test
    fun testCompassMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().compassMargins(intArrayOf(0, 1, 2, 3)).compassMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().compassMargins(intArrayOf(0, 0, 0, 0)).compassMargins
            )
        )
    }

    @Test
    fun testLogoEnabled() {
        Assert.assertTrue(MapboxMapOptions().logoEnabled(true).logoEnabled)
        Assert.assertFalse(MapboxMapOptions().logoEnabled(false).logoEnabled)
    }

    @Test
    fun testLogoGravity() {
        Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MapboxMapOptions().logoGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MapboxMapOptions().logoGravity(Gravity.BOTTOM).logoGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MapboxMapOptions().logoGravity(Gravity.BOTTOM).logoGravity.toLong()
        )
    }

    @Test
    fun testLogoMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().logoMargins(intArrayOf(0, 1, 2, 3)).logoMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().logoMargins(intArrayOf(0, 0, 0, 0)).logoMargins
            )
        )
    }

    @Test
    fun testAttributionTintColor() {
        Assert.assertEquals(-1, MapboxMapOptions().attributionTintColor)
        Assert.assertEquals(
            Color.RED,
            MapboxMapOptions().attributionTintColor(Color.RED).attributionTintColor
        )
    }

    @Test
    fun testAttributionEnabled() {
        Assert.assertTrue(MapboxMapOptions().attributionEnabled(true).attributionEnabled)
        Assert.assertFalse(MapboxMapOptions().attributionEnabled(false).attributionEnabled)
    }

    @Test
    fun testAttributionGravity() {
        Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MapboxMapOptions().attributionGravity
        )
        Assert.assertEquals(
            Gravity.BOTTOM,
            MapboxMapOptions().attributionGravity(Gravity.BOTTOM).attributionGravity
        )
        Assert.assertNotEquals(
            Gravity.START.toLong(),
            MapboxMapOptions().attributionGravity(Gravity.BOTTOM).attributionGravity.toLong()
        )
    }

    @Test
    fun testAttributionMargins() {
        Assert.assertTrue(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().attributionMargins(intArrayOf(0, 1, 2, 3)).attributionMargins
            )
        )
        Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().attributionMargins(intArrayOf(0, 0, 0, 0)).attributionMargins
            )
        )
    }

    @Test
    fun testMinZoom() {
        Assert.assertEquals(
            MapboxConstants.MINIMUM_ZOOM.toDouble(),
            MapboxMapOptions().minZoomPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapboxMapOptions().minZoomPreference(5.0).minZoomPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapboxMapOptions().minZoomPreference(5.0).minZoomPreference,
            DELTA
        )
    }

    @Test
    fun testMaxZoom() {
        Assert.assertEquals(
            MapboxConstants.MAXIMUM_ZOOM.toDouble(),
            MapboxMapOptions().maxZoomPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapboxMapOptions().maxZoomPreference(5.0).maxZoomPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapboxMapOptions().maxZoomPreference(5.0).maxZoomPreference,
            DELTA
        )
    }

    @Test
    fun testMinPitch() {
        Assert.assertEquals(
            MapboxConstants.MINIMUM_PITCH.toDouble(),
            MapboxMapOptions().minPitchPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapboxMapOptions().minPitchPreference(5.0).minPitchPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapboxMapOptions().minPitchPreference(5.0).minPitchPreference,
            DELTA
        )
    }

    @Test
    fun testMaxPitch() {
        Assert.assertEquals(
            MapboxConstants.MAXIMUM_PITCH.toDouble(),
            MapboxMapOptions().maxPitchPreference,
            DELTA
        )
        Assert.assertEquals(
            5.0,
            MapboxMapOptions().maxPitchPreference(5.0).maxPitchPreference,
            DELTA
        )
        Assert.assertNotEquals(
            2.0,
            MapboxMapOptions().maxPitchPreference(5.0).maxPitchPreference,
            DELTA
        )
    }

    @Test
    fun testTiltGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().tiltGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().tiltGesturesEnabled(true).tiltGesturesEnabled)
        Assert.assertFalse(MapboxMapOptions().tiltGesturesEnabled(false).tiltGesturesEnabled)
    }

    @Test
    fun testScrollGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().scrollGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().scrollGesturesEnabled(true).scrollGesturesEnabled)
        Assert.assertFalse(MapboxMapOptions().scrollGesturesEnabled(false).scrollGesturesEnabled)
    }

    @Test
    fun testHorizontalScrollGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().horizontalScrollGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().horizontalScrollGesturesEnabled(true).horizontalScrollGesturesEnabled)
        Assert.assertFalse(MapboxMapOptions().horizontalScrollGesturesEnabled(false).horizontalScrollGesturesEnabled)
    }

    @Test
    fun testZoomGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().zoomGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().zoomGesturesEnabled(true).zoomGesturesEnabled)
        Assert.assertFalse(MapboxMapOptions().zoomGesturesEnabled(false).zoomGesturesEnabled)
    }

    @Test
    fun testRotateGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().rotateGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().rotateGesturesEnabled(true).rotateGesturesEnabled)
        Assert.assertFalse(MapboxMapOptions().rotateGesturesEnabled(false).rotateGesturesEnabled)
    }

    @Test
    fun testCamera() {
        val position = CameraPosition.Builder().build()
        Assert.assertEquals(
            CameraPosition.Builder(position).build(),
            MapboxMapOptions().camera(position).camera
        )
        Assert.assertNotEquals(
            CameraPosition.Builder().target(LatLng(1.0, 1.0)),
            MapboxMapOptions().camera(position)
        )
        Assert.assertNull(MapboxMapOptions().camera)
    }

    @Test
    fun testPrefetchesTiles() {
        // Default value
        Assert.assertTrue(MapboxMapOptions().prefetchesTiles)

        // Check mutations
        Assert.assertTrue(MapboxMapOptions().setPrefetchesTiles(true).prefetchesTiles)
        Assert.assertFalse(MapboxMapOptions().setPrefetchesTiles(false).prefetchesTiles)
    }

    @Test
    fun testPrefetchZoomDelta() {
        // Default value
        Assert.assertEquals(4, MapboxMapOptions().prefetchZoomDelta)

        // Check mutations
        Assert.assertEquals(
            5,
            MapboxMapOptions().setPrefetchZoomDelta(5).prefetchZoomDelta
        )
    }

    @Test
    fun testCrossSourceCollisions() {
        // Default value
        Assert.assertTrue(MapboxMapOptions().crossSourceCollisions)

        // check mutations
        Assert.assertTrue(MapboxMapOptions().crossSourceCollisions(true).crossSourceCollisions)
        Assert.assertFalse(MapboxMapOptions().crossSourceCollisions(false).crossSourceCollisions)
    }

    @Test
    fun testLocalIdeographFontFamily_enabledByDefault() {
        val options = MapboxMapOptions.createFromAttributes(RuntimeEnvironment.application, null)
        Assert.assertEquals(
            MapboxConstants.DEFAULT_FONT,
            options.localIdeographFontFamily
        )
    }

    companion object {
        private const val DELTA = 1e-15
    }
}
