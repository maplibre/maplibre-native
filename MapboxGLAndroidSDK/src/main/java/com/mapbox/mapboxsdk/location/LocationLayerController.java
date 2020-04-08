package com.mapbox.mapboxsdk.location;

import android.graphics.Bitmap;
import android.graphics.PointF;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.mapbox.geojson.Feature;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.location.modes.RenderMode;
import com.mapbox.mapboxsdk.log.Logger;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.style.expressions.Expression;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BACKGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BEARING_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.BEARING_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_ICON;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_LAYER;
import static com.mapbox.mapboxsdk.location.LocationComponentConstants.FOREGROUND_STALE_ICON;
import static com.mapbox.mapboxsdk.style.expressions.Expression.interpolate;
import static com.mapbox.mapboxsdk.style.expressions.Expression.linear;
import static com.mapbox.mapboxsdk.style.expressions.Expression.stop;
import static com.mapbox.mapboxsdk.style.expressions.Expression.zoom;

final class LocationLayerController {

  private static final String TAG = "Mbgl-LocationLayerController";

  @RenderMode.Mode
  private int renderMode;

  private final MapboxMap mapboxMap;
  private Style style;
  private final LayerBitmapProvider bitmapProvider;
  private LocationComponentOptions options;
  private final OnRenderModeChangedListener internalRenderModeChangedListener;
  private final boolean useSpecializedLocationLayer;

  private boolean isHidden = true;
  private boolean isStale;

  private LocationComponentPositionManager positionManager;

  private LocationLayerRenderer locationLayerRenderer;

  LocationLayerController(MapboxMap mapboxMap, Style style,
                          LayerSourceProvider layerSourceProvider,
                          LayerFeatureProvider featureProvider,
                          LayerBitmapProvider bitmapProvider,
                          @NonNull LocationComponentOptions options,
                          @NonNull OnRenderModeChangedListener internalRenderModeChangedListener,
                          boolean useSpecializedLocationLayer) {
    this.mapboxMap = mapboxMap;
    this.style = style;
    this.bitmapProvider = bitmapProvider;
    this.internalRenderModeChangedListener = internalRenderModeChangedListener;
    this.useSpecializedLocationLayer = useSpecializedLocationLayer;
    this.isStale = options.enableStaleState();
    if (useSpecializedLocationLayer) {
      locationLayerRenderer = new IndicatorLocationLayerRenderer(style, layerSourceProvider);
    } else {
      locationLayerRenderer = new SymbolLocationLayerRenderer(style, layerSourceProvider, featureProvider, isStale);
    }
    initializeComponents(style, options);
  }

  void initializeComponents(Style style, LocationComponentOptions options) {
    this.style = style;
    this.positionManager = new LocationComponentPositionManager(style, options.layerAbove(), options.layerBelow());
    locationLayerRenderer.initializeComponents(style);
    locationLayerRenderer.addLayers(positionManager);
    applyStyle(options);

    if (isHidden) {
      hide();
    } else {
      show();
    }
  }

  void applyStyle(@NonNull LocationComponentOptions options) {
    if (positionManager.update(options.layerAbove(), options.layerBelow())) {
      locationLayerRenderer.removeLayers();
      locationLayerRenderer.addLayers(positionManager);
      if (isHidden) {
        hide();
      }
    }

    this.options = options;
    styleBitmaps(options);
    locationLayerRenderer.styleAccuracy(options.accuracyAlpha(), options.accuracyColor());
    styleScaling(options);
    determineIconsSource(options);

    if (!isHidden) {
      show();
    }
  }

  void setRenderMode(@RenderMode.Mode int renderMode) {
    if (this.renderMode == renderMode) {
      return;
    }
    this.renderMode = renderMode;

    styleBitmaps(options);
    determineIconsSource(options);
    if (!isHidden) {
      show();
    }
    internalRenderModeChangedListener.onRenderModeChanged(renderMode);
  }

  int getRenderMode() {
    return renderMode;
  }

  //
  // Layer action
  //

  void show() {
    isHidden = false;
    locationLayerRenderer.show(renderMode, isStale);
  }

  void hide() {
    isHidden = true;
    locationLayerRenderer.hide();
  }

  boolean isHidden() {
    return isHidden;
  }

  boolean isConsumingCompass() {
    return renderMode == RenderMode.COMPASS;
  }

  //
  // Styling
  //

  private void styleBitmaps(LocationComponentOptions options) {
    // shadow
    Bitmap shadowBitmap = null;
    if (options.elevation() > 0) {
      // Only add icon elevation if the values greater than 0.
      shadowBitmap = bitmapProvider.generateShadowBitmap(options);
    }

    // background
    Bitmap backgroundBitmap = bitmapProvider.generateBitmap(
      options.backgroundDrawable(), options.backgroundTintColor()
    );
    Bitmap backgroundStaleBitmap = bitmapProvider.generateBitmap(
      options.backgroundDrawableStale(), options.backgroundStaleTintColor()
    );

    // compass bearing
    Bitmap bearingBitmap = bitmapProvider.generateBitmap(options.bearingDrawable(), options.bearingTintColor());

    // foreground
    Bitmap foregroundBitmap = bitmapProvider.generateBitmap(
      options.foregroundDrawable(), options.foregroundTintColor()
    );
    Bitmap foregroundStaleBitmap = bitmapProvider.generateBitmap(
      options.foregroundDrawableStale(), options.foregroundStaleTintColor()
    );
    if (renderMode == RenderMode.GPS) {
      foregroundBitmap = bitmapProvider.generateBitmap(
        options.gpsDrawable(), options.foregroundTintColor()
      );
      foregroundStaleBitmap = bitmapProvider.generateBitmap(
        options.gpsDrawable(), options.foregroundStaleTintColor()
      );
    }

    locationLayerRenderer.addBitmaps(
      renderMode,
      shadowBitmap,
      backgroundBitmap,
      backgroundStaleBitmap,
      bearingBitmap,
      foregroundBitmap,
      foregroundStaleBitmap
    );
  }

  private void styleScaling(@NonNull LocationComponentOptions options) {
    Expression scaleExpression = interpolate(linear(), zoom(),
      stop(mapboxMap.getMinZoomLevel(), options.minZoomIconScale()),
      stop(mapboxMap.getMaxZoomLevel(), options.maxZoomIconScale())
    );

    locationLayerRenderer.styleScaling(scaleExpression);
  }

  private void determineIconsSource(LocationComponentOptions options) {
    String foregroundIconString = buildIconString(
      renderMode == RenderMode.GPS ? options.gpsName() : options.foregroundName(), FOREGROUND_ICON);
    String foregroundStaleIconString = buildIconString(options.foregroundStaleName(), FOREGROUND_STALE_ICON);
    String backgroundIconString = buildIconString(options.backgroundName(), BACKGROUND_ICON);
    String backgroundStaleIconString = buildIconString(options.backgroundStaleName(), BACKGROUND_STALE_ICON);
    String bearingIconString = buildIconString(options.bearingName(), BEARING_ICON);

    locationLayerRenderer.updateIconIds(
      foregroundIconString,
      foregroundStaleIconString,
      backgroundIconString,
      backgroundStaleIconString,
      bearingIconString
    );
  }

  @NonNull
  private String buildIconString(@Nullable String bitmapName, @NonNull String drawableName) {
    if (bitmapName != null) {
      if (useSpecializedLocationLayer) {
        Logger.e(TAG, bitmapName + " replacement ID provided for an unsupported specialized location layer");
        return drawableName;
      }
      return bitmapName;
    }
    return drawableName;
  }

  void setLocationsStale(boolean isStale) {
    this.isStale = isStale;
    locationLayerRenderer.setLocationStale(isStale, renderMode);
  }

  //
  // Map click event
  //

  boolean onMapClick(@NonNull LatLng point) {
    PointF screenLoc = mapboxMap.getProjection().toScreenLocation(point);
    List<Feature> features = mapboxMap.queryRenderedFeatures(screenLoc,
      BACKGROUND_LAYER,
      FOREGROUND_LAYER,
      BEARING_LAYER
    );
    return !features.isEmpty();
  }

  private final MapboxAnimator.AnimationsValueChangeListener<LatLng> latLngValueListener =
    new MapboxAnimator.AnimationsValueChangeListener<LatLng>() {
      @Override
      public void onNewAnimationValue(LatLng value) {
        locationLayerRenderer.setLatLng(value);
      }
    };

  private final MapboxAnimator.AnimationsValueChangeListener<Float> gpsBearingValueListener =
    new MapboxAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float value) {
        locationLayerRenderer.setGpsBearing(value);
      }
    };

  private final MapboxAnimator.AnimationsValueChangeListener<Float> compassBearingValueListener =
    new MapboxAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float value) {
        locationLayerRenderer.setCompassBearing(value);
      }
    };

  private final MapboxAnimator.AnimationsValueChangeListener<Float> accuracyValueListener =
    new MapboxAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float value) {
        locationLayerRenderer.setAccuracyRadius(value);
      }
    };

  Set<AnimatorListenerHolder> getAnimationListeners() {
    Set<AnimatorListenerHolder> holders = new HashSet<>();
    holders.add(new AnimatorListenerHolder(MapboxAnimator.ANIMATOR_LAYER_LATLNG, latLngValueListener));

    if (renderMode == RenderMode.GPS) {
      holders.add(new AnimatorListenerHolder(MapboxAnimator.ANIMATOR_LAYER_GPS_BEARING, gpsBearingValueListener));
    } else if (renderMode == RenderMode.COMPASS) {
      holders.add(
        new AnimatorListenerHolder(MapboxAnimator.ANIMATOR_LAYER_COMPASS_BEARING, compassBearingValueListener));
    }

    if (renderMode == RenderMode.COMPASS || renderMode == RenderMode.NORMAL) {
      holders.add(new AnimatorListenerHolder(MapboxAnimator.ANIMATOR_LAYER_ACCURACY, accuracyValueListener));
    }

    return holders;
  }

  void cameraBearingUpdated(double bearing) {
    if (renderMode != RenderMode.GPS) {
      locationLayerRenderer.cameraBearingUpdated(bearing);
    }
  }

  void cameraTiltUpdated(double tilt) {
    locationLayerRenderer.cameraTiltUpdated(tilt);
  }
}