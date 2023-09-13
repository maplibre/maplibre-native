//package org.maplibre.android.annotations;
//
//import static org.maplibre.android.constants.GeometryConstants.MAX_MERCATOR_LATITUDE;
//import static org.maplibre.android.constants.GeometryConstants.MIN_MERCATOR_LATITUDE;
//
//import android.graphics.PointF;
//
//import androidx.annotation.ColorInt;
//import androidx.annotation.NonNull;
//import androidx.annotation.Nullable;
//import androidx.annotation.UiThread;
//
//import com.google.gson.JsonNull;
//import com.google.gson.JsonObject;
//import com.mapbox.android.gestures.MoveDistancesObject;
//import com.mapbox.geojson.Geometry;
//import com.mapbox.geojson.LineString;
//import com.mapbox.geojson.Point;
//
//import org.maplibre.android.geometry.LatLng;
//import org.maplibre.android.maps.Projection;
//import org.maplibre.android.style.layers.Property;
//import org.maplibre.android.utils.ColorUtils;
//
//import java.util.ArrayList;
//import java.util.List;
//
//@UiThread
//public class Line extends AbstractAnnotation<LineString> {
//
//  private final AnnotationManager<?, Line, ?, ?, ?, ?> annotationManager;
//
//  /**
//   * Create a line.
//   *
//   * @param id         the id of the line
//   * @param jsonObject the features of the annotation
//   * @param geometry   the geometry of the annotation
//   */
//  Line(long id, AnnotationManager<?, Line, ?, ?, ?, ?> annotationManager, JsonObject jsonObject, LineString geometry) {
//    super(id, jsonObject, geometry);
//    this.annotationManager = annotationManager;
//  }
//
//  @Override
//  void setUsedDataDrivenProperties() {
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_JOIN) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_JOIN);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_OPACITY) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_OPACITY);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_COLOR) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_COLOR);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_WIDTH) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_WIDTH);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_GAP_WIDTH) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_GAP_WIDTH);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_OFFSET) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_OFFSET);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_BLUR) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_BLUR);
//    }
//    if (!(jsonObject.get(LineOptions.PROPERTY_LINE_PATTERN) instanceof JsonNull)) {
//      annotationManager.enableDataDrivenProperty(LineOptions.PROPERTY_LINE_PATTERN);
//    }
//  }
//
//  /**
//   * Set a list of LatLng for the line, which represents the locations of the line on the map
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param latLngs a list of the locations of the line in a longitude and latitude pairs
//   */
//  public void setLatLngs(List<LatLng> latLngs) {
//    List<Point> points = new ArrayList<>();
//    for (LatLng latLng : latLngs) {
//      points.add(Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude()));
//    }
//    geometry = LineString.fromLngLats(points);
//  }
//
//  /**
//   * Get a list of LatLng for the line, which represents the locations of the line on the map
//   *
//   * @return a list of the locations of the line in a latitude and longitude pairs
//   */
//  @NonNull
//  public List<LatLng> getLatLngs() {
//    LineString lineString = (LineString) geometry;
//    List<LatLng> latLngs = new ArrayList<>();
//    for (Point point : lineString.coordinates()) {
//      latLngs.add(new LatLng(point.latitude(), point.longitude()));
//    }
//    return latLngs;
//  }
//
//  // Property accessors
//
//  /**
//   * Get the LineJoin property
//   * <p>
//   * The display of lines when joining.
//   * </p>
//   *
//   * @return property wrapper value around String
//   */
//  public String getLineJoin() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_JOIN).getAsString();
//  }
//
//  /**
//   * Set the LineJoin property
//   * <p>
//   * The display of lines when joining.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for String
//   */
//  public void setLineJoin(@Property.LINE_JOIN String value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_JOIN, value);
//  }
//
//  /**
//   * Get the LineOpacity property
//   * <p>
//   * The opacity at which the line will be drawn.
//   * </p>
//   *
//   * @return property wrapper value around Float
//   */
//  public Float getLineOpacity() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_OPACITY).getAsFloat();
//  }
//
//  /**
//   * Set the LineOpacity property
//   * <p>
//   * The opacity at which the line will be drawn.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for Float
//   */
//  public void setLineOpacity(Float value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_OPACITY, value);
//  }
//
//  /**
//   * Get the LineColor property
//   * <p>
//   * The color with which the line will be drawn.
//   * </p>
//   *
//   * @return color value for String
//   */
//  @ColorInt
//  public int getLineColorAsInt() {
//    return ColorUtils.rgbaToColor(jsonObject.get(LineOptions.PROPERTY_LINE_COLOR).getAsString());
//  }
//
//  /**
//   * Get the LineColor property
//   * <p>
//   * The color with which the line will be drawn.
//   * </p>
//   *
//   * @return color value for String
//   */
//  public String getLineColor() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_COLOR).getAsString();
//  }
//
//  /**
//   * Set the LineColor property
//   * <p>
//   * The color with which the line will be drawn.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param color value for String
//   */
//  public void setLineColor(@ColorInt int color) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_COLOR, ColorUtils.colorToRgbaString(color));
//  }
//
//  /**
//   * Set the LineColor property
//   * <p>
//   * The color with which the line will be drawn.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param color value for String
//   */
//  public void setLineColor(@NonNull String color) {
//    jsonObject.addProperty("line-color", color);
//  }
//
//  /**
//   * Get the LineWidth property
//   * <p>
//   * Stroke thickness.
//   * </p>
//   *
//   * @return property wrapper value around Float
//   */
//  public Float getLineWidth() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_WIDTH).getAsFloat();
//  }
//
//  /**
//   * Set the LineWidth property
//   * <p>
//   * Stroke thickness.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for Float
//   */
//  public void setLineWidth(Float value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_WIDTH, value);
//  }
//
//  /**
//   * Get the LineGapWidth property
//   * <p>
//   * Draws a line casing outside of a line's actual path. Value indicates the width of the inner gap.
//   * </p>
//   *
//   * @return property wrapper value around Float
//   */
//  public Float getLineGapWidth() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_GAP_WIDTH).getAsFloat();
//  }
//
//  /**
//   * Set the LineGapWidth property
//   * <p>
//   * Draws a line casing outside of a line's actual path. Value indicates the width of the inner gap.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for Float
//   */
//  public void setLineGapWidth(Float value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_GAP_WIDTH, value);
//  }
//
//  /**
//   * Get the LineOffset property
//   * <p>
//   * The line's offset. For linear features, a positive value offsets the line to the right, relative to the direction
//   * of the line, and a negative value to the left. For polygon features, a positive value results in an inset, and a
//   * negative value results in an outset.
//   * </p>
//   *
//   * @return property wrapper value around Float
//   */
//  public Float getLineOffset() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_OFFSET).getAsFloat();
//  }
//
//  /**
//   * Set the LineOffset property
//   * <p>
//   * The line's offset. For linear features, a positive value offsets the line to the right, relative to the direction
//   * of the line, and a negative value to the left. For polygon features, a positive value results in an inset, and a
//   * negative value results in an outset.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for Float
//   */
//  public void setLineOffset(Float value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_OFFSET, value);
//  }
//
//  /**
//   * Get the LineBlur property
//   * <p>
//   * Blur applied to the line, in density-independent pixels.
//   * </p>
//   *
//   * @return property wrapper value around Float
//   */
//  public Float getLineBlur() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_BLUR).getAsFloat();
//  }
//
//  /**
//   * Set the LineBlur property
//   * <p>
//   * Blur applied to the line, in density-independent pixels.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for Float
//   */
//  public void setLineBlur(Float value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_BLUR, value);
//  }
//
//  /**
//   * Get the LinePattern property
//   * <p>
//   * Name of image in sprite to use for drawing image lines. For seamless patterns, image width must be a factor of two
//   * (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
//   * </p>
//   *
//   * @return property wrapper value around String
//   */
//  public String getLinePattern() {
//    return jsonObject.get(LineOptions.PROPERTY_LINE_PATTERN).getAsString();
//  }
//
//  /**
//   * Set the LinePattern property
//   * <p>
//   * Name of image in sprite to use for drawing image lines. For seamless patterns, image width must be a factor of two
//   * (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
//   * </p>
//   * <p>
//   * To update the line on the map use {@link LineManager#update(AbstractAnnotation)}.
//   * <p>
//   *
//   * @param value constant property value for String
//   */
//  public void setLinePattern(String value) {
//    jsonObject.addProperty(LineOptions.PROPERTY_LINE_PATTERN, value);
//  }
//
//  @Override
//  @Nullable
//  Geometry getOffsetGeometry(@NonNull Projection projection, @NonNull MoveDistancesObject moveDistancesObject,
//                             float touchAreaShiftX, float touchAreaShiftY) {
//    List<Point> originalPoints = geometry.coordinates();
//    List<Point> resultingPoints = new ArrayList<>(originalPoints.size());
//    for (Point jsonPoint : originalPoints) {
//      PointF pointF = projection.toScreenLocation(new LatLng(jsonPoint.latitude(), jsonPoint.longitude()));
//      pointF.x -= moveDistancesObject.getDistanceXSinceLast();
//      pointF.y -= moveDistancesObject.getDistanceYSinceLast();
//
//      LatLng latLng = projection.fromScreenLocation(pointF);
//      if (latLng.getLatitude() > MAX_MERCATOR_LATITUDE || latLng.getLatitude() < MIN_MERCATOR_LATITUDE) {
//        return null;
//      }
//      resultingPoints.add(Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude()));
//    }
//
//    return LineString.fromLngLats(resultingPoints);
//  }
//
//  @Override
//  String getName() {
//    return "Line";
//  }
//}
