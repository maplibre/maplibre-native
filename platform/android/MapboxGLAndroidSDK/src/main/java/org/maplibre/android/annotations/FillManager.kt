package org.maplibre.android.annotations

import androidx.annotation.UiThread
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
 *
 * @param maplibreMap    the map object to add fills to
 * @param style          a valid a fully loaded style object
 * @param belowLayerId   the id of the layer above the fill layer
 * @param aboveLayerId   the id of the layer below the fill layer
 * @param geoJsonOptions options for the internal source
 */
class FillManager @UiThread internal constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap,
    style: Style,
    coreElementProvider: CoreElementProvider<FillLayer> = FillElementProvider(),
    belowLayerId: String? = null,
    aboveLayerId: String? = null,
    geoJsonOptions: GeoJsonOptions? = null,
    draggableAnnotationController: DraggableAnnotationController = DraggableAnnotationController.getInstance(
        mapView,
        maplibreMap
    )
) : AnnotationManager<FillLayer, Fill>(
    mapView,
    maplibreMap,
    style,
    coreElementProvider,
    draggableAnnotationController,
    belowLayerId,
    aboveLayerId,
    geoJsonOptions
) {

    override fun generateDataDrivenPropertyExpression(property: String): PropertyValue<Expression> = when (property) {
        Fill.PROPERTY_FILL_OPACITY -> PropertyFactory.fillOpacity(
            Expression.get(Fill.PROPERTY_FILL_OPACITY)
        )

        Fill.PROPERTY_FILL_COLOR -> PropertyFactory.fillColor(
            Expression.get(Fill.PROPERTY_FILL_COLOR)
        )

        Fill.PROPERTY_FILL_OUTLINE_COLOR -> PropertyFactory.fillOutlineColor(
            Expression.get(Fill.PROPERTY_FILL_OUTLINE_COLOR)
        )

        Fill.PROPERTY_FILL_PATTERN -> PropertyFactory.fillPattern(
            Expression.get(Fill.PROPERTY_FILL_PATTERN)
        )

        else -> throw IllegalArgumentException(
            "$property is not a valid data-driven property for a fill."
        )
    }

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
        FillElementProvider(),
        belowLayerId,
        aboveLayerId,
        geoJsonOptions
    )

    override fun addDragListener(d: OnFillDragListener) {
        super.addDragListener(d)
    }

    override fun removeDragListener(d: OnFillDragListener) {
        super.removeDragListener(d)
    }

    override fun addClickListener(u: OnFillClickListener) {
        super.addClickListener(u)
    }

    override fun removeClickListener(u: OnFillClickListener) {
        super.removeClickListener(u)
    }

    override fun addLongClickListener(v: OnFillLongClickListener) {
        super.addLongClickListener(v)
    }

    override fun removeLongClickListener(v: OnFillLongClickListener) {
        super.removeLongClickListener(v)
    }

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
    override fun setFilter(expression: Expression) {
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
        private const val PROPERTY_FILL_ANTIALIAS: String = "fill-antialias"
        private const val PROPERTY_FILL_TRANSLATE: String = "fill-translate"
        private const val PROPERTY_FILL_TRANSLATE_ANCHOR: String = "fill-translate-anchor"
    }
}
