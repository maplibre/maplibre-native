package org.maplibre.android.annotations

import androidx.annotation.UiThread
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
) : AnnotationManager<SymbolLayer, KSymbol>(
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
        belowLayerId: String? = null,
        aboveLayerId: String? = null,
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

    override fun generateDataDrivenPropertyExpression(property: String): PropertyValue<Expression> = when (property) {
        KSymbol.PROPERTY_SYMBOL_SORT_KEY -> PropertyFactory.symbolSortKey(
            Expression.get(KSymbol.PROPERTY_SYMBOL_SORT_KEY)
        )

        KSymbol.PROPERTY_ICON_SIZE -> PropertyFactory.iconSize(
            Expression.get(KSymbol.PROPERTY_ICON_SIZE)
        )

        KSymbol.PROPERTY_ICON_IMAGE -> PropertyFactory.iconImage(
            Expression.get(KSymbol.PROPERTY_ICON_IMAGE)
        )

        KSymbol.PROPERTY_ICON_ROTATE -> PropertyFactory.iconRotate(
            Expression.get(KSymbol.PROPERTY_ICON_ROTATE)
        )

        KSymbol.PROPERTY_ICON_OFFSET -> PropertyFactory.iconOffset(
            Expression.get(KSymbol.PROPERTY_ICON_OFFSET)
        )

        KSymbol.PROPERTY_ICON_ANCHOR -> PropertyFactory.iconAnchor(
            Expression.get(KSymbol.PROPERTY_ICON_ANCHOR)
        )

        KSymbol.PROPERTY_TEXT_FIELD -> PropertyFactory.textField(
            Expression.get(KSymbol.PROPERTY_TEXT_FIELD)
        )

        KSymbol.PROPERTY_TEXT_FONT -> PropertyFactory.textFont(
            Expression.get(KSymbol.PROPERTY_TEXT_FONT)
        )

        KSymbol.PROPERTY_TEXT_SIZE -> PropertyFactory.textSize(
            Expression.get(KSymbol.PROPERTY_TEXT_SIZE)
        )

        KSymbol.PROPERTY_TEXT_MAX_WIDTH -> PropertyFactory.textMaxWidth(
            Expression.get(KSymbol.PROPERTY_TEXT_MAX_WIDTH)
        )

        KSymbol.PROPERTY_TEXT_LETTER_SPACING -> PropertyFactory.textLetterSpacing(
            Expression.get(KSymbol.PROPERTY_TEXT_LETTER_SPACING)
        )

        KSymbol.PROPERTY_TEXT_JUSTIFY -> PropertyFactory.textJustify(
            Expression.get(KSymbol.PROPERTY_TEXT_JUSTIFY)
        )

        KSymbol.PROPERTY_TEXT_RADIAL_OFFSET -> PropertyFactory.textRadialOffset(
            Expression.get(KSymbol.PROPERTY_TEXT_RADIAL_OFFSET)
        )

        KSymbol.PROPERTY_TEXT_ANCHOR -> PropertyFactory.textAnchor(
            Expression.get(KSymbol.PROPERTY_TEXT_ANCHOR)
        )

        KSymbol.PROPERTY_TEXT_ROTATE -> PropertyFactory.textRotate(
            Expression.get(KSymbol.PROPERTY_TEXT_ROTATE)
        )

        KSymbol.PROPERTY_TEXT_TRANSFORM -> PropertyFactory.textTransform(
            Expression.get(KSymbol.PROPERTY_TEXT_TRANSFORM)
        )

        KSymbol.PROPERTY_TEXT_OFFSET -> PropertyFactory.textOffset(
            Expression.get(KSymbol.PROPERTY_TEXT_OFFSET)
        )

        KSymbol.PROPERTY_ICON_OPACITY -> PropertyFactory.iconOpacity(
            Expression.get(KSymbol.PROPERTY_ICON_OPACITY)

        )

        KSymbol.PROPERTY_ICON_COLOR -> PropertyFactory.iconColor(
            Expression.get(KSymbol.PROPERTY_ICON_COLOR)
        )

        KSymbol.PROPERTY_ICON_HALO_COLOR -> PropertyFactory.iconHaloColor(
            Expression.get(KSymbol.PROPERTY_ICON_HALO_COLOR)
        )

        KSymbol.PROPERTY_ICON_HALO_WIDTH -> PropertyFactory.iconHaloWidth(
            Expression.get(KSymbol.PROPERTY_ICON_HALO_WIDTH)
        )

        KSymbol.PROPERTY_ICON_HALO_BLUR -> PropertyFactory.iconHaloBlur(
            Expression.get(KSymbol.PROPERTY_ICON_HALO_BLUR)
        )

        KSymbol.PROPERTY_TEXT_OPACITY -> PropertyFactory.textOpacity(
            Expression.get(KSymbol.PROPERTY_TEXT_OPACITY)
        )

        KSymbol.PROPERTY_TEXT_COLOR -> PropertyFactory.textColor(
            Expression.get(KSymbol.PROPERTY_TEXT_COLOR)
        )

        KSymbol.PROPERTY_TEXT_HALO_COLOR -> PropertyFactory.textHaloColor(
            Expression.get(KSymbol.PROPERTY_TEXT_HALO_COLOR)
        )

        KSymbol.PROPERTY_TEXT_HALO_WIDTH -> PropertyFactory.textHaloWidth(
            Expression.get(KSymbol.PROPERTY_TEXT_HALO_WIDTH)
        )

        KSymbol.PROPERTY_TEXT_HALO_BLUR -> PropertyFactory.textHaloBlur(
            Expression.get(KSymbol.PROPERTY_TEXT_HALO_BLUR)
        )

        else -> throw IllegalArgumentException(
            "$property is not a valid data-driven property for a symbol."
        )
    }

    override fun addDragListener(d: OnSymbolDragListener) {
        super.addDragListener(d)
    }

    override fun removeDragListener(d: OnSymbolDragListener) {
        super.removeDragListener(d)
    }

    override fun addClickListener(u: OnSymbolClickListener) {
        super.addClickListener(u)
    }

    override fun removeClickListener(u: OnSymbolClickListener) {
        super.removeClickListener(u)
    }

    override fun addLongClickListener(v: OnSymbolLongClickListener) {
        super.addLongClickListener(v)
    }

    override fun removeLongClickListener(v: OnSymbolLongClickListener) {
        super.removeLongClickListener(v)
    }

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
            constantPropertyUsageMap[PROPERTY_TEXT_VARIABLE_ANCHOR] = propertyValue
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
            constantPropertyUsageMap[PROPERTY_ICON_TRANSLATE] = propertyValue
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
            constantPropertyUsageMap[PROPERTY_TEXT_TRANSLATE] = propertyValue
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
