package org.maplibre.android.location;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.geojson.Feature;
import org.maplibre.geojson.Point;

class LayerFeatureProvider {

  public static final String featureId = "maplibre:location-component";

  @NonNull
  Feature generateLocationFeature(@Nullable Feature locationFeature, boolean isStale) {
    if (locationFeature != null) {
      return locationFeature;
    }
    locationFeature = Feature.fromGeometry(Point.fromLngLat(0.0, 0.0), null, featureId, null);
    locationFeature.addNumberProperty(LocationComponentConstants.PROPERTY_GPS_BEARING, 0f);
    locationFeature.addNumberProperty(LocationComponentConstants.PROPERTY_COMPASS_BEARING, 0f);
    locationFeature.addBooleanProperty(LocationComponentConstants.PROPERTY_LOCATION_STALE, isStale);
    return locationFeature;
  }
}
