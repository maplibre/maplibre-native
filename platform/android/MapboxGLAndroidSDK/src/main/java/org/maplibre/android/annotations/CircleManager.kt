package org.maplibre.android.annotations

import androidx.annotation.UiThread
import com.mapbox.geojson.FeatureCollection
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
 */
class CircleManager @UiThread internal constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap,
    style: Style,
    coreElementProvider: CoreElementProvider<CircleLayer>,
    belowLayerId: String?,
    aboveLayerId: String?,
    geoJsonOptions: GeoJsonOptions?,
    draggableAnnotationController: DraggableAnnotationController
) : AnnotationManager<CircleLayer, Circle, CircleOptions, OnCircleDragListener, OnCircleClickListener, OnCircleLongClickListener>(
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
     * Create a circle manager, used to manage circles.
     *
     * @param maplibreMap the map object to add circles to
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
     * Create a circle manager, used to manage circles.
     *
     * @param maplibreMap    the map object to add circles to
     * @param style        a valid a fully loaded style object
     * @param belowLayerId the id of the layer above the circle layer
     * @param aboveLayerId the id of the layer below the circle layer
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
     * Create a circle manager, used to manage circles.
     *
     * @param maplibreMap      the map object to add circles to
     * @param style          a valid a fully loaded style object
     * @param belowLayerId   the id of the layer above the circle layer
     * @param aboveLayerId   the id of the layer below the circle layer
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
        CircleElementProvider(),
        belowLayerId,
        aboveLayerId,
        geoJsonOptions,
        DraggableAnnotationController.getInstance(mapView, maplibreMap)
    )

    public override fun initializeDataDrivenPropertyMap() {
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_RADIUS] = false
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_COLOR] = false
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_BLUR] = false
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_OPACITY] = false
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_STROKE_WIDTH] = false
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_STROKE_COLOR] = false
        dataDrivenPropertyUsageMap[CircleOptions.PROPERTY_CIRCLE_STROKE_OPACITY] = false
    }

    override fun setDataDrivenPropertyIsUsed(property: String) {
        when (property) {
            CircleOptions.PROPERTY_CIRCLE_RADIUS -> layer.setProperties(
                PropertyFactory.circleRadius(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_RADIUS)
                )
            )

            CircleOptions.PROPERTY_CIRCLE_COLOR -> layer.setProperties(
                PropertyFactory.circleColor(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_COLOR)
                )
            )

            CircleOptions.PROPERTY_CIRCLE_BLUR -> layer.setProperties(
                PropertyFactory.circleBlur(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_BLUR)
                )
            )

            CircleOptions.PROPERTY_CIRCLE_OPACITY -> layer.setProperties(
                PropertyFactory.circleOpacity(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_OPACITY)
                )
            )

            CircleOptions.PROPERTY_CIRCLE_STROKE_WIDTH -> layer.setProperties(
                PropertyFactory.circleStrokeWidth(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_STROKE_WIDTH)
                )
            )

            CircleOptions.PROPERTY_CIRCLE_STROKE_COLOR -> layer.setProperties(
                PropertyFactory.circleStrokeColor(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_STROKE_COLOR)
                )
            )

            CircleOptions.PROPERTY_CIRCLE_STROKE_OPACITY -> layer.setProperties(
                PropertyFactory.circleStrokeOpacity(
                    Expression.get(CircleOptions.PROPERTY_CIRCLE_STROKE_OPACITY)
                )
            )
        }
    }

    /**
     * Create a list of circles on the map.
     *
     *
     * Circles are going to be created only for features with a matching geometry.
     *
     *
     * All supported properties are:<br></br>
     * CircleOptions.PROPERTY_CIRCLE_RADIUS - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_COLOR - String<br></br>
     * CircleOptions.PROPERTY_CIRCLE_BLUR - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_OPACITY - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_STROKE_WIDTH - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_STROKE_COLOR - String<br></br>
     * CircleOptions.PROPERTY_CIRCLE_STROKE_OPACITY - Float<br></br>
     * Learn more about above properties in the [Style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/).
     *
     *
     * Out of spec properties:<br></br>
     * "is-draggable" - Boolean, true if the circle should be draggable, false otherwise
     *
     * @param json the GeoJSON defining the list of circles to build
     * @return the list of built circles
     */
    @UiThread
    fun create(json: String): List<Circle?> =
        create(FeatureCollection.fromJson(json))

    /**
     * Create a list of circles on the map.
     *
     *
     * Circles are going to be created only for features with a matching geometry.
     *
     *
     * All supported properties are:<br></br>
     * CircleOptions.PROPERTY_CIRCLE_RADIUS - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_COLOR - String<br></br>
     * CircleOptions.PROPERTY_CIRCLE_BLUR - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_OPACITY - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_STROKE_WIDTH - Float<br></br>
     * CircleOptions.PROPERTY_CIRCLE_STROKE_COLOR - String<br></br>
     * CircleOptions.PROPERTY_CIRCLE_STROKE_OPACITY - Float<br></br>
     * Learn more about above properties in the [Style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/).
     *
     *
     * Out of spec properties:<br></br>
     * "is-draggable" - Boolean, true if the circle should be draggable, false otherwise
     *
     * @param featureCollection the featureCollection defining the list of circles to build
     * @return the list of built circles
     */
    @UiThread
    fun create(featureCollection: FeatureCollection): List<Circle> =
        featureCollection.features()?.mapNotNull { CircleOptions.fromFeature(it) }
            .let { create(it ?: emptyList()) }

    /**
     * Key of the id of the annotation
     */
    override val annotationIdKey: String
        get() = Circle.ID_KEY

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
