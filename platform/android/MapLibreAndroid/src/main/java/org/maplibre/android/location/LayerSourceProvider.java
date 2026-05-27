package org.maplibre.android.location;

import androidx.annotation.NonNull;

import org.maplibre.geojson.Feature;

import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.CircleLayer;
import org.maplibre.android.style.layers.Layer;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyFactory;
import org.maplibre.android.style.layers.SymbolLayer;
import org.maplibre.android.style.layers.TransitionOptions;
import org.maplibre.android.style.sources.GeoJsonOptions;
import org.maplibre.android.style.sources.GeoJsonSource;

import java.util.HashSet;
import java.util.Set;

class LayerSourceProvider {

  private static final String EMPTY_STRING = "";

  @NonNull
  GeoJsonSource generateSource(Feature locationFeature) {
    return generateSource(locationFeature, false);
  }

  @NonNull
  GeoJsonSource generateSource(Feature locationFeature, Boolean synchronousUpdate) {
    // maxZoom 22 (was 16): the upstream default of 16 leaves the puck source heavily
    // overzoomed at any view zoom > 16, which the native renderer represents by re-using
    // a single z=16 source tile scaled up to cover many viewport tiles. An overzoomed
    // tile's geometric center can sit far from the puck along the camera-forward axis,
    // which makes a center-only "is this tile in view?" test (e.g. tile-distance
    // culling in RenderSymbolLayer) drop the tile even when the puck is dead-center on
    // screen, causing the puck to disappear / stick at high zoom or moderate pitch.
    // Bumping to 22 means the puck's tile sits at the same zoom as the viewport at all
    // realistic zoom levels; the source contains a single point feature so the cost of
    // a higher maxZoom is negligible. The C++ renderer also uses a tile-corner-based
    // distance check (see `TransformState::getNearestCameraToTileDistance`) for any
    // overzoom that still slips through (e.g. view zoom > 22).
    return new GeoJsonSource(
      LocationComponentConstants.LOCATION_SOURCE,
      locationFeature,
      new GeoJsonOptions().withMaxZoom(22).withSynchronousUpdate(synchronousUpdate)
    );
  }

  @NonNull
  Layer generateLayer(@NonNull String layerId) {
    SymbolLayer layer = new SymbolLayer(layerId, LocationComponentConstants.LOCATION_SOURCE);
    layer.setProperties(
      PropertyFactory.iconAllowOverlap(true),
      PropertyFactory.iconIgnorePlacement(true),
      PropertyFactory.iconRotationAlignment(Property.ICON_ROTATION_ALIGNMENT_MAP),
      PropertyFactory.iconRotate(
        Expression.match(Expression.literal(layerId), Expression.literal(0f),
          Expression.stop(LocationComponentConstants.FOREGROUND_LAYER,
            Expression.get(LocationComponentConstants.PROPERTY_GPS_BEARING)),
          Expression.stop(LocationComponentConstants.BACKGROUND_LAYER,
            Expression.get(LocationComponentConstants.PROPERTY_GPS_BEARING)),
          Expression.stop(LocationComponentConstants.SHADOW_LAYER,
            Expression.get(LocationComponentConstants.PROPERTY_GPS_BEARING)),
          Expression.stop(LocationComponentConstants.BEARING_LAYER,
            Expression.get(LocationComponentConstants.PROPERTY_COMPASS_BEARING))
        )
      ),
      PropertyFactory.iconImage(
        Expression.match(Expression.literal(layerId), Expression.literal(EMPTY_STRING),
          Expression.stop(LocationComponentConstants.FOREGROUND_LAYER, Expression.switchCase(
            Expression.get(LocationComponentConstants.PROPERTY_LOCATION_STALE),
            Expression.get(LocationComponentConstants.PROPERTY_FOREGROUND_STALE_ICON),
            Expression.get(LocationComponentConstants.PROPERTY_FOREGROUND_ICON))),
          Expression.stop(LocationComponentConstants.BACKGROUND_LAYER, Expression.switchCase(
            Expression.get(LocationComponentConstants.PROPERTY_LOCATION_STALE),
            Expression.get(LocationComponentConstants.PROPERTY_BACKGROUND_STALE_ICON),
            Expression.get(LocationComponentConstants.PROPERTY_BACKGROUND_ICON))),
          Expression.stop(LocationComponentConstants.SHADOW_LAYER,
            Expression.literal(LocationComponentConstants.SHADOW_ICON)),
          Expression.stop(LocationComponentConstants.BEARING_LAYER,
            Expression.get(LocationComponentConstants.PROPERTY_BEARING_ICON))
        )
      ),
      PropertyFactory.iconOffset(
        Expression.match(Expression.literal(layerId), Expression.literal(new Float[] {0f, 0f}),
          Expression.stop(Expression.literal(LocationComponentConstants.FOREGROUND_LAYER),
            Expression.get(LocationComponentConstants.PROPERTY_FOREGROUND_ICON_OFFSET)),
          Expression.stop(Expression.literal(LocationComponentConstants.SHADOW_LAYER),
            Expression.get(LocationComponentConstants.PROPERTY_SHADOW_ICON_OFFSET))
        )
      )
    );
    return layer;
  }

  @NonNull
  Layer generateAccuracyLayer() {
    return new CircleLayer(LocationComponentConstants.ACCURACY_LAYER, LocationComponentConstants.LOCATION_SOURCE)
      .withProperties(
        PropertyFactory.circleRadius(Expression.get(LocationComponentConstants.PROPERTY_ACCURACY_RADIUS)),
        PropertyFactory.circleColor(Expression.get(LocationComponentConstants.PROPERTY_ACCURACY_COLOR)),
        PropertyFactory.circleOpacity(Expression.get(LocationComponentConstants.PROPERTY_ACCURACY_ALPHA)),
        PropertyFactory.circleStrokeColor(Expression.get(LocationComponentConstants.PROPERTY_ACCURACY_COLOR)),
        PropertyFactory.circlePitchAlignment(Property.CIRCLE_PITCH_ALIGNMENT_MAP)
      );
  }

  Set<String> getEmptyLayerSet() {
    return new HashSet<>();
  }

  LocationLayerRenderer getSymbolLocationLayerRenderer(LayerFeatureProvider featureProvider,
                                                       boolean isStale) {
    return new SymbolLocationLayerRenderer(this, featureProvider, isStale);
  }

  LocationLayerRenderer getIndicatorLocationLayerRenderer() {
    return new IndicatorLocationLayerRenderer(this);
  }

  Layer generateLocationComponentLayer() {
    LocationIndicatorLayer layer = new LocationIndicatorLayer(LocationComponentConstants.FOREGROUND_LAYER);
    layer.setLocationTransition(new TransitionOptions(0, 0));
    layer.setProperties(
        LocationPropertyFactory.perspectiveCompensation(0.9f),
        LocationPropertyFactory.imageTiltDisplacement(4f)
    );
    return layer;
  }

  /**
   * Adds a {@link CircleLayer} to the map to support the {@link LocationComponent} pulsing UI functionality.
   *
   * @return a {@link CircleLayer} with the correct data-driven styling. Tilting the map will keep the pulsing
   * layer aligned with the map plane.
   */
  @NonNull
  Layer generatePulsingCircleLayer() {
    return new CircleLayer(LocationComponentConstants.PULSING_CIRCLE_LAYER, LocationComponentConstants.LOCATION_SOURCE)
        .withProperties(
            PropertyFactory.circlePitchAlignment(Property.CIRCLE_PITCH_ALIGNMENT_MAP)
        );
  }
}
