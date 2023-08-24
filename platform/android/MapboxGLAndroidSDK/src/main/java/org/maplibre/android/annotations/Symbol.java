package org.maplibre.android.annotations;

import static org.maplibre.android.constants.GeometryConstants.MAX_MERCATOR_LATITUDE;
import static org.maplibre.android.constants.GeometryConstants.MIN_MERCATOR_LATITUDE;

import android.graphics.PointF;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import com.google.gson.JsonArray;
import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import com.mapbox.android.gestures.MoveDistancesObject;
import com.mapbox.geojson.Geometry;
import com.mapbox.geojson.Point;

import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.maps.Projection;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyFactory;
import org.maplibre.android.utils.ColorUtils;

@UiThread
public class Symbol extends AbstractAnnotation<Point> {

  private final AnnotationManager<?, Symbol, ?, ?, ?, ?> annotationManager;

  /**
   * Create a symbol.
   *
   * @param id         the id of the symbol
   * @param jsonObject the features of the annotation
   * @param geometry   the geometry of the annotation
   */
  Symbol(long id, AnnotationManager<?, Symbol, ?, ?, ?, ?> annotationManager, JsonObject jsonObject, Point geometry) {
    super(id, jsonObject, geometry);
    this.annotationManager = annotationManager;
  }

  @Override
  void setUsedDataDrivenProperties() {
    if (!(jsonObject.get(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_SIZE) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_SIZE);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_IMAGE) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_IMAGE);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_ROTATE) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_ROTATE);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_OFFSET) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_OFFSET);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_ANCHOR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_ANCHOR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_FIELD) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_FIELD);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_FONT) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_FONT);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_SIZE) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_SIZE);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_JUSTIFY) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_JUSTIFY);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_ANCHOR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_ANCHOR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_ROTATE) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_ROTATE);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_TRANSFORM) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_TRANSFORM);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_OFFSET) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_OFFSET);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_OPACITY) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_OPACITY);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_COLOR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_COLOR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_COLOR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_HALO_COLOR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_WIDTH) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_HALO_WIDTH);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_BLUR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_ICON_HALO_BLUR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_OPACITY) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_OPACITY);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_COLOR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_COLOR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_COLOR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_HALO_COLOR);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH);
    }
    if (!(jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_BLUR) instanceof JsonNull)) {
      annotationManager.enableDataDrivenProperty(SymbolOptions.PROPERTY_TEXT_HALO_BLUR);
    }
  }

  /**
   * Set the LatLng of the symbol, which represents the location of the symbol on the map
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param latLng the location of the symbol in a latitude and longitude pair
   */
  public void setLatLng(LatLng latLng) {
    geometry = Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude());
  }

  /**
   * Get the LatLng of the symbol, which represents the location of the symbol on the map
   *
   * @return the location of the symbol
   */
  @NonNull
  public LatLng getLatLng() {
    return new LatLng(geometry.latitude(), geometry.longitude());
  }

  // Property accessors

  /**
   * Get the SymbolSortKey property
   * <p>
   * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features
   * with a lower sort key when they overlap. Features with a lower sort key will have priority over other features when
   * doing placement.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getSymbolSortKey() {
    return jsonObject.get(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY).getAsFloat();
  }

  /**
   * Set the SymbolSortKey property
   * <p>
   * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features
   * with a lower sort key when they overlap. Features with a lower sort key will have priority over other features when
   * doing placement.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setSymbolSortKey(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY, value);
  }

  /**
   * Get the IconSize property
   * <p>
   * Scales the original size of the icon by the provided factor. The new pixel size of the image will be the original
   * pixel size multiplied by {@link PropertyFactory#iconSize}. 1 is the original size; 3 triples the size of the
   * image.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getIconSize() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_SIZE).getAsFloat();
  }

  /**
   * Set the IconSize property
   * <p>
   * Scales the original size of the icon by the provided factor. The new pixel size of the image will be the original
   * pixel size multiplied by {@link PropertyFactory#iconSize}. 1 is the original size; 3 triples the size of the
   * image.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setIconSize(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_SIZE, value);
  }

  /**
   * Get the IconImage property
   * <p>
   * Name of image in sprite to use for drawing an image background.
   * </p>
   *
   * @return property wrapper value around String
   */
  public String getIconImage() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_IMAGE).getAsString();
  }

  /**
   * Set the IconImage property
   * <p>
   * Name of image in sprite to use for drawing an image background.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String
   */
  public void setIconImage(String value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_IMAGE, value);
  }

  /**
   * Get the IconRotate property
   * <p>
   * Rotates the icon clockwise.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getIconRotate() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_ROTATE).getAsFloat();
  }

  /**
   * Set the IconRotate property
   * <p>
   * Rotates the icon clockwise.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setIconRotate(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_ROTATE, value);
  }

  /**
   * Get the IconOffset property
   * <p>
   * Offset distance of icon from its anchor. Positive values indicate right and down, while negative values indicate
   * left and up. Each component is multiplied by the value of {@link PropertyFactory#iconSize} to obtain the final
   * offset in density-independent pixels. When combined with {@link PropertyFactory#iconRotate} the offset will be as
   * if the rotated direction was up.
   * </p>
   *
   * @return PointF value for Float[]
   */
  public PointF getIconOffset() {
    JsonArray jsonArray = jsonObject.getAsJsonArray(SymbolOptions.PROPERTY_ICON_OFFSET);
    return new PointF(jsonArray.get(0).getAsFloat(), jsonArray.get(1).getAsFloat());
  }

  /**
   * Set the IconOffset property.
   * <p>
   * Offset distance of icon from its anchor. Positive values indicate right and down, while negative values indicate
   * left and up. Each component is multiplied by the value of {@link PropertyFactory#iconSize} to obtain the final
   * offset in density-independent pixels. When combined with {@link PropertyFactory#iconRotate} the offset will be as
   * if the rotated direction was up.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param pointF value for Float[]
   */
  public void setIconOffset(PointF pointF) {
    JsonArray jsonArray = new JsonArray();
    jsonArray.add(pointF.x);
    jsonArray.add(pointF.y);
    jsonObject.add(SymbolOptions.PROPERTY_ICON_OFFSET, jsonArray);
  }

  /**
   * Get the IconAnchor property
   * <p>
   * Part of the icon placed closest to the anchor.
   * </p>
   *
   * @return property wrapper value around String
   */
  public String getIconAnchor() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_ANCHOR).getAsString();
  }

  /**
   * Set the IconAnchor property
   * <p>
   * Part of the icon placed closest to the anchor.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String
   */
  public void setIconAnchor(@Property.ICON_ANCHOR String value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_ANCHOR, value);
  }

  /**
   * Get the TextField property
   * <p>
   * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with
   * default/inherited formatting options.
   * </p>
   *
   * @return property wrapper value around String
   */
  public String getTextField() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_FIELD).getAsString();
  }

  /**
   * Set the TextField property
   * <p>
   * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with
   * default/inherited formatting options.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String
   */
  public void setTextField(String value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_FIELD, value);
  }

  /**
   * Get the TextFont property
   * <p>
   * Font stack to use for displaying text.
   * </p>
   *
   * @return property wrapper value around String[]
   */
  public String[] getTextFont() {
    JsonArray jsonArray = jsonObject.getAsJsonArray(SymbolOptions.PROPERTY_TEXT_FONT);
    String[] value = new String[jsonArray.size()];
    for (int i = 0; i < jsonArray.size(); i++) {
      value[i] = jsonArray.get(i).getAsString();
    }
    return value;
  }

  /**
   * Set the TextFont property.
   * <p>
   * Font stack to use for displaying text.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String[]
   */
  public void setTextFont(String[] value) {
    JsonArray jsonArray = new JsonArray();
    for (String element : value) {
      jsonArray.add(element);
    }
    jsonObject.add(SymbolOptions.PROPERTY_TEXT_FONT, jsonArray);
  }

  /**
   * Get the TextSize property
   * <p>
   * Font size.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextSize() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_SIZE).getAsFloat();
  }

  /**
   * Set the TextSize property
   * <p>
   * Font size.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextSize(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_SIZE, value);
  }

  /**
   * Get the TextMaxWidth property
   * <p>
   * The maximum line width for text wrapping.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextMaxWidth() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH).getAsFloat();
  }

  /**
   * Set the TextMaxWidth property
   * <p>
   * The maximum line width for text wrapping.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextMaxWidth(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH, value);
  }

  /**
   * Get the TextLetterSpacing property
   * <p>
   * Text tracking amount.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextLetterSpacing() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING).getAsFloat();
  }

  /**
   * Set the TextLetterSpacing property
   * <p>
   * Text tracking amount.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextLetterSpacing(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING, value);
  }

  /**
   * Get the TextJustify property
   * <p>
   * Text justification options.
   * </p>
   *
   * @return property wrapper value around String
   */
  public String getTextJustify() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_JUSTIFY).getAsString();
  }

  /**
   * Set the TextJustify property
   * <p>
   * Text justification options.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String
   */
  public void setTextJustify(@Property.TEXT_JUSTIFY String value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_JUSTIFY, value);
  }

  /**
   * Get the TextRadialOffset property
   * <p>
   * Radial offset of text, in the direction of the symbol's anchor. Useful in combination with
   * {@link PropertyFactory#textVariableAnchor}, which doesn't support the two-dimensional
   * {@link PropertyFactory#textOffset}.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextRadialOffset() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET).getAsFloat();
  }

  /**
   * Set the TextRadialOffset property
   * <p>
   * Radial offset of text, in the direction of the symbol's anchor. Useful in combination with
   * {@link PropertyFactory#textVariableAnchor}, which doesn't support the two-dimensional
   * {@link PropertyFactory#textOffset}.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextRadialOffset(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET, value);
  }

  /**
   * Get the TextAnchor property
   * <p>
   * Part of the text placed closest to the anchor.
   * </p>
   *
   * @return property wrapper value around String
   */
  public String getTextAnchor() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_ANCHOR).getAsString();
  }

  /**
   * Set the TextAnchor property
   * <p>
   * Part of the text placed closest to the anchor.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String
   */
  public void setTextAnchor(@Property.TEXT_ANCHOR String value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_ANCHOR, value);
  }

  /**
   * Get the TextRotate property
   * <p>
   * Rotates the text clockwise.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextRotate() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_ROTATE).getAsFloat();
  }

  /**
   * Set the TextRotate property
   * <p>
   * Rotates the text clockwise.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextRotate(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_ROTATE, value);
  }

  /**
   * Get the TextTransform property
   * <p>
   * Specifies how to capitalize text, similar to the CSS {@link PropertyFactory#textTransform} property.
   * </p>
   *
   * @return property wrapper value around String
   */
  public String getTextTransform() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_TRANSFORM).getAsString();
  }

  /**
   * Set the TextTransform property
   * <p>
   * Specifies how to capitalize text, similar to the CSS {@link PropertyFactory#textTransform} property.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for String
   */
  public void setTextTransform(@Property.TEXT_TRANSFORM String value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_TRANSFORM, value);
  }

  /**
   * Get the TextOffset property
   * <p>
   * Offset distance of text from its anchor. Positive values indicate right and down, while negative values indicate
   * left and up.
   * </p>
   *
   * @return PointF value for Float[]
   */
  public PointF getTextOffset() {
    JsonArray jsonArray = jsonObject.getAsJsonArray(SymbolOptions.PROPERTY_TEXT_OFFSET);
    return new PointF(jsonArray.get(0).getAsFloat(), jsonArray.get(1).getAsFloat());
  }

  /**
   * Set the TextOffset property.
   * <p>
   * Offset distance of text from its anchor. Positive values indicate right and down, while negative values indicate
   * left and up.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param pointF value for Float[]
   */
  public void setTextOffset(PointF pointF) {
    JsonArray jsonArray = new JsonArray();
    jsonArray.add(pointF.x);
    jsonArray.add(pointF.y);
    jsonObject.add(SymbolOptions.PROPERTY_TEXT_OFFSET, jsonArray);
  }

  /**
   * Get the IconOpacity property
   * <p>
   * The opacity at which the icon will be drawn.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getIconOpacity() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_OPACITY).getAsFloat();
  }

  /**
   * Set the IconOpacity property
   * <p>
   * The opacity at which the icon will be drawn.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setIconOpacity(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_OPACITY, value);
  }

  /**
   * Get the IconColor property
   * <p>
   * The color of the icon. This can only be used with sdf icons.
   * </p>
   *
   * @return color value for String
   */
  @ColorInt
  public int getIconColorAsInt() {
    return ColorUtils.rgbaToColor(jsonObject.get(SymbolOptions.PROPERTY_ICON_COLOR).getAsString());
  }

  /**
   * Get the IconColor property
   * <p>
   * The color of the icon. This can only be used with sdf icons.
   * </p>
   *
   * @return color value for String
   */
  public String getIconColor() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_COLOR).getAsString();
  }

  /**
   * Set the IconColor property
   * <p>
   * The color of the icon. This can only be used with sdf icons.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setIconColor(@ColorInt int color) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_COLOR, ColorUtils.colorToRgbaString(color));
  }

  /**
   * Set the IconColor property
   * <p>
   * The color of the icon. This can only be used with sdf icons.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setIconColor(@NonNull String color) {
    jsonObject.addProperty("icon-color", color);
  }

  /**
   * Get the IconHaloColor property
   * <p>
   * The color of the icon's halo. Icon halos can only be used with SDF icons.
   * </p>
   *
   * @return color value for String
   */
  @ColorInt
  public int getIconHaloColorAsInt() {
    return ColorUtils.rgbaToColor(jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_COLOR).getAsString());
  }

  /**
   * Get the IconHaloColor property
   * <p>
   * The color of the icon's halo. Icon halos can only be used with SDF icons.
   * </p>
   *
   * @return color value for String
   */
  public String getIconHaloColor() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_COLOR).getAsString();
  }

  /**
   * Set the IconHaloColor property
   * <p>
   * The color of the icon's halo. Icon halos can only be used with SDF icons.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setIconHaloColor(@ColorInt int color) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_HALO_COLOR, ColorUtils.colorToRgbaString(color));
  }

  /**
   * Set the IconHaloColor property
   * <p>
   * The color of the icon's halo. Icon halos can only be used with SDF icons.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setIconHaloColor(@NonNull String color) {
    jsonObject.addProperty("icon-halo-color", color);
  }

  /**
   * Get the IconHaloWidth property
   * <p>
   * Distance of halo to the icon outline.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getIconHaloWidth() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_WIDTH).getAsFloat();
  }

  /**
   * Set the IconHaloWidth property
   * <p>
   * Distance of halo to the icon outline.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setIconHaloWidth(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_HALO_WIDTH, value);
  }

  /**
   * Get the IconHaloBlur property
   * <p>
   * Fade out the halo towards the outside.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getIconHaloBlur() {
    return jsonObject.get(SymbolOptions.PROPERTY_ICON_HALO_BLUR).getAsFloat();
  }

  /**
   * Set the IconHaloBlur property
   * <p>
   * Fade out the halo towards the outside.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setIconHaloBlur(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_ICON_HALO_BLUR, value);
  }

  /**
   * Get the TextOpacity property
   * <p>
   * The opacity at which the text will be drawn.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextOpacity() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_OPACITY).getAsFloat();
  }

  /**
   * Set the TextOpacity property
   * <p>
   * The opacity at which the text will be drawn.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextOpacity(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_OPACITY, value);
  }

  /**
   * Get the TextColor property
   * <p>
   * The color with which the text will be drawn.
   * </p>
   *
   * @return color value for String
   */
  @ColorInt
  public int getTextColorAsInt() {
    return ColorUtils.rgbaToColor(jsonObject.get(SymbolOptions.PROPERTY_TEXT_COLOR).getAsString());
  }

  /**
   * Get the TextColor property
   * <p>
   * The color with which the text will be drawn.
   * </p>
   *
   * @return color value for String
   */
  public String getTextColor() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_COLOR).getAsString();
  }

  /**
   * Set the TextColor property
   * <p>
   * The color with which the text will be drawn.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setTextColor(@ColorInt int color) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_COLOR, ColorUtils.colorToRgbaString(color));
  }

  /**
   * Set the TextColor property
   * <p>
   * The color with which the text will be drawn.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setTextColor(@NonNull String color) {
    jsonObject.addProperty("text-color", color);
  }

  /**
   * Get the TextHaloColor property
   * <p>
   * The color of the text's halo, which helps it stand out from backgrounds.
   * </p>
   *
   * @return color value for String
   */
  @ColorInt
  public int getTextHaloColorAsInt() {
    return ColorUtils.rgbaToColor(jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_COLOR).getAsString());
  }

  /**
   * Get the TextHaloColor property
   * <p>
   * The color of the text's halo, which helps it stand out from backgrounds.
   * </p>
   *
   * @return color value for String
   */
  public String getTextHaloColor() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_COLOR).getAsString();
  }

  /**
   * Set the TextHaloColor property
   * <p>
   * The color of the text's halo, which helps it stand out from backgrounds.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setTextHaloColor(@ColorInt int color) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_HALO_COLOR, ColorUtils.colorToRgbaString(color));
  }

  /**
   * Set the TextHaloColor property
   * <p>
   * The color of the text's halo, which helps it stand out from backgrounds.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param color value for String
   */
  public void setTextHaloColor(@NonNull String color) {
    jsonObject.addProperty("text-halo-color", color);
  }

  /**
   * Get the TextHaloWidth property
   * <p>
   * Distance of halo to the font outline. Max text halo width is 1/4 of the font-size.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextHaloWidth() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH).getAsFloat();
  }

  /**
   * Set the TextHaloWidth property
   * <p>
   * Distance of halo to the font outline. Max text halo width is 1/4 of the font-size.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextHaloWidth(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH, value);
  }

  /**
   * Get the TextHaloBlur property
   * <p>
   * The halo's fadeout distance towards the outside.
   * </p>
   *
   * @return property wrapper value around Float
   */
  public Float getTextHaloBlur() {
    return jsonObject.get(SymbolOptions.PROPERTY_TEXT_HALO_BLUR).getAsFloat();
  }

  /**
   * Set the TextHaloBlur property
   * <p>
   * The halo's fadeout distance towards the outside.
   * </p>
   * <p>
   * To update the symbol on the map use {@link SymbolManager#update(AbstractAnnotation)}.
   * <p>
   *
   * @param value constant property value for Float
   */
  public void setTextHaloBlur(Float value) {
    jsonObject.addProperty(SymbolOptions.PROPERTY_TEXT_HALO_BLUR, value);
  }

  @Override
  @Nullable
  Geometry getOffsetGeometry(@NonNull Projection projection, @NonNull MoveDistancesObject moveDistancesObject,
                             float touchAreaShiftX, float touchAreaShiftY) {
    PointF pointF = new PointF(
      moveDistancesObject.getCurrentX() - touchAreaShiftX,
      moveDistancesObject.getCurrentY() - touchAreaShiftY
    );

    LatLng latLng = projection.fromScreenLocation(pointF);
    if (latLng.getLatitude() > MAX_MERCATOR_LATITUDE || latLng.getLatitude() < MIN_MERCATOR_LATITUDE) {
      return null;
    }

    return Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude());
  }

  @Override
  String getName() {
    return "Symbol";
  }
}
