package org.maplibre.android.snapshotter.buildings

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.geojson.Polygon

class GeoJsonViewportFilterTest {

    @Test
    fun boundsIntersect_overlapping() {
        val a = LatLngBounds.from(10.0, 10.0, 0.0, 0.0)
        val b = LatLngBounds.from(5.0, 5.0, 5.0, 5.0)
        assertTrue(GeoJsonViewportFilter.boundsIntersect(a, b))
    }

    @Test
    fun boundsIntersect_disjoint() {
        val a = LatLngBounds.from(10.0, 10.0, 0.0, 0.0)
        val b = LatLngBounds.from(20.0, 20.0, 11.0, 11.0)
        assertFalse(GeoJsonViewportFilter.boundsIntersect(a, b))
    }

    @Test
    fun filterFeaturesInBounds_keepsInsidePoint() {
        val viewport = LatLngBounds.from(51.2, 5.8, 51.0, 5.5)
        val inside = Feature.fromGeometry(Point.fromLngLat(5.7, 51.1))
        val outside = Feature.fromGeometry(Point.fromLngLat(4.0, 50.0))
        val collection = FeatureCollection.fromFeatures(listOf(inside, outside))
        val filtered = GeoJsonViewportFilter.filterFeaturesInBounds(collection, viewport)
        assertEquals(1, filtered.features()?.size ?: 0)
    }

    @Test
    fun filterFeaturesInBounds_keepsIntersectingPolygon() {
        val viewport = LatLngBounds.from(51.15, 5.75, 51.14, 5.74)
        val ring = listOf(
            Point.fromLngLat(5.741, 51.145),
            Point.fromLngLat(5.743, 51.145),
            Point.fromLngLat(5.743, 51.146),
            Point.fromLngLat(5.741, 51.146),
            Point.fromLngLat(5.741, 51.145)
        )
        val poly = Feature.fromGeometry(Polygon.fromLngLats(listOf(ring)))
        val far = Feature.fromGeometry(Point.fromLngLat(0.0, 0.0))
        val collection = FeatureCollection.fromFeatures(listOf(poly, far))
        val filtered = GeoJsonViewportFilter.filterFeaturesInBounds(collection, viewport)
        assertEquals(1, filtered.features()?.size ?: 0)
    }
}
