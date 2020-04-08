package com.mapbox.mapboxsdk.location;

import android.graphics.Bitmap;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.Point;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.location.modes.RenderMode;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.style.expressions.Expression;
import com.mapbox.mapboxsdk.style.layers.Layer;
import com.mapbox.mapboxsdk.style.layers.SymbolLayer;
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource;

import java.util.Set;

import static com.mapbox.mapboxsdk.location.LocationComponentConstants.ACCURACY_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BEARING_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BEARING_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.LOCATION_SOURCE;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_ACCURACY_ALPHA;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_ACCURACY_COLOR;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_ACCURACY_RADIUS;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_BACKGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_BACKGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_BEARING_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_COMPASS_BEARING;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_FOREGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_FOREGROUND_ICON_OFFSET;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_FOREGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_GPS_BEARING;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_LOCATION_STALE;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.PROPERTY_SHADOW_ICON_OFFSET;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.SHADOW_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.SHADOW_LAYER;
import static com.mapbox.mapboxsdk.style.layers.Property.NONE;
import static com.mapbox.mapboxsdk.style.layers.Property.VISIBLE;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconSize;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.visibility;
import static com.mapbox.mapboxsdk.utils.ColorUtils.colorToRgbaString;

final class SymbolLocationLayerRenderer implements LocationLayerRenderer {
  private Style style;
  private final LayerSourceProvider layerSourceProvider;

  private final Set<String> layerSet;
  private Feature locationFeature;
  private GeoJsonSource locationSource;

  SymbolLocationLayerRenderer(Style style,
                              LayerSourceProvider layerSourceProvider,
                              LayerFeatureProvider featureProvider,
                              boolean isStale) {
    this.style = style;
    this.layerSourceProvider = layerSourceProvider;
    this.layerSet = layerSourceProvider.getEmptyLayerSet();
    this.locationFeature = featureProvider.generateLocationFeature(locationFeature, isStale);
  }

  @Override
  public void initializeComponents(Style style) {
    this.style = style;
    addLocationSource();
  }

  @Override
  public void addLayers(LocationComponentPositionManager positionManager) {
    // positions the top-most reference layer
    Layer layer = layerSourceProvider.generateLayer(BEARING_LAYER);
    positionManager.addLayerToMap(layer);
    layerSet.add(layer.getId());

    // adds remaining layers while keeping the order
    addSymbolLayer(FOREGROUND_LAYER, BEARING_LAYER);
    addSymbolLayer(BACKGROUND_LAYER, FOREGROUND_LAYER);
    addSymbolLayer(SHADOW_LAYER, BACKGROUND_LAYER);
    addAccuracyLayer();
  }

  @Override
  public void removeLayers() {
    for (String layerId : layerSet) {
      style.removeLayer(layerId);
    }
    layerSet.clear();
  }

  @Override
  public void hide() {
    for (String layerId : layerSet) {
      setLayerVisibility(layerId, false);
    }
  }

  @Override
  public void cameraTiltUpdated(double tilt) {
    updateForegroundOffset(tilt);
  }

  @Override
  public void cameraBearingUpdated(double bearing) {
    updateForegroundBearing((float) bearing);
  }

  @Override
  public void show(@RenderMode.Mode int renderMode, boolean isStale) {
    switch (renderMode) {
      case RenderMode.NORMAL:
        setLayerVisibility(SHADOW_LAYER, true);
        setLayerVisibility(FOREGROUND_LAYER, true);
        setLayerVisibility(BACKGROUND_LAYER, true);
        setLayerVisibility(ACCURACY_LAYER, !isStale);
        setLayerVisibility(BEARING_LAYER, false);
        break;
      case RenderMode.COMPASS:
        setLayerVisibility(SHADOW_LAYER, true);
        setLayerVisibility(FOREGROUND_LAYER, true);
        setLayerVisibility(BACKGROUND_LAYER, true);
        setLayerVisibility(ACCURACY_LAYER, !isStale);
        setLayerVisibility(BEARING_LAYER, true);
        break;
      case RenderMode.GPS:
        setLayerVisibility(SHADOW_LAYER, false);
        setLayerVisibility(FOREGROUND_LAYER, true);
        setLayerVisibility(BACKGROUND_LAYER, true);
        setLayerVisibility(ACCURACY_LAYER, false);
        setLayerVisibility(BEARING_LAYER, false);
        break;
      default:
        break;
    }
  }

  @Override
  public void styleAccuracy(float accuracyAlpha, int accuracyColor) {
    locationFeature.addNumberProperty(PROPERTY_ACCURACY_ALPHA, accuracyAlpha);
    locationFeature.addStringProperty(PROPERTY_ACCURACY_COLOR, colorToRgbaString(accuracyColor));
    refreshSource();
  }

  @Override
  public void setLatLng(LatLng latLng) {
    Point point = Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude());
    setLocationPoint(point);
  }

  @Override
  public void setGpsBearing(Float gpsBearing) {
    setBearingProperty(PROPERTY_GPS_BEARING, gpsBearing);
  }

  @Override
  public void setCompassBearing(Float compassBearing) {
    setBearingProperty(PROPERTY_COMPASS_BEARING, compassBearing);
  }

  @Override
  public void setAccuracyRadius(Float accuracy) {
    updateAccuracyRadius(accuracy);
  }

  @Override
  public void styleScaling(Expression scaleExpression) {
    for (String layerId : layerSet) {
      Layer layer = style.getLayer(layerId);
      if (layer instanceof SymbolLayer) {
        layer.setProperties(
          iconSize(scaleExpression)
        );
      }
    }
  }

  @Override
  public void setLocationStale(boolean isStale, int renderMode) {
    locationFeature.addBooleanProperty(PROPERTY_LOCATION_STALE, isStale);
    refreshSource();
    if (renderMode != RenderMode.GPS) {
      setLayerVisibility(ACCURACY_LAYER, !isStale);
    }
  }

  @Override
  public void updateIconIds(String foregroundIconString, String foregroundStaleIconString, String backgroundIconString,
                            String backgroundStaleIconString, String bearingIconString) {
    locationFeature.addStringProperty(PROPERTY_FOREGROUND_ICON, foregroundIconString);
    locationFeature.addStringProperty(PROPERTY_BACKGROUND_ICON, backgroundIconString);
    locationFeature.addStringProperty(PROPERTY_FOREGROUND_STALE_ICON, foregroundStaleIconString);
    locationFeature.addStringProperty(PROPERTY_BACKGROUND_STALE_ICON, backgroundStaleIconString);
    locationFeature.addStringProperty(PROPERTY_BEARING_ICON, bearingIconString);
    refreshSource();
  }

  @Override
  public void addBitmaps(@RenderMode.Mode int renderMode, @Nullable Bitmap shadowBitmap, Bitmap backgroundBitmap,
                         Bitmap backgroundStaleBitmap, Bitmap bearingBitmap,
                         Bitmap foregroundBitmap, Bitmap foregroundBitmapStale) {
    if (shadowBitmap != null) {
      style.addImage(SHADOW_ICON, shadowBitmap);
    } else {
      style.removeImage(SHADOW_ICON);
    }
    style.addImage(BACKGROUND_ICON, backgroundBitmap);
    style.addImage(BACKGROUND_STALE_ICON, backgroundStaleBitmap);
    style.addImage(BEARING_ICON, bearingBitmap);
    style.addImage(FOREGROUND_ICON, foregroundBitmap);
    style.addImage(FOREGROUND_STALE_ICON, foregroundBitmapStale);
  }

  private void updateForegroundOffset(double tilt) {
    JsonArray foregroundJsonArray = new JsonArray();
    foregroundJsonArray.add(0f);
    foregroundJsonArray.add((float) (-0.05 * tilt));
    locationFeature.addProperty(PROPERTY_FOREGROUND_ICON_OFFSET, foregroundJsonArray);

    JsonArray backgroundJsonArray = new JsonArray();
    backgroundJsonArray.add(0f);
    backgroundJsonArray.add((float) (0.05 * tilt));
    locationFeature.addProperty(PROPERTY_SHADOW_ICON_OFFSET, backgroundJsonArray);

    refreshSource();
  }

  private void updateForegroundBearing(float bearing) {
    setBearingProperty(PROPERTY_GPS_BEARING, bearing);
  }

  private void setLayerVisibility(@NonNull String layerId, boolean visible) {
    Layer layer = style.getLayer(layerId);
    if (layer != null) {
      String targetVisibility = visible ? VISIBLE : NONE;
      if (!layer.getVisibility().value.equals(targetVisibility)) {
        layer.setProperties(visibility(visible ? VISIBLE : NONE));
      }
    }
  }

  private void addSymbolLayer(@NonNull String layerId, @NonNull String beforeLayerId) {
    Layer layer = layerSourceProvider.generateLayer(layerId);
    addLayerToMap(layer, beforeLayerId);
  }

  private void addAccuracyLayer() {
    Layer accuracyLayer = layerSourceProvider.generateAccuracyLayer();
    addLayerToMap(accuracyLayer, BACKGROUND_LAYER);
  }

  private void addLayerToMap(Layer layer, @NonNull String idBelowLayer) {
    style.addLayerBelow(layer, idBelowLayer);
    layerSet.add(layer.getId());
  }

  private void addLocationSource() {
    locationSource = layerSourceProvider.generateSource(locationFeature);
    style.addSource(locationSource);
  }

  private void refreshSource() {
    GeoJsonSource source = style.getSourceAs(LOCATION_SOURCE);
    if (source != null) {
      locationSource.setGeoJson(locationFeature);
    }
  }

  private void setLocationPoint(Point locationPoint) {
    JsonObject properties = locationFeature.properties();
    if (properties != null) {
      locationFeature = Feature.fromGeometry(locationPoint, properties);
      refreshSource();
    }
  }

  private void setBearingProperty(@NonNull String propertyId, float bearing) {
    locationFeature.addNumberProperty(propertyId, bearing);
    refreshSource();
  }

  private void updateAccuracyRadius(float accuracy) {
    locationFeature.addNumberProperty(PROPERTY_ACCURACY_RADIUS, accuracy);
    refreshSource();
  }
}
