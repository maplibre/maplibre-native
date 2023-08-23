package org.maplibre.android.annotations;

import static org.maplibre.android.style.expressions.Expression.all;
import static org.maplibre.android.style.expressions.Expression.get;
import static org.maplibre.android.style.expressions.Expression.gt;
import static org.maplibre.android.style.expressions.Expression.gte;
import static org.maplibre.android.style.expressions.Expression.has;
import static org.maplibre.android.style.expressions.Expression.literal;
import static org.maplibre.android.style.expressions.Expression.lt;
import static org.maplibre.android.style.layers.PropertyFactory.circleColor;
import static org.maplibre.android.style.layers.PropertyFactory.circleRadius;
import static org.maplibre.android.style.layers.PropertyFactory.textAllowOverlap;
import static org.maplibre.android.style.layers.PropertyFactory.textColor;
import static org.maplibre.android.style.layers.PropertyFactory.textField;
import static org.maplibre.android.style.layers.PropertyFactory.textIgnorePlacement;
import static org.maplibre.android.style.layers.PropertyFactory.textSize;

import android.graphics.Color;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.util.Pair;

import org.maplibre.android.maps.Style;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.CircleLayer;
import org.maplibre.android.style.layers.SymbolLayer;

/**
 * Options to show and configure symbol clustering with using SymbolManager.
 * <p>
 * It exposes a minimal of configuration options, a more advanced setup can be created manually with
 * using CircleLayer and SymbolLayers directly.
 * </p>
 */
public class ClusterOptions {

  private int clusterRadius;
  private int clusterMaxZoom;
  private Pair<Integer, Integer>[] colorLevels;

  @Nullable
  private Expression circleRadius;
  @Nullable
  private Expression textColor;
  @Nullable
  private Expression textSize;
  @Nullable
  private Expression textField;

  /**
   * Creates a default ClusterOptions object
   */
  public ClusterOptions() {
    clusterRadius = 50;
    clusterMaxZoom = 14;
    colorLevels = new Pair[]{new Pair(0, Color.BLUE)};
  }

  /**
   * Get the cluster radius, 50 by default.
   *
   * @return the radius at which the cluster dissolves
   */
  public int getClusterRadius() {
    return clusterRadius;
  }

  /**
   * Set the cluster radius, 50 by default.
   *
   * @param clusterRadius the radius at which the cluster dissolves
   * @return this
   */
  public ClusterOptions withClusterRadius(int clusterRadius) {
    this.clusterRadius = clusterRadius;
    return this;
  }

  /**
   * Set the cluster maximum, 14 by default.
   *
   * @return the cluster maximum zoom, at this zoom level clusters dissolve automatically
   */
  public int getClusterMaxZoom() {
    return clusterMaxZoom;
  }

  /**
   * Set the cluster maximum zoom, 14 by default.
   *
   * @param clusterMaxZoom the cluster maximum zoom, at this zoom level clusters dissolve automatically
   * @return this
   */
  public ClusterOptions withClusterMaxZoom(int clusterMaxZoom) {
    this.clusterMaxZoom = clusterMaxZoom;
    return this;
  }

  /**
   * Get the cluster color levels, which a pair constructed with amount of point and a int color value.
   *
   * @return the cluster color levels array
   */
  @NonNull
  public Pair<Integer, Integer>[] getColorLevels() {
    return colorLevels;
  }

  /**
   * Set the cluster color levels, which a pair constructed with amount of point and a int color value.
   *
   * @param colorLevels the cluster color levels array
   * @return this
   */
  public ClusterOptions withColorLevels(@NonNull Pair<Integer, Integer>[] colorLevels) {
    this.colorLevels = colorLevels;
    return this;
  }

  /**
   * Get the circle radius of the cluster items, 18 by default
   *
   * @return the cluster item circle radius
   */
  @Nullable
  public Expression getCircleRadius() {
    return circleRadius;
  }

  /**
   * Set the circle radius of cluster items. 18 by default
   *
   * @param circleRadius the cluster item circle radius
   * @return this
   */
  public ClusterOptions withCircleRadius(@Nullable Expression circleRadius) {
    this.circleRadius = circleRadius;
    return this;
  }

  /**
   * Get the text color of cluster item. White by default
   *
   * @return the cluster item text color
   */
  @Nullable
  public Expression getTextColor() {
    return textColor;
  }

  /**
   * Set the text color of cluster item. White by default.
   *
   * @param textColor the cluster item text color
   * @return this
   */
  public ClusterOptions withTextColor(@Nullable Expression textColor) {
    this.textColor = textColor;
    return this;
  }

  /**
   * Get the text size of cluster item. 12 by default.
   *
   * @return the cluster item text size
   */
  @Nullable
  public Expression getTextSize() {
    return textSize;
  }

  /**
   * Set the text size of cluster item. 12 by default.
   *
   * @param textSize the cluster item text size
   * @return this
   */
  public ClusterOptions withTextSize(@Nullable Expression textSize) {
    this.textSize = textSize;
    return this;
  }

  /**
   * Set the text field of a cluster item. toNumber(get("point_count")) by default.
   *
   * @return the cluster item text field
   */
  @Nullable
  public Expression getTextField() {
    return textField;
  }

  /**
   * Set the text field of a cluster item. toNumber(get("point_count")) by default.
   *
   * @param textField the cluster item text field
   * @return this
   */
  public ClusterOptions withTextField(@Nullable Expression textField) {
    this.textField = textField;
    return this;
  }

  /**
   * Apply cluster options to a style and source id.
   *
   * @param style    style to apply this to
   * @param sourceId source id to apply this to
   */
  public void apply(@NonNull Style style, @NonNull String sourceId) {
    for (int i = 0; i < colorLevels.length; i++) {
      style.addLayer(createClusterLevelLayer(i, colorLevels, sourceId));
    }
    style.addLayer(createClusterTextLayer(sourceId));
  }

  private CircleLayer createClusterLevelLayer(int level, Pair<Integer, Integer>[] levels, String sourceId) {
    CircleLayer circles = new CircleLayer("mapbox-android-cluster-circle" + level, sourceId);
    circles.setProperties(
      circleColor(levels[level].second),
      circleRadius(circleRadius != null ? circleRadius : literal(18.0f))
    );

    Expression pointCount = Expression.toNumber(get("point_count"));
    circles.setFilter(
      level == 0
        ? all(has("point_count"),
        gte(pointCount, literal(levels[level].first))
      ) : all(has("point_count"),
        gt(pointCount, literal(levels[level].first)),
        lt(pointCount, literal(levels[level - 1].first))
      )
    );
    return circles;
  }

  private SymbolLayer createClusterTextLayer(String sourceId) {
    return new SymbolLayer("mapbox-android-cluster-text", sourceId)
      .withProperties(
        textField(textField != null ? textField : get("point_count")),
        textSize(textSize != null ? textSize : literal(12f)),
        textColor(textColor != null ? textColor : Expression.color(Color.WHITE)),
        textIgnorePlacement(true),
        textAllowOverlap(true)
      );
  }
}
