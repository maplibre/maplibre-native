package org.maplibre.android.annotations

import androidx.annotation.UiThread
import com.mapbox.geojson.FeatureCollection
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.Property.ICON_TEXT_FIT
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonOptions

/**
 * The symbol manager allows to add symbols to a map.
 *
 * @param maplibreMap    the map object to add symbols to
 * @param style          a valid a fully loaded style object
 * @param belowLayerId   the id of the layer above the symbol layer
 * @param aboveLayerId   the id of the layer below the symbol layer
 * @param geoJsonOptions options for the internal source
 */
class SymbolManager @UiThread internal constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap,
    style: Style,
    coreElementProvider: CoreElementProvider<SymbolLayer> = SymbolElementProvider(),
    belowLayerId: String? = null,
    aboveLayerId: String? = null,
    geoJsonOptions: GeoJsonOptions? = null,
    draggableAnnotationController: DraggableAnnotationController = DraggableAnnotationController.getInstance(
        mapView,
        maplibreMap
    )
) : AnnotationManager<SymbolLayer, Symbol, SymbolOptions, OnSymbolDragListener, OnSymbolClickListener, OnSymbolLongClickListener>(
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
     * Create a symbol manager, used to manage symbols.
     *
     * @param maplibreMap  the map object to add symbols to
     * @param style        a valid a fully loaded style object
     * @param belowLayerId the id of the layer above the symbol layer
     * @param aboveLayerId the id of the layer below the symbol layer
     */
    @UiThread
    @JvmOverloads
    constructor(
        mapView: MapView,
        maplibreMap: MapLibreMap,
        style: Style,
        belowLayerId: String? = null,
        aboveLayerId: String? = null,
        geoJsonOptions: GeoJsonOptions? = null
    ) : this(
        mapView,
        maplibreMap,
        style,
        SymbolElementProvider(),
        belowLayerId,
        aboveLayerId,
        geoJsonOptions
    )

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
    constructor(
        mapView: MapView,
        maplibreMap: MapLibreMap,
        style: Style,
        belowLayerId: String?,
        aboveLayerId: String?,
        clusterOptions: ClusterOptions
    ) : this(
        mapView,
        maplibreMap,
        style,
        belowLayerId,
        aboveLayerId,
        GeoJsonOptions().withCluster(true).withClusterRadius(clusterOptions.clusterRadius)
            .withClusterMaxZoom(clusterOptions.clusterMaxZoom)
    ) {
        clusterOptions.apply(style, coreElementProvider.sourceId)
    }

    override fun initializeDataDrivenPropertyMap() = listOf(
        SymbolOptions.PROPERTY_SYMBOL_SORT_KEY,
        SymbolOptions.PROPERTY_ICON_SIZE,
        SymbolOptions.PROPERTY_ICON_IMAGE,
        SymbolOptions.PROPERTY_ICON_ROTATE,
        SymbolOptions.PROPERTY_ICON_OFFSET,
        SymbolOptions.PROPERTY_ICON_ANCHOR,
        SymbolOptions.PROPERTY_TEXT_FIELD,
        SymbolOptions.PROPERTY_TEXT_FONT,
        SymbolOptions.PROPERTY_TEXT_SIZE,
        SymbolOptions.PROPERTY_TEXT_MAX_WIDTH,
        SymbolOptions.PROPERTY_TEXT_LETTER_SPACING,
        SymbolOptions.PROPERTY_TEXT_JUSTIFY,
        SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET,
        SymbolOptions.PROPERTY_TEXT_ANCHOR,
        SymbolOptions.PROPERTY_TEXT_ROTATE,
        SymbolOptions.PROPERTY_TEXT_TRANSFORM,
        SymbolOptions.PROPERTY_TEXT_OFFSET,
        SymbolOptions.PROPERTY_ICON_OPACITY,
        SymbolOptions.PROPERTY_ICON_COLOR,
        SymbolOptions.PROPERTY_ICON_HALO_COLOR,
        SymbolOptions.PROPERTY_ICON_HALO_WIDTH,
        SymbolOptions.PROPERTY_ICON_HALO_BLUR,
        SymbolOptions.PROPERTY_TEXT_OPACITY,
        SymbolOptions.PROPERTY_TEXT_COLOR,
        SymbolOptions.PROPERTY_TEXT_HALO_COLOR,
        SymbolOptions.PROPERTY_TEXT_HALO_WIDTH,
        SymbolOptions.PROPERTY_TEXT_HALO_BLUR
    ).associateWith { false }.let { dataDrivenPropertyUsageMap.putAll(it) }

    override fun setDataDrivenPropertyIsUsed(property: String) {
        when (property) {
            SymbolOptions.PROPERTY_SYMBOL_SORT_KEY -> layer.setProperties(
                PropertyFactory.symbolSortKey(
                    Expression.get(SymbolOptions.PROPERTY_SYMBOL_SORT_KEY)
                )
            )

            SymbolOptions.PROPERTY_ICON_SIZE -> layer.setProperties(
                PropertyFactory.iconSize(
                    Expression.get(SymbolOptions.PROPERTY_ICON_SIZE)
                )
            )

            SymbolOptions.PROPERTY_ICON_IMAGE -> layer.setProperties(
                PropertyFactory.iconImage(
                    Expression.get(SymbolOptions.PROPERTY_ICON_IMAGE)
                )
            )

            SymbolOptions.PROPERTY_ICON_ROTATE -> layer.setProperties(
                PropertyFactory.iconRotate(
                    Expression.get(SymbolOptions.PROPERTY_ICON_ROTATE)
                )
            )

            SymbolOptions.PROPERTY_ICON_OFFSET -> layer.setProperties(
                PropertyFactory.iconOffset(
                    Expression.get(SymbolOptions.PROPERTY_ICON_OFFSET)
                )
            )

            SymbolOptions.PROPERTY_ICON_ANCHOR -> layer.setProperties(
                PropertyFactory.iconAnchor(
                    Expression.get(SymbolOptions.PROPERTY_ICON_ANCHOR)
                )
            )

            SymbolOptions.PROPERTY_TEXT_FIELD -> layer.setProperties(
                PropertyFactory.textField(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_FIELD)
                )
            )

            SymbolOptions.PROPERTY_TEXT_FONT -> layer.setProperties(
                PropertyFactory.textFont(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_FONT)
                )
            )

            SymbolOptions.PROPERTY_TEXT_SIZE -> layer.setProperties(
                PropertyFactory.textSize(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_SIZE)
                )
            )

            SymbolOptions.PROPERTY_TEXT_MAX_WIDTH -> layer.setProperties(
                PropertyFactory.textMaxWidth(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_MAX_WIDTH)
                )
            )

            SymbolOptions.PROPERTY_TEXT_LETTER_SPACING -> layer.setProperties(
                PropertyFactory.textLetterSpacing(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_LETTER_SPACING)
                )
            )

            SymbolOptions.PROPERTY_TEXT_JUSTIFY -> layer.setProperties(
                PropertyFactory.textJustify(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_JUSTIFY)
                )
            )

            SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET -> layer.setProperties(
                PropertyFactory.textRadialOffset(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET)
                )
            )

            SymbolOptions.PROPERTY_TEXT_ANCHOR -> layer.setProperties(
                PropertyFactory.textAnchor(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_ANCHOR)
                )
            )

            SymbolOptions.PROPERTY_TEXT_ROTATE -> layer.setProperties(
                PropertyFactory.textRotate(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_ROTATE)
                )
            )

            SymbolOptions.PROPERTY_TEXT_TRANSFORM -> layer.setProperties(
                PropertyFactory.textTransform(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_TRANSFORM)
                )
            )

            SymbolOptions.PROPERTY_TEXT_OFFSET -> layer.setProperties(
                PropertyFactory.textOffset(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_OFFSET)
                )
            )

            SymbolOptions.PROPERTY_ICON_OPACITY -> layer.setProperties(
                PropertyFactory.iconOpacity(
                    Expression.get(SymbolOptions.PROPERTY_ICON_OPACITY)
                )
            )

            SymbolOptions.PROPERTY_ICON_COLOR -> layer.setProperties(
                PropertyFactory.iconColor(
                    Expression.get(SymbolOptions.PROPERTY_ICON_COLOR)
                )
            )

            SymbolOptions.PROPERTY_ICON_HALO_COLOR -> layer.setProperties(
                PropertyFactory.iconHaloColor(
                    Expression.get(SymbolOptions.PROPERTY_ICON_HALO_COLOR)
                )
            )

            SymbolOptions.PROPERTY_ICON_HALO_WIDTH -> layer.setProperties(
                PropertyFactory.iconHaloWidth(
                    Expression.get(SymbolOptions.PROPERTY_ICON_HALO_WIDTH)
                )
            )

            SymbolOptions.PROPERTY_ICON_HALO_BLUR -> layer.setProperties(
                PropertyFactory.iconHaloBlur(
                    Expression.get(SymbolOptions.PROPERTY_ICON_HALO_BLUR)
                )
            )

            SymbolOptions.PROPERTY_TEXT_OPACITY -> layer.setProperties(
                PropertyFactory.textOpacity(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_OPACITY)
                )
            )

            SymbolOptions.PROPERTY_TEXT_COLOR -> layer.setProperties(
                PropertyFactory.textColor(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_COLOR)
                )
            )

            SymbolOptions.PROPERTY_TEXT_HALO_COLOR -> layer.setProperties(
                PropertyFactory.textHaloColor(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_HALO_COLOR)
                )
            )

            SymbolOptions.PROPERTY_TEXT_HALO_WIDTH -> layer.setProperties(
                PropertyFactory.textHaloWidth(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_HALO_WIDTH)
                )
            )

            SymbolOptions.PROPERTY_TEXT_HALO_BLUR -> layer.setProperties(
                PropertyFactory.textHaloBlur(
                    Expression.get(SymbolOptions.PROPERTY_TEXT_HALO_BLUR)
                )
            )
        }
    }

    /**
     * Create a list of symbols on the map.
     *
     *
     * Symbols are going to be created only for features with a matching geometry.
     *
     *
     * All supported properties are:<br></br>
     * SymbolOptions.PROPERTY_SYMBOL_SORT_KEY - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_SIZE - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_IMAGE - String<br></br>
     * SymbolOptions.PROPERTY_ICON_ROTATE - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_OFFSET - Float[]<br></br>
     * SymbolOptions.PROPERTY_ICON_ANCHOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_FIELD - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_FONT - String[]<br></br>
     * SymbolOptions.PROPERTY_TEXT_SIZE - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_MAX_WIDTH - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_LETTER_SPACING - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_JUSTIFY - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_ANCHOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_ROTATE - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_TRANSFORM - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_OFFSET - Float[]<br></br>
     * SymbolOptions.PROPERTY_ICON_OPACITY - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_ICON_HALO_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_ICON_HALO_WIDTH - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_HALO_BLUR - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_OPACITY - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_HALO_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_HALO_WIDTH - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_HALO_BLUR - Float<br></br>
     * Learn more about above properties in the [Style specification](https://maplibre.org/maplibre-style-spec/).
     *
     *
     * Out of spec properties:<br></br>
     * "is-draggable" - Boolean, true if the symbol should be draggable, false otherwise
     *
     * @param json the GeoJSON defining the list of symbols to build
     * @return the list of built symbols
     */
    @UiThread
    fun create(json: String): List<Symbol?> {
        return create(FeatureCollection.fromJson(json))
    }

    /**
     * Create a list of symbols on the map.
     *
     *
     * Symbols are going to be created only for features with a matching geometry.
     *
     *
     * All supported properties are:<br></br>
     * SymbolOptions.PROPERTY_SYMBOL_SORT_KEY - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_SIZE - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_IMAGE - String<br></br>
     * SymbolOptions.PROPERTY_ICON_ROTATE - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_OFFSET - Float[]<br></br>
     * SymbolOptions.PROPERTY_ICON_ANCHOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_FIELD - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_FONT - String[]<br></br>
     * SymbolOptions.PROPERTY_TEXT_SIZE - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_MAX_WIDTH - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_LETTER_SPACING - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_JUSTIFY - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_RADIAL_OFFSET - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_ANCHOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_ROTATE - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_TRANSFORM - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_OFFSET - Float[]<br></br>
     * SymbolOptions.PROPERTY_ICON_OPACITY - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_ICON_HALO_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_ICON_HALO_WIDTH - Float<br></br>
     * SymbolOptions.PROPERTY_ICON_HALO_BLUR - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_OPACITY - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_HALO_COLOR - String<br></br>
     * SymbolOptions.PROPERTY_TEXT_HALO_WIDTH - Float<br></br>
     * SymbolOptions.PROPERTY_TEXT_HALO_BLUR - Float<br></br>
     * Learn more about above properties in the [Style specification](https://maplibre.org/maplibre-style-spec/).
     *
     *
     * Out of spec properties:<br></br>
     * "is-draggable" - Boolean, true if the symbol should be draggable, false otherwise
     *
     * @param featureCollection the featureCollection defining the list of symbols to build
     * @return the list of built symbols
     */
    @UiThread
    fun create(featureCollection: FeatureCollection): List<Symbol> = featureCollection.features()?.mapNotNull {
        SymbolOptions.fromFeature(it)
    }.let { create(it ?: emptyList()) }

    /**
     * Key of the id of the annotation.
     */
    override val annotationIdKey: String
        get() = Symbol.ID_KEY

    // Property accessors
    /**
     * Label placement relative to its geometry.
     */
    var symbolPlacement: String?
        get() = layer.symbolPlacement.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.symbolPlacement(value)
            constantPropertyUsageMap[PROPERTY_SYMBOL_PLACEMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Distance between two symbol anchors.
     */
    var symbolSpacing: Float?
        get() = layer.symbolSpacing.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.symbolSpacing(value)
            constantPropertyUsageMap[PROPERTY_SYMBOL_SPACING] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, the symbols will not cross tile edges to avoid mutual collisions. Recommended in layers that don't have enough padding in the vector tile to prevent collisions, or if it is a point symbol layer placed after a line symbol layer.
     */
    var symbolAvoidEdges: Boolean?
        get() = layer.symbolAvoidEdges.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.symbolAvoidEdges(value)
            constantPropertyUsageMap[PROPERTY_SYMBOL_AVOID_EDGES] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, the icon will be visible even if it collides with other previously drawn symbols.
     */
    var iconAllowOverlap: Boolean?
        get() = layer.iconAllowOverlap.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconAllowOverlap(value)
            constantPropertyUsageMap[PROPERTY_ICON_ALLOW_OVERLAP] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, other symbols can be visible even if they collide with the icon.
     */
    var iconIgnorePlacement: Boolean?
        get() = layer.iconIgnorePlacement.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconIgnorePlacement(value)
            constantPropertyUsageMap[PROPERTY_ICON_IGNORE_PLACEMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, text will display without their corresponding icons when the icon collides with other symbols and the text does not.
     */
    var iconOptional: Boolean?
        get() = layer.iconOptional.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconOptional(value)
            constantPropertyUsageMap[PROPERTY_ICON_OPTIONAL] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * In combination with [symbolPlacement], determines the rotation behavior of icons.
     */
    var iconRotationAlignment: String?
        get() = layer.iconRotationAlignment.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconRotationAlignment(value)
            constantPropertyUsageMap[PROPERTY_ICON_ROTATION_ALIGNMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Scales the icon to fit around the associated text.
     */
    var iconTextFit: String?
        get() = layer.iconTextFit.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconTextFit(value)
            constantPropertyUsageMap[PROPERTY_ICON_TEXT_FIT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Size of the additional area added to dimensions determined by [Property.ICON_TEXT_FIT], in clockwise order: top, right, bottom, left.
     */
    var iconTextFitPadding: Array<Float?>?
        get() = layer.iconTextFitPadding.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconTextFitPadding(value)
            constantPropertyUsageMap[PROPERTY_ICON_TEXT_FIT_PADDING] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Size of the additional area around the icon bounding box used for detecting symbol collisions.
     */
    var iconPadding: Float?
        get() = layer.iconPadding.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconPadding(value)
            constantPropertyUsageMap[PROPERTY_ICON_PADDING] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, the icon may be flipped to prevent it from being rendered upside-down.
     */
    var iconKeepUpright: Boolean?
        get() = layer.iconKeepUpright.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconKeepUpright(value)
            constantPropertyUsageMap[PROPERTY_ICON_KEEP_UPRIGHT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Orientation of icon when map is pitched.
     */
    var iconPitchAlignment: String?
        get() = layer.iconPitchAlignment.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconPitchAlignment(value)
            constantPropertyUsageMap[PROPERTY_ICON_PITCH_ALIGNMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Orientation of text when map is pitched.
     */
    var textPitchAlignment: String?
        get() = layer.textPitchAlignment.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textPitchAlignment(value)
            constantPropertyUsageMap[PROPERTY_TEXT_PITCH_ALIGNMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * In combination with [symbolPlacement], determines the rotation behavior of the individual glyphs forming the text.
     */
    var textRotationAlignment: String?
        get() = layer.textRotationAlignment.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textRotationAlignment(value)
            constantPropertyUsageMap[PROPERTY_TEXT_ROTATION_ALIGNMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Text leading value for multi-line text.
     */
    var textLineHeight: Float?
        get() = layer.textLineHeight.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textLineHeight(value)
            constantPropertyUsageMap[PROPERTY_TEXT_LINE_HEIGHT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * To increase the chance of placing high-priority labels on the map, you can provide an array of [Property.TEXT_ANCHOR] locations: the render will attempt to place the label at each location, in order, before moving onto the next label. Use `text-justify: auto` to choose justification based on anchor position. To apply an offset, use the [PropertyFactory.textRadialOffset] instead of the two-dimensional [PropertyFactory.textOffset].
     */
    var textVariableAnchor: Array<String?>?
        get() = layer.textVariableAnchor.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textVariableAnchor(value)
            constantPropertyUsageMap.put(PROPERTY_TEXT_VARIABLE_ANCHOR, propertyValue)
            layer.setProperties(propertyValue)
        }

    /**
     * Maximum angle change between adjacent characters.
     */
    var textMaxAngle: Float?
        get() = layer.textMaxAngle.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textMaxAngle(value)
            constantPropertyUsageMap[PROPERTY_TEXT_MAX_ANGLE] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Size of the additional area around the text bounding box used for detecting symbol collisions.
     */
    var textPadding: Float?
        get() = layer.textPadding.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textPadding(value)
            constantPropertyUsageMap[PROPERTY_TEXT_PADDING] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, the text may be flipped vertically to prevent it from being rendered upside-down.
     */
    var textKeepUpright: Boolean?
        get() = layer.textKeepUpright.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textKeepUpright(value)
            constantPropertyUsageMap[PROPERTY_TEXT_KEEP_UPRIGHT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, the text will be visible even if it collides with other previously drawn symbols.
     */
    var textAllowOverlap: Boolean?
        get() = layer.textAllowOverlap.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textAllowOverlap(value)
            constantPropertyUsageMap[PROPERTY_TEXT_ALLOW_OVERLAP] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, other symbols can be visible even if they collide with the text.
     */
    var textIgnorePlacement: Boolean?
        get() = layer.textIgnorePlacement.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textIgnorePlacement(value)
            constantPropertyUsageMap[PROPERTY_TEXT_IGNORE_PLACEMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * If true, icons will display without their corresponding text when the text collides with other symbols and the icon does not.
     */
    var textOptional: Boolean?
        get() = layer.textOptional.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textOptional(value)
            constantPropertyUsageMap[PROPERTY_TEXT_OPTIONAL] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Distance that the icon's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     */
    var iconTranslate: Array<Float?>?
        get() = layer.iconTranslate.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconTranslate(value)
            constantPropertyUsageMap.put(PROPERTY_ICON_TRANSLATE, propertyValue)
            layer.setProperties(propertyValue)
        }

    /**
     * Controls the frame of reference for [PropertyFactory.iconTranslate].
     */
    var iconTranslateAnchor: String?
        get() {
            return layer.iconTranslateAnchor.value
        }
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.iconTranslateAnchor(value)
            constantPropertyUsageMap[PROPERTY_ICON_TRANSLATE_ANCHOR] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Distance that the text's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     */
    var textTranslate: Array<Float?>?
        get() = layer.textTranslate.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textTranslate(value)
            constantPropertyUsageMap.put(PROPERTY_TEXT_TRANSLATE, propertyValue)
            layer.setProperties(propertyValue)
        }

    /**
     * Controls the frame of reference for [PropertyFactory.textTranslate].
     */
    var textTranslateAnchor: String?
        get() = layer.textTranslateAnchor.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.textTranslateAnchor(value)
            constantPropertyUsageMap[PROPERTY_TEXT_TRANSLATE_ANCHOR] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Set filter on the managed symbols.
     */
    override fun setFilter(expression: Expression) {
        layerFilter = expression
        layer.setFilter(expression)
    }

    val filter: Expression?
        /**
         * Get filter of the managed symbols.
         */
        get() {
            return layer.filter
        }

    companion object {
        private const val PROPERTY_SYMBOL_PLACEMENT: String = "symbol-placement"
        private const val PROPERTY_SYMBOL_SPACING: String = "symbol-spacing"
        private const val PROPERTY_SYMBOL_AVOID_EDGES: String = "symbol-avoid-edges"
        private const val PROPERTY_ICON_ALLOW_OVERLAP: String = "icon-allow-overlap"
        private const val PROPERTY_ICON_IGNORE_PLACEMENT: String = "icon-ignore-placement"
        private const val PROPERTY_ICON_OPTIONAL: String = "icon-optional"
        private const val PROPERTY_ICON_ROTATION_ALIGNMENT: String = "icon-rotation-alignment"
        private const val PROPERTY_ICON_TEXT_FIT: String = "icon-text-fit"
        private const val PROPERTY_ICON_TEXT_FIT_PADDING: String = "icon-text-fit-padding"
        private const val PROPERTY_ICON_PADDING: String = "icon-padding"
        private const val PROPERTY_ICON_KEEP_UPRIGHT: String = "icon-keep-upright"
        private const val PROPERTY_ICON_PITCH_ALIGNMENT: String = "icon-pitch-alignment"
        private const val PROPERTY_TEXT_PITCH_ALIGNMENT: String = "text-pitch-alignment"
        private const val PROPERTY_TEXT_ROTATION_ALIGNMENT: String = "text-rotation-alignment"
        private const val PROPERTY_TEXT_LINE_HEIGHT: String = "text-line-height"
        private const val PROPERTY_TEXT_VARIABLE_ANCHOR: String = "text-variable-anchor"
        private const val PROPERTY_TEXT_MAX_ANGLE: String = "text-max-angle"
        private const val PROPERTY_TEXT_PADDING: String = "text-padding"
        private const val PROPERTY_TEXT_KEEP_UPRIGHT: String = "text-keep-upright"
        private const val PROPERTY_TEXT_ALLOW_OVERLAP: String = "text-allow-overlap"
        private const val PROPERTY_TEXT_IGNORE_PLACEMENT: String = "text-ignore-placement"
        private const val PROPERTY_TEXT_OPTIONAL: String = "text-optional"
        private const val PROPERTY_ICON_TRANSLATE: String = "icon-translate"
        private const val PROPERTY_ICON_TRANSLATE_ANCHOR: String = "icon-translate-anchor"
        private const val PROPERTY_TEXT_TRANSLATE: String = "text-translate"
        private const val PROPERTY_TEXT_TRANSLATE_ANCHOR: String = "text-translate-anchor"
    }
}
