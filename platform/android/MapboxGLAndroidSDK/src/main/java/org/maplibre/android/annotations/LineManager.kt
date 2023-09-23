//package org.maplibre.android.annotations
//
//import androidx.annotation.UiThread
//import com.mapbox.geojson.FeatureCollection
//import org.maplibre.android.maps.MapLibreMap
//import org.maplibre.android.maps.MapView
//import org.maplibre.android.maps.Style
//import org.maplibre.android.style.expressions.Expression
//import org.maplibre.android.style.layers.LineLayer
//import org.maplibre.android.style.layers.PropertyFactory
//import org.maplibre.android.style.layers.PropertyValue
//import org.maplibre.android.style.sources.GeoJsonOptions
//
///**
// * The line manager allows to add lines to a map.
// *
// * @param maplibreMap      the map object to add lines to
// * @param style          a valid a fully loaded style object
// * @param belowLayerId   the id of the layer above the line layer
// * @param aboveLayerId   the id of the layer below the line layer
// * @param geoJsonOptions options for the internal source
// */
//class LineManager @UiThread internal constructor(
//    mapView: MapView,
//    maplibreMap: MapLibreMap,
//    style: Style,
//    coreElementProvider: CoreElementProvider<LineLayer> = LineElementProvider(),
//    belowLayerId: String? = null,
//    aboveLayerId: String? = null,
//    geoJsonOptions: GeoJsonOptions? = null,
//    draggableAnnotationController: DraggableAnnotationController =
//        DraggableAnnotationController.getInstance(mapView, maplibreMap)
//) : AnnotationManager<LineLayer, Line, LineOptions, OnLineDragListener, OnLineClickListener, OnLineLongClickListener>(
//    mapView,
//    maplibreMap,
//    style,
//    coreElementProvider,
//    draggableAnnotationController,
//    belowLayerId,
//    aboveLayerId,
//    geoJsonOptions
//) {
//
//    constructor(
//        mapView: MapView,
//        maplibreMap: MapLibreMap,
//        style: Style,
//        belowLayerId: String? = null,
//        aboveLayerId: String? = null,
//        geoJsonOptions: GeoJsonOptions? = null
//    ) : this(
//        mapView,
//        maplibreMap,
//        style,
//        LineElementProvider(),
//        belowLayerId,
//        aboveLayerId,
//        geoJsonOptions
//    )
//
//    override fun initializeDataDrivenPropertyMap() =
//        listOf(
//            LineOptions.PROPERTY_LINE_JOIN,
//            LineOptions.PROPERTY_LINE_OPACITY,
//            LineOptions.PROPERTY_LINE_COLOR,
//            LineOptions.PROPERTY_LINE_WIDTH,
//            LineOptions.PROPERTY_LINE_GAP_WIDTH,
//            LineOptions.PROPERTY_LINE_OFFSET,
//            LineOptions.PROPERTY_LINE_BLUR,
//            LineOptions.PROPERTY_LINE_PATTERN
//        ).associateWith { false }.let { dataDrivenPropertyUsageMap.putAll(it) }
//
//    override fun setDataDrivenPropertyIsUsed(property: String) {
//        when (property) {
//            LineOptions.PROPERTY_LINE_JOIN -> layer.setProperties(
//                PropertyFactory.lineJoin(
//                    Expression.get(LineOptions.PROPERTY_LINE_JOIN)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_OPACITY -> layer.setProperties(
//                PropertyFactory.lineOpacity(
//                    Expression.get(LineOptions.PROPERTY_LINE_OPACITY)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_COLOR -> layer.setProperties(
//                PropertyFactory.lineColor(
//                    Expression.get(LineOptions.PROPERTY_LINE_COLOR)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_WIDTH -> layer.setProperties(
//                PropertyFactory.lineWidth(
//                    Expression.get(LineOptions.PROPERTY_LINE_WIDTH)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_GAP_WIDTH -> layer.setProperties(
//                PropertyFactory.lineGapWidth(
//                    Expression.get(LineOptions.PROPERTY_LINE_GAP_WIDTH)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_OFFSET -> layer.setProperties(
//                PropertyFactory.lineOffset(
//                    Expression.get(LineOptions.PROPERTY_LINE_OFFSET)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_BLUR -> layer.setProperties(
//                PropertyFactory.lineBlur(
//                    Expression.get(LineOptions.PROPERTY_LINE_BLUR)
//                )
//            )
//
//            LineOptions.PROPERTY_LINE_PATTERN -> layer.setProperties(
//                PropertyFactory.linePattern(
//                    Expression.get(LineOptions.PROPERTY_LINE_PATTERN)
//                )
//            )
//        }
//    }
//
//    /**
//     * Create a list of lines on the map.
//     *
//     *
//     * Lines are going to be created only for features with a matching geometry.
//     *
//     *
//     * All supported properties are:<br></br>
//     * LineOptions.PROPERTY_LINE_JOIN - String<br></br>
//     * LineOptions.PROPERTY_LINE_OPACITY - Float<br></br>
//     * LineOptions.PROPERTY_LINE_COLOR - String<br></br>
//     * LineOptions.PROPERTY_LINE_WIDTH - Float<br></br>
//     * LineOptions.PROPERTY_LINE_GAP_WIDTH - Float<br></br>
//     * LineOptions.PROPERTY_LINE_OFFSET - Float<br></br>
//     * LineOptions.PROPERTY_LINE_BLUR - Float<br></br>
//     * LineOptions.PROPERTY_LINE_PATTERN - String<br></br>
//     * Learn more about above properties in the [Style specification](https://maplibre.org/maplibre-style-spec/).
//     *
//     *
//     * Out of spec properties:<br></br>
//     * "is-draggable" - Boolean, true if the line should be draggable, false otherwise
//     *
//     * @param json the GeoJSON defining the list of lines to build
//     * @return the list of built lines
//     */
//    @UiThread
//    fun create(json: String): List<Line> = create(FeatureCollection.fromJson(json))
//
//    /**
//     * Create a list of lines on the map.
//     *
//     *
//     * Lines are going to be created only for features with a matching geometry.
//     *
//     *
//     * All supported properties are:<br></br>
//     * LineOptions.PROPERTY_LINE_JOIN - String<br></br>
//     * LineOptions.PROPERTY_LINE_OPACITY - Float<br></br>
//     * LineOptions.PROPERTY_LINE_COLOR - String<br></br>
//     * LineOptions.PROPERTY_LINE_WIDTH - Float<br></br>
//     * LineOptions.PROPERTY_LINE_GAP_WIDTH - Float<br></br>
//     * LineOptions.PROPERTY_LINE_OFFSET - Float<br></br>
//     * LineOptions.PROPERTY_LINE_BLUR - Float<br></br>
//     * LineOptions.PROPERTY_LINE_PATTERN - String<br></br>
//     * Learn more about above properties in the [Style specification](https://maplibre.org/maplibre-style-spec/).
//     *
//     *
//     * Out of spec properties:<br></br>
//     * "is-draggable" - Boolean, true if the line should be draggable, false otherwise
//     *
//     * @param featureCollection the featureCollection defining the list of lines to build
//     * @return the list of built lines
//     */
//    @UiThread
//    fun create(featureCollection: FeatureCollection): List<Line> =
//        featureCollection.features()?.mapNotNull { LineOptions.fromFeature(it) }
//            .let { create(it ?: emptyList()) }
//
//    /**
//     * Key of the id of the annotation
//     */
//    override val annotationIdKey: String
//        get() = Line.ID_KEY
//
//    // Property accessors
//    /**
//     * The display of line endings.
//     */
//    var lineCap: String?
//        get() = layer.lineCap.value
//        set(value) {
//            val propertyValue: PropertyValue<*> = PropertyFactory.lineCap(value)
//            constantPropertyUsageMap[PROPERTY_LINE_CAP] = propertyValue
//            layer.setProperties(propertyValue)
//        }
//
//    /**
//     * Used to automatically convert miter joins to bevel joins for sharp angles.
//     */
//    var lineMiterLimit: Float?
//        get() = layer.lineMiterLimit.value
//        set(value) {
//            val propertyValue: PropertyValue<*> = PropertyFactory.lineMiterLimit(value)
//            constantPropertyUsageMap[PROPERTY_LINE_MITER_LIMIT] = propertyValue
//            layer.setProperties(propertyValue)
//        }
//
//    /**
//     * Used to automatically convert round joins to miter joins for shallow angles.
//     */
//    var lineRoundLimit: Float?
//        get() = layer.lineRoundLimit.value
//        set(value) {
//            val propertyValue: PropertyValue<*> = PropertyFactory.lineRoundLimit(value)
//            constantPropertyUsageMap[PROPERTY_LINE_ROUND_LIMIT] = propertyValue
//            layer.setProperties(propertyValue)
//        }
//
//    /**
//     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
//     */
//    var lineTranslate: Array<Float?>?
//        get() = layer.lineTranslate.value
//        set(value) {
//            val propertyValue: PropertyValue<*> = PropertyFactory.lineTranslate(value)
//            constantPropertyUsageMap[PROPERTY_LINE_TRANSLATE] = propertyValue
//            layer.setProperties(propertyValue)
//        }
//
//    /**
//     * Controls the frame of reference for [PropertyFactory.lineTranslate].
//     */
//    var lineTranslateAnchor: String?
//        get() = layer.lineTranslateAnchor.value
//        set(value) {
//            val propertyValue: PropertyValue<*> = PropertyFactory.lineTranslateAnchor(value)
//            constantPropertyUsageMap[PROPERTY_LINE_TRANSLATE_ANCHOR] = propertyValue
//            layer.setProperties(propertyValue)
//        }
//
//    /**
//     * Specifies the lengths of the alternating dashes and gaps that form the dash pattern. The lengths are later scaled by the line width. To convert a dash length to density-independent pixels, multiply the length by the current line width. Note that GeoJSON sources with `lineMetrics: true` specified won't render dashed lines to the expected scale. Also note that zoom-dependent expressions will be evaluated only at integer zoom levels.
//     */
//    var lineDasharray: Array<Float?>?
//        get() = layer.lineDasharray.value
//        set(value) {
//            val propertyValue: PropertyValue<*> = PropertyFactory.lineDasharray(value)
//            constantPropertyUsageMap[PROPERTY_LINE_DASHARRAY] = propertyValue
//            layer.setProperties(propertyValue)
//        }
//
//    /**
//     * Set filter on the managed lines.
//     */
//    override fun setFilter(expression: Expression) {
//        layerFilter = expression
//        layer.setFilter(expression)
//    }
//
//    val filter: Expression?
//        /**
//         * Get filter of the managed lines.
//         */
//        get() = layer.filter
//
//    companion object {
//        private const val PROPERTY_LINE_CAP: String = "line-cap"
//        private const val PROPERTY_LINE_MITER_LIMIT: String = "line-miter-limit"
//        private const val PROPERTY_LINE_ROUND_LIMIT: String = "line-round-limit"
//        private const val PROPERTY_LINE_TRANSLATE: String = "line-translate"
//        private const val PROPERTY_LINE_TRANSLATE_ANCHOR: String = "line-translate-anchor"
//        private const val PROPERTY_LINE_DASHARRAY: String = "line-dasharray"
//    }
//}
