package org.maplibre.android.annotations;

import static org.maplibre.android.annotations.ConvertUtils.convertArray;
import static org.maplibre.android.annotations.ConvertUtils.toFloatArray;
import static org.maplibre.android.annotations.ConvertUtils.toStringArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.Point;

import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyFactory;

/**
 * Builder class from which a symbol is created.
 */
public class SymbolOptions extends Options<Symbol> {

  private boolean isDraggable;
  private JsonElement data;
  private Point geometry;
  private Float symbolSortKey;
  private Float iconSize;
  private String iconImage;
  private Float iconRotate;
  private Float[] iconOffset;
  private String iconAnchor;
  private String textField;
  private String[] textFont;
  private Float textSize;
  private Float textMaxWidth;
  private Float textLetterSpacing;
  private String textJustify;
  private Float textRadialOffset;
  private String textAnchor;
  private Float textRotate;
  private String textTransform;
  private Float[] textOffset;
  private Float iconOpacity;
  private String iconColor;
  private String iconHaloColor;
  private Float iconHaloWidth;
  private Float iconHaloBlur;
  private Float textOpacity;
  private String textColor;
  private String textHaloColor;
  private Float textHaloWidth;
  private Float textHaloBlur;

  static final String PROPERTY_SYMBOL_SORT_KEY = "symbol-sort-key";
  static final String PROPERTY_ICON_SIZE = "icon-size";
  static final String PROPERTY_ICON_IMAGE = "icon-image";
  static final String PROPERTY_ICON_ROTATE = "icon-rotate";
  static final String PROPERTY_ICON_OFFSET = "icon-offset";
  static final String PROPERTY_ICON_ANCHOR = "icon-anchor";
  static final String PROPERTY_TEXT_FIELD = "text-field";
  static final String PROPERTY_TEXT_FONT = "text-font";
  static final String PROPERTY_TEXT_SIZE = "text-size";
  static final String PROPERTY_TEXT_MAX_WIDTH = "text-max-width";
  static final String PROPERTY_TEXT_LETTER_SPACING = "text-letter-spacing";
  static final String PROPERTY_TEXT_JUSTIFY = "text-justify";
  static final String PROPERTY_TEXT_RADIAL_OFFSET = "text-radial-offset";
  static final String PROPERTY_TEXT_ANCHOR = "text-anchor";
  static final String PROPERTY_TEXT_ROTATE = "text-rotate";
  static final String PROPERTY_TEXT_TRANSFORM = "text-transform";
  static final String PROPERTY_TEXT_OFFSET = "text-offset";
  static final String PROPERTY_ICON_OPACITY = "icon-opacity";
  static final String PROPERTY_ICON_COLOR = "icon-color";
  static final String PROPERTY_ICON_HALO_COLOR = "icon-halo-color";
  static final String PROPERTY_ICON_HALO_WIDTH = "icon-halo-width";
  static final String PROPERTY_ICON_HALO_BLUR = "icon-halo-blur";
  static final String PROPERTY_TEXT_OPACITY = "text-opacity";
  static final String PROPERTY_TEXT_COLOR = "text-color";
  static final String PROPERTY_TEXT_HALO_COLOR = "text-halo-color";
  static final String PROPERTY_TEXT_HALO_WIDTH = "text-halo-width";
  static final String PROPERTY_TEXT_HALO_BLUR = "text-halo-blur";
  private static final String PROPERTY_IS_DRAGGABLE = "is-draggable";

  /**
   * Set symbol-sort-key to initialise the symbol with.
   * <p>
   * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key when they overlap. Features with a lower sort key will have priority over other features when doing placement.
   * </p>
   *
   * @param symbolSortKey the symbol-sort-key value
   * @return this
   */
  public SymbolOptions withSymbolSortKey(Float symbolSortKey) {
    this.symbolSortKey = symbolSortKey;
    return this;
  }

  /**
   * Get the current configured  symbol-sort-key for the symbol
   * <p>
   * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key when they overlap. Features with a lower sort key will have priority over other features when doing placement.
   * </p>
   *
   * @return symbolSortKey value
   */
  public Float getSymbolSortKey() {
    return symbolSortKey;
  }

  /**
   * Set icon-size to initialise the symbol with.
   * <p>
   * Scales the original size of the icon by the provided factor. The new pixel size of the image will be the original pixel size multiplied by {@link PropertyFactory#iconSize}. 1 is the original size; 3 triples the size of the image.
   * </p>
   *
   * @param iconSize the icon-size value
   * @return this
   */
  public SymbolOptions withIconSize(Float iconSize) {
    this.iconSize = iconSize;
    return this;
  }

  /**
   * Get the current configured  icon-size for the symbol
   * <p>
   * Scales the original size of the icon by the provided factor. The new pixel size of the image will be the original pixel size multiplied by {@link PropertyFactory#iconSize}. 1 is the original size; 3 triples the size of the image.
   * </p>
   *
   * @return iconSize value
   */
  public Float getIconSize() {
    return iconSize;
  }

  /**
   * Set icon-image to initialise the symbol with.
   * <p>
   * Name of image in sprite to use for drawing an image background.
   * </p>
   *
   * @param iconImage the icon-image value
   * @return this
   */
  public SymbolOptions withIconImage(String iconImage) {
    this.iconImage = iconImage;
    return this;
  }

  /**
   * Get the current configured  icon-image for the symbol
   * <p>
   * Name of image in sprite to use for drawing an image background.
   * </p>
   *
   * @return iconImage value
   */
  public String getIconImage() {
    return iconImage;
  }

  /**
   * Set icon-rotate to initialise the symbol with.
   * <p>
   * Rotates the icon clockwise.
   * </p>
   *
   * @param iconRotate the icon-rotate value
   * @return this
   */
  public SymbolOptions withIconRotate(Float iconRotate) {
    this.iconRotate = iconRotate;
    return this;
  }

  /**
   * Get the current configured  icon-rotate for the symbol
   * <p>
   * Rotates the icon clockwise.
   * </p>
   *
   * @return iconRotate value
   */
  public Float getIconRotate() {
    return iconRotate;
  }

  /**
   * Set icon-offset to initialise the symbol with.
   * <p>
   * Offset distance of icon from its anchor. Positive values indicate right and down, while negative values indicate left and up. Each component is multiplied by the value of {@link PropertyFactory#iconSize} to obtain the final offset in density-independent pixels. When combined with {@link PropertyFactory#iconRotate} the offset will be as if the rotated direction was up.
   * </p>
   *
   * @param iconOffset the icon-offset value
   * @return this
   */
  public SymbolOptions withIconOffset(Float[] iconOffset) {
    this.iconOffset = iconOffset;
    return this;
  }

  /**
   * Get the current configured  icon-offset for the symbol
   * <p>
   * Offset distance of icon from its anchor. Positive values indicate right and down, while negative values indicate left and up. Each component is multiplied by the value of {@link PropertyFactory#iconSize} to obtain the final offset in density-independent pixels. When combined with {@link PropertyFactory#iconRotate} the offset will be as if the rotated direction was up.
   * </p>
   *
   * @return iconOffset value
   */
  public Float[] getIconOffset() {
    return iconOffset;
  }

  /**
   * Set icon-anchor to initialise the symbol with.
   * <p>
   * Part of the icon placed closest to the anchor.
   * </p>
   *
   * @param iconAnchor the icon-anchor value
   * @return this
   */
  public SymbolOptions withIconAnchor(@Property.ICON_ANCHOR String iconAnchor) {
    this.iconAnchor = iconAnchor;
    return this;
  }

  /**
   * Get the current configured  icon-anchor for the symbol
   * <p>
   * Part of the icon placed closest to the anchor.
   * </p>
   *
   * @return iconAnchor value
   */
  public String getIconAnchor() {
    return iconAnchor;
  }

  /**
   * Set text-field to initialise the symbol with.
   * <p>
   * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with default/inherited formatting options.
   * </p>
   *
   * @param textField the text-field value
   * @return this
   */
  public SymbolOptions withTextField(String textField) {
    this.textField = textField;
    return this;
  }

  /**
   * Get the current configured  text-field for the symbol
   * <p>
   * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with default/inherited formatting options.
   * </p>
   *
   * @return textField value
   */
  public String getTextField() {
    return textField;
  }

  /**
   * Set text-font to initialise the symbol with.
   * <p>
   * Font stack to use for displaying text.
   * </p>
   *
   * @param textFont the text-font value
   * @return this
   */
  public SymbolOptions withTextFont(String[] textFont) {
    this.textFont = textFont;
    return this;
  }

  /**
   * Get the current configured  text-font for the symbol
   * <p>
   * Font stack to use for displaying text.
   * </p>
   *
   * @return textFont value
   */
  public String[] getTextFont() {
    return textFont;
  }

  /**
   * Set text-size to initialise the symbol with.
   * <p>
   * Font size.
   * </p>
   *
   * @param textSize the text-size value
   * @return this
   */
  public SymbolOptions withTextSize(Float textSize) {
    this.textSize = textSize;
    return this;
  }

  /**
   * Get the current configured  text-size for the symbol
   * <p>
   * Font size.
   * </p>
   *
   * @return textSize value
   */
  public Float getTextSize() {
    return textSize;
  }

  /**
   * Set text-max-width to initialise the symbol with.
   * <p>
   * The maximum line width for text wrapping.
   * </p>
   *
   * @param textMaxWidth the text-max-width value
   * @return this
   */
  public SymbolOptions withTextMaxWidth(Float textMaxWidth) {
    this.textMaxWidth = textMaxWidth;
    return this;
  }

  /**
   * Get the current configured  text-max-width for the symbol
   * <p>
   * The maximum line width for text wrapping.
   * </p>
   *
   * @return textMaxWidth value
   */
  public Float getTextMaxWidth() {
    return textMaxWidth;
  }

  /**
   * Set text-letter-spacing to initialise the symbol with.
   * <p>
   * Text tracking amount.
   * </p>
   *
   * @param textLetterSpacing the text-letter-spacing value
   * @return this
   */
  public SymbolOptions withTextLetterSpacing(Float textLetterSpacing) {
    this.textLetterSpacing = textLetterSpacing;
    return this;
  }

  /**
   * Get the current configured  text-letter-spacing for the symbol
   * <p>
   * Text tracking amount.
   * </p>
   *
   * @return textLetterSpacing value
   */
  public Float getTextLetterSpacing() {
    return textLetterSpacing;
  }

  /**
   * Set text-justify to initialise the symbol with.
   * <p>
   * Text justification options.
   * </p>
   *
   * @param textJustify the text-justify value
   * @return this
   */
  public SymbolOptions withTextJustify(@Property.TEXT_JUSTIFY String textJustify) {
    this.textJustify = textJustify;
    return this;
  }

  /**
   * Get the current configured  text-justify for the symbol
   * <p>
   * Text justification options.
   * </p>
   *
   * @return textJustify value
   */
  public String getTextJustify() {
    return textJustify;
  }

  /**
   * Set text-radial-offset to initialise the symbol with.
   * <p>
   * Radial offset of text, in the direction of the symbol's anchor. Useful in combination with {@link PropertyFactory#textVariableAnchor}, which doesn't support the two-dimensional {@link PropertyFactory#textOffset}.
   * </p>
   *
   * @param textRadialOffset the text-radial-offset value
   * @return this
   */
  public SymbolOptions withTextRadialOffset(Float textRadialOffset) {
    this.textRadialOffset = textRadialOffset;
    return this;
  }

  /**
   * Get the current configured  text-radial-offset for the symbol
   * <p>
   * Radial offset of text, in the direction of the symbol's anchor. Useful in combination with {@link PropertyFactory#textVariableAnchor}, which doesn't support the two-dimensional {@link PropertyFactory#textOffset}.
   * </p>
   *
   * @return textRadialOffset value
   */
  public Float getTextRadialOffset() {
    return textRadialOffset;
  }

  /**
   * Set text-anchor to initialise the symbol with.
   * <p>
   * Part of the text placed closest to the anchor.
   * </p>
   *
   * @param textAnchor the text-anchor value
   * @return this
   */
  public SymbolOptions withTextAnchor(@Property.TEXT_ANCHOR String textAnchor) {
    this.textAnchor = textAnchor;
    return this;
  }

  /**
   * Get the current configured  text-anchor for the symbol
   * <p>
   * Part of the text placed closest to the anchor.
   * </p>
   *
   * @return textAnchor value
   */
  public String getTextAnchor() {
    return textAnchor;
  }

  /**
   * Set text-rotate to initialise the symbol with.
   * <p>
   * Rotates the text clockwise.
   * </p>
   *
   * @param textRotate the text-rotate value
   * @return this
   */
  public SymbolOptions withTextRotate(Float textRotate) {
    this.textRotate = textRotate;
    return this;
  }

  /**
   * Get the current configured  text-rotate for the symbol
   * <p>
   * Rotates the text clockwise.
   * </p>
   *
   * @return textRotate value
   */
  public Float getTextRotate() {
    return textRotate;
  }

  /**
   * Set text-transform to initialise the symbol with.
   * <p>
   * Specifies how to capitalize text, similar to the CSS {@link PropertyFactory#textTransform} property.
   * </p>
   *
   * @param textTransform the text-transform value
   * @return this
   */
  public SymbolOptions withTextTransform(@Property.TEXT_TRANSFORM String textTransform) {
    this.textTransform = textTransform;
    return this;
  }

  /**
   * Get the current configured  text-transform for the symbol
   * <p>
   * Specifies how to capitalize text, similar to the CSS {@link PropertyFactory#textTransform} property.
   * </p>
   *
   * @return textTransform value
   */
  public String getTextTransform() {
    return textTransform;
  }

  /**
   * Set text-offset to initialise the symbol with.
   * <p>
   * Offset distance of text from its anchor. Positive values indicate right and down, while negative values indicate left and up.
   * </p>
   *
   * @param textOffset the text-offset value
   * @return this
   */
  public SymbolOptions withTextOffset(Float[] textOffset) {
    this.textOffset = textOffset;
    return this;
  }

  /**
   * Get the current configured  text-offset for the symbol
   * <p>
   * Offset distance of text from its anchor. Positive values indicate right and down, while negative values indicate left and up.
   * </p>
   *
   * @return textOffset value
   */
  public Float[] getTextOffset() {
    return textOffset;
  }

  /**
   * Set icon-opacity to initialise the symbol with.
   * <p>
   * The opacity at which the icon will be drawn.
   * </p>
   *
   * @param iconOpacity the icon-opacity value
   * @return this
   */
  public SymbolOptions withIconOpacity(Float iconOpacity) {
    this.iconOpacity = iconOpacity;
    return this;
  }

  /**
   * Get the current configured  icon-opacity for the symbol
   * <p>
   * The opacity at which the icon will be drawn.
   * </p>
   *
   * @return iconOpacity value
   */
  public Float getIconOpacity() {
    return iconOpacity;
  }

  /**
   * Set icon-color to initialise the symbol with.
   * <p>
   * The color of the icon. This can only be used with sdf icons.
   * </p>
   *
   * @param iconColor the icon-color value
   * @return this
   */
  public SymbolOptions withIconColor(String iconColor) {
    this.iconColor = iconColor;
    return this;
  }

  /**
   * Get the current configured  icon-color for the symbol
   * <p>
   * The color of the icon. This can only be used with sdf icons.
   * </p>
   *
   * @return iconColor value
   */
  public String getIconColor() {
    return iconColor;
  }

  /**
   * Set icon-halo-color to initialise the symbol with.
   * <p>
   * The color of the icon's halo. Icon halos can only be used with SDF icons.
   * </p>
   *
   * @param iconHaloColor the icon-halo-color value
   * @return this
   */
  public SymbolOptions withIconHaloColor(String iconHaloColor) {
    this.iconHaloColor = iconHaloColor;
    return this;
  }

  /**
   * Get the current configured  icon-halo-color for the symbol
   * <p>
   * The color of the icon's halo. Icon halos can only be used with SDF icons.
   * </p>
   *
   * @return iconHaloColor value
   */
  public String getIconHaloColor() {
    return iconHaloColor;
  }

  /**
   * Set icon-halo-width to initialise the symbol with.
   * <p>
   * Distance of halo to the icon outline.
   * </p>
   *
   * @param iconHaloWidth the icon-halo-width value
   * @return this
   */
  public SymbolOptions withIconHaloWidth(Float iconHaloWidth) {
    this.iconHaloWidth = iconHaloWidth;
    return this;
  }

  /**
   * Get the current configured  icon-halo-width for the symbol
   * <p>
   * Distance of halo to the icon outline.
   * </p>
   *
   * @return iconHaloWidth value
   */
  public Float getIconHaloWidth() {
    return iconHaloWidth;
  }

  /**
   * Set icon-halo-blur to initialise the symbol with.
   * <p>
   * Fade out the halo towards the outside.
   * </p>
   *
   * @param iconHaloBlur the icon-halo-blur value
   * @return this
   */
  public SymbolOptions withIconHaloBlur(Float iconHaloBlur) {
    this.iconHaloBlur = iconHaloBlur;
    return this;
  }

  /**
   * Get the current configured  icon-halo-blur for the symbol
   * <p>
   * Fade out the halo towards the outside.
   * </p>
   *
   * @return iconHaloBlur value
   */
  public Float getIconHaloBlur() {
    return iconHaloBlur;
  }

  /**
   * Set text-opacity to initialise the symbol with.
   * <p>
   * The opacity at which the text will be drawn.
   * </p>
   *
   * @param textOpacity the text-opacity value
   * @return this
   */
  public SymbolOptions withTextOpacity(Float textOpacity) {
    this.textOpacity = textOpacity;
    return this;
  }

  /**
   * Get the current configured  text-opacity for the symbol
   * <p>
   * The opacity at which the text will be drawn.
   * </p>
   *
   * @return textOpacity value
   */
  public Float getTextOpacity() {
    return textOpacity;
  }

  /**
   * Set text-color to initialise the symbol with.
   * <p>
   * The color with which the text will be drawn.
   * </p>
   *
   * @param textColor the text-color value
   * @return this
   */
  public SymbolOptions withTextColor(String textColor) {
    this.textColor = textColor;
    return this;
  }

  /**
   * Get the current configured  text-color for the symbol
   * <p>
   * The color with which the text will be drawn.
   * </p>
   *
   * @return textColor value
   */
  public String getTextColor() {
    return textColor;
  }

  /**
   * Set text-halo-color to initialise the symbol with.
   * <p>
   * The color of the text's halo, which helps it stand out from backgrounds.
   * </p>
   *
   * @param textHaloColor the text-halo-color value
   * @return this
   */
  public SymbolOptions withTextHaloColor(String textHaloColor) {
    this.textHaloColor = textHaloColor;
    return this;
  }

  /**
   * Get the current configured  text-halo-color for the symbol
   * <p>
   * The color of the text's halo, which helps it stand out from backgrounds.
   * </p>
   *
   * @return textHaloColor value
   */
  public String getTextHaloColor() {
    return textHaloColor;
  }

  /**
   * Set text-halo-width to initialise the symbol with.
   * <p>
   * Distance of halo to the font outline. Max text halo width is 1/4 of the font-size.
   * </p>
   *
   * @param textHaloWidth the text-halo-width value
   * @return this
   */
  public SymbolOptions withTextHaloWidth(Float textHaloWidth) {
    this.textHaloWidth = textHaloWidth;
    return this;
  }

  /**
   * Get the current configured  text-halo-width for the symbol
   * <p>
   * Distance of halo to the font outline. Max text halo width is 1/4 of the font-size.
   * </p>
   *
   * @return textHaloWidth value
   */
  public Float getTextHaloWidth() {
    return textHaloWidth;
  }

  /**
   * Set text-halo-blur to initialise the symbol with.
   * <p>
   * The halo's fadeout distance towards the outside.
   * </p>
   *
   * @param textHaloBlur the text-halo-blur value
   * @return this
   */
  public SymbolOptions withTextHaloBlur(Float textHaloBlur) {
    this.textHaloBlur = textHaloBlur;
    return this;
  }

  /**
   * Get the current configured  text-halo-blur for the symbol
   * <p>
   * The halo's fadeout distance towards the outside.
   * </p>
   *
   * @return textHaloBlur value
   */
  public Float getTextHaloBlur() {
    return textHaloBlur;
  }

  /**
   * Set the LatLng of the symbol, which represents the location of the symbol on the map
   *
   * @param latLng the location of the symbol in a longitude and latitude pair
   * @return this
   */
  public SymbolOptions withLatLng(LatLng latLng) {
    geometry = Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude());
    return this;
  }

  /**
   * Get the LatLng of the symbol, which represents the location of the symbol on the map
   *
   * @return the location of the symbol in a longitude and latitude pair
   */
  public LatLng getLatLng() {
    if (geometry == null) {
      return null;
    }
    return new LatLng(geometry.latitude(), geometry.longitude());
  }

  /**
   * Set the geometry of the symbol, which represents the location of the symbol on the map
   *
   * @param geometry the location of the symbol
   * @return this
   */
  public SymbolOptions withGeometry(Point geometry) {
    this.geometry = geometry;
    return this;
  }

  /**
   * Get the geometry of the symbol, which represents the location of the symbol on the map
   *
   * @return the location of the symbol
   */
  public Point getGeometry() {
    return geometry;
  }

  /**
   * Returns whether this symbol is draggable, meaning it can be dragged across the screen when touched and moved.
   *
   * @return draggable when touched
   */
  public boolean getDraggable() {
    return isDraggable;
  }

  /**
   * Set whether this symbol should be draggable,
   * meaning it can be dragged across the screen when touched and moved.
   *
   * @param draggable should be draggable
   */
  public SymbolOptions withDraggable(boolean draggable) {
    isDraggable = draggable;
    return this;
  }

  /**
   * Set the arbitrary json data of the annotation.
   *
   * @param jsonElement the arbitrary json element data
   */
  public SymbolOptions withData(@Nullable JsonElement jsonElement) {
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
  Symbol build(long id, AnnotationManager<?, Symbol, ?, ?, ?, ?> annotationManager) {
    if (geometry == null) {
      throw new RuntimeException("geometry field is required");
    }
    JsonObject jsonObject = new JsonObject();
    jsonObject.addProperty(PROPERTY_SYMBOL_SORT_KEY, symbolSortKey);
    jsonObject.addProperty(PROPERTY_ICON_SIZE, iconSize);
    jsonObject.addProperty(PROPERTY_ICON_IMAGE, iconImage);
    jsonObject.addProperty(PROPERTY_ICON_ROTATE, iconRotate);
    jsonObject.add(PROPERTY_ICON_OFFSET, convertArray(iconOffset));
    jsonObject.addProperty(PROPERTY_ICON_ANCHOR, iconAnchor);
    jsonObject.addProperty(PROPERTY_TEXT_FIELD, textField);
    jsonObject.add(PROPERTY_TEXT_FONT, convertArray(textFont));
    jsonObject.addProperty(PROPERTY_TEXT_SIZE, textSize);
    jsonObject.addProperty(PROPERTY_TEXT_MAX_WIDTH, textMaxWidth);
    jsonObject.addProperty(PROPERTY_TEXT_LETTER_SPACING, textLetterSpacing);
    jsonObject.addProperty(PROPERTY_TEXT_JUSTIFY, textJustify);
    jsonObject.addProperty(PROPERTY_TEXT_RADIAL_OFFSET, textRadialOffset);
    jsonObject.addProperty(PROPERTY_TEXT_ANCHOR, textAnchor);
    jsonObject.addProperty(PROPERTY_TEXT_ROTATE, textRotate);
    jsonObject.addProperty(PROPERTY_TEXT_TRANSFORM, textTransform);
    jsonObject.add(PROPERTY_TEXT_OFFSET, convertArray(textOffset));
    jsonObject.addProperty(PROPERTY_ICON_OPACITY, iconOpacity);
    jsonObject.addProperty(PROPERTY_ICON_COLOR, iconColor);
    jsonObject.addProperty(PROPERTY_ICON_HALO_COLOR, iconHaloColor);
    jsonObject.addProperty(PROPERTY_ICON_HALO_WIDTH, iconHaloWidth);
    jsonObject.addProperty(PROPERTY_ICON_HALO_BLUR, iconHaloBlur);
    jsonObject.addProperty(PROPERTY_TEXT_OPACITY, textOpacity);
    jsonObject.addProperty(PROPERTY_TEXT_COLOR, textColor);
    jsonObject.addProperty(PROPERTY_TEXT_HALO_COLOR, textHaloColor);
    jsonObject.addProperty(PROPERTY_TEXT_HALO_WIDTH, textHaloWidth);
    jsonObject.addProperty(PROPERTY_TEXT_HALO_BLUR, textHaloBlur);
    Symbol symbol = new Symbol(id, annotationManager, jsonObject, geometry);
    symbol.setDraggable(isDraggable);
    symbol.setData(data);
    return symbol;
  }

  /**
   * Creates SymbolOptions out of a Feature.
   *
   * @param feature feature to be converted
   */
  @Nullable
  static SymbolOptions fromFeature(@NonNull Feature feature) {
    if (feature.geometry() == null) {
      throw new RuntimeException("geometry field is required");
    }
    if (!(feature.geometry() instanceof Point)) {
      return null;
    }

    SymbolOptions options = new SymbolOptions();
    options.geometry = (Point) feature.geometry();
    if (feature.hasProperty(PROPERTY_SYMBOL_SORT_KEY)) {
      options.symbolSortKey = feature.getProperty(PROPERTY_SYMBOL_SORT_KEY).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_ICON_SIZE)) {
      options.iconSize = feature.getProperty(PROPERTY_ICON_SIZE).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_ICON_IMAGE)) {
      options.iconImage = feature.getProperty(PROPERTY_ICON_IMAGE).getAsString();
    }
    if (feature.hasProperty(PROPERTY_ICON_ROTATE)) {
      options.iconRotate = feature.getProperty(PROPERTY_ICON_ROTATE).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_ICON_OFFSET)) {
      options.iconOffset = toFloatArray(feature.getProperty(PROPERTY_ICON_OFFSET).getAsJsonArray());
    }
    if (feature.hasProperty(PROPERTY_ICON_ANCHOR)) {
      options.iconAnchor = feature.getProperty(PROPERTY_ICON_ANCHOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_FIELD)) {
      options.textField = feature.getProperty(PROPERTY_TEXT_FIELD).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_FONT)) {
      options.textFont = toStringArray(feature.getProperty(PROPERTY_TEXT_FONT).getAsJsonArray());
    }
    if (feature.hasProperty(PROPERTY_TEXT_SIZE)) {
      options.textSize = feature.getProperty(PROPERTY_TEXT_SIZE).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_MAX_WIDTH)) {
      options.textMaxWidth = feature.getProperty(PROPERTY_TEXT_MAX_WIDTH).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_LETTER_SPACING)) {
      options.textLetterSpacing = feature.getProperty(PROPERTY_TEXT_LETTER_SPACING).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_JUSTIFY)) {
      options.textJustify = feature.getProperty(PROPERTY_TEXT_JUSTIFY).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_RADIAL_OFFSET)) {
      options.textRadialOffset = feature.getProperty(PROPERTY_TEXT_RADIAL_OFFSET).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_ANCHOR)) {
      options.textAnchor = feature.getProperty(PROPERTY_TEXT_ANCHOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_ROTATE)) {
      options.textRotate = feature.getProperty(PROPERTY_TEXT_ROTATE).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_TRANSFORM)) {
      options.textTransform = feature.getProperty(PROPERTY_TEXT_TRANSFORM).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_OFFSET)) {
      options.textOffset = toFloatArray(feature.getProperty(PROPERTY_TEXT_OFFSET).getAsJsonArray());
    }
    if (feature.hasProperty(PROPERTY_ICON_OPACITY)) {
      options.iconOpacity = feature.getProperty(PROPERTY_ICON_OPACITY).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_ICON_COLOR)) {
      options.iconColor = feature.getProperty(PROPERTY_ICON_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_ICON_HALO_COLOR)) {
      options.iconHaloColor = feature.getProperty(PROPERTY_ICON_HALO_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_ICON_HALO_WIDTH)) {
      options.iconHaloWidth = feature.getProperty(PROPERTY_ICON_HALO_WIDTH).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_ICON_HALO_BLUR)) {
      options.iconHaloBlur = feature.getProperty(PROPERTY_ICON_HALO_BLUR).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_OPACITY)) {
      options.textOpacity = feature.getProperty(PROPERTY_TEXT_OPACITY).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_COLOR)) {
      options.textColor = feature.getProperty(PROPERTY_TEXT_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_HALO_COLOR)) {
      options.textHaloColor = feature.getProperty(PROPERTY_TEXT_HALO_COLOR).getAsString();
    }
    if (feature.hasProperty(PROPERTY_TEXT_HALO_WIDTH)) {
      options.textHaloWidth = feature.getProperty(PROPERTY_TEXT_HALO_WIDTH).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_TEXT_HALO_BLUR)) {
      options.textHaloBlur = feature.getProperty(PROPERTY_TEXT_HALO_BLUR).getAsFloat();
    }
    if (feature.hasProperty(PROPERTY_IS_DRAGGABLE)) {
      options.isDraggable = feature.getProperty(PROPERTY_IS_DRAGGABLE).getAsBoolean();
    }
    return options;
  }
}
