package org.maplibre.android.annotations

import androidx.annotation.UiThread
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.style.sources.GeoJsonOptions

/**
 * The circle manager allows to add circles to a map.
 *
 * @param maplibreMap    the map object to add circles to
 * @param style          a valid a fully loaded style object
 * @param belowLayerId   the id of the layer above the circle layer
 * @param aboveLayerId   the id of the layer below the circle layer
 * @param geoJsonOptions options for the internal source
 */
class CircleManager @UiThread internal constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap,
    style: Style,
    coreElementProvider: CoreElementProvider<CircleLayer> = CircleElementProvider(),
    belowLayerId: String? = null,
    aboveLayerId: String? = null,
    geoJsonOptions: GeoJsonOptions? = null,
    draggableAnnotationController: DraggableAnnotationController = DraggableAnnotationController.getInstance(
        mapView,
        maplibreMap
    )
) : AnnotationManager<CircleLayer, Circle>(
    mapView,
    maplibreMap,
    style,
    coreElementProvider,
    draggableAnnotationController,
    belowLayerId,
    aboveLayerId,
    geoJsonOptions
) {
    @JvmOverloads
    @UiThread
    constructor(
        mapView: MapView,
        maplibreMap: MapLibreMap,
        style: Style,
        belowLayerId: String? = null,
        aboveLayerId: String? = null,
        geoJsonOptions: GeoJsonOptions? = null
    ) : this(
        mapView = mapView,
        maplibreMap = maplibreMap,
        style = style,
        coreElementProvider = CircleElementProvider(),
        belowLayerId = belowLayerId,
        aboveLayerId = aboveLayerId,
        geoJsonOptions = geoJsonOptions
    )

    override fun generateDataDrivenPropertyExpression(property: String): PropertyValue<Expression> = when (property) {
        Circle.PROPERTY_CIRCLE_RADIUS -> PropertyFactory.circleRadius(
            Expression.get(Circle.PROPERTY_CIRCLE_RADIUS)
        )

        Circle.PROPERTY_CIRCLE_COLOR -> PropertyFactory.circleColor(
            Expression.get(Circle.PROPERTY_CIRCLE_COLOR)
        )

        Circle.PROPERTY_CIRCLE_BLUR -> PropertyFactory.circleBlur(
            Expression.get(Circle.PROPERTY_CIRCLE_BLUR)
        )

        Circle.PROPERTY_CIRCLE_OPACITY -> PropertyFactory.circleOpacity(
            Expression.get(Circle.PROPERTY_CIRCLE_OPACITY)
        )

        Circle.PROPERTY_CIRCLE_STROKE_WIDTH -> PropertyFactory.circleStrokeWidth(
            Expression.get(Circle.PROPERTY_CIRCLE_STROKE_WIDTH)

        )

        Circle.PROPERTY_CIRCLE_STROKE_COLOR -> PropertyFactory.circleStrokeColor(
            Expression.get(Circle.PROPERTY_CIRCLE_STROKE_COLOR)
        )

        Circle.PROPERTY_CIRCLE_STROKE_OPACITY -> PropertyFactory.circleStrokeOpacity(
            Expression.get(Circle.PROPERTY_CIRCLE_STROKE_OPACITY)
        )

        else -> throw IllegalArgumentException(
            "$property is not a valid data-driven property for a circle."
        )
    }

    override fun addDragListener(d: OnCircleDragListener) {
        super.addDragListener(d)
    }

    override fun removeDragListener(d: OnCircleDragListener) {
        super.removeDragListener(d)
    }

    override fun addClickListener(u: OnCircleClickListener) {
        super.addClickListener(u)
    }

    override fun removeClickListener(u: OnCircleClickListener) {
        super.removeClickListener(u)
    }

    override fun addLongClickListener(v: OnCircleLongClickListener) {
        super.addLongClickListener(v)
    }

    override fun removeLongClickListener(v: OnCircleLongClickListener) {
        super.removeLongClickListener(v)
    }

    // Property accessors
    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     */
    var circleTranslate: Array<Float?>?
        get() = layer.circleTranslate.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.circleTranslate(value)
            constantPropertyUsageMap[PROPERTY_CIRCLE_TRANSLATE] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Controls the frame of reference for [PropertyFactory.circleTranslate].
     */
    var circleTranslateAnchor: String
        get() = layer.circleTranslateAnchor.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.circleTranslateAnchor(value)
            constantPropertyUsageMap[PROPERTY_CIRCLE_TRANSLATE_ANCHOR] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Controls the scaling behavior of the circle when the map is pitched.
     */
    var circlePitchScale: String?
        get() = layer.circlePitchScale.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.circlePitchScale(value)
            constantPropertyUsageMap[PROPERTY_CIRCLE_PITCH_SCALE] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Orientation of circle when map is pitched.
     */
    var circlePitchAlignment: String
        get() = layer.circlePitchAlignment.value
        set(value) {
            val propertyValue: PropertyValue<*> = PropertyFactory.circlePitchAlignment(value)
            constantPropertyUsageMap[PROPERTY_CIRCLE_PITCH_ALIGNMENT] = propertyValue
            layer.setProperties(propertyValue)
        }

    /**
     * Set filter on the managed circles.
     */
    override fun setFilter(expression: Expression) {
        layerFilter = expression
        layer.setFilter(expression)
    }

    val filter: Expression?
        /**
         * Get filter of the managed circles.
         */
        get() {
            return layer.filter
        }

    companion object {
        private const val PROPERTY_CIRCLE_TRANSLATE: String = "circle-translate"
        private const val PROPERTY_CIRCLE_TRANSLATE_ANCHOR: String = "circle-translate-anchor"
        private const val PROPERTY_CIRCLE_PITCH_SCALE: String = "circle-pitch-scale"
        private const val PROPERTY_CIRCLE_PITCH_ALIGNMENT: String = "circle-pitch-alignment"
    }
}
