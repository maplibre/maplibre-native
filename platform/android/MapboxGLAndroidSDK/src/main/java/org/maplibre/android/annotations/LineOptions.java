package org.maplibre.android.annotations;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.LineString;
import com.mapbox.geojson.Point;

import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.style.layers.Property;

import java.util.ArrayList;
import java.util.List;

/**
 * Builder class from which a line is created.
 */
public class LineOptions extends Options<Line> {

  private boolean isDraggable;
  private JsonElement data;
  private LineString geometry;
  private String lineJoin;
  private Float lineOpacity;
  private String lineColor;
  private Float lineWidth;
  private Float lineGapWidth;
  private Float lineOffset;
  private Float lineBlur;
  private String linePattern;

  static final String PROPERTY_LINE_JOIN = "line-join";
  static final String PROPERTY_LINE_OPACITY = "line-opacity";
  static final String PROPERTY_LINE_COLOR = "line-color";
  static final String PROPERTY_LINE_WIDTH = "line-width";
  static final String PROPERTY_LINE_GAP_WIDTH = "line-gap-width";
  static final String PROPERTY_LINE_OFFSET = "line-offset";
  static final String PROPERTY_LINE_BLUR = "line-blur";
  static final String PROPERTY_LINE_PATTERN = "line-pattern";
  private static final String PROPERTY_IS_DRAGGABLE = "is-draggable";

  /**
   * Set line-join to initialise the line with.
   * <p>
   * The display of lines when joining.
   * </p>
   *
   * @param lineJoin the line-join value
   * @return this
   */
  public LineOptions withLineJoin(@Property.LINE_JOIN String lineJoin) {
    this.lineJoin = lineJoin;
    return this;
  }

  /**
   * Get the current configured  line-join for the line
   * <p>
   * The display of lines when joining.
   * </p>
   *
   * @return lineJoin value
   */
  public String getLineJoin() {
    return lineJoin;
  }

  /**
   * Set line-opacity to initialise the line with.
   * <p>
   * The opacity at which the line will be drawn.
   * </p>
   *
   * @param lineOpacity the line-opacity value
   * @return this
   */
  public LineOptions withLineOpacity(Float lineOpacity) {
    this.lineOpacity = lineOpacity;
    return this;
  }

  /**
   * Get the current configured  line-opacity for the line
   * <p>
   * The opacity at which the line will be drawn.
   * </p>
   *
   * @return lineOpacity value
   */
  public Float getLineOpacity() {
    return lineOpacity;
  }

  /**
   * Set line-color to initialise the line with.
   * <p>
   * The color with which the line will be drawn.
   * </p>
   *
   * @param lineColor the line-color value
   * @return this
   */
  public LineOptions withLineColor(String lineColor) {
    this.lineColor = lineColor;
    return this;
  }

  /**
   * Get the current configured  line-color for the line
   * <p>
   * The color with which the line will be drawn.
   * </p>
   *
   * @return lineColor value
   */
  public String getLineColor() {
    return lineColor;
  }

  /**
   * Set line-width to initialise the line with.
   * <p>
   * Stroke thickness.
   * </p>
   *
   * @param lineWidth the line-width value
   * @return this
   */
  public LineOptions withLineWidth(Float lineWidth) {
    this.lineWidth = lineWidth;
    return this;
  }

  /**
   * Get the current configured  line-width for the line
   * <p>
   * Stroke thickness.
   * </p>
   *
   * @return lineWidth value
   */
  public Float getLineWidth() {
    return lineWidth;
  }

  /**
   * Set line-gap-width to initialise the line with.
   * <p>
   * Draws a line casing outside of a line's actual path. Value indicates the width of the inner gap.
   * </p>
   *
   * @param lineGapWidth the line-gap-width value
   * @return this
   */
  public LineOptions withLineGapWidth(Float lineGapWidth) {
    this.lineGapWidth = lineGapWidth;
    return this;
  }

  /**
   * Get the current configured  line-gap-width for the line
   * <p>
   * Draws a line casing outside of a line's actual path. Value indicates the width of the inner gap.
   * </p>
   *
   * @return lineGapWidth value
   */
  public Float getLineGapWidth() {
    return lineGapWidth;
  }

  /**
   * Set line-offset to initialise the line with.
   * <p>
   * The line's offset. For linear features, a positive value offsets the line to the right, relative to the direction of the line, and a negative value to the left. For polygon features, a positive value results in an inset, and a negative value results in an outset.
   * </p>
   *
   * @param lineOffset the line-offset value
   * @return this
   */
  public LineOptions withLineOffset(Float lineOffset) {
    this.lineOffset = lineOffset;
    return this;
  }

  /**
   * Get the current configured  line-offset for the line
   * <p>
   * The line's offset. For linear features, a positive value offsets the line to the right, relative to the direction of the line, and a negative value to the left. For polygon features, a positive value results in an inset, and a negative value results in an outset.
   * </p>
   *
   * @return lineOffset value
   */
  public Float getLineOffset() {
    return lineOffset;
  }

  /**
   * Set line-blur to initialise the line with.
   * <p>
   * Blur applied to the line, in density-independent pixels.
   * </p>
   *
   * @param lineBlur the line-blur value
   * @return this
   */
  public LineOptions withLineBlur(Float lineBlur) {
    this.lineBlur = lineBlur;
    return this;
  }

  /**
   * Get the current configured  line-blur for the line
   * <p>
   * Blur applied to the line, in density-independent pixels.
   * </p>
   *
   * @return lineBlur value
   */
  public Float getLineBlur() {
    return lineBlur;
  }

  /**
   * Set line-pattern to initialise the line with.
   * <p>
   * Name of image in sprite to use for drawing image lines. For seamless patterns, image width must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
   * </p>
   *
   * @param linePattern the line-pattern value
   * @return this
   */
  public LineOptions withLinePattern(String linePattern) {
    this.linePattern = linePattern;
    return this;
  }

  /**
   * Get the current configured  line-pattern for the line
   * <p>
   * Name of image in sprite to use for drawing image lines. For seamless patterns, image width must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
   * </p>
   *
   * @return linePattern value
   */
  public String getLinePattern() {
    return linePattern;
  }

  /**
   * Set a list of LatLng for the line, which represents the locations of the line on the map
   *
   * @param latLngs a list of the locations of the line in a longitude and latitude pairs
   * @return this
   */
  public LineOptions withLatLngs(List<LatLng> latLngs) {
    List<Point> points = new ArrayList<>();
    for (LatLng latLng : latLngs) {
      points.add(Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude()));
    }
    geometry = LineString.fromLngLats(points);
    return this;
  }

  /**
   * Get a list of LatLng for the line, which represents the locations of the line on the map
   *
   * @return a list of the locations of the line in a longitude and latitude pairs
   */
  public List<LatLng> getLatLngs() {
    List<LatLng> latLngs = new ArrayList<>();
    if (geometry != null) {
      for (Point coordinate : geometry.coordinates()) {
        latLngs.add(new LatLng(coordinate.latitude(), coordinate.longitude()));
      }
    }
    return latLngs;
  }

  /**
   * Set the geometry of the line, which represents the location of the line on the map
   *
   * @param geometry the location of the line
   * @return this
   */
  public LineOptions withGeometry(LineString geometry) {
    this.geometry = geometry;
    return this;
  }

  /**
   * Get the geometry of the line, which represents the location of the line on the map
   *
   * @return the location of the line
   */
  public LineString getGeometry() {
    return geometry;
  }

  /**
   * Returns whether this line is draggable, meaning it can be dragged across the screen when touched and moved.
   *
   * @return draggable when touched
   */
  public boolean getDraggable() {
    return isDraggable;
  }

  /**
   * Set whether this line should be draggable,
   * meaning it can be dragged across the screen when touched and moved.
   *
   * @param draggable should be draggable
   */
  public LineOptions withDraggable(boolean draggable) {
    isDraggable = draggable;
    return this;
  }

  /**
   * Set the arbitrary json data of the annotation.
   *
   * @param jsonElement the arbitrary json element data
   */
  public LineOptions withData(@Nullable JsonElement jsonElement) {
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
  Line build(long id, AnnotationManager<?, Line, ?, ?, ?, ?> annotationManager) {
    if (geometry == null) {
      throw new RuntimeException("geometry field is required");
    }
    JsonObject jsonObject = new JsonObject();
    jsonObject.addProperty(PROPERTY_LINE_JOIN, lineJoin);
    jsonObject.addProperty(PROPERTY_LINE_OPACITY, lineOpacity);
    jsonObject.addProperty(PROPERTY_LINE_COLOR, lineColor);
    jsonObject.addProperty(PROPERTY_LINE_WIDTH, lineWidth);
    jsonObject.addProperty(PROPERTY_LINE_GAP_WIDTH, lineGapWidth);
    jsonObject.addProperty(PROPERTY_LINE_OFFSET, lineOffset);
    jsonObject.addProperty(PROPERTY_LINE_BLUR, lineBlur);
    jsonObject.addProperty(PROPERTY_LINE_PATTERN, linePattern);
    Line line = new Line(id, annotationManager, jsonObject, geometry);
    line.setDraggable(isDraggable);
    line.setData(data);
    return line;
  }

  /**
   * Creates LineOptions out of a Feature.
   *
   * @param feature feature to be converted
   */
  @Nullable
  static LineOptions fromFeature(@NonNull Feature feature) {
    if (feature.geometry() == null) {
      throw new RuntimeException("geometry field is required");
    }
    if (!(feature.geometry() instanceof LineString)) {
      return null;
    }

    LineOptions options = new LineOptions();
    options.geometry = (LineString) feature.geometry();
    if (feature.hasProperty(PROPERTY_LINE_JOIN)) {
      options.lineJoin = feature.getProperty(PROPERTY_LINE_JOIN).getAsString();
    }
    if (feature.hasProperty(PROPERTY_LINE_OPACITY)) {
      options.lineOpacity = feature.getProperty(PROPERTY_LINE_OPACITY).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_LINE_COLOR)) {
      options.lineColor = feature.getProperty(PROPERTY_LINE_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_LINE_WIDTH)) {
      options.lineWidth = feature.getProperty(PROPERTY_LINE_WIDTH).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_LINE_GAP_WIDTH)) {
      options.lineGapWidth = feature.getProperty(PROPERTY_LINE_GAP_WIDTH).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_LINE_OFFSET)) {
      options.lineOffset = feature.getProperty(PROPERTY_LINE_OFFSET).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_LINE_BLUR)) {
      options.lineBlur = feature.getProperty(PROPERTY_LINE_BLUR).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_LINE_PATTERN)) {
      options.linePattern = feature.getProperty(PROPERTY_LINE_PATTERN).getAsString();
    }
    if (feature.hasProperty(PROPERTY_IS_DRAGGABLE)) {
      options.isDraggable = feature.getProperty(PROPERTY_IS_DRAGGABLE).getAsBoolean();
    }
    return options;
  }
}
