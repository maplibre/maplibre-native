package org.maplibre.android.annotations;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.Point;

import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.style.layers.PropertyFactory;

/**
 * Builder class from which a circle is created.
 */
public class CircleOptions extends Options<Circle> {

  private boolean isDraggable;
  private JsonElement data;
  private Point geometry;
  private Float circleRadius;
  private String circleColor;
  private Float circleBlur;
  private Float circleOpacity;
  private Float circleStrokeWidth;
  private String circleStrokeColor;
  private Float circleStrokeOpacity;

  static final String PROPERTY_CIRCLE_RADIUS = "circle-radius";
  static final String PROPERTY_CIRCLE_COLOR = "circle-color";
  static final String PROPERTY_CIRCLE_BLUR = "circle-blur";
  static final String PROPERTY_CIRCLE_OPACITY = "circle-opacity";
  static final String PROPERTY_CIRCLE_STROKE_WIDTH = "circle-stroke-width";
  static final String PROPERTY_CIRCLE_STROKE_COLOR = "circle-stroke-color";
  static final String PROPERTY_CIRCLE_STROKE_OPACITY = "circle-stroke-opacity";
  private static final String PROPERTY_IS_DRAGGABLE = "is-draggable";

  /**
   * Set circle-radius to initialise the circle with.
   * <p>
   * Circle radius.
   * </p>
   *
   * @param circleRadius the circle-radius value
   * @return this
   */
  public CircleOptions withCircleRadius(Float circleRadius) {
    this.circleRadius = circleRadius;
    return this;
  }

  /**
   * Get the current configured  circle-radius for the circle
   * <p>
   * Circle radius.
   * </p>
   *
   * @return circleRadius value
   */
  public Float getCircleRadius() {
    return circleRadius;
  }

  /**
   * Set circle-color to initialise the circle with.
   * <p>
   * The fill color of the circle.
   * </p>
   *
   * @param circleColor the circle-color value
   * @return this
   */
  public CircleOptions withCircleColor(String circleColor) {
    this.circleColor = circleColor;
    return this;
  }

  /**
   * Get the current configured  circle-color for the circle
   * <p>
   * The fill color of the circle.
   * </p>
   *
   * @return circleColor value
   */
  public String getCircleColor() {
    return circleColor;
  }

  /**
   * Set circle-blur to initialise the circle with.
   * <p>
   * Amount to blur the circle. 1 blurs the circle such that only the centerpoint is full opacity.
   * </p>
   *
   * @param circleBlur the circle-blur value
   * @return this
   */
  public CircleOptions withCircleBlur(Float circleBlur) {
    this.circleBlur = circleBlur;
    return this;
  }

  /**
   * Get the current configured  circle-blur for the circle
   * <p>
   * Amount to blur the circle. 1 blurs the circle such that only the centerpoint is full opacity.
   * </p>
   *
   * @return circleBlur value
   */
  public Float getCircleBlur() {
    return circleBlur;
  }

  /**
   * Set circle-opacity to initialise the circle with.
   * <p>
   * The opacity at which the circle will be drawn.
   * </p>
   *
   * @param circleOpacity the circle-opacity value
   * @return this
   */
  public CircleOptions withCircleOpacity(Float circleOpacity) {
    this.circleOpacity = circleOpacity;
    return this;
  }

  /**
   * Get the current configured  circle-opacity for the circle
   * <p>
   * The opacity at which the circle will be drawn.
   * </p>
   *
   * @return circleOpacity value
   */
  public Float getCircleOpacity() {
    return circleOpacity;
  }

  /**
   * Set circle-stroke-width to initialise the circle with.
   * <p>
   * The width of the circle's stroke. Strokes are placed outside of the {@link PropertyFactory#circleRadius}.
   * </p>
   *
   * @param circleStrokeWidth the circle-stroke-width value
   * @return this
   */
  public CircleOptions withCircleStrokeWidth(Float circleStrokeWidth) {
    this.circleStrokeWidth = circleStrokeWidth;
    return this;
  }

  /**
   * Get the current configured  circle-stroke-width for the circle
   * <p>
   * The width of the circle's stroke. Strokes are placed outside of the {@link PropertyFactory#circleRadius}.
   * </p>
   *
   * @return circleStrokeWidth value
   */
  public Float getCircleStrokeWidth() {
    return circleStrokeWidth;
  }

  /**
   * Set circle-stroke-color to initialise the circle with.
   * <p>
   * The stroke color of the circle.
   * </p>
   *
   * @param circleStrokeColor the circle-stroke-color value
   * @return this
   */
  public CircleOptions withCircleStrokeColor(String circleStrokeColor) {
    this.circleStrokeColor = circleStrokeColor;
    return this;
  }

  /**
   * Get the current configured  circle-stroke-color for the circle
   * <p>
   * The stroke color of the circle.
   * </p>
   *
   * @return circleStrokeColor value
   */
  public String getCircleStrokeColor() {
    return circleStrokeColor;
  }

  /**
   * Set circle-stroke-opacity to initialise the circle with.
   * <p>
   * The opacity of the circle's stroke.
   * </p>
   *
   * @param circleStrokeOpacity the circle-stroke-opacity value
   * @return this
   */
  public CircleOptions withCircleStrokeOpacity(Float circleStrokeOpacity) {
    this.circleStrokeOpacity = circleStrokeOpacity;
    return this;
  }

  /**
   * Get the current configured  circle-stroke-opacity for the circle
   * <p>
   * The opacity of the circle's stroke.
   * </p>
   *
   * @return circleStrokeOpacity value
   */
  public Float getCircleStrokeOpacity() {
    return circleStrokeOpacity;
  }

  /**
   * Set the LatLng of the circle, which represents the location of the circle on the map
   *
   * @param latLng the location of the circle in a longitude and latitude pair
   * @return this
   */
  public CircleOptions withLatLng(LatLng latLng) {
    geometry = Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude());
    return this;
  }

  /**
   * Get the LatLng of the circle, which represents the location of the circle on the map
   *
   * @return the location of the circle in a longitude and latitude pair
   */
  public LatLng getLatLng() {
    if (geometry == null) {
      return null;
    }
    return new LatLng(geometry.latitude(), geometry.longitude());
  }

  /**
   * Set the geometry of the circle, which represents the location of the circle on the map
   *
   * @param geometry the location of the circle
   * @return this
   */
  public CircleOptions withGeometry(Point geometry) {
    this.geometry = geometry;
    return this;
  }

  /**
   * Get the geometry of the circle, which represents the location of the circle on the map
   *
   * @return the location of the circle
   */
  public Point getGeometry() {
    return geometry;
  }

  /**
   * Returns whether this circle is draggable, meaning it can be dragged across the screen when touched and moved.
   *
   * @return draggable when touched
   */
  public boolean getDraggable() {
    return isDraggable;
  }

  /**
   * Set whether this circle should be draggable,
   * meaning it can be dragged across the screen when touched and moved.
   *
   * @param draggable should be draggable
   */
  public CircleOptions withDraggable(boolean draggable) {
    isDraggable = draggable;
    return this;
  }

  /**
   * Set the arbitrary json data of the annotation.
   *
   * @param jsonElement the arbitrary json element data
   */
  public CircleOptions withData(@Nullable JsonElement jsonElement) {
    this.data = jsonElement;
    return this;
  }

  /**
   * Get the arbitrary json data of the annotation.
   *
   * @return the arbitrary json object data if set, else null
   */
  @Nullable
  public JsonElement getData() {
    return data;
  }

  @Override
  Circle build(long id, AnnotationManager<?, Circle, ?, ?, ?, ?> annotationManager) {
    if (geometry == null) {
      throw new RuntimeException("geometry field is required");
    }
    JsonObject jsonObject = new JsonObject();
    jsonObject.addProperty(PROPERTY_CIRCLE_RADIUS, circleRadius);
    jsonObject.addProperty(PROPERTY_CIRCLE_COLOR, circleColor);
    jsonObject.addProperty(PROPERTY_CIRCLE_BLUR, circleBlur);
    jsonObject.addProperty(PROPERTY_CIRCLE_OPACITY, circleOpacity);
    jsonObject.addProperty(PROPERTY_CIRCLE_STROKE_WIDTH, circleStrokeWidth);
    jsonObject.addProperty(PROPERTY_CIRCLE_STROKE_COLOR, circleStrokeColor);
    jsonObject.addProperty(PROPERTY_CIRCLE_STROKE_OPACITY, circleStrokeOpacity);
    Circle circle = new Circle(id, annotationManager, jsonObject, geometry);
    circle.setDraggable(isDraggable);
    circle.setData(data);
    return circle;
  }

  /**
   * Creates CircleOptions out of a Feature.
   *
   * @param feature feature to be converted
   */
  @Nullable
  static CircleOptions fromFeature(@NonNull Feature feature) {
    if (feature.geometry() == null) {
      throw new RuntimeException("geometry field is required");
    }
    if (!(feature.geometry() instanceof Point)) {
      return null;
    }

    CircleOptions options = new CircleOptions();
    options.geometry = (Point) feature.geometry();
    if (feature.hasProperty(PROPERTY_CIRCLE_RADIUS)) {
      options.circleRadius = feature.getProperty(PROPERTY_CIRCLE_RADIUS).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_CIRCLE_COLOR)) {
      options.circleColor = feature.getProperty(PROPERTY_CIRCLE_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_CIRCLE_BLUR)) {
      options.circleBlur = feature.getProperty(PROPERTY_CIRCLE_BLUR).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_CIRCLE_OPACITY)) {
      options.circleOpacity = feature.getProperty(PROPERTY_CIRCLE_OPACITY).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_CIRCLE_STROKE_WIDTH)) {
      options.circleStrokeWidth = feature.getProperty(PROPERTY_CIRCLE_STROKE_WIDTH).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_CIRCLE_STROKE_COLOR)) {
      options.circleStrokeColor = feature.getProperty(PROPERTY_CIRCLE_STROKE_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_CIRCLE_STROKE_OPACITY)) {
      options.circleStrokeOpacity = feature.getProperty(PROPERTY_CIRCLE_STROKE_OPACITY).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_IS_DRAGGABLE)) {
      options.isDraggable = feature.getProperty(PROPERTY_IS_DRAGGABLE).getAsBoolean();
    }
    return options;
  }
}
