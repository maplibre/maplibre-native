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
        junit.framework.Assert.assertFalse(MapboxMapOptions().debugActive)
        Assert.assertTrue(MapboxMapOptions().debugActive(true).debugActive)
        junit.framework.Assert.assertFalse(MapboxMapOptions().debugActive(false).debugActive)
    }

    @Test
    fun testCompassEnabled() {
        Assert.assertTrue(MapboxMapOptions().compassEnabled(true).compassEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().compassEnabled(false).compassEnabled)
    }

    @Test
    fun testCompassGravity() {
        junit.framework.Assert.assertEquals(
            Gravity.TOP or Gravity.END,
            MapboxMapOptions().compassGravity
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().compassMargins(intArrayOf(0, 0, 0, 0)).compassMargins
            )
        )
    }

    @Test
    fun testLogoEnabled() {
        Assert.assertTrue(MapboxMapOptions().logoEnabled(true).logoEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().logoEnabled(false).logoEnabled)
    }

    @Test
    fun testLogoGravity() {
        junit.framework.Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MapboxMapOptions().logoGravity
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().logoMargins(intArrayOf(0, 0, 0, 0)).logoMargins
            )
        )
    }

    @Test
    fun testAttributionTintColor() {
        junit.framework.Assert.assertEquals(-1, MapboxMapOptions().attributionTintColor)
        junit.framework.Assert.assertEquals(
            Color.RED,
            MapboxMapOptions().attributionTintColor(Color.RED).attributionTintColor
        )
    }

    @Test
    fun testAttributionEnabled() {
        Assert.assertTrue(MapboxMapOptions().attributionEnabled(true).attributionEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().attributionEnabled(false).attributionEnabled)
    }

    @Test
    fun testAttributionGravity() {
        junit.framework.Assert.assertEquals(
            Gravity.BOTTOM or Gravity.START,
            MapboxMapOptions().attributionGravity
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertFalse(
            Arrays.equals(
                intArrayOf(0, 1, 2, 3),
                MapboxMapOptions().attributionMargins(intArrayOf(0, 0, 0, 0)).attributionMargins
            )
        )
    }

    @Test
    fun testMinZoom() {
        junit.framework.Assert.assertEquals(
            MapboxConstants.MINIMUM_ZOOM.toDouble(),
            MapboxMapOptions().minZoomPreference,
            DELTA
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertEquals(
            MapboxConstants.MAXIMUM_ZOOM.toDouble(),
            MapboxMapOptions().maxZoomPreference,
            DELTA
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertEquals(
            MapboxConstants.MINIMUM_PITCH.toDouble(),
            MapboxMapOptions().minPitchPreference,
            DELTA
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertEquals(
            MapboxConstants.MAXIMUM_PITCH.toDouble(),
            MapboxMapOptions().maxPitchPreference,
            DELTA
        )
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertFalse(MapboxMapOptions().tiltGesturesEnabled(false).tiltGesturesEnabled)
    }

    @Test
    fun testScrollGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().scrollGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().scrollGesturesEnabled(true).scrollGesturesEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().scrollGesturesEnabled(false).scrollGesturesEnabled)
    }

    @Test
    fun testHorizontalScrollGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().horizontalScrollGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().horizontalScrollGesturesEnabled(true).horizontalScrollGesturesEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().horizontalScrollGesturesEnabled(false).horizontalScrollGesturesEnabled)
    }

    @Test
    fun testZoomGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().zoomGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().zoomGesturesEnabled(true).zoomGesturesEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().zoomGesturesEnabled(false).zoomGesturesEnabled)
    }

    @Test
    fun testRotateGesturesEnabled() {
        Assert.assertTrue(MapboxMapOptions().rotateGesturesEnabled)
        Assert.assertTrue(MapboxMapOptions().rotateGesturesEnabled(true).rotateGesturesEnabled)
        junit.framework.Assert.assertFalse(MapboxMapOptions().rotateGesturesEnabled(false).rotateGesturesEnabled)
    }

    @Test
    fun testCamera() {
        val position = CameraPosition.Builder().build()
        junit.framework.Assert.assertEquals(
            CameraPosition.Builder(position).build(),
            MapboxMapOptions().camera(position).camera
        )
        Assert.assertNotEquals(
            CameraPosition.Builder().target(LatLng(1.0, 1.0)),
            MapboxMapOptions().camera(position)
        )
        junit.framework.Assert.assertNull(MapboxMapOptions().camera)
    }

    @Test
    fun testPrefetchesTiles() {
        // Default value
        Assert.assertTrue(MapboxMapOptions().prefetchesTiles)

        // Check mutations
        Assert.assertTrue(MapboxMapOptions().setPrefetchesTiles(true).prefetchesTiles)
        junit.framework.Assert.assertFalse(MapboxMapOptions().setPrefetchesTiles(false).prefetchesTiles)
    }

    @Test
    fun testPrefetchZoomDelta() {
        // Default value
        junit.framework.Assert.assertEquals(4, MapboxMapOptions().prefetchZoomDelta)

        // Check mutations
        junit.framework.Assert.assertEquals(
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
        junit.framework.Assert.assertFalse(MapboxMapOptions().crossSourceCollisions(false).crossSourceCollisions)
    }

    @Test
    fun testLocalIdeographFontFamily_enabledByDefault() {
        val options = MapboxMapOptions.createFromAttributes(RuntimeEnvironment.application, null)
        junit.framework.Assert.assertEquals(
            MapboxConstants.DEFAULT_FONT,
            options.localIdeographFontFamily
        )
    }

    companion object {
        private const val DELTA = 1e-15
    }
}
