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
import org.maplibre.android.style.layers.FillLayer;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyFactory;
import org.maplibre.android.style.layers.PropertyValue;
import org.maplibre.android.style.sources.GeoJsonOptions;

import java.util.ArrayList;
import java.util.List;

/**
 * The fill manager allows to add fills to a map.
 */
public class FillManager extends AnnotationManager<FillLayer, Fill, FillOptions, OnFillDragListener, OnFillClickListener, OnFillLongClickListener> {

    private static final String PROPERTY_FILL_ANTIALIAS = "fill-antialias";
    private static final String PROPERTY_FILL_TRANSLATE = "fill-translate";
    private static final String PROPERTY_FILL_TRANSLATE_ANCHOR = "fill-translate-anchor";

    /**
     * Create a fill manager, used to manage fills.
     *
     * @param maplibreMap the map object to add fills to
     * @param style     a valid a fully loaded style object
     */
    @UiThread
    public FillManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style) {
        this(mapView, maplibreMap, style, null, null, (GeoJsonOptions) null);
    }

    /**
     * Create a fill manager, used to manage fills.
     *
     * @param maplibreMap    the map object to add fills to
     * @param style        a valid a fully loaded style object
     * @param belowLayerId the id of the layer above the fill layer
     * @param aboveLayerId the id of the layer below the fill layer
     */
    @UiThread
    public FillManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @Nullable String belowLayerId, @Nullable String aboveLayerId) {
        this(mapView, maplibreMap, style, belowLayerId, aboveLayerId, (GeoJsonOptions) null);
    }

    /**
     * Create a fill manager, used to manage fills.
     *
     * @param maplibreMap      the map object to add fills to
     * @param style          a valid a fully loaded style object
     * @param belowLayerId   the id of the layer above the fill layer
     * @param aboveLayerId   the id of the layer below the fill layer
     * @param geoJsonOptions options for the internal source
     */
    @UiThread
    public FillManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @Nullable String belowLayerId, @Nullable String aboveLayerId, @Nullable GeoJsonOptions geoJsonOptions) {
        this(mapView, maplibreMap, style, new FillElementProvider(), belowLayerId, aboveLayerId, geoJsonOptions, DraggableAnnotationController.getInstance(mapView, maplibreMap));
    }

    @UiThread
    FillManager(@NonNull MapView mapView, @NonNull MapLibreMap maplibreMap, @NonNull Style style, @NonNull CoreElementProvider<FillLayer> coreElementProvider, @Nullable String belowLayerId, @Nullable String aboveLayerId, @Nullable GeoJsonOptions geoJsonOptions, DraggableAnnotationController draggableAnnotationController) {
        super(mapView, maplibreMap, style, coreElementProvider, draggableAnnotationController, belowLayerId, aboveLayerId, geoJsonOptions);
    }

    @Override
    void initializeDataDrivenPropertyMap() {
        dataDrivenPropertyUsageMap.put(FillOptions.PROPERTY_FILL_OPACITY, false);
        dataDrivenPropertyUsageMap.put(FillOptions.PROPERTY_FILL_COLOR, false);
        dataDrivenPropertyUsageMap.put(FillOptions.PROPERTY_FILL_OUTLINE_COLOR, false);
        dataDrivenPropertyUsageMap.put(FillOptions.PROPERTY_FILL_PATTERN, false);
    }

    @Override
    protected void setDataDrivenPropertyIsUsed(@NonNull String property) {
        switch (property) {
            case FillOptions.PROPERTY_FILL_OPACITY:
                layer.setProperties(fillOpacity(get(FillOptions.PROPERTY_FILL_OPACITY)));
                break;
            case FillOptions.PROPERTY_FILL_COLOR:
                layer.setProperties(fillColor(get(FillOptions.PROPERTY_FILL_COLOR)));
                break;
            case FillOptions.PROPERTY_FILL_OUTLINE_COLOR:
                layer.setProperties(fillOutlineColor(get(FillOptions.PROPERTY_FILL_OUTLINE_COLOR)));
                break;
            case FillOptions.PROPERTY_FILL_PATTERN:
                layer.setProperties(fillPattern(get(FillOptions.PROPERTY_FILL_PATTERN)));
                break;
        }
    }

    /**
     * Create a list of fills on the map.
     * <p>
     * Fills are going to be created only for features with a matching geometry.
     * <p>
     * All supported properties are:<br>
     * FillOptions.PROPERTY_FILL_OPACITY - Float<br>
     * FillOptions.PROPERTY_FILL_COLOR - String<br>
     * FillOptions.PROPERTY_FILL_OUTLINE_COLOR - String<br>
     * FillOptions.PROPERTY_FILL_PATTERN - String<br>
     * Learn more about above properties in the <a href="https://www.mapbox.com/mapbox-gl-js/style-spec/">Style specification</a>.
     * <p>
     * Out of spec properties:<br>
     * "is-draggable" - Boolean, true if the fill should be draggable, false otherwise
     *
     * @param json the GeoJSON defining the list of fills to build
     * @return the list of built fills
     */
    @UiThread
    public List<Fill> create(@NonNull String json) {
        return create(FeatureCollection.fromJson(json));
    }

    /**
     * Create a list of fills on the map.
     * <p>
     * Fills are going to be created only for features with a matching geometry.
     * <p>
     * All supported properties are:<br>
     * FillOptions.PROPERTY_FILL_OPACITY - Float<br>
     * FillOptions.PROPERTY_FILL_COLOR - String<br>
     * FillOptions.PROPERTY_FILL_OUTLINE_COLOR - String<br>
     * FillOptions.PROPERTY_FILL_PATTERN - String<br>
     * Learn more about above properties in the <a href="https://www.mapbox.com/mapbox-gl-js/style-spec/">Style specification</a>.
     * <p>
     * Out of spec properties:<br>
     * "is-draggable" - Boolean, true if the fill should be draggable, false otherwise
     *
     * @param featureCollection the featureCollection defining the list of fills to build
     * @return the list of built fills
     */
    @UiThread
    public List<Fill> create(@NonNull FeatureCollection featureCollection) {
        List<Feature> features = featureCollection.features();
        List<FillOptions> options = new ArrayList<>();
        if (features != null) {
            for (Feature feature : features) {
                FillOptions option = FillOptions.fromFeature(feature);
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
        return Fill.ID_KEY;
    }

    // Property accessors

    /**
     * Get the FillAntialias property
     * <p>
     * Whether or not the fill should be antialiased.
     * </p>
     *
     * @return property wrapper value around Boolean
     */
    public Boolean getFillAntialias() {
        return layer.getFillAntialias().value;
    }

    /**
     * Set the FillAntialias property
     * <p>
     * Whether or not the fill should be antialiased.
     * </p>
     *
     * @param value property wrapper value around Boolean
     */
    public void setFillAntialias(Boolean value) {
        PropertyValue propertyValue = fillAntialias(value);
        constantPropertyUsageMap.put(PROPERTY_FILL_ANTIALIAS, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the FillTranslate property
     * <p>
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     * </p>
     *
     * @return property wrapper value around Float[]
     */
    public Float[] getFillTranslate() {
        return layer.getFillTranslate().value;
    }

    /**
     * Set the FillTranslate property
     * <p>
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     * </p>
     *
     * @param value property wrapper value around Float[]
     */
    public void setFillTranslate(Float[] value) {
        PropertyValue propertyValue = fillTranslate(value);
        constantPropertyUsageMap.put(PROPERTY_FILL_TRANSLATE, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Get the FillTranslateAnchor property
     * <p>
     * Controls the frame of reference for {@link PropertyFactory#fillTranslate}.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getFillTranslateAnchor() {
        return layer.getFillTranslateAnchor().value;
    }

    /**
     * Set the FillTranslateAnchor property
     * <p>
     * Controls the frame of reference for {@link PropertyFactory#fillTranslate}.
     * </p>
     *
     * @param value property wrapper value around String
     */
    public void setFillTranslateAnchor(@Property.FILL_TRANSLATE_ANCHOR String value) {
        PropertyValue propertyValue = fillTranslateAnchor(value);
        constantPropertyUsageMap.put(PROPERTY_FILL_TRANSLATE_ANCHOR, propertyValue);
        layer.setProperties(propertyValue);
    }

    /**
     * Set filter on the managed fills.
     *
     * @param expression expression
     */
    @Override
    public void setFilter(@NonNull Expression expression) {
        layerFilter = expression;
        layer.setFilter(layerFilter);
    }

    /**
     * Get filter of the managed fills.
     *
     * @return expression
     */
    @Nullable
    public Expression getFilter() {
        return layer.getFilter();
    }
}
