package org.maplibre.android.annotations

import androidx.annotation.UiThread
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.style.sources.GeoJsonOptions

/**
 * The line manager allows to add lines to a map.
 *
 * @param maplibreMap      the map object to add lines to
 * @param style          a valid a fully loaded style object
 * @param belowLayerId   the id of the layer above the line layer
 * @param aboveLayerId   the id of the layer below the line layer
 * @param geoJsonOptions options for the internal source
 */
class LineManager @UiThread internal constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap,
    style: Style,
    coreElementProvider: CoreElementProvider<LineLayer> = LineElementProvider(),
    belowLayerId: String? = null,
    aboveLayerId: String? = null,
    geoJsonOptions: GeoJsonOptions? = null,
    draggableAnnotationController: DraggableAnnotationController =
        DraggableAnnotationController.getInstance(mapView, maplibreMap)
) : AnnotationManager<LineLayer, Line>(
    mapView,
    maplibreMap,
    style,
    coreElementProvider,
    draggableAnnotationController,
    belowLayerId,
    aboveLayerId,
    geoJsonOptions
) {

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
        LineElementProvider(),
        belowLayerId,
        aboveLayerId,
        geoJsonOptions
    )

    override fun generateDataDrivenPropertyExpression(property: String): PropertyValue<Expression> = when (property) {
        Line.PROPERTY_LINE_SORT_KEY -> PropertyFactory.lineSortKey(
            Expression.get(Line.PROPERTY_LINE_SORT_KEY)
        )

        Line.PROPERTY_LINE_JOIN -> PropertyFactory.lineJoin(
            Expression.get(Line.PROPERTY_LINE_JOIN)
        )

        Line.PROPERTY_LINE_OPACITY -> PropertyFactory.lineOpacity(
            Expression.get(Line.PROPERTY_LINE_OPACITY)
        )

        Line.PROPERTY_LINE_COLOR -> PropertyFactory.lineColor(
            Expression.get(Line.PROPERTY_LINE_COLOR)
        )

        Line.PROPERTY_LINE_WIDTH -> PropertyFactory.lineWidth(
            Expression.get(Line.PROPERTY_LINE_WIDTH)
        )

        Line.PROPERTY_LINE_GAP_WIDTH -> PropertyFactory.lineGapWidth(
            Expression.get(Line.PROPERTY_LINE_GAP_WIDTH)
        )

        Line.PROPERTY_LINE_OFFSET -> PropertyFactory.lineOffset(
            Expression.get(Line.PROPERTY_LINE_OFFSET)
        )

        Line.PROPERTY_LINE_BLUR -> PropertyFactory.lineBlur(
            Expression.get(Line.PROPERTY_LINE_BLUR)
        )

        Line.PROPERTY_LINE_PATTERN -> PropertyFactory.linePattern(
            Expression.get(Line.PROPERTY_LINE_PATTERN)
        )

        else -> throw IllegalArgumentException(
            "$property is not a valid data-driven property for a line."
        )
    }

    override fun addDragListener(d: OnLineDragListener) {
        super.addDragListener(d)
    }

    override fun removeDragListener(d: OnLineDragListener) {
        super.removeDragListener(d)
    }

    override fun addClickListener(u: OnLineClickListener) {
        super.addClickListener(u)
    }

    override fun removeClickListener(u: OnLineClickListener) {
        super.removeClickListener(u)
    }

    override fun addLongClickListener(v: OnLineLongClickListener) {
        super.addLongClickListener(v)
    }

    override fun removeLongClickListener(v: OnLineLongClickListener) {
        super.removeLongClickListener(v)
    }

    // Property accessors
    /**
     * The display of line endings.
     */
    var lineCap: String?
        get() = layer.lineCap.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.lineCap(value)
            constantPropertyUsageMap[PROPERTY_LINE_CAP] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Used to automatically convert miter joins to bevel joins for sharp angles.
     */
    var lineMiterLimit: Float?
        get() = layer.lineMiterLimit.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.lineMiterLimit(value)
            constantPropertyUsageMap[PROPERTY_LINE_MITER_LIMIT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Used to automatically convert round joins to miter joins for shallow angles.
     */
    var lineRoundLimit: Float?
        get() = layer.lineRoundLimit.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.lineRoundLimit(value)
            constantPropertyUsageMap[PROPERTY_LINE_ROUND_LIMIT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     */
    var lineTranslate: Array<Float?>?
        get() = layer.lineTranslate.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.lineTranslate(value)
            constantPropertyUsageMap[PROPERTY_LINE_TRANSLATE] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Controls the frame of reference for [PropertyFactory.lineTranslate].
     */
    var lineTranslateAnchor: String?
        get() = layer.lineTranslateAnchor.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.lineTranslateAnchor(value)
            constantPropertyUsageMap[PROPERTY_LINE_TRANSLATE_ANCHOR] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Specifies the lengths of the alternating dashes and gaps that form the dash pattern. The lengths are later scaled by the line width. To convert a dash length to density-independent pixels, multiply the length by the current line width. Note that GeoJSON sources with `lineMetrics: true` specified won't render dashed lines to the expected scale. Also note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     */
    var lineDasharray: Array<Float?>?
        get() = layer.lineDasharray.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.lineDasharray(value)
            constantPropertyUsageMap[PROPERTY_LINE_DASHARRAY] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Set filter on the managed lines.
     */
    override fun setFilter(expression: Expression) {
        layerFilter = expression
        layer.setFilter(expression)
    }

    val filter: Expression?
        /**
         * Get filter of the managed lines.
         */
        get() = layer.filter

    companion object {
        private const val PROPERTY_LINE_CAP: String = "line-cap"
        private const val PROPERTY_LINE_MITER_LIMIT: String = "line-miter-limit"
        private const val PROPERTY_LINE_ROUND_LIMIT: String = "line-round-limit"
        private const val PROPERTY_LINE_TRANSLATE: String = "line-translate"
        private const val PROPERTY_LINE_TRANSLATE_ANCHOR: String = "line-translate-anchor"
        private const val PROPERTY_LINE_DASHARRAY: String = "line-dasharray"
    }
}
