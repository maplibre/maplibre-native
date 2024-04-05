package org.maplibre.android.location;

import android.graphics.Bitmap;
import android.graphics.PointF;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.geojson.Feature;
import org.maplibre.android.location.modes.RenderMode;
import org.maplibre.android.log.Logger;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.maps.Style;
import org.maplibre.android.style.expressions.Expression;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static org.maplibre.android.location.LocationComponentConstants.BACKGROUND_ICON;
import static org.maplibre.android.location.LocationComponentConstants.BACKGROUND_LAYER;
import static org.maplibre.android.location.LocationComponentConstants.BACKGROUND_STALE_ICON;
import static org.maplibre.android.location.LocationComponentConstants.BEARING_ICON;
import static org.maplibre.android.location.LocationComponentConstants.BEARING_LAYER;
import static org.maplibre.android.location.LocationComponentConstants.FOREGROUND_ICON;
import static org.maplibre.android.location.LocationComponentConstants.FOREGROUND_LAYER;
import static org.maplibre.android.location.LocationComponentConstants.FOREGROUND_STALE_ICON;
import static org.maplibre.android.style.expressions.Expression.interpolate;
import static org.maplibre.android.style.expressions.Expression.linear;
import static org.maplibre.android.style.expressions.Expression.stop;
import static org.maplibre.android.style.expressions.Expression.zoom;

import org.maplibre.android.geometry.LatLng;

final class LocationLayerController {

  private static final String TAG = "Mbgl-LocationLayerController";

  @RenderMode.Mode
  private int renderMode;

  private final MapLibreMap maplibreMap;
  private final LayerBitmapProvider bitmapProvider;
  private LocationComponentOptions options;
  private final OnRenderModeChangedListener internalRenderModeChangedListener;
  private final boolean useSpecializedLocationLayer;

  private boolean isHidden = true;
  private boolean isStale;

  private LocationComponentPositionManager positionManager;

  private LocationLayerRenderer locationLayerRenderer;

  LocationLayerController(MapLibreMap maplibreMap, Style style,
                          LayerSourceProvider layerSourceProvider,
                          LayerFeatureProvider featureProvider,
                          LayerBitmapProvider bitmapProvider,
                          @NonNull LocationComponentOptions options,
                          @NonNull OnRenderModeChangedListener internalRenderModeChangedListener,
                          boolean useSpecializedLocationLayer) {
    this.maplibreMap = maplibreMap;
    this.bitmapProvider = bitmapProvider;
    this.internalRenderModeChangedListener = internalRenderModeChangedListener;
    this.useSpecializedLocationLayer = useSpecializedLocationLayer;
    this.isStale = options.enableStaleState();
    if (useSpecializedLocationLayer) {
      locationLayerRenderer = layerSourceProvider.getIndicatorLocationLayerRenderer();
    } else {
      locationLayerRenderer =
        layerSourceProvider.getSymbolLocationLayerRenderer(featureProvider, isStale);
    }
    initializeComponents(style, options);
  }

  void initializeComponents(Style style, LocationComponentOptions options) {
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
    locationLayerRenderer.stylePulsingCircle(options);
    determineIconsSource(options);

    if (!isHidden) {
      show();
    }
  }

  void setGpsBearing(float gpsBearing) {
    locationLayerRenderer.setGpsBearing(gpsBearing);
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
      stop(maplibreMap.getMinZoomLevel(), options.minZoomIconScale()),
      stop(maplibreMap.getMaxZoomLevel(), options.maxZoomIconScale())
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
    PointF screenLoc = maplibreMap.getProjection().toScreenLocation(point);
    List<Feature> features = maplibreMap.queryRenderedFeatures(screenLoc,
      BACKGROUND_LAYER,
      FOREGROUND_LAYER,
      BEARING_LAYER
    );
    return !features.isEmpty();
  }

  private final MapLibreAnimator.AnimationsValueChangeListener<LatLng> latLngValueListener =
    new MapLibreAnimator.AnimationsValueChangeListener<LatLng>() {
      @Override
      public void onNewAnimationValue(LatLng value) {
        locationLayerRenderer.setLatLng(value);
      }
  };

  private final MapLibreAnimator.AnimationsValueChangeListener<Float> gpsBearingValueListener =
    new MapLibreAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float value) {
        locationLayerRenderer.setGpsBearing(value);
      }
  };

  private final MapLibreAnimator.AnimationsValueChangeListener<Float> compassBearingValueListener =
    new MapLibreAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float value) {
        locationLayerRenderer.setCompassBearing(value);
      }
  };

  private final MapLibreAnimator.AnimationsValueChangeListener<Float> accuracyValueListener =
    new MapLibreAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float value) {
        locationLayerRenderer.setAccuracyRadius(value);
      }
  };

  /**
   * The listener that handles the updating of the pulsing circle's radius and opacity.
   */
  private final MapLibreAnimator.AnimationsValueChangeListener<Float> pulsingCircleRadiusListener =
    new MapLibreAnimator.AnimationsValueChangeListener<Float>() {
      @Override
      public void onNewAnimationValue(Float newPulseRadiusValue) {
        Float newPulseOpacityValue = null;
        if (options.pulseFadeEnabled()) {
          newPulseOpacityValue = (float) 1 - ((newPulseRadiusValue / 100) * 3);
        }
        locationLayerRenderer.updatePulsingUi(newPulseRadiusValue, newPulseOpacityValue);
      }
  };

  Set<AnimatorListenerHolder> getAnimationListeners() {
    Set<AnimatorListenerHolder> holders = new HashSet<>();
    holders.add(new AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_LATLNG, latLngValueListener));

    if (renderMode == RenderMode.GPS) {
      holders.add(new AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING, gpsBearingValueListener));
    } else if (renderMode == RenderMode.COMPASS) {
      holders.add(
        new AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING, compassBearingValueListener));
    }

    if (renderMode == RenderMode.COMPASS || renderMode == RenderMode.NORMAL) {
      holders.add(new AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_ACCURACY, accuracyValueListener));
    }

    if (options.pulseEnabled()) {
      holders.add(new AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE,
          pulsingCircleRadiusListener));
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

  void adjustPulsingCircleLayerVisibility(boolean visible) {
    locationLayerRenderer.adjustPulsingCircleLayerVisibility(visible);
  }
}