package org.maplibre.android.annotations;

import static org.maplibre.android.style.expressions.Expression.get;
import static org.maplibre.android.style.layers.PropertyFactory.*;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import com.mapbox.geojson.Feature;
import com.mapbox.geojson.FeatureCollection;
import org.maplibre.android.maps.MapView;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.maps.Style;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyFactory;
import org.maplibre.android.style.layers.PropertyValue;
import org.maplibre.android.style.layers.SymbolLayer;
import org.maplibre.android.style.sources.GeoJsonOptions;

import java.util.ArrayList;
import java.util.List;

/**
 * The symbol manager allows to add symbols to a map.
 */
public class SymbolManager extends AnnotationManager<SymbolLayer, Symbol, SymbolOptions, OnSymbolDragListener, OnSymbolClickListener, OnSymbolLongClickListener> {

    private static final String PROPERTY_SYMBOL_PLACEMENT = "symbol-placement";
    private static final String PROPERTY_SYMBOL_SPACING = "symbol-spacing";
    private static final String PROPERTY_SYMBOL_AVOID_EDGES = "symbol-avoid-edges";
    private static final String PROPERTY_ICON_ALLOW_OVERLAP = "icon-allow-overlap";
    private static final String PROPERTY_ICON_IGNORE_PLACEMENT = "icon-ignore-placement";
    private static final String PROPERTY_ICON_OPTIONAL = "icon-optional";
    private static final String PROPERTY_ICON_ROTATION_ALIGNMENT = "icon-rotation-alignment";
    private static final String PROPERTY_ICON_TEXT_FIT = "icon-text-fit";
    private static final String PROPERTY_ICON_TEXT_FIT_PADDING = "icon-text-fit-padding";
    private static final String PROPERTY_ICON_PADDING = "icon-padding";
    private static final String PROPERTY_ICON_KEEP_UPRIGHT = "icon-keep-upright";
    private static final String PROPERTY_ICON_PITCH_ALIGNMENT = "icon-pitch-alignment";
    private static final String PROPERTY_TEXT_PITCH_ALIGNMENT = "text-pitch-alignment";
    private static final String PROPERTY_TEXT_ROTATION_ALIGNMENT = "text-rotation-alignment";
    private static final String PROPERTY_TEXT_LINE_HEIGHT = "text-line-height";
    private static final String PROPERTY_TEXT_VARIABLE_ANCHOR = "text-variable-anchor";
    private static final String PROPERTY_TEXT_MAX_ANGLE = "text-max-angle";
    private static final String PROPERTY_TEXT_PADDING = "text-padding";
    private static final String PROPERTY_TEXT_KEEP_UPRIGHT = "text-keep-upright";
    private static final String PROPERTY_TEXT_ALLOW_OVERLAP = "text-allow-overlap";
    private static final String PROPERTY_TEXT_IGNORE_PLACEMENT = "text-ignore-placement";
    private static final String PROPERTY_TEXT_OPTIONAL = "text-optional";
    private static final String PROPERTY_ICON_TRANSLATE = "icon-translate";
    private static final String PROPERTY_ICON_TRANSLATE_ANCHOR = "icon-translate-anchor";
    private static final String PROPERTY_TEXT_TRANSLATE = "text-translate";
    private static final String PROPERTY_TEXT_TRANSLATE_ANCHOR = "text-translate-anchor";

    /**
     * Create a symbol manager, used to manage symbols.
     *
     * @param maplibreMap the map object to add symbols to
     * @param style     a valid a fully loaded style object
     */
    @UiThread
    public SymbolManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style) {
        this(mapView, maplibreMap, style, null, null, (GeoJsonOptions) null);
    }

    /**
     * Create a symbol manager, used to manage symbols.
     *
     * @param maplibreMap    the map object to add symbols to
     * @param style        a valid a fully loaded style object
     * @param belowLayerId the id of the layer above the symbol layer
     * @param aboveLayerId the id of the layer below the symbol layer
     */
    @UiThread
    public SymbolManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @Nullable String belowLayerId, @Nullable String aboveLayerId) {
        this(mapView, maplibreMap, style, belowLayerId, aboveLayerId, (GeoJsonOptions) null);
    }

    /**
     * Create a symbol manager, used to manage symbols.
     *
     * @param maplibreMap      the map object to add symbols to
     * @param style          a valid a fully loaded style object
     * @param belowLayerId   the id of the layer above the symbol layer
     * @param aboveLayerId   the id of the layer below the symbol layer
     * @param geoJsonOptions options for the internal source
     */
    @UiThread
    public SymbolManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @Nullable String belowLayerId, @Nullable String aboveLayerId, @Nullable GeoJsonOptions geoJsonOptions) {
        this(mapView, maplibreMap, style, new SymbolElementProvider(), belowLayerId, aboveLayerId, geoJsonOptions, DraggableAnnotationController.getInstance(mapView, maplibreMap));
    }

    /**
     * Create a symbol manager, used to manage symbols.
     *
     * @param maplibreMap      the map object to add symbols to
     * @param style          a valid a fully loaded style object
     * @param belowLayerId   the id of the layer above the symbol layer
     * @param aboveLayerId   the id of the layer below the symbol layer
     * @param clusterOptions options for the clustering configuration
     */
    @UiThread
    public SymbolManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @Nullable String belowLayerId, @Nullable String aboveLayerId, @NonNull ClusterOptions clusterOptions) {
        this(mapView, maplibreMap, style, new SymbolElementProvider(), belowLayerId, aboveLayerId, new GeoJsonOptions().withCluster(true).withClusterRadius(clusterOptions.getClusterRadius()).withClusterMaxZoom(clusterOptions.getClusterMaxZoom()), DraggableAnnotationController.getInstance(mapView, maplibreMap));
        clusterOptions.apply(style, coreElementProvider.getSourceId());
    }

    @UiThread
    SymbolManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @NonNull CoreElementProvider<SymbolLayer> coreElementProvider, @Nullable String belowLayerId, @Nullable String aboveLayerId, @Nullable GeoJsonOptions geoJsonOptions, DraggableAnnotationController draggableAnnotationController) {
        super(mapView, maplibreMap, style, coreElementProvider, draggableAnnotationController, belowLayerId, aboveLayerId, geoJsonOptions);
    }

    @Override
    void initializeDataDrivenPropertyMap() {
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_SIZE, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_IMAGE, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_ROTATE, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_OFFSET, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_ANCHOR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_FIELD, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_FONT, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_SIZE, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_JUSTIFY, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_ANCHOR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_ROTATE, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_TRANSFORM, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_OFFSET, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_OPACITY, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_COLOR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_HALO_COLOR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_HALO_WIDTH, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_ICON_HALO_BLUR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_OPACITY, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_COLOR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_HALO_COLOR, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH, false);
        dataDrivenPropertyUsageMap.put(SymbolOptions.PROPERTY_TEXT_HALO_BLUR, false);
    }

    @Override
    protected void setDataDrivenPropertyIsUsed(@NonNull String property) {
        switch (property) {
            case SymbolOptions.PROPERTY_SYMBOL_SORT_KEY:
                layer.setProperties(symbolSortKey(get(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY)));
                break;
            case SymbolOptions.PROPERTY_ICON_SIZE:
                layer.setProperties(iconSize(get(SymbolOptions.PROPERTY_ICON_SIZE)));
                break;
            case SymbolOptions.PROPERTY_ICON_IMAGE:
                layer.setProperties(iconImage(get(SymbolOptions.PROPERTY_ICON_IMAGE)));
                break;
            case SymbolOptions.PROPERTY_ICON_ROTATE:
                layer.setProperties(iconRotate(get(SymbolOptions.PROPERTY_ICON_ROTATE)));
                break;
            case SymbolOptions.PROPERTY_ICON_OFFSET:
                layer.setProperties(iconOffset(get(SymbolOptions.PROPERTY_ICON_OFFSET)));
                break;
            case SymbolOptions.PROPERTY_ICON_ANCHOR:
                layer.setProperties(iconAnchor(get(SymbolOptions.PROPERTY_ICON_ANCHOR)));
                break;
            case SymbolOptions.PROPERTY_TEXT_FIELD:
                layer.setProperties(textField(get(SymbolOptions.PROPERTY_TEXT_FIELD)));
                break;
            case SymbolOptions.PROPERTY_TEXT_FONT:
                layer.setProperties(textFont(get(SymbolOptions.PROPERTY_TEXT_FONT)));
                break;
            case SymbolOptions.PROPERTY_TEXT_SIZE:
                layer.setProperties(textSize(get(SymbolOptions.PROPERTY_TEXT_SIZE)));
                break;
            case SymbolOptions.PROPERTY_TEXT_MAX_WIDTH:
                layer.setProperties(textMaxWidth(get(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH)));
                break;
            case SymbolOptions.PROPERTY_TEXT_LETTER_SPACING:
                layer.setProperties(textLetterSpacing(get(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING)));
                break;
            case SymbolOptions.PROPERTY_TEXT_JUSTIFY:
                layer.setProperties(textJustify(get(SymbolOptions.PROPERTY_TEXT_JUSTIFY)));
                break;
            case SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET:
                layer.setProperties(textRadialOffset(get(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET)));
                break;
            case SymbolOptions.PROPERTY_TEXT_ANCHOR:
                layer.setProperties(textAnchor(get(SymbolOptions.PROPERTY_TEXT_ANCHOR)));
                break;
            case SymbolOptions.PROPERTY_TEXT_ROTATE:
                layer.setProperties(textRotate(get(SymbolOptions.PROPERTY_TEXT_ROTATE)));
                break;
            case SymbolOptions.PROPERTY_TEXT_TRANSFORM:
                layer.setProperties(textTransform(get(SymbolOptions.PROPERTY_TEXT_TRANSFORM)));
                break;
            case SymbolOptions.PROPERTY_TEXT_OFFSET:
                layer.setProperties(textOffset(get(SymbolOptions.PROPERTY_TEXT_OFFSET)));
                break;
            case SymbolOptions.PROPERTY_ICON_OPACITY:
                layer.setProperties(iconOpacity(get(SymbolOptions.PROPERTY_ICON_OPACITY)));
                break;
            case SymbolOptions.PROPERTY_ICON_COLOR:
                layer.setProperties(iconColor(get(SymbolOptions.PROPERTY_ICON_COLOR)));
                break;
            case SymbolOptions.PROPERTY_ICON_HALO_COLOR:
                layer.setProperties(iconHaloColor(get(SymbolOptions.PROPERTY_ICON_HALO_COLOR)));
                break;
            case SymbolOptions.PROPERTY_ICON_HALO_WIDTH:
                layer.setProperties(iconHaloWidth(get(SymbolOptions.PROPERTY_ICON_HALO_WIDTH)));
                break;
            case SymbolOptions.PROPERTY_ICON_HALO_BLUR:
                layer.setProperties(iconHaloBlur(get(SymbolOptions.PROPERTY_ICON_HALO_BLUR)));
                break;
            case SymbolOptions.PROPERTY_TEXT_OPACITY:
                layer.setProperties(textOpacity(get(SymbolOptions.PROPERTY_TEXT_OPACITY)));
                break;
            case SymbolOptions.PROPERTY_TEXT_COLOR:
                layer.setProperties(textColor(get(SymbolOptions.PROPERTY_TEXT_COLOR)));
                break;
            case SymbolOptions.PROPERTY_TEXT_HALO_COLOR:
                layer.setProperties(textHaloColor(get(SymbolOptions.PROPERTY_TEXT_HALO_COLOR)));
                break;
            case SymbolOptions.PROPERTY_TEXT_HALO_WIDTH:
                layer.setProperties(textHaloWidth(get(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH)));
                break;
            case SymbolOptions.PROPERTY_TEXT_HALO_BLUR:
                layer.setProperties(textHaloBlur(get(SymbolOptions.PROPERTY_TEXT_HALO_BLUR)));
                break;
        }
    }

    /**
     * Create a list of symbols on the map.
     * <p>
     * Symbols are going to be created only for features with a matching geometry.
     * <p>
     * All supported properties are:<br>
     * SymbolOptions.PROPERTY_SYMBOL_SORT_KEY - Float<br>
     * SymbolOptions.PROPERTY_ICON_SIZE - Float<br>
     * SymbolOptions.PROPERTY_ICON_IMAGE - String<br>
     * SymbolOptions.PROPERTY_ICON_ROTATE - Float<br>
     * SymbolOptions.PROPERTY_ICON_OFFSET - Float[]<br>
     * SymbolOptions.PROPERTY_ICON_ANCHOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_FIELD - String<br>
     * SymbolOptions.PROPERTY_TEXT_FONT - String[]<br>
     * SymbolOptions.PROPERTY_TEXT_SIZE - Float<br>
     * SymbolOptions.PROPERTY_TEXT_MAX_WIDTH - Float<br>
     * SymbolOptions.PROPERTY_TEXT_LETTER_SPACING - Float<br>
     * SymbolOptions.PROPERTY_TEXT_JUSTIFY - String<br>
     * SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET - Float<br>
     * SymbolOptions.PROPERTY_TEXT_ANCHOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_ROTATE - Float<br>
     * SymbolOptions.PROPERTY_TEXT_TRANSFORM - String<br>
     * SymbolOptions.PROPERTY_TEXT_OFFSET - Float[]<br>
     * SymbolOptions.PROPERTY_ICON_OPACITY - Float<br>
     * SymbolOptions.PROPERTY_ICON_COLOR - String<br>
     * SymbolOptions.PROPERTY_ICON_HALO_COLOR - String<br>
     * SymbolOptions.PROPERTY_ICON_HALO_WIDTH - Float<br>
     * SymbolOptions.PROPERTY_ICON_HALO_BLUR - Float<br>
     * SymbolOptions.PROPERTY_TEXT_OPACITY - Float<br>
     * SymbolOptions.PROPERTY_TEXT_COLOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_HALO_COLOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_HALO_WIDTH - Float<br>
     * SymbolOptions.PROPERTY_TEXT_HALO_BLUR - Float<br>
     * Learn more about above properties in the <a href="https://www.mapbox.com/mapbox-gl-js/style-spec/">Style specification</a>.
     * <p>
     * Out of spec properties:<br>
     * "is-draggable" - Boolean, true if the symbol should be draggable, false otherwise
     *
     * @param json the GeoJSON defining the list of symbols to build
     * @return the list of built symbols
     */
    @UiThread
    public List<Symbol> create(@NonNull String json) {
        return create(FeatureCollection.fromJson(json));
    }

    /**
     * Create a list of symbols on the map.
     * <p>
     * Symbols are going to be created only for features with a matching geometry.
     * <p>
     * All supported properties are:<br>
     * SymbolOptions.PROPERTY_SYMBOL_SORT_KEY - Float<br>
     * SymbolOptions.PROPERTY_ICON_SIZE - Float<br>
     * SymbolOptions.PROPERTY_ICON_IMAGE - String<br>
     * SymbolOptions.PROPERTY_ICON_ROTATE - Float<br>
     * SymbolOptions.PROPERTY_ICON_OFFSET - Float[]<br>
     * SymbolOptions.PROPERTY_ICON_ANCHOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_FIELD - String<br>
     * SymbolOptions.PROPERTY_TEXT_FONT - String[]<br>
     * SymbolOptions.PROPERTY_TEXT_SIZE - Float<br>
     * SymbolOptions.PROPERTY_TEXT_MAX_WIDTH - Float<br>
     * SymbolOptions.PROPERTY_TEXT_LETTER_SPACING - Float<br>
     * SymbolOptions.PROPERTY_TEXT_JUSTIFY - String<br>
     * SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET - Float<br>
     * SymbolOptions.PROPERTY_TEXT_ANCHOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_ROTATE - Float<br>
     * SymbolOptions.PROPERTY_TEXT_TRANSFORM - String<br>
     * SymbolOptions.PROPERTY_TEXT_OFFSET - Float[]<br>
     * SymbolOptions.PROPERTY_ICON_OPACITY - Float<br>
     * SymbolOptions.PROPERTY_ICON_COLOR - String<br>
     * SymbolOptions.PROPERTY_ICON_HALO_COLOR - String<br>
     * SymbolOptions.PROPERTY_ICON_HALO_WIDTH - Float<br>
     * SymbolOptions.PROPERTY_ICON_HALO_BLUR - Float<br>
     * SymbolOptions.PROPERTY_TEXT_OPACITY - Float<br>
     * SymbolOptions.PROPERTY_TEXT_COLOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_HALO_COLOR - String<br>
     * SymbolOptions.PROPERTY_TEXT_HALO_WIDTH - Float<br>
     * SymbolOptions.PROPERTY_TEXT_HALO_BLUR - Float<br>
     * Learn more about above properties in the <a href="https://www.mapbox.com/mapbox-gl-js/style-spec/">Style specification</a>.
     * <p>
     * Out of spec properties:<br>
     * "is-draggable" - Boolean, true if the symbol should be draggable, false otherwise
     *
     * @param featureCollection the featureCollection defining the list of symbols to build
     * @return the list of built symbols
     */
    @UiThread
    public List<Symbol> create(@NonNull FeatureCollection featureCollection) {
        List<Feature> features = featureCollection.features();
        List<SymbolOptions> options = new ArrayList<>();
        if (features != null) {
            for (Feature feature : features) {
                SymbolOptions option = SymbolOptions.fromFeature(feature);
                if (option != null) {
                    options.add(option);
                }
            }
        }
        return create(options);
    }

    /**
     * Get the key of the id of the annotation.
     *
     * @return the key of the id of the annotation
     */
    @Override
    String getAnnotationIdKey() {
        return Symbol.ID_KEY;
    }

    // Property accessors

    /**
     * Get the SymbolPlacement property
     * <p>
     * Label placement relative to its geometry.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getSymbolPlacement() {
        return layer.getSymbolPlacement().value;
    }

    /**
     * Set the SymbolPlacement property
     * <p>
     * Label placement relative to its geometry.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setSymbolPlacement(@Property.SYMBOL_PLACEMENT String value) {
        PropertyValue propertyValue = symbolPlacement(value);
        constantPropertyUsageMap.put(PROPERTY_SYMBOL_PLACEMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the SymbolSpacing property
     * <p>
     * Distance between two symbol anchors.
     * </p>
     *
     * @return property wrapper value around Float
     */
    public Float getSymbolSpacing() {
        return layer.getSymbolSpacing().value;
    }

    /**
     * Set the SymbolSpacing property
     * <p>
     * Distance between two symbol anchors.
     * </p>
     *
     * @param value property wrapper value around Float
     */
    public void setSymbolSpacing(Float value) {
        PropertyValue propertyValue = symbolSpacing(value);
        constantPropertyUsageMap.put(PROPERTY_SYMBOL_SPACING, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the SymbolAvoidEdges property
     * <p>
     * If true, the symbols will not cross tile edges to avoid mutual collisions. Recommended in layers that don't have enough padding in the vector tile to prevent collisions, or if it is a point symbol layer placed after a line symbol layer.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getSymbolAvoidEdges() {
        return layer.getSymbolAvoidEdges().value;
    }

    /**
     * Set the SymbolAvoidEdges property
     * <p>
     * If true, the symbols will not cross tile edges to avoid mutual collisions. Recommended in layers that don't have enough padding in the vector tile to prevent collisions, or if it is a point symbol layer placed after a line symbol layer.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setSymbolAvoidEdges(Boolean value) {
        PropertyValue propertyValue = symbolAvoidEdges(value);
        constantPropertyUsageMap.put(PROPERTY_SYMBOL_AVOID_EDGES, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconAllowOverlap property
     * <p>
     * If true, the icon will be visible even if it collides with other previously drawn symbols.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getIconAllowOverlap() {
        return layer.getIconAllowOverlap().value;
    }

    /**
     * Set the IconAllowOverlap property
     * <p>
     * If true, the icon will be visible even if it collides with other previously drawn symbols.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setIconAllowOverlap(Boolean value) {
        PropertyValue propertyValue = iconAllowOverlap(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_ALLOW_OVERLAP, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconIgnorePlacement property
     * <p>
     * If true, other symbols can be visible even if they collide with the icon.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getIconIgnorePlacement() {
        return layer.getIconIgnorePlacement().value;
    }

    /**
     * Set the IconIgnorePlacement property
     * <p>
     * If true, other symbols can be visible even if they collide with the icon.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setIconIgnorePlacement(Boolean value) {
        PropertyValue propertyValue = iconIgnorePlacement(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_IGNORE_PLACEMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconOptional property
     * <p>
     * If true, text will display without their corresponding icons when the icon collides with other symbols and the text does not.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getIconOptional() {
        return layer.getIconOptional().value;
    }

    /**
     * Set the IconOptional property
     * <p>
     * If true, text will display without their corresponding icons when the icon collides with other symbols and the text does not.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setIconOptional(Boolean value) {
        PropertyValue propertyValue = iconOptional(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_OPTIONAL, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconRotationAlignment property
     * <p>
     * In combination with {@link Property.SYMBOL_PLACEMENT}, determines the rotation behavior of icons.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getIconRotationAlignment() {
        return layer.getIconRotationAlignment().value;
    }

    /**
     * Set the IconRotationAlignment property
     * <p>
     * In combination with {@link Property.SYMBOL_PLACEMENT}, determines the rotation behavior of icons.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setIconRotationAlignment(@Property.ICON_ROTATION_ALIGNMENT String value) {
        PropertyValue propertyValue = iconRotationAlignment(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_ROTATION_ALIGNMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconTextFit property
     * <p>
     * Scales the icon to fit around the associated text.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getIconTextFit() {
        return layer.getIconTextFit().value;
    }

    /**
     * Set the IconTextFit property
     * <p>
     * Scales the icon to fit around the associated text.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setIconTextFit(@Property.ICON_TEXT_FIT String value) {
        PropertyValue propertyValue = iconTextFit(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_TEXT_FIT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconTextFitPadding property
     * <p>
     * Size of the additional area added to dimensions determined by {@link Property.ICON_TEXT_FIT}, in clockwise order: top, right, bottom, left.
     * </p>
     *
     * @return property wrapper value around Float[]
     */
    public Float[] getIconTextFitPadding() {
        return layer.getIconTextFitPadding().value;
    }

    /**
     * Set the IconTextFitPadding property
     * <p>
     * Size of the additional area added to dimensions determined by {@link Property.ICON_TEXT_FIT}, in clockwise order: top, right, bottom, left.
     * </p>
     *
     * @param value property wrapper value around Float[]
     */
    public void setIconTextFitPadding(Float[] value) {
        PropertyValue propertyValue = iconTextFitPadding(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_TEXT_FIT_PADDING, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconPadding property
     * <p>
     * Size of the additional area around the icon bounding box used for detecting symbol collisions.
     * </p>
     *
     * @return property wrapper value around Float
     */
    public Float getIconPadding() {
        return layer.getIconPadding().value;
    }

    /**
     * Set the IconPadding property
     * <p>
     * Size of the additional area around the icon bounding box used for detecting symbol collisions.
     * </p>
     *
     * @param value property wrapper value around Float
     */
    public void setIconPadding(Float value) {
        PropertyValue propertyValue = iconPadding(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_PADDING, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconKeepUpright property
     * <p>
     * If true, the icon may be flipped to prevent it from being rendered upside-down.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getIconKeepUpright() {
        return layer.getIconKeepUpright().value;
    }

    /**
     * Set the IconKeepUpright property
     * <p>
     * If true, the icon may be flipped to prevent it from being rendered upside-down.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setIconKeepUpright(Boolean value) {
        PropertyValue propertyValue = iconKeepUpright(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_KEEP_UPRIGHT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconPitchAlignment property
     * <p>
     * Orientation of icon when map is pitched.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getIconPitchAlignment() {
        return layer.getIconPitchAlignment().value;
    }

    /**
     * Set the IconPitchAlignment property
     * <p>
     * Orientation of icon when map is pitched.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setIconPitchAlignment(@Property.ICON_PITCH_ALIGNMENT String value) {
        PropertyValue propertyValue = iconPitchAlignment(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_PITCH_ALIGNMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextPitchAlignment property
     * <p>
     * Orientation of text when map is pitched.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getTextPitchAlignment() {
        return layer.getTextPitchAlignment().value;
    }

    /**
     * Set the TextPitchAlignment property
     * <p>
     * Orientation of text when map is pitched.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setTextPitchAlignment(@Property.TEXT_PITCH_ALIGNMENT String value) {
        PropertyValue propertyValue = textPitchAlignment(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_PITCH_ALIGNMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextRotationAlignment property
     * <p>
     * In combination with {@link Property.SYMBOL_PLACEMENT}, determines the rotation behavior of the individual glyphs forming the text.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getTextRotationAlignment() {
        return layer.getTextRotationAlignment().value;
    }

    /**
     * Set the TextRotationAlignment property
     * <p>
     * In combination with {@link Property.SYMBOL_PLACEMENT}, determines the rotation behavior of the individual glyphs forming the text.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setTextRotationAlignment(@Property.TEXT_ROTATION_ALIGNMENT String value) {
        PropertyValue propertyValue = textRotationAlignment(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_ROTATION_ALIGNMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextLineHeight property
     * <p>
     * Text leading value for multi-line text.
     * </p>
     *
     * @return property wrapper value around Float
     */
    public Float getTextLineHeight() {
        return layer.getTextLineHeight().value;
    }

    /**
     * Set the TextLineHeight property
     * <p>
     * Text leading value for multi-line text.
     * </p>
     *
     * @param value property wrapper value around Float
     */
    public void setTextLineHeight(Float value) {
        PropertyValue propertyValue = textLineHeight(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_LINE_HEIGHT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextVariableAnchor property
     * <p>
     * To increase the chance of placing high-priority labels on the map, you can provide an array of {@link Property.TEXT_ANCHOR} locations: the render will attempt to place the label at each location, in order, before moving onto the next label. Use `text-justify: auto` to choose justification based on anchor position. To apply an offset, use the {@link PropertyFactory#textRadialOffset} instead of the two-dimensional {@link PropertyFactory#textOffset}.
     * </p>
     *
     * @return property wrapper value around String[]
     */
    public String[] getTextVariableAnchor() {
        return layer.getTextVariableAnchor().value;
    }

    /**
     * Set the TextVariableAnchor property
     * <p>
     * To increase the chance of placing high-priority labels on the map, you can provide an array of {@link Property.TEXT_ANCHOR} locations: the render will attempt to place the label at each location, in order, before moving onto the next label. Use `text-justify: auto` to choose justification based on anchor position. To apply an offset, use the {@link PropertyFactory#textRadialOffset} instead of the two-dimensional {@link PropertyFactory#textOffset}.
     * </p>
     *
     * @param value property wrapper value around String[]
     */
    public void setTextVariableAnchor(String[] value) {
        PropertyValue propertyValue = textVariableAnchor(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_VARIABLE_ANCHOR, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextMaxAngle property
     * <p>
     * Maximum angle change between adjacent characters.
     * </p>
     *
     * @return property wrapper value around Float
     */
    public Float getTextMaxAngle() {
        return layer.getTextMaxAngle().value;
    }

    /**
     * Set the TextMaxAngle property
     * <p>
     * Maximum angle change between adjacent characters.
     * </p>
     *
     * @param value property wrapper value around Float
     */
    public void setTextMaxAngle(Float value) {
        PropertyValue propertyValue = textMaxAngle(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_MAX_ANGLE, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextPadding property
     * <p>
     * Size of the additional area around the text bounding box used for detecting symbol collisions.
     * </p>
     *
     * @return property wrapper value around Float
     */
    public Float getTextPadding() {
        return layer.getTextPadding().value;
    }

    /**
     * Set the TextPadding property
     * <p>
     * Size of the additional area around the text bounding box used for detecting symbol collisions.
     * </p>
     *
     * @param value property wrapper value around Float
     */
    public void setTextPadding(Float value) {
        PropertyValue propertyValue = textPadding(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_PADDING, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextKeepUpright property
     * <p>
     * If true, the text may be flipped vertically to prevent it from being rendered upside-down.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getTextKeepUpright() {
        return layer.getTextKeepUpright().value;
    }

    /**
     * Set the TextKeepUpright property
     * <p>
     * If true, the text may be flipped vertically to prevent it from being rendered upside-down.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setTextKeepUpright(Boolean value) {
        PropertyValue propertyValue = textKeepUpright(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_KEEP_UPRIGHT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextAllowOverlap property
     * <p>
     * If true, the text will be visible even if it collides with other previously drawn symbols.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getTextAllowOverlap() {
        return layer.getTextAllowOverlap().value;
    }

    /**
     * Set the TextAllowOverlap property
     * <p>
     * If true, the text will be visible even if it collides with other previously drawn symbols.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setTextAllowOverlap(Boolean value) {
        PropertyValue propertyValue = textAllowOverlap(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_ALLOW_OVERLAP, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextIgnorePlacement property
     * <p>
     * If true, other symbols can be visible even if they collide with the text.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getTextIgnorePlacement() {
        return layer.getTextIgnorePlacement().value;
    }

    /**
     * Set the TextIgnorePlacement property
     * <p>
     * If true, other symbols can be visible even if they collide with the text.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setTextIgnorePlacement(Boolean value) {
        PropertyValue propertyValue = textIgnorePlacement(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_IGNORE_PLACEMENT, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextOptional property
     * <p>
     * If true, icons will display without their corresponding text when the text collides with other symbols and the icon does not.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getTextOptional() {
        return layer.getTextOptional().value;
    }

    /**
     * Set the TextOptional property
     * <p>
     * If true, icons will display without their corresponding text when the text collides with other symbols and the icon does not.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setTextOptional(Boolean value) {
        PropertyValue propertyValue = textOptional(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_OPTIONAL, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconTranslate property
     * <p>
     * Distance that the icon's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     * </p>
     *
     * @return property wrapper value around Float[]
     */
    public Float[] getIconTranslate() {
        return layer.getIconTranslate().value;
    }

    /**
     * Set the IconTranslate property
     * <p>
     * Distance that the icon's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     * </p>
     *
     * @param value property wrapper value around Float[]
     */
    public void setIconTranslate(Float[] value) {
        PropertyValue propertyValue = iconTranslate(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_TRANSLATE, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the IconTranslateAnchor property
     * <p>
     * Controls the frame of reference for {@link PropertyFactory#iconTranslate}.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getIconTranslateAnchor() {
        return layer.getIconTranslateAnchor().value;
    }

    /**
     * Set the IconTranslateAnchor property
     * <p>
     * Controls the frame of reference for {@link PropertyFactory#iconTranslate}.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setIconTranslateAnchor(@Property.ICON_TRANSLATE_ANCHOR String value) {
        PropertyValue propertyValue = iconTranslateAnchor(value);
        constantPropertyUsageMap.put(PROPERTY_ICON_TRANSLATE_ANCHOR, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextTranslate property
     * <p>
     * Distance that the text's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     * </p>
     *
     * @return property wrapper value around Float[]
     */
    public Float[] getTextTranslate() {
        return layer.getTextTranslate().value;
    }

    /**
     * Set the TextTranslate property
     * <p>
     * Distance that the text's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     * </p>
     *
     * @param value property wrapper value around Float[]
     */
    public void setTextTranslate(Float[] value) {
        PropertyValue propertyValue = textTranslate(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_TRANSLATE, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the TextTranslateAnchor property
     * <p>
     * Controls the frame of reference for {@link PropertyFactory#textTranslate}.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getTextTranslateAnchor() {
        return layer.getTextTranslateAnchor().value;
    }

    /**
     * Set the TextTranslateAnchor property
     * <p>
     * Controls the frame of reference for {@link PropertyFactory#textTranslate}.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setTextTranslateAnchor(@Property.TEXT_TRANSLATE_ANCHOR String value) {
        PropertyValue propertyValue = textTranslateAnchor(value);
        constantPropertyUsageMap.put(PROPERTY_TEXT_TRANSLATE_ANCHOR, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Set filter on the managed symbols.
     *
     * @param expression expression
     */
    @Override
    public void setFilter(@NonNull Expression expression) {
        layerFilter = expression;
        layer.setFilter(layerFilter);
    }

    /**
     * Get filter of the managed symbols.
     *
     * @return expression
     */
    @Nullable
    public Expression getFilter() {
        return layer.getFilter();
    }
}
