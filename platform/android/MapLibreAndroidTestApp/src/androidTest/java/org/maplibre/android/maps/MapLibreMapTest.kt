package org.maplibre.android.maps

import android.graphics.Color
import android.view.View
import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.UiController
import androidx.test.espresso.ViewAction
import androidx.test.espresso.matcher.ViewMatchers
import junit.framework.TestCase
import org.hamcrest.Matcher
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Ignore
import org.junit.Test
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.annotations.PolylineOptions
import org.maplibre.android.exceptions.InvalidMarkerPositionException
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap.InfoWindowAdapter
import org.maplibre.android.maps.renderer.MapRenderer.RenderingRefreshMode
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.EspressoTest

// J2K: IDE suggest "Add 'fun' modifier

/**
 * This test is responsible for testing the public API.
 *
 *
 * Methods executed on MapLibreMap are called from a ViewAction to ensure correct synchronisation
 * with the application UI-thread.
 *
 */
class MapLibreMapTest : EspressoTest() {
    //
    // InfoWindow
    //
    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testConcurrentInfoWindowEnabled() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            maplibreMap.isAllowConcurrentMultipleOpenInfoWindows = true
            assertTrue("ConcurrentWindows should be true", maplibreMap.isAllowConcurrentMultipleOpenInfoWindows)
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testConcurrentInfoWindowDisabled() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            maplibreMap.isAllowConcurrentMultipleOpenInfoWindows = false
            TestCase.assertFalse("ConcurrentWindows should be false", maplibreMap.isAllowConcurrentMultipleOpenInfoWindows)
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testInfoWindowAdapter() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val infoWindowAdapter = InfoWindowAdapter { marker: Marker? -> null }
            maplibreMap.infoWindowAdapter = infoWindowAdapter
            assertEquals("InfoWindowAdpter should be the same", infoWindowAdapter, maplibreMap.infoWindowAdapter)
        })
    }

    //
    // Annotations
    //
    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddMarker() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val marker = maplibreMap.addMarker(markerOptions)
            assertTrue("Marker should be contained", maplibreMap.markers.contains(marker))
        })
    }

    @Test(expected = InvalidMarkerPositionException::class)
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddMarkerInvalidPosition() {
        MarkerOptions().marker
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddMarkers() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerList: MutableList<BaseMarkerOptions<*, *>> = ArrayList()
            val markerOptions1 = MarkerOptions().position(LatLng()).title("a")
            val markerOptions2 = MarkerOptions().position(LatLng()).title("b")
            markerList.add(markerOptions1)
            markerList.add(markerOptions2)
            val markers = maplibreMap.addMarkers(markerList)
            assertEquals("Markers size should be 2", 2, maplibreMap.markers.size.toLong())
            assertTrue(maplibreMap.markers.contains(markers[0]))
            assertTrue(maplibreMap.markers.contains(markers[1]))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddMarkersEmpty() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerList: List<BaseMarkerOptions<*, *>> = ArrayList()
            maplibreMap.addMarkers(markerList)
            assertEquals("Markers size should be 0", 0, maplibreMap.markers.size.toLong())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddMarkersSingleMarker() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerList: MutableList<BaseMarkerOptions<*, *>> = ArrayList()
            val markerOptions = MarkerOptions().title("a").position(LatLng())
            markerList.add(markerOptions)
            val markers = maplibreMap.addMarkers(markerList)
            assertEquals("Markers size should be 1", 1, maplibreMap.markers.size.toLong())
            assertTrue(maplibreMap.markers.contains(markers[0]))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddPolygon() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polygonOptions = PolygonOptions().add(LatLng())
            val polygon = maplibreMap.addPolygon(polygonOptions)
            assertTrue("Polygon should be contained", maplibreMap.polygons.contains(polygon))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    @Ignore("Needs to be investigated https://github.com/maplibre/maplibre-native/issues/3425")
    fun testAddEmptyPolygon() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polygonOptions = PolygonOptions()
            val polygon = maplibreMap.addPolygon(polygonOptions)
            assertTrue("Polygon should be ignored", !maplibreMap.polygons.contains(polygon))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddPolygons() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polygonList: MutableList<PolygonOptions> = ArrayList()
            val polygonOptions1 = PolygonOptions().fillColor(Color.BLACK).add(LatLng())
            val polygonOptions2 = PolygonOptions().fillColor(Color.WHITE).add(LatLng())
            val polygonOptions3 = PolygonOptions()
            polygonList.add(polygonOptions1)
            polygonList.add(polygonOptions2)
            polygonList.add(polygonOptions3)
            maplibreMap.addPolygons(polygonList)
            assertEquals("Polygons size should be 2", 2, maplibreMap.polygons.size.toLong())
            assertTrue(maplibreMap.polygons.contains(polygonOptions1.polygon))
            assertTrue(maplibreMap.polygons.contains(polygonOptions2.polygon))
            assertTrue("Polygon should be ignored", !maplibreMap.polygons.contains(polygonOptions3.polygon))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun addPolygonsEmpty() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            maplibreMap.addPolygons(ArrayList())
            assertEquals("Polygons size should be 0", 0, maplibreMap.polygons.size.toLong())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun addPolygonsSingle() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polygonList: MutableList<PolygonOptions> = ArrayList()
            val polygonOptions = PolygonOptions().fillColor(Color.BLACK).add(LatLng())
            polygonList.add(polygonOptions)
            maplibreMap.addPolygons(polygonList)
            assertEquals("Polygons size should be 1", 1, maplibreMap.polygons.size.toLong())
            assertTrue(maplibreMap.polygons.contains(polygonOptions.polygon))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddPolyline() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polylineOptions = PolylineOptions().add(LatLng())
            val polyline = maplibreMap.addPolyline(polylineOptions)
            assertTrue("Polyline should be contained", maplibreMap.polylines.contains(polyline))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    @Ignore("Needs to be investigated https://github.com/maplibre/maplibre-native/issues/3425")
    fun testAddEmptyPolyline() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { _: UiController?, _: View? ->
            val polylineOptions = PolylineOptions()
            val polyline = maplibreMap.addPolyline(polylineOptions)
            assertTrue("Polyline should be ignored", !maplibreMap.polylines.contains(polyline))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddPolylines() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polylineList: MutableList<PolylineOptions> = ArrayList()
            val polygonOptions1 = PolylineOptions().color(Color.BLACK).add(LatLng())
            val polygonOptions2 = PolylineOptions().color(Color.WHITE).add(LatLng())
            val polygonOptions3 = PolylineOptions()
            polylineList.add(polygonOptions1)
            polylineList.add(polygonOptions2)
            polylineList.add(polygonOptions3)
            maplibreMap.addPolylines(polylineList)
            assertEquals("Polygons size should be 2", 2, maplibreMap.polylines.size.toLong())
            assertTrue(maplibreMap.polylines.contains(polygonOptions1.polyline))
            assertTrue(maplibreMap.polylines.contains(polygonOptions2.polyline))
            assertTrue(
                "Polyline should be ignored", !maplibreMap.polylines.contains(polygonOptions3.polyline)
            )
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddPolylinesEmpty() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            maplibreMap.addPolylines(ArrayList())
            assertEquals("Polygons size should be 0", 0, maplibreMap.polylines.size.toLong())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testAddPolylinesSingle() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polylineList: MutableList<PolylineOptions> = ArrayList()
            val polygonOptions = PolylineOptions().color(Color.BLACK).add(LatLng())
            polylineList.add(polygonOptions)
            maplibreMap.addPolylines(polylineList)
            assertEquals("Polygons size should be 1", 1, maplibreMap.polylines.size.toLong())
            assertTrue(maplibreMap.polylines.contains(polygonOptions.polyline))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemoveMarker() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val marker = maplibreMap.addMarker(markerOptions)
            maplibreMap.removeMarker(marker)
            assertTrue("Markers should be empty", maplibreMap.markers.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemovePolygon() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polygonOptions = PolygonOptions()
            val polygon = maplibreMap.addPolygon(polygonOptions)
            maplibreMap.removePolygon(polygon)
            assertTrue("Polygons should be empty", maplibreMap.polylines.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemovePolyline() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val polylineOptions = PolylineOptions()
            val polyline = maplibreMap.addPolyline(polylineOptions)
            maplibreMap.removePolyline(polyline)
            assertTrue("Polylines should be empty", maplibreMap.polylines.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemoveAnnotation() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val marker = maplibreMap.addMarker(markerOptions)
            maplibreMap.removeAnnotation(marker)
            assertTrue("Annotations should be empty", maplibreMap.annotations.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemoveAnnotationById() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            maplibreMap.addMarker(markerOptions)
            // id will always be 0 in unit tests
            maplibreMap.removeAnnotation(0)
            assertTrue("Annotations should be empty", maplibreMap.annotations.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemoveAnnotations() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerList: MutableList<BaseMarkerOptions<*, *>> = ArrayList()
            val markerOptions1 = MarkerOptions().title("a").position(LatLng())
            val markerOptions2 = MarkerOptions().title("b").position(LatLng())
            markerList.add(markerOptions1)
            markerList.add(markerOptions2)
            maplibreMap.addMarkers(markerList)
            maplibreMap.removeAnnotations()
            assertTrue("Annotations should be empty", maplibreMap.annotations.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testClear() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerList: MutableList<BaseMarkerOptions<*, *>> = ArrayList()
            val markerOptions1 = MarkerOptions().title("a").position(LatLng())
            val markerOptions2 = MarkerOptions().title("b").position(LatLng())
            markerList.add(markerOptions1)
            markerList.add(markerOptions2)
            maplibreMap.addMarkers(markerList)
            maplibreMap.clear()
            assertTrue("Annotations should be empty", maplibreMap.annotations.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testRemoveAnnotationsByList() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerList: MutableList<BaseMarkerOptions<*, *>> = ArrayList()
            val markerOptions1 = MarkerOptions().title("a").position(LatLng())
            val markerOptions2 = MarkerOptions().title("b").position(LatLng())
            markerList.add(markerOptions1)
            markerList.add(markerOptions2)
            val markers = maplibreMap.addMarkers(markerList)
            val marker = maplibreMap.addMarker(MarkerOptions().position(LatLng()).title("c"))
            maplibreMap.removeAnnotations(markers)
            assertTrue("Annotations should not be empty", maplibreMap.annotations.size == 1)
            assertTrue("Marker should be contained", maplibreMap.annotations.contains(marker))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testGetAnnotationById() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val initialMarker = maplibreMap.addMarker(markerOptions)
            val retrievedMarker = maplibreMap.getAnnotation(0) as Marker?
            assertEquals("Markers should match", initialMarker, retrievedMarker)
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testGetAnnotations() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(
            MapLibreMapAction { uiController: UiController?, view: View? -> TestCase.assertNotNull("Annotations should be non null", maplibreMap.annotations) }
        )
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testGetMarkers() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(
            MapLibreMapAction { uiController: UiController?, view: View? -> TestCase.assertNotNull("Markers should be non null", maplibreMap.markers) }
        )
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testGetPolygons() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? -> TestCase.assertNotNull("Polygons should be non null", maplibreMap.polygons) }
        )
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testGetPolylines() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? -> TestCase.assertNotNull("Polylines should be non null", maplibreMap.polylines) }
        )
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testGetSelectedMarkers() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? -> TestCase.assertNotNull("Selected markers should be non null", maplibreMap.selectedMarkers) }
        )
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testSelectMarker() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val marker = maplibreMap.addMarker(markerOptions)
            maplibreMap.selectMarker(marker)
            assertTrue("Marker should be contained", maplibreMap.selectedMarkers.contains(marker))
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testDeselectMarker() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val marker = maplibreMap.addMarker(markerOptions)
            maplibreMap.selectMarker(marker)
            maplibreMap.deselectMarker(marker)
            assertTrue("Selected markers should be empty", maplibreMap.selectedMarkers.isEmpty())
        })
    }

    @Test
    @Deprecated("remove this test when removing deprecated annotations")
    fun testDeselectMarkers() {
        validateTestSetup()
        onView(ViewMatchers.withId(R.id.mapView)).perform(MapLibreMapAction { uiController: UiController?, view: View? ->
            val markerOptions = MarkerOptions().position(LatLng())
            val marker1 = maplibreMap.addMarker(markerOptions)
            val marker2 = maplibreMap.addMarker(markerOptions)
            maplibreMap.selectMarker(marker1)
            maplibreMap.selectMarker(marker2)
            maplibreMap.deselectMarkers()
            assertTrue("Selected markers should be empty", maplibreMap.selectedMarkers.isEmpty())
        })
    }

    @Test
    fun testTileCache() {
        validateTestSetup()
        rule.runOnUiThread {
            maplibreMap.tileCacheEnabled = false
            assertTrue(maplibreMap.tileCacheEnabled == false)

            maplibreMap.tileCacheEnabled = true
            assertTrue(maplibreMap.tileCacheEnabled == true)
        }
    }

    @Test
    fun testRenderingRefreshMode() {
        validateTestSetup()
        rule.runOnUiThread {
            mapView = rule.getActivity().findViewById(R.id.mapView)
            // Default RenderingRefreshMode is WHEN_DIRTY
            assertTrue(mapView.getRenderingRefreshMode() == RenderingRefreshMode.WHEN_DIRTY)
            // Switch to CONTINUOUS rendering
            mapView.setRenderingRefreshMode(RenderingRefreshMode.CONTINUOUS)
            assertTrue(mapView.getRenderingRefreshMode() == RenderingRefreshMode.CONTINUOUS)
        }
    }

    @Test
    fun testSetFrustumOffset() {
        validateTestSetup()
        rule.runOnUiThread {
            // RectF(left, top, right, bottom) - frustum offset represents insets in pixels
            val offset = android.graphics.RectF(20f, 50f, 30f, 40f)
            maplibreMap.setFrustumOffset(offset)
            // Verify that setFrustumOffset was called without throwing an exception
            // Since there's no getter, we can only verify it doesn't crash
        }
    }

    @Test
    fun testSetFrustumOffsetZero() {
        validateTestSetup()
        rule.runOnUiThread {
            val zeroOffset = android.graphics.RectF(0f, 0f, 0f, 0f)
            maplibreMap.setFrustumOffset(zeroOffset)
            // Verify that setFrustumOffset with zero values works
        }
    }

    @Test
    fun testSetFrustumOffsetMultipleChanges() {
        validateTestSetup()
        rule.runOnUiThread {
            val firstOffset = android.graphics.RectF(10f, 20f, 30f, 40f)
            maplibreMap.setFrustumOffset(firstOffset)

            val secondOffset = android.graphics.RectF(100f, 200f, 300f, 400f)
            maplibreMap.setFrustumOffset(secondOffset)

            val zeroOffset = android.graphics.RectF(0f, 0f, 0f, 0f)
            maplibreMap.setFrustumOffset(zeroOffset)
            // Verify that multiple changes to frustum offset work correctly
        }
    }

    @Deprecated("remove this class when removing deprecated annotations")
    inner class MapLibreMapAction internal constructor(private val invokeViewAction: InvokeViewAction) : ViewAction {
        override fun getConstraints(): Matcher<View> {
            return ViewMatchers.isDisplayed()
        }

        override fun getDescription(): String {
            return javaClass.simpleName
        }

        override fun perform(uiController: UiController, view: View) {
            invokeViewAction.onViewAction(uiController, view)
        }
    }

    @Deprecated("remove this interface when removing deprecated annotations")
    internal fun interface InvokeViewAction {
        fun onViewAction(uiController: UiController?, view: View?)
    }
}
