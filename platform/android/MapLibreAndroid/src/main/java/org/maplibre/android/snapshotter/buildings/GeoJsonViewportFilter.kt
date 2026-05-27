package org.maplibre.android.snapshotter.buildings

import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Geometry
import org.maplibre.turf.TurfMeasurement
import kotlin.math.cos
import kotlin.math.max
import kotlin.math.min

/**
 * Filters GeoJSON [Feature]s to those whose Turf bounding box intersects a geographic viewport.
 *
 * Use this before attaching large [FeatureCollection]s to a [org.maplibre.android.style.sources.GeoJsonSource]
 * used with [org.maplibre.android.snapshotter.MapSnapshotter], so only buildings (or other polygons) near the
 * snapshot camera are loaded into memory.
 *
 * @see BuildingExtrusionSnapshot
 */
object GeoJsonViewportFilter {

    /**
     * Returns a new collection containing only features whose geometry bbox intersects [viewport],
     * optionally expanded by [paddingMeters] (approximate, using the viewport center latitude).
     */
    @JvmStatic
    @JvmOverloads
    fun filterFeaturesInBounds(
        collection: FeatureCollection,
        viewport: LatLngBounds,
        paddingMeters: Double = 0.0
    ): FeatureCollection {
        val expanded = expandBounds(viewport, paddingMeters)
        val out = ArrayList<Feature>()
        val features = collection.features() ?: emptyList()
        for (feature in features) {
            val geometry = feature.geometry() ?: continue
            if (featureBoundsIntersectsViewport(geometry, expanded)) {
                out.add(feature)
            }
        }
        return FeatureCollection.fromFeatures(out)
    }

    /**
     * Returns true if the axis-aligned bounding boxes of two [LatLngBounds] intersect.
     */
    @JvmStatic
    fun boundsIntersect(a: LatLngBounds, b: LatLngBounds): Boolean {
        return !(a.longitudeEast < b.longitudeWest || a.longitudeWest > b.longitudeEast ||
            a.latitudeNorth < b.latitudeSouth || a.latitudeSouth > b.latitudeNorth)
    }

    private fun featureBoundsIntersectsViewport(geometry: Geometry, viewport: LatLngBounds): Boolean {
        val bbox = try {
            TurfMeasurement.bbox(geometry)
        } catch (_: Throwable) {
            return false
        }
        if (bbox.size < 4) {
            return false
        }
        // Turf bbox: [minLng, minLat, maxLng, maxLat]
        val featureBounds = LatLngBounds.from(bbox[3], bbox[2], bbox[1], bbox[0])
        return boundsIntersect(featureBounds, viewport)
    }

    private fun expandBounds(bounds: LatLngBounds, paddingMeters: Double): LatLngBounds {
        if (paddingMeters <= 0.0) {
            return bounds
        }
        val centerLat = (bounds.latitudeNorth + bounds.latitudeSouth) / 2.0
        val latPad = paddingMeters / 111_320.0
        val cosLat = cos(Math.toRadians(centerLat)).coerceAtLeast(1e-6)
        val lonPad = paddingMeters / (111_320.0 * cosLat)
        return LatLngBounds.from(
            min(bounds.latitudeNorth + latPad, 90.0),
            min(bounds.longitudeEast + lonPad, GeometryConstants.MAX_LONGITUDE),
            max(bounds.latitudeSouth - latPad, -90.0),
            max(bounds.longitudeWest - lonPad, GeometryConstants.MIN_LONGITUDE)
        )
    }
}
