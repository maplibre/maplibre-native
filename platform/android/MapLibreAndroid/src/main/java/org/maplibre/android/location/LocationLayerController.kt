package org.maplibre.android.location

import android.graphics.Bitmap
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_LAYER
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.BEARING_ICON
import org.maplibre.android.location.LocationComponentConstants.BEARING_LAYER
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_LAYER
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_STALE_ICON
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.Companion.interpolate
import org.maplibre.android.style.expressions.Expression.Companion.linear
import org.maplibre.android.style.expressions.Expression.Companion.stop
import org.maplibre.android.style.expressions.Expression.Companion.zoom

internal class LocationLayerController(
    private val maplibreMap: MapLibreMap,
    style: Style,
    layerSourceProvider: LayerSourceProvider,
    featureProvider: LayerFeatureProvider,
    private val bitmapProvider: LayerBitmapProvider,
    initialOptions: LocationComponentOptions,
    private val internalRenderModeChangedListener: OnRenderModeChangedListener,
    private val useSpecializedLocationLayer: Boolean,
) {
    private var options: LocationComponentOptions = initialOptions

    var isHidden = true
        private set

    private var isStale: Boolean = initialOptions.enableStaleState()

    private lateinit var positionManager: LocationComponentPositionManager

    private val locationLayerRenderer: LocationLayerRenderer =
        if (useSpecializedLocationLayer) {
            layerSourceProvider.indicatorLocationLayerRenderer
        } else {
            layerSourceProvider.getSymbolLocationLayerRenderer(featureProvider, isStale)
        }

    init {
        initializeComponents(style, initialOptions)
    }

    fun initializeComponents(
        style: Style,
        options: LocationComponentOptions,
    ) {
        positionManager =
            LocationComponentPositionManager(
                style,
                options.layerAbove(),
                options.layerBelow(),
                options.bearingOnTop(),
            )
        locationLayerRenderer.initializeComponents(style)
        locationLayerRenderer.addLayers(positionManager)
        applyStyle(options)

        if (isHidden) {
            hide()
        } else {
            show()
        }
    }

    fun applyStyle(options: LocationComponentOptions) {
        if (positionManager.update(options.layerAbove(), options.layerBelow(), options.bearingOnTop())) {
            locationLayerRenderer.removeLayers()
            locationLayerRenderer.addLayers(positionManager)
            if (isHidden) {
                hide()
            }
        }

        this.options = options
        styleBitmaps(options)
        locationLayerRenderer.styleAccuracy(options.accuracyAlpha(), options.accuracyColor())
        styleScaling(options)
        locationLayerRenderer.stylePulsingCircle(options)
        determineIconsSource(options)

        if (!isHidden) {
            show()
        }
    }

    fun setGpsBearing(gpsBearing: Float) {
        locationLayerRenderer.setGpsBearing(gpsBearing)
    }

    @get:RenderMode.Mode
    var renderMode: Int = 0
        set(
            @RenderMode.Mode value,
        ) {
            if (field == value) {
                return
            }
            field = value

            styleBitmaps(options)
            determineIconsSource(options)
            if (!isHidden) {
                show()
            }
            internalRenderModeChangedListener.onRenderModeChanged(value)
        }

    //
    // Layer action
    //

    fun show() {
        isHidden = false
        locationLayerRenderer.show(renderMode, isStale)
    }

    fun hide() {
        isHidden = true
        locationLayerRenderer.hide()
    }

    val isConsumingCompass: Boolean
        get() = renderMode == RenderMode.COMPASS

    //
    // Styling
    //

    private fun styleBitmaps(options: LocationComponentOptions) {
        // shadow
        var shadowBitmap: Bitmap? = null
        if (options.elevation() > 0) {
            // Only add icon elevation if the values greater than 0.
            shadowBitmap = bitmapProvider.generateShadowBitmap(options)
        }

        // background
        val backgroundBitmap =
            bitmapProvider.generateBitmap(
                options.backgroundDrawable(),
                options.backgroundTintColor(),
            )
        val backgroundStaleBitmap =
            bitmapProvider.generateBitmap(
                options.backgroundDrawableStale(),
                options.backgroundStaleTintColor(),
            )

        // compass bearing
        val bearingBitmap = bitmapProvider.generateBitmap(options.bearingDrawable(), options.bearingTintColor())

        // foreground
        var foregroundBitmap =
            bitmapProvider.generateBitmap(
                options.foregroundDrawable(),
                options.foregroundTintColor(),
            )
        var foregroundStaleBitmap =
            bitmapProvider.generateBitmap(
                options.foregroundDrawableStale(),
                options.foregroundStaleTintColor(),
            )
        if (renderMode == RenderMode.GPS) {
            foregroundBitmap = bitmapProvider.generateBitmap(options.gpsDrawable(), options.foregroundTintColor())
            foregroundStaleBitmap =
                bitmapProvider.generateBitmap(
                    options.gpsDrawable(),
                    options.foregroundStaleTintColor(),
                )
        }

        locationLayerRenderer.addBitmaps(
            renderMode,
            shadowBitmap,
            backgroundBitmap,
            backgroundStaleBitmap,
            bearingBitmap,
            foregroundBitmap,
            foregroundStaleBitmap,
        )
    }

    private fun styleScaling(options: LocationComponentOptions) {
        val scaleExpression =
            interpolate(
                linear(),
                zoom(),
                stop(maplibreMap.minZoomLevel, options.minZoomIconScale()),
                stop(maplibreMap.maxZoomLevel, options.maxZoomIconScale()),
            )

        locationLayerRenderer.styleScaling(scaleExpression)
    }

    private fun determineIconsSource(options: LocationComponentOptions) {
        val foregroundIconString =
            buildIconString(
                if (renderMode == RenderMode.GPS) options.gpsName() else options.foregroundName(),
                FOREGROUND_ICON,
            )
        val foregroundStaleIconString = buildIconString(options.foregroundStaleName(), FOREGROUND_STALE_ICON)
        val backgroundIconString = buildIconString(options.backgroundName(), BACKGROUND_ICON)
        val backgroundStaleIconString = buildIconString(options.backgroundStaleName(), BACKGROUND_STALE_ICON)
        val bearingIconString = buildIconString(options.bearingName(), BEARING_ICON)

        locationLayerRenderer.updateIconIds(
            foregroundIconString,
            foregroundStaleIconString,
            backgroundIconString,
            backgroundStaleIconString,
            bearingIconString,
        )
    }

    private fun buildIconString(
        bitmapName: String?,
        drawableName: String,
    ): String {
        if (bitmapName != null) {
            if (useSpecializedLocationLayer) {
                Logger.e(TAG, "$bitmapName replacement ID provided for an unsupported specialized location layer")
                return drawableName
            }
            return bitmapName
        }
        return drawableName
    }

    fun setLocationsStale(isStale: Boolean) {
        this.isStale = isStale
        locationLayerRenderer.setLocationStale(isStale, renderMode)
    }

    //
    // Map click event
    //

    fun onMapClick(point: LatLng): Boolean {
        val screenLoc = maplibreMap.projection.toScreenLocation(point)
        val features =
            maplibreMap.queryRenderedFeatures(
                screenLoc,
                BACKGROUND_LAYER,
                FOREGROUND_LAYER,
                BEARING_LAYER,
            )
        return features.isNotEmpty()
    }

    private val latLngValueListener =
        AnimationsValueChangeListener<LatLng> { value ->
            locationLayerRenderer.setLatLng(value)
        }

    private val gpsBearingValueListener =
        AnimationsValueChangeListener<Float> { value ->
            locationLayerRenderer.setGpsBearing(value)
        }

    private val compassBearingValueListener =
        AnimationsValueChangeListener<Float> { value ->
            locationLayerRenderer.setCompassBearing(value)
        }

    private val accuracyValueListener =
        AnimationsValueChangeListener<Float> { value ->
            locationLayerRenderer.setAccuracyRadius(value)
        }

    /**
     * The listener that handles the updating of the pulsing circle's radius and opacity.
     */
    private val pulsingCircleRadiusListener =
        AnimationsValueChangeListener<Float> { newPulseRadiusValue ->
            var newPulseOpacityValue: Float? = null
            if (options.pulseFadeEnabled() == true) {
                newPulseOpacityValue = 1f - ((newPulseRadiusValue / 100) * 3)
            }
            locationLayerRenderer.updatePulsingUi(newPulseRadiusValue, newPulseOpacityValue)
        }

    val animationListeners: Set<AnimatorListenerHolder>
        get() {
            val holders = mutableSetOf<AnimatorListenerHolder>()
            holders.add(AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_LATLNG, latLngValueListener))

            if (renderMode == RenderMode.GPS) {
                holders.add(
                    AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING, gpsBearingValueListener),
                )
            } else if (renderMode == RenderMode.COMPASS) {
                holders.add(
                    AnimatorListenerHolder(
                        MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
                        compassBearingValueListener,
                    ),
                )
            }

            if (renderMode == RenderMode.COMPASS || renderMode == RenderMode.NORMAL) {
                holders.add(AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_LAYER_ACCURACY, accuracyValueListener))
            }

            if (options.pulseEnabled() == true) {
                holders.add(
                    AnimatorListenerHolder(MapLibreAnimator.ANIMATOR_PULSING_CIRCLE, pulsingCircleRadiusListener),
                )
            }
            return holders
        }

    fun cameraBearingUpdated(bearing: Double) {
        if (renderMode != RenderMode.GPS) {
            locationLayerRenderer.cameraBearingUpdated(bearing)
        }
    }

    fun cameraTiltUpdated(tilt: Double) {
        locationLayerRenderer.cameraTiltUpdated(tilt)
    }

    fun adjustPulsingCircleLayerVisibility(visible: Boolean) {
        locationLayerRenderer.adjustPulsingCircleLayerVisibility(visible)
    }

    private companion object {
        private const val TAG = "Mbgl-LocationLayerController"
    }
}
