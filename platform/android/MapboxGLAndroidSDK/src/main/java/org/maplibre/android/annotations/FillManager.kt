package org.maplibre.android.annotations

import androidx.annotation.UiThread
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.style.sources.GeoJsonOptions

/**
 * The fill manager allows to add fills to a map.
 */
class FillManager @UiThread internal constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap,
    style: Style,
    coreElementProvider: CoreElementProvider<FillLayer>,
    belowLayerId: String?,
    aboveLayerId: String?,
    geoJsonOptions: GeoJsonOptions?,
    draggableAnnotationController: DraggableAnnotationController
) : AnnotationManager<FillLayer, Fill, FillOptions, OnFillDragListener, OnFillClickListener, OnFillLongClickListener>(
    mapView,
    maplibreMap,
    style,
    coreElementProvider,
    draggableAnnotationController,
    belowLayerId,
    aboveLayerId,
    geoJsonOptions
) {
    /**
     * Create a fill manager, used to manage fills.
     *
     * @param maplibreMap the map object to add fills to
     * @param style     a valid a fully loaded style object
     */
    @UiThread
    constructor(mapView: MapView, maplibreMap: MapLibreMap, style: Style) : this(
        mapView,
        maplibreMap,
        style,
        null,
        null,
        null as GeoJsonOptions?
    )

    /**
     * Create a fill manager, used to manage fills.
     *
     * @param maplibreMap    the map object to add fills to
     * @param style        a valid a fully loaded style object
     * @param belowLayerId the id of the layer above the fill layer
     * @param aboveLayerId the id of the layer below the fill layer
     */
    @UiThread
    constructor(
        mapView: MapView,
        maplibreMap: MapLibreMap,
        style: Style,
        belowLayerId: String?,
        aboveLayerId: String?
    ) : this(mapView, maplibreMap, style, belowLayerId, aboveLayerId, null as GeoJsonOptions?)

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
    constructor(
        mapView: MapView,
        maplibreMap: MapLibreMap,
        style: Style,
        belowLayerId: String?,
        aboveLayerId: String?,
        geoJsonOptions: GeoJsonOptions?
    ) : this(
        mapView,
        maplibreMap,
        style,
        FillElementProvider(),
        belowLayerId,
        aboveLayerId,
        geoJsonOptions,
        DraggableAnnotationController.getInstance(mapView, maplibreMap)
    )

    public override fun initializeDataDrivenPropertyMap() {
        dataDrivenPropertyUsageMap[FillOptions.PROPERTY_FILL_OPACITY] = false
        dataDrivenPropertyUsageMap[FillOptions.PROPERTY_FILL_COLOR] = false
        dataDrivenPropertyUsageMap[FillOptions.PROPERTY_FILL_OUTLINE_COLOR] = false
        dataDrivenPropertyUsageMap[FillOptions.PROPERTY_FILL_PATTERN] = false
    }

    override fun setDataDrivenPropertyIsUsed(property: String) {
        when (property) {
            FillOptions.PROPERTY_FILL_OPACITY -> layer.setProperties(
                PropertyFactory.fillOpacity(
                    Expression.get(FillOptions.PROPERTY_FILL_OPACITY)
                )
            )

            FillOptions.PROPERTY_FILL_COLOR -> layer.setProperties(
                PropertyFactory.fillColor(
                    Expression.get(FillOptions.PROPERTY_FILL_COLOR)
                )
            )

            FillOptions.PROPERTY_FILL_OUTLINE_COLOR -> layer.setProperties(
                PropertyFactory.fillOutlineColor(
                    Expression.get(FillOptions.PROPERTY_FILL_OUTLINE_COLOR)
                )
            )

            FillOptions.PROPERTY_FILL_PATTERN -> layer.setProperties(
                PropertyFactory.fillPattern(
                    Expression.get(FillOptions.PROPERTY_FILL_PATTERN)
                )
            )
        }
    }

    /**
     * Create a list of fills on the map.
     *
     *
     * Fills are going to be created only for features with a matching geometry.
     *
     *
     * All supported properties are:<br></br>
     * FillOptions.PROPERTY_FILL_OPACITY - Float<br></br>
     * FillOptions.PROPERTY_FILL_COLOR - String<br></br>
     * FillOptions.PROPERTY_FILL_OUTLINE_COLOR - String<br></br>
     * FillOptions.PROPERTY_FILL_PATTERN - String<br></br>
     * Learn more about above properties in the [Style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/).
     *
     *
     * Out of spec properties:<br></br>
     * "is-draggable" - Boolean, true if the fill should be draggable, false otherwise
     *
     * @param json the GeoJSON defining the list of fills to build
     * @return the list of built fills
     */
    @UiThread
    fun create(json: String): List<Fill?>? {
        return create(FeatureCollection.fromJson(json))
    }

    /**
     * Create a list of fills on the map.
     *
     *
     * Fills are going to be created only for features with a matching geometry.
     *
     *
     * All supported properties are:<br></br>
     * FillOptions.PROPERTY_FILL_OPACITY - Float<br></br>
     * FillOptions.PROPERTY_FILL_COLOR - String<br></br>
     * FillOptions.PROPERTY_FILL_OUTLINE_COLOR - String<br></br>
     * FillOptions.PROPERTY_FILL_PATTERN - String<br></br>
     * Learn more about above properties in the [Style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/).
     *
     *
     * Out of spec properties:<br></br>
     * "is-draggable" - Boolean, true if the fill should be draggable, false otherwise
     *
     * @param featureCollection the featureCollection defining the list of fills to build
     * @return the list of built fills
     */
    @UiThread
    fun create(featureCollection: FeatureCollection): List<Fill?>? =
        featureCollection.features()?.mapNotNull { FillOptions.fromFeature(it) }
            .let { create(it ?: emptyList()) }

    /**
     * Key of the id of the annotation
     */
    override val annotationIdKey: String
        /**
         * Get the  of the annotation.
         *
         * @return the key of the id of the annotation
         */
        get() = Fill.ID_KEY

    // Property accessors
    /**
     * Whether or not the fill should be antialiased.
     */
    var fillAntialias: Boolean?
        get() = layer.fillAntialias.value

        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.fillAntialias(value)
            constantPropertyUsageMap[PROPERTY_FILL_ANTIALIAS] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     */
    var fillTranslate: Array<Float?>?
        get() = layer.fillTranslate.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.fillTranslate(value)
            constantPropertyUsageMap[PROPERTY_FILL_TRANSLATE] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Controls the frame of reference for [PropertyFactory.fillTranslate].
     */
    var fillTranslateAnchor: String?
        get() = layer.fillTranslateAnchor.value

        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.fillTranslateAnchor(value)
            constantPropertyUsageMap[PROPERTY_FILL_TRANSLATE_ANCHOR] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Set filter on the managed fills.
     *
     * @param expression expression
     */
    public override fun setFilter(expression: Expression) {
        layerFilter = expression
        layer.setFilter(expression)
    }

    val filter: Expression?
        /**
         * Get filter of the managed fills.
         */
        get() {
            return layer.filter
        }

    companion object {
        private val PROPERTY_FILL_ANTIALIAS: String = "fill-antialias"
        private val PROPERTY_FILL_TRANSLATE: String = "fill-translate"
        private val PROPERTY_FILL_TRANSLATE_ANCHOR: String = "fill-translate-anchor"
    }
}