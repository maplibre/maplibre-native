package org.maplibre.android.location

import org.maplibre.geojson.Feature
import org.maplibre.geojson.Point

internal class LayerFeatureProvider {
    fun generateLocationFeature(
        locationFeature: Feature?,
        isStale: Boolean,
    ): Feature {
        if (locationFeature != null) {
            return locationFeature
        }
        val feature = Feature.fromGeometry(Point.fromLngLat(0.0, 0.0))
        feature.addNumberProperty(LocationComponentConstants.PROPERTY_GPS_BEARING, 0f)
        feature.addNumberProperty(LocationComponentConstants.PROPERTY_COMPASS_BEARING, 0f)
        feature.addBooleanProperty(LocationComponentConstants.PROPERTY_LOCATION_STALE, isStale)
        return feature
    }
}
