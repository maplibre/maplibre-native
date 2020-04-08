package com.mapbox.mapboxsdk.location;

import android.graphics.Bitmap;

import androidx.annotation.Nullable;

import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.location.modes.RenderMode;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.style.expressions.Expression;
import com.mapbox.mapboxsdk.style.layers.LocationIndicatorLayer;
import com.mapbox.mapboxsdk.style.layers.PropertyFactory;
import com.mapbox.mapboxsdk.utils.BitmapUtils;
import com.mapbox.mapboxsdk.utils.ColorUtils;

import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BEARING_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BEARING_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.SHADOW_ICON;
import static com.mapbox.mapboxsdk.style.expressions.Expression.rgba;
import static com.mapbox.mapboxsdk.style.layers.Property.NONE;
import static com.mapbox.mapboxsdk.style.layers.Property.VISIBLE;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.visibility;

class IndicatorLocationLayerRenderer implements LocationLayerRenderer {

  private Style style;
  private final LayerSourceProvider layerSourceProvider;
  private LocationIndicatorLayer layer;

  @Nullable
  private LatLng latLng;

  IndicatorLocationLayerRenderer(Style style,
                                 LayerSourceProvider layerSourceProvider) {
    this.style = style;
    this.layerSourceProvider = layerSourceProvider;
  }

  @Override
  public void initializeComponents(Style style) {
    this.style = style;
  }

  @Override
  public void addLayers(LocationComponentPositionManager positionManager) {
    layer = layerSourceProvider.generateLocationComponentLayer();
    setLayerLocation(latLng);
    positionManager.addLayerToMap(layer);
  }

  @Override
  public void removeLayers() {
    style.removeLayer(layer);
  }

  @Override
  public void hide() {
    setLayerVisibility(false);
  }

  @Override
  public void cameraTiltUpdated(double tilt) {
    // ignored
  }

  @Override
  public void cameraBearingUpdated(double bearing) {
    // ignored
  }

  @Override
  public void show(@RenderMode.Mode int renderMode, boolean isStale) {
    setImages(renderMode, isStale);
    setLayerVisibility(true);
  }

  @Override
  public void styleAccuracy(float accuracyAlpha, int accuracyColor) {
    float[] colorArray = ColorUtils.colorToRgbaArray(accuracyColor);
    colorArray[3] = accuracyAlpha;
    Expression rgbaExpression = rgba(colorArray[0], colorArray[1], colorArray[2], colorArray[3]);
    layer.setProperties(
      PropertyFactory.accuracyRadiusColor(rgbaExpression),
      PropertyFactory.accuracyRadiusBorderColor(rgbaExpression)
    );
  }

  @Override
  public void setLatLng(LatLng latLng) {
    setLayerLocation(latLng);
  }

  @Override
  public void setGpsBearing(Float gpsBearing) {
    setLayerBearing(gpsBearing);
  }

  @Override
  public void setCompassBearing(Float compassBearing) {
    setLayerBearing(compassBearing);
  }

  @Override
  public void setAccuracyRadius(Float accuracy) {
    layer.setProperties(
      PropertyFactory.accuracyRadius(accuracy)
    );
  }

  @Override
  public void styleScaling(Expression scaleExpression) {
    layer.setProperties(
      PropertyFactory.shadowImageSize(scaleExpression),
      PropertyFactory.bearingImageSize(scaleExpression),
      PropertyFactory.topImageSize(scaleExpression)
    );
  }

  @Override
  public void setLocationStale(boolean isStale, int renderMode) {
    setImages(renderMode, isStale);
  }

  @Override
  public void updateIconIds(String foregroundIconString, String foregroundStaleIconString, String backgroundIconString,
                            String backgroundStaleIconString, String bearingIconString) {
    // not supported
  }

  @Override
  public void addBitmaps(@RenderMode.Mode int renderMode, @Nullable Bitmap shadowBitmap, Bitmap backgroundBitmap,
                         Bitmap backgroundStaleBitmap, Bitmap bearingBitmap,
                         Bitmap foregroundBitmap, Bitmap foregroundStaleBitmap) {
    if (shadowBitmap != null) {
      style.addImage(SHADOW_ICON, shadowBitmap);
    } else {
      style.removeImage(SHADOW_ICON);
    }
    style.addImage(FOREGROUND_ICON, foregroundBitmap);
    style.addImage(FOREGROUND_STALE_ICON, foregroundStaleBitmap);

    if (renderMode == RenderMode.COMPASS) {
      float leftOffset = (bearingBitmap.getWidth() - backgroundBitmap.getWidth()) / 2f;
      float topOffset = (bearingBitmap.getHeight() - backgroundBitmap.getHeight()) / 2f;
      style.addImage(BEARING_ICON, BitmapUtils.mergeBitmap(bearingBitmap, backgroundBitmap, leftOffset, topOffset));

      float staleLeftOffset = (bearingBitmap.getWidth() - backgroundStaleBitmap.getWidth()) / 2f;
      float staleTopOffset = (bearingBitmap.getHeight() - backgroundStaleBitmap.getHeight()) / 2f;
      style.addImage(BEARING_STALE_ICON,
        BitmapUtils.mergeBitmap(bearingBitmap, backgroundStaleBitmap, staleLeftOffset, staleTopOffset));
    } else {
      style.addImage(BACKGROUND_ICON, backgroundBitmap);
      style.addImage(BACKGROUND_STALE_ICON, backgroundStaleBitmap);
      style.addImage(BEARING_ICON, bearingBitmap);
    }
  }

  private void setLayerVisibility(boolean visible) {
    layer.setProperties(visibility(visible ? VISIBLE : NONE));
  }

  private void setLayerLocation(@Nullable LatLng latLng) {
    this.latLng = latLng;
    if (latLng != null) {
      Double[] values = new Double[] {latLng.getLatitude(), latLng.getLongitude(), 0d};
      layer.setProperties(
        PropertyFactory.location(values)
      );
    }
  }

  private void setLayerBearing(double bearing) {
    layer.setProperties(
      PropertyFactory.bearing(bearing)
    );
  }

  private void setImages(@RenderMode.Mode int renderMode, boolean isStale) {
    String topImage = "";
    String bearingImage = "";
    String shadowImage = "";

    switch (renderMode) {
      case RenderMode.COMPASS:
        topImage = isStale ? FOREGROUND_STALE_ICON : FOREGROUND_ICON;
        bearingImage = isStale ? BEARING_STALE_ICON : BEARING_ICON;
        shadowImage = SHADOW_ICON;
        break;
      case RenderMode.GPS:
        topImage = "";
        bearingImage = isStale ? FOREGROUND_STALE_ICON : FOREGROUND_ICON;
        shadowImage = "";
        setAccuracyRadius(0f);
        break;
      case RenderMode.NORMAL:
        topImage = isStale ? FOREGROUND_STALE_ICON : FOREGROUND_ICON;
        bearingImage = isStale ? BACKGROUND_STALE_ICON : BACKGROUND_ICON;
        shadowImage = SHADOW_ICON;
        break;
    }
    layer.setProperties(
      PropertyFactory.topImage(topImage),
      PropertyFactory.bearingImage(bearingImage),
      PropertyFactory.shadowImage(shadowImage)
    );
  }
}
