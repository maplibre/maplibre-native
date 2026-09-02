package org.maplibre.android.location

import android.graphics.Bitmap
import com.google.gson.JsonArray
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.ACCURACY_LAYER
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_LAYER
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.BEARING_ICON
import org.maplibre.android.location.LocationComponentConstants.BEARING_LAYER
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_LAYER
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_STALE_ICON
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
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_PULSING_OPACITY
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_PULSING_RADIUS
import org.maplibre.android.location.LocationComponentConstants.PROPERTY_SHADOW_ICON_OFFSET
import org.maplibre.android.location.LocationComponentConstants.PULSING_CIRCLE_LAYER
import org.maplibre.android.location.LocationComponentConstants.SHADOW_ICON
import org.maplibre.android.location.LocationComponentConstants.SHADOW_LAYER
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.Companion.get
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.Property.NONE
import org.maplibre.android.style.layers.Property.VISIBLE
import org.maplibre.android.style.layers.PropertyFactory.circleColor
import org.maplibre.android.style.layers.PropertyFactory.circleOpacity
import org.maplibre.android.style.layers.PropertyFactory.circleRadius
import org.maplibre.android.style.layers.PropertyFactory.circleStrokeColor
import org.maplibre.android.style.layers.PropertyFactory.iconSize
import org.maplibre.android.style.layers.PropertyFactory.visibility
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.utils.ColorUtils.colorToRgbaString
import org.maplibre.geojson.Feature
import org.maplibre.geojson.Point

internal class SymbolLocationLayerRenderer(
    private val layerSourceProvider: LayerSourceProvider,
    featureProvider: LayerFeatureProvider,
    isStale: Boolean,
) : LocationLayerRenderer {
    private lateinit var style: Style

    private val layerSet: MutableSet<String> = layerSourceProvider.emptyLayerSet
    private var locationFeature: Feature = featureProvider.generateLocationFeature(null, isStale)
    private lateinit var locationSource: GeoJsonSource

    override fun initializeComponents(style: Style) {
        this.style = style
        addLocationSource()
    }

    override fun addLayers(positionManager: LocationComponentPositionManager) {
        if (positionManager.bearingOnTop) {
            // positions the top-most reference layer
            val layer = layerSourceProvider.generateLayer(BEARING_LAYER)
            positionManager.addLayerToMap(layer)
            layerSet.add(layer.id)

            // adds remaining layers while keeping the order
            addSymbolLayer(FOREGROUND_LAYER, BEARING_LAYER)
            addSymbolLayer(BACKGROUND_LAYER, FOREGROUND_LAYER)
            addSymbolLayer(SHADOW_LAYER, BACKGROUND_LAYER)
        } else {
            // positions the top-most reference layer
            val layer = layerSourceProvider.generateLayer(FOREGROUND_LAYER)
            positionManager.addLayerToMap(layer)
            layerSet.add(layer.id)

            // adds remaining layers while keeping the order
            addSymbolLayer(BACKGROUND_LAYER, FOREGROUND_LAYER)
            addSymbolLayer(BEARING_LAYER, BACKGROUND_LAYER)
            addSymbolLayer(SHADOW_LAYER, BEARING_LAYER)
        }

        addAccuracyLayer()
        addPulsingCircleLayerToMap()
    }

    override fun removeLayers() {
        for (layerId in layerSet) {
            style.removeLayer(layerId)
        }
        layerSet.clear()
    }

    override fun hide() {
        for (layerId in layerSet) {
            setLayerVisibility(layerId, false)
        }
    }

    override fun cameraTiltUpdated(tilt: Double) {
        updateForegroundOffset(tilt)
    }

    override fun cameraBearingUpdated(bearing: Double) {
        updateForegroundBearing(bearing.toFloat())
    }

    override fun show(
        @RenderMode.Mode renderMode: Int,
        isStale: Boolean,
    ) {
        when (renderMode) {
            RenderMode.NORMAL -> {
                setLayerVisibility(SHADOW_LAYER, true)
                setLayerVisibility(FOREGROUND_LAYER, true)
                setLayerVisibility(BACKGROUND_LAYER, true)
                setLayerVisibility(ACCURACY_LAYER, !isStale)
                setLayerVisibility(BEARING_LAYER, false)
            }

            RenderMode.COMPASS -> {
                setLayerVisibility(SHADOW_LAYER, true)
                setLayerVisibility(FOREGROUND_LAYER, true)
                setLayerVisibility(BACKGROUND_LAYER, true)
                setLayerVisibility(ACCURACY_LAYER, !isStale)
                setLayerVisibility(BEARING_LAYER, true)
            }

            RenderMode.GPS -> {
                setLayerVisibility(SHADOW_LAYER, false)
                setLayerVisibility(FOREGROUND_LAYER, true)
                setLayerVisibility(BACKGROUND_LAYER, true)
                setLayerVisibility(ACCURACY_LAYER, false)
                setLayerVisibility(BEARING_LAYER, false)
            }
        }
    }

    override fun styleAccuracy(
        accuracyAlpha: Float,
        accuracyColor: Int,
    ) {
        locationFeature.addNumberProperty(PROPERTY_ACCURACY_ALPHA, accuracyAlpha)
        locationFeature.addStringProperty(PROPERTY_ACCURACY_COLOR, colorToRgbaString(accuracyColor))
        refreshSource()
    }

    override fun setLatLng(latLng: LatLng) {
        setLocationPoint(Point.fromLngLat(latLng.longitude, latLng.latitude))
    }

    override fun setGpsBearing(gpsBearing: Float) {
        setBearingProperty(PROPERTY_GPS_BEARING, gpsBearing)
    }

    override fun setCompassBearing(compassBearing: Float) {
        setBearingProperty(PROPERTY_COMPASS_BEARING, compassBearing)
    }

    override fun setAccuracyRadius(accuracy: Float) {
        updateAccuracyRadius(accuracy)
    }

    override fun styleScaling(scaleExpression: Expression) {
        for (layerId in layerSet) {
            val layer = style.getLayer(layerId)
            if (layer is SymbolLayer) {
                layer.setProperties(
                    iconSize(scaleExpression),
                )
            }
        }
    }

    override fun setLocationStale(
        isStale: Boolean,
        renderMode: Int,
    ) {
        locationFeature.addBooleanProperty(PROPERTY_LOCATION_STALE, isStale)
        refreshSource()
        if (renderMode != RenderMode.GPS) {
            setLayerVisibility(ACCURACY_LAYER, !isStale)
        }
    }

    override fun updateIconIds(
        foregroundIconString: String,
        foregroundStaleIconString: String,
        backgroundIconString: String,
        backgroundStaleIconString: String,
        bearingIconString: String,
    ) {
        locationFeature.addStringProperty(PROPERTY_FOREGROUND_ICON, foregroundIconString)
        locationFeature.addStringProperty(PROPERTY_BACKGROUND_ICON, backgroundIconString)
        locationFeature.addStringProperty(PROPERTY_FOREGROUND_STALE_ICON, foregroundStaleIconString)
        locationFeature.addStringProperty(PROPERTY_BACKGROUND_STALE_ICON, backgroundStaleIconString)
        locationFeature.addStringProperty(PROPERTY_BEARING_ICON, bearingIconString)
        refreshSource()
    }

    override fun addBitmaps(
        @RenderMode.Mode renderMode: Int,
        shadowBitmap: Bitmap?,
        backgroundBitmap: Bitmap?,
        backgroundStaleBitmap: Bitmap?,
        bearingBitmap: Bitmap?,
        foregroundBitmap: Bitmap?,
        foregroundStaleBitmap: Bitmap?,
    ) {
        updateBitmap(SHADOW_ICON, shadowBitmap)
        updateBitmap(BACKGROUND_ICON, backgroundBitmap)
        updateBitmap(BACKGROUND_STALE_ICON, backgroundStaleBitmap)
        updateBitmap(BEARING_ICON, bearingBitmap)
        updateBitmap(FOREGROUND_ICON, foregroundBitmap)
        updateBitmap(FOREGROUND_STALE_ICON, foregroundStaleBitmap)
    }

    private fun updateBitmap(
        name: String,
        bitmap: Bitmap?,
    ) {
        if (bitmap == null) {
            style.removeImage(name)
        } else {
            style.addImage(name, bitmap)
        }
    }

    private fun updateForegroundOffset(tilt: Double) {
        val foregroundJsonArray = JsonArray()
        foregroundJsonArray.add(0f)
        foregroundJsonArray.add((-0.05 * tilt).toFloat())
        locationFeature.addProperty(PROPERTY_FOREGROUND_ICON_OFFSET, foregroundJsonArray)

        val backgroundJsonArray = JsonArray()
        backgroundJsonArray.add(0f)
        backgroundJsonArray.add((0.05 * tilt).toFloat())
        locationFeature.addProperty(PROPERTY_SHADOW_ICON_OFFSET, backgroundJsonArray)

        refreshSource()
    }

    private fun updateForegroundBearing(bearing: Float) {
        setBearingProperty(PROPERTY_GPS_BEARING, bearing)
    }

    private fun setLayerVisibility(
        layerId: String,
        visible: Boolean,
    ) {
        if (!style.isFullyLoaded) {
            return
        }

        val layer = style.getLayer(layerId)
        if (layer != null) {
            val targetVisibility = if (visible) VISIBLE else NONE
            if (layer.visibility.value != targetVisibility) {
                layer.setProperties(visibility(targetVisibility))
            }
        }
    }

    /**
     * Adjust the visibility of the pulsing LocationComponent circle.
     */
    override fun adjustPulsingCircleLayerVisibility(visible: Boolean) {
        setLayerVisibility(PULSING_CIRCLE_LAYER, visible)
    }

    /**
     * Adjust the the pulsing LocationComponent circle based on the set options.
     */
    override fun stylePulsingCircle(options: LocationComponentOptions) {
        val pulsingCircleLayer = style.getLayer(PULSING_CIRCLE_LAYER)
        if (pulsingCircleLayer != null) {
            setLayerVisibility(PULSING_CIRCLE_LAYER, true)
            pulsingCircleLayer.setProperties(
                circleRadius(get(PROPERTY_PULSING_RADIUS)),
                circleColor(options.pulseColor()!!),
                circleStrokeColor(options.pulseColor()!!),
                circleOpacity(get(PROPERTY_PULSING_OPACITY)),
            )
        }
    }

    /**
     * Adjust the visual appearance of the pulsing LocationComponent circle.
     */
    override fun updatePulsingUi(
        radius: Float,
        opacity: Float?,
    ) {
        locationFeature.addNumberProperty(PROPERTY_PULSING_RADIUS, radius)
        if (opacity != null) {
            locationFeature.addNumberProperty(PROPERTY_PULSING_OPACITY, opacity)
        }
        refreshSource()
    }

    private fun addSymbolLayer(
        layerId: String,
        beforeLayerId: String,
    ) {
        val layer = layerSourceProvider.generateLayer(layerId)
        addLayerToMap(layer, beforeLayerId)
    }

    private fun addAccuracyLayer() {
        val accuracyLayer = layerSourceProvider.generateAccuracyLayer()
        addLayerToMap(accuracyLayer, BACKGROUND_LAYER)
    }

    /**
     * Add the pulsing LocationComponent circle to the map for future use, if need be.
     */
    private fun addPulsingCircleLayerToMap() {
        val pulsingCircleLayer = layerSourceProvider.generatePulsingCircleLayer()
        addLayerToMap(pulsingCircleLayer, ACCURACY_LAYER)
    }

    private fun addLayerToMap(
        layer: Layer,
        idBelowLayer: String,
    ) {
        style.addLayerBelow(layer, idBelowLayer)
        layerSet.add(layer.id)
    }

    private fun addLocationSource() {
        locationSource = layerSourceProvider.generateSource(locationFeature, true)
        style.addSource(locationSource)
    }

    private fun refreshSource() {
        // prevents exception when other style has been set with an update in flight
        // https://github.com/maplibre/maplibre-native/issues/3348
        if (!style.isFullyLoaded) {
            return
        }
        val source = style.getSourceAs<GeoJsonSource>(LOCATION_SOURCE)
        if (source != null) {
            locationSource.setGeoJson(locationFeature)
        }
    }

    private fun setLocationPoint(locationPoint: Point) {
        val properties = locationFeature.properties()
        if (properties != null) {
            locationFeature = Feature.fromGeometry(locationPoint, properties)
            refreshSource()
        }
    }

    private fun setBearingProperty(
        propertyId: String,
        bearing: Float,
    ) {
        locationFeature.addNumberProperty(propertyId, bearing)
        refreshSource()
    }

    private fun updateAccuracyRadius(accuracy: Float) {
        locationFeature.addNumberProperty(PROPERTY_ACCURACY_RADIUS, accuracy)
        refreshSource()
    }
}
