package org.maplibre.android.location

import org.maplibre.android.location.LocationComponentConstants.ACCURACY_LAYER
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_LAYER
import org.maplibre.android.location.LocationComponentConstants.BEARING_LAYER
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_LAYER
import org.maplibre.android.location.LocationComponentConstants.LOCATION_SOURCE
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_ACCURACY_ALPHA
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_ACCURACY_COLOR
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_ACCURACY_RADIUS
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_BACKGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_BACKGROUND_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_BEARING_ICON
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_COMPASS_BEARING
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_FOREGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_FOREGROUND_ICON_OFFSET
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_FOREGROUND_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_GPS_BEARING
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_LOCATION_STALE
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_SHADOW_ICON_OFFSET
import org.maplibre.android.location.LocationComponentConstants.PULSING_CIRCLE_LAYER
import org.maplibre.android.location.LocationComponentConstants.SHADOW_ICON
import org.maplibre.android.location.LocationComponentConstants.SHADOW_LAYER
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.geojson.Feature

internal class LayerSourceProvider {
    fun generateSource(
        locationFeature: Feature?,
        synchronousUpdate: Boolean = false,
    ): GeoJsonSource =
        GeoJsonSource(
            LOCATION_SOURCE,
            locationFeature,
            GeoJsonOptions().withMaxZoom(16).withSynchronousUpdate(synchronousUpdate),
        )

    fun generateLayer(layerId: String): Layer {
        val layer = SymbolLayer(layerId, LOCATION_SOURCE)
        layer.setProperties(
            PropertyFactory.iconAllowOverlap(true),
            PropertyFactory.iconIgnorePlacement(true),
            PropertyFactory.iconRotationAlignment(Property.ICON_ROTATION_ALIGNMENT_MAP),
            PropertyFactory.iconRotate(
                Expression.match(
                    Expression.literal(layerId),
                    Expression.literal(0f),
                    Expression.stop(FOREGROUND_LAYER, Expression.get(PROPERTY_GPS_BEARING)),
                    Expression.stop(BACKGROUND_LAYER, Expression.get(PROPERTY_GPS_BEARING)),
                    Expression.stop(SHADOW_LAYER, Expression.get(PROPERTY_GPS_BEARING)),
                    Expression.stop(BEARING_LAYER, Expression.get(PROPERTY_COMPASS_BEARING)),
                ),
            ),
            PropertyFactory.iconImage(
                Expression.match(
                    Expression.literal(layerId),
                    Expression.literal(EMPTY_STRING),
                    Expression.stop(
                        FOREGROUND_LAYER,
                        Expression.switchCase(
                            Expression.get(PROPERTY_LOCATION_STALE),
                            Expression.get(PROPERTY_FOREGROUND_STALE_ICON),
                            Expression.get(PROPERTY_FOREGROUND_ICON),
                        ),
                    ),
                    Expression.stop(
                        BACKGROUND_LAYER,
                        Expression.switchCase(
                            Expression.get(PROPERTY_LOCATION_STALE),
                            Expression.get(PROPERTY_BACKGROUND_STALE_ICON),
                            Expression.get(PROPERTY_BACKGROUND_ICON),
                        ),
                    ),
                    Expression.stop(SHADOW_LAYER, Expression.literal(SHADOW_ICON)),
                    Expression.stop(BEARING_LAYER, Expression.get(PROPERTY_BEARING_ICON)),
                ),
            ),
            PropertyFactory.iconOffset(
                Expression.match(
                    Expression.literal(layerId),
                    Expression.literal(arrayOf(0f, 0f)),
                    Expression.stop(
                        Expression.literal(FOREGROUND_LAYER),
                        Expression.get(PROPERTY_FOREGROUND_ICON_OFFSET),
                    ),
                    Expression.stop(
                        Expression.literal(SHADOW_LAYER),
                        Expression.get(PROPERTY_SHADOW_ICON_OFFSET),
                    ),
                ),
            ),
        )
        return layer
    }

    fun generateAccuracyLayer(): Layer =
        CircleLayer(ACCURACY_LAYER, LOCATION_SOURCE)
            .withProperties(
                PropertyFactory.circleRadius(Expression.get(PROPERTY_ACCURACY_RADIUS)),
                PropertyFactory.circleColor(Expression.get(PROPERTY_ACCURACY_COLOR)),
                PropertyFactory.circleOpacity(Expression.get(PROPERTY_ACCURACY_ALPHA)),
                PropertyFactory.circleStrokeColor(Expression.get(PROPERTY_ACCURACY_COLOR)),
                PropertyFactory.circlePitchAlignment(Property.CIRCLE_PITCH_ALIGNMENT_MAP),
            )

    val emptyLayerSet: MutableSet<String>
        get() = HashSet()

    fun getSymbolLocationLayerRenderer(
        featureProvider: LayerFeatureProvider,
        isStale: Boolean,
    ): LocationLayerRenderer = SymbolLocationLayerRenderer(this, featureProvider, isStale)

    val indicatorLocationLayerRenderer: LocationLayerRenderer
        get() = IndicatorLocationLayerRenderer(this)

    fun generateLocationComponentLayer(): Layer {
        val layer = LocationIndicatorLayer(FOREGROUND_LAYER)
        layer.locationTransition = TransitionOptions(0, 0)
        layer.setProperties(
            LocationPropertyFactory.perspectiveCompensation(0.9f),
            LocationPropertyFactory.imageTiltDisplacement(4f),
        )
        return layer
    }

    /**
     * Adds a [CircleLayer] to the map to support the [LocationComponent] pulsing UI functionality.
     *
     * @return a [CircleLayer] with the correct data-driven styling. Tilting the map will keep the pulsing
     * layer aligned with the map plane.
     */
    fun generatePulsingCircleLayer(): Layer =
        CircleLayer(PULSING_CIRCLE_LAYER, LOCATION_SOURCE)
            .withProperties(
                PropertyFactory.circlePitchAlignment(Property.CIRCLE_PITCH_ALIGNMENT_MAP),
            )

    private companion object {
        private const val EMPTY_STRING = ""
    }
}
