package org.maplibre.android.testapp.projection

import android.graphics.PointF
import android.graphics.RectF
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.CoordinateReferenceSystem
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.sources.CustomGeometrySource
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.style.sources.GeometryTileProvider
import org.maplibre.android.testapp.activity.BaseTest
import org.maplibre.android.testapp.activity.projection.NationalGridActivity
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.geojson.Polygon
import java.util.concurrent.ConcurrentLinkedQueue
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import kotlin.math.abs

/**
 * Tests for [Style.addProjection], [Style.removeProjection] and maps drawn in a projection registered from Kotlin.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class CoordinateReferenceSystemTest : BaseTest() {

    override fun getActivityClass(): Class<*> = NationalGridActivity::class.java

    @Test
    fun refusesWhatCoreRefuses() {
        val tileMatrixSet = CoordinateReferenceSystem.TileMatrixSet(-180.0, 180.0, 360.0)
        val projection = TestProjections.banded("test:any", false).projection
        assertThrows(IllegalArgumentException::class.java) {
            Style.addProjection(CoordinateReferenceSystem("no-colon", tileMatrixSet, projection))
        }
        assertThrows(IllegalArgumentException::class.java) {
            Style.addProjection(CoordinateReferenceSystem(NationalGrid.NAME, tileMatrixSet, projection))
        }
        for (extent in listOf(0.0, -360.0, Double.NaN)) {
            assertThrows(IllegalArgumentException::class.java) {
                Style.addProjection(
                    CoordinateReferenceSystem("test:empty", tileMatrixSet.copy(extentAtZoom0 = extent), projection)
                )
            }
        }
        assertThrows(IllegalArgumentException::class.java) { Style.removeProjection("mercator") }
        Style.removeProjection("test:never-registered")
    }

    @Test
    fun measuresMetersPerPixelAsGlJs() {
        withProjection(TestProjections.polarStereographic("test:polar-scale")) {
            loadStyle("test:polar-scale", LatLng(70.0, -45.0), 4.0)
            // GL JS's numbers for the same projection and cameras, from the core test
            // `Map.MetersPerPixelInARegisteredProjection`. They depend on the location and zoom, not the center.
            val cameras = listOf(
                Triple(LatLng(70.0, -45.0), 4.0, 1022.8325840486776),
                Triple(LatLng(81.0, 40.0), 6.0, 261.9986619836209),
                Triple(LatLng(60.1, -100.2), 8.0, 61.535774394909105)
            )
            for ((location, zoom, metersPerPixel) in cameras) {
                val measured = onMain {
                    maplibreMap.moveCamera(
                        CameraUpdateFactory.newCameraPosition(
                            CameraPosition.Builder().target(location).zoom(zoom).bearing(0.0).tilt(0.0).build()
                        )
                    )
                    maplibreMap.projection.getMetersPerPixelAtLatitude(location.latitude)
                }
                assertEquals(metersPerPixel, measured, metersPerPixel * 1e-9)
            }
        }
    }

    @Test
    fun measuresALatitudePastAPoleAtTheClampedLatitude() {
        // In any projection, a latitude past a pole or NaN gets Mercator's scale at the latitude it clamps to.
        withProjection(TestProjections.polarStereographic("test:polar-past-pole")) {
            loadStyle("mercator", LatLng(70.0, -45.0), 4.0)
            val atPole = onMain { maplibreMap.projection.getMetersPerPixelAtLatitude(90.0) }
            for (projection in listOf("mercator", "test:polar-past-pole")) {
                loadStyle(projection, LatLng(70.0, -45.0), 4.0)
                for (latitude in listOf(100.0, -100.0, Double.NaN)) {
                    val measured = onMain { maplibreMap.projection.getMetersPerPixelAtLatitude(latitude) }
                    assertEquals("$latitude in $projection", atPole, measured, atPole * 1e-12)
                }
            }
        }
    }

    @Test
    fun tilesGeoJsonFromKotlinInTheProjection() {
        withProjection(TestProjections.polarStereographic("test:polar-geojson")) {
            loadStyle("test:polar-geojson", LatLng(75.0, 0.0), 3.0)
            // Data given before the source joins the style, data converted before it joins, and data set after.
            addCircles("before") { GeoJsonSource("before", Point.fromLngLat(0.0, 75.0)) }
            val converted = onMain { GeoJsonSource("converted", Point.fromLngLat(-5.0, 74.0)) }
            Thread.sleep(CONVERSION_MILLIS)
            addCircles("converted") { converted }
            awaitSourceData("after") {
                val after = GeoJsonSource("after")
                addCircleLayer(after)
                after.setGeoJson(Point.fromLngLat(5.0, 74.0))
            }
            assertCircleAt("before", LatLng(75.0, 0.0))
            assertCircleAt("converted", LatLng(74.0, -5.0))
            assertCircleAt("after", LatLng(74.0, 5.0))
        }
    }

    @Test
    fun keepsAProjectionRemovedWhileAMapUsesIt() {
        val name = "test:polar-removed"
        withProjection(TestProjections.polarStereographic(name)) {
            loadStyle(name, LatLng(75.0, 0.0), 3.0)
            val place = LatLng(75.0, 0.0)
            val before = onMain { maplibreMap.projection.toScreenLocation(place) }

            Style.removeProjection(name)
            assertEquals(before, onMain { maplibreMap.projection.toScreenLocation(place) })
            addCircles("points") { GeoJsonSource("points", Point.fromLngLat(place.longitude, place.latitude)) }
            assertCircleAt("points", place)

            Style.addProjection(TestProjections.polarStereographic(name))
        }
    }

    @Test
    fun leavesOutAPositionTheConverterHasNoAnswerFor() {
        withProjection(TestProjections.banded("test:band", false)) {
            loadStyle("test:band", LatLng(10.0, 0.0), 2.0)
            addCircles("points") { GeoJsonSource("points", westInEast()) }
            assertEquals(setOf("west", "east"), namesDrawn("points"))
        }
    }

    @Test
    fun givesNoAnswerForACallThatThrows() {
        withProjection(TestProjections.banded("test:band-throws", true)) {
            loadStyle("test:band-throws", LatLng(10.0, 0.0), 2.0)
            addCircles("thrown") { GeoJsonSource("thrown", westInEast()) }
            assertEquals(emptySet<String>(), namesDrawn("thrown"))
            val outside = westInEast().features()!!.filter { it.getStringProperty("name") != "in" }
            addCircles("answered") { GeoJsonSource("answered", FeatureCollection.fromFeatures(outside)) }
            assertEquals(setOf("west", "east"), namesDrawn("answered"))
        }
    }

    @Test
    fun customGeometrySourceGetsEachTilesBoundsInTheProjection() {
        val crs = ArcticProjection.coordinateReferenceSystem
        withProjection(crs) {
            loadStyle(crs.name, LatLng(75.0, -45.0), 3.0)
            val requests = ConcurrentLinkedQueue<Pair<LatLngBounds, Int>>()
            val provider = object : GeometryTileProvider {
                override fun getFeaturesForBounds(bounds: LatLngBounds, zoomLevel: Int): FeatureCollection {
                    requests.add(bounds to zoomLevel)
                    return FeatureCollection.fromFeature(Feature.fromGeometry(boxPolygon(bounds)))
                }
            }
            val idle = CountDownLatch(1)
            val onIdle = MapView.OnDidBecomeIdleListener { if (requests.isNotEmpty()) idle.countDown() }
            rule.runOnUiThread {
                mapView.addOnDidBecomeIdleListener(onIdle)
                maplibreMap.style!!.addSource(CustomGeometrySource("tiles", provider))
                maplibreMap.style!!.addLayer(FillLayer("tiles", "tiles"))
            }
            val drawn = idle.await(WAIT_SECONDS, TimeUnit.SECONDS)
            rule.runOnUiThread { mapView.removeOnDidBecomeIdleListener(onIdle) }
            assertTrue("The custom geometry source's tiles are drawn", drawn)

            // Each tile draws the bounds it was given, so the view is covered only if each tile's bounds contain it.
            val uncovered = onMain {
                (1..GRID_SIDE).flatMap { i -> (1..GRID_SIDE).map { j -> i to j } }.map { (i, j) ->
                    PointF(mapView.width * i / (GRID_SIDE + 1f), mapView.height * j / (GRID_SIDE + 1f))
                }.filter { maplibreMap.queryRenderedFeatures(it, "tiles").isEmpty() }
            }
            assertEquals("Points of the view outside every tile's box", emptyList<PointF>(), uncovered)

            for ((bounds, zoom) in requests) {
                val tiles = (0 until (1 shl zoom)).flatMap { x -> (0 until (1 shl zoom)).map { y -> x to y } }
                assertTrue(
                    "$bounds at zoom $zoom is the box of a tile's corners",
                    tiles.any { (x, y) -> sameBounds(bounds, tileCornersBox(crs, zoom, x, y)) }
                )
            }
        }
    }

    /** A polygon around the box, with each side in steps, so that it follows the parallels and meridians. */
    private fun boxPolygon(bounds: LatLngBounds): Polygon {
        val west = bounds.longitudeWest
        val east = bounds.longitudeEast
        val south = bounds.latitudeSouth
        val north = bounds.latitudeNorth
        val ring = (0 until BOX_STEPS).map { Point.fromLngLat(lerp(west, east, it), south) } +
            (0 until BOX_STEPS).map { Point.fromLngLat(east, lerp(south, north, it)) } +
            (0 until BOX_STEPS).map { Point.fromLngLat(lerp(east, west, it), north) } +
            (0..BOX_STEPS).map { Point.fromLngLat(west, lerp(north, south, it)) }
        return Polygon.fromLngLats(listOf(ring))
    }

    private fun lerp(from: Double, to: Double, step: Int): Double = from + (to - from) * step / BOX_STEPS

    /** GL JS's `tileIdToLngLatBounds`: the box of the longitudes and latitudes of the tile's corners. */
    private fun tileCornersBox(crs: CoordinateReferenceSystem, z: Int, x: Int, y: Int): LatLngBounds {
        val tileMatrixSet = crs.tileMatrixSet
        val corners = listOf(x to y, x + 1 to y, x + 1 to y + 1, x to y + 1)
        val positions = corners.flatMap { (cornerX, cornerY) ->
            listOf(
                tileMatrixSet.originX + cornerX.toDouble() / (1 shl z) * tileMatrixSet.extentAtZoom0,
                tileMatrixSet.originY - cornerY.toDouble() / (1 shl z) * tileMatrixSet.extentAtZoom0
            )
        }.toDoubleArray()
        val lngLats = DoubleArray(positions.size)
        crs.projection.inverse(positions, lngLats, corners.size)
        val longitudes = lngLats.filterIndexed { i, _ -> i % 2 == 0 }
        val latitudes = lngLats.filterIndexed { i, _ -> i % 2 == 1 }
        return LatLngBounds.from(latitudes.max(), longitudes.max(), latitudes.min(), longitudes.min())
    }

    private fun sameBounds(a: LatLngBounds, b: LatLngBounds): Boolean =
        listOf(
            a.latitudeNorth - b.latitudeNorth,
            a.longitudeEast - b.longitudeEast,
            a.latitudeSouth - b.latitudeSouth,
            a.longitudeWest - b.longitudeWest
        ).all { abs(it) < BOUNDS_TOLERANCE_DEGREES }

    /** Points west of the converter's band, in it and east of it, in one call to the converter. */
    private fun westInEast(): FeatureCollection = FeatureCollection.fromFeatures(
        listOf("west" to 10.0, "in" to 20.0, "east" to 30.0).map { (name, longitude) ->
            Feature.fromGeometry(Point.fromLngLat(longitude, 10.0)).apply { addStringProperty("name", name) }
        }
    )

    private fun withProjection(projection: CoordinateReferenceSystem, test: () -> Unit) {
        Style.addProjection(projection)
        try {
            test()
        } finally {
            Style.removeProjection(projection.name)
        }
    }

    /** Loads an empty style in the projection and moves the camera to [center] and [zoom]. */
    private fun loadStyle(projection: String, center: LatLng, zoom: Double) {
        val latch = CountDownLatch(1)
        val json = """{"version": 8, "projection": {"type": "$projection"}, "sources": {}, "layers": []}"""
        rule.runOnUiThread {
            maplibreMap.setStyle(Style.Builder().fromJson(json)) {
                maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(center, zoom))
                latch.countDown()
            }
        }
        assertTrue("The style loads", latch.await(WAIT_SECONDS, TimeUnit.SECONDS))
    }

    /** Adds the source and a circle layer of the same name, and waits until the source's data is drawn. */
    private fun addCircles(id: String, source: () -> GeoJsonSource) = awaitSourceData(id) { addCircleLayer(source()) }

    private fun addCircleLayer(source: GeoJsonSource) {
        maplibreMap.style!!.addSource(source)
        maplibreMap.style!!.addLayer(CircleLayer(source.id, source.id))
    }

    /** Runs [change] on the main thread and waits until the source has new data and the map is idle again. */
    private fun awaitSourceData(id: String, change: () -> Unit) {
        val latch = CountDownLatch(1)
        var changed = false
        val onSourceChanged = MapView.OnSourceChangedListener { if (it == id) changed = true }
        val onIdle = MapView.OnDidBecomeIdleListener { if (changed) latch.countDown() }
        rule.runOnUiThread {
            mapView.addOnSourceChangedListener(onSourceChanged)
            mapView.addOnDidBecomeIdleListener(onIdle)
            change()
        }
        val drawn = latch.await(WAIT_SECONDS, TimeUnit.SECONDS)
        rule.runOnUiThread {
            mapView.removeOnSourceChangedListener(onSourceChanged)
            mapView.removeOnDidBecomeIdleListener(onIdle)
        }
        assertTrue("The data of source $id is drawn", drawn)
    }

    private fun assertCircleAt(layer: String, place: LatLng) {
        val features = onMain {
            maplibreMap.queryRenderedFeatures(maplibreMap.projection.toScreenLocation(place), layer)
        }
        assertEquals("A circle of $layer at $place", 1, features.size)
        val point = features[0].geometry() as Point
        assertEquals(0.0, place.distanceTo(LatLng(point.latitude(), point.longitude())), QUERY_TOLERANCE_METERS)
    }

    private fun namesDrawn(layer: String): Set<String> = onMain {
        val view = RectF(0f, 0f, mapView.width.toFloat(), mapView.height.toFloat())
        maplibreMap.queryRenderedFeatures(view, layer).map { it.getStringProperty("name") }.toSet()
    }

    private fun <T> onMain(block: () -> T): T {
        var result: Result<T>? = null
        rule.runOnUiThread { result = runCatching(block) }
        return result!!.getOrThrow()
    }

    private companion object {
        const val WAIT_SECONDS = 30L

        /** A queried feature comes back from its tile, rounded to the tile's units: 128 m at zoom 3 here. */
        const val QUERY_TOLERANCE_METERS = 200.0

        /** Long enough for a source to convert a point on the main thread's behalf. */
        const val CONVERSION_MILLIS = 1000L

        /** The view is checked at this many points on a side. */
        const val GRID_SIDE = 5

        /** The steps on each side of a box polygon. */
        const val BOX_STEPS = 32

        /** Core and the test take the same corners through the same inverse, so their boxes agree this closely. */
        const val BOUNDS_TOLERANCE_DEGREES = 1e-9
    }
}
