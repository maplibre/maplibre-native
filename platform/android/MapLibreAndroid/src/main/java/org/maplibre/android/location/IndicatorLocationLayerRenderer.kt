package org.maplibre.android.location

import android.graphics.Bitmap
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.BACKGROUND_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.BEARING_ICON
import org.maplibre.android.location.LocationComponentConstants.BEARING_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_ICON
import org.maplibre.android.location.LocationComponentConstants.FOREGROUND_STALE_ICON
import org.maplibre.android.location.LocationComponentConstants.SHADOW_ICON
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.Companion.rgba
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.Property.NONE
import org.maplibre.android.style.layers.Property.VISIBLE
import org.maplibre.android.utils.BitmapUtils
import org.maplibre.android.utils.ColorUtils

internal class IndicatorLocationLayerRenderer(
    private val layerSourceProvider: LayerSourceProvider,
) : LocationLayerRenderer {
    private lateinit var style: Style
    private lateinit var layer: Layer

    private var lastLatLng: LatLng? = null
    private var lastBearing = 0.0
    private var lastAccuracy = 0f

    override fun initializeComponents(style: Style) {
        this.style = style
        layer = layerSourceProvider.generateLocationComponentLayer()
        lastLatLng?.let { setLatLng(it) }
        setLayerBearing(lastBearing)
        setAccuracyRadius(lastAccuracy)
    }

    override fun addLayers(positionManager: LocationComponentPositionManager) {
        positionManager.addLayerToMap(layer)
    }

    override fun removeLayers() {
        style.removeLayer(layer)
    }

    override fun hide() {
        setLayerVisibility(false)
    }

    override fun cameraTiltUpdated(tilt: Double) {
        // ignored
    }

    override fun cameraBearingUpdated(bearing: Double) {
        // ignored
    }

    override fun show(
        @RenderMode.Mode renderMode: Int,
        isStale: Boolean,
    ) {
        setImages(renderMode, isStale)
        setLayerVisibility(true)
    }

    override fun styleAccuracy(
        accuracyAlpha: Float,
        accuracyColor: Int,
    ) {
        val colorArray = ColorUtils.colorToRgbaArray(accuracyColor)
        colorArray[3] = accuracyAlpha
        val rgbaExpression = rgba(colorArray[0], colorArray[1], colorArray[2], colorArray[3])
        layer.setProperties(
            LocationPropertyFactory.accuracyRadiusColor(rgbaExpression),
            LocationPropertyFactory.accuracyRadiusBorderColor(rgbaExpression),
        )
    }

    override fun setLatLng(latLng: LatLng) {
        setLayerLocation(latLng)
    }

    override fun setGpsBearing(gpsBearing: Float) {
        setLayerBearing(gpsBearing.toDouble())
    }

    override fun setCompassBearing(compassBearing: Float) {
        setLayerBearing(compassBearing.toDouble())
    }

    override fun setAccuracyRadius(accuracy: Float) {
        layer.setProperties(
            LocationPropertyFactory.accuracyRadius(accuracy),
        )
        lastAccuracy = accuracy
    }

    override fun styleScaling(scaleExpression: Expression) {
        layer.setProperties(
            LocationPropertyFactory.shadowImageSize(scaleExpression),
            LocationPropertyFactory.bearingImageSize(scaleExpression),
            LocationPropertyFactory.topImageSize(scaleExpression),
        )
    }

    override fun setLocationStale(
        isStale: Boolean,
        renderMode: Int,
    ) {
        setImages(renderMode, isStale)
    }

    override fun updateIconIds(
        foregroundIconString: String,
        foregroundStaleIconString: String,
        backgroundIconString: String,
        backgroundStaleIconString: String,
        bearingIconString: String,
    ) {
        // not supported
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
        updateBitmap(FOREGROUND_ICON, foregroundBitmap)
        updateBitmap(FOREGROUND_STALE_ICON, foregroundStaleBitmap)

        if (renderMode == RenderMode.COMPASS) {
            val leftOffset = (bearingBitmap!!.width - backgroundBitmap!!.width) / 2f
            val topOffset = (bearingBitmap.height - backgroundBitmap.height) / 2f
            updateBitmap(BEARING_ICON, BitmapUtils.mergeBitmap(bearingBitmap, backgroundBitmap, leftOffset, topOffset))

            val staleLeftOffset = (bearingBitmap.width - backgroundStaleBitmap!!.width) / 2f
            val staleTopOffset = (bearingBitmap.height - backgroundStaleBitmap.height) / 2f
            updateBitmap(
                BEARING_STALE_ICON,
                BitmapUtils.mergeBitmap(bearingBitmap, backgroundStaleBitmap, staleLeftOffset, staleTopOffset),
            )
        } else {
            updateBitmap(BACKGROUND_ICON, backgroundBitmap)
            updateBitmap(BACKGROUND_STALE_ICON, backgroundStaleBitmap)
            updateBitmap(BEARING_ICON, bearingBitmap)
        }
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

    private fun setLayerVisibility(visible: Boolean) {
        layer.setProperties(LocationPropertyFactory.visibility(if (visible) VISIBLE else NONE))
    }

    private fun setLayerLocation(latLng: LatLng) {
        val values = arrayOf(latLng.latitude, latLng.longitude, 0.0)
        layer.setProperties(
            LocationPropertyFactory.location(values),
        )
        lastLatLng = latLng
    }

    private fun setLayerBearing(bearing: Double) {
        layer.setProperties(
            LocationPropertyFactory.bearing(bearing),
        )
        lastBearing = bearing
    }

    /**
     * Adjust the visibility of the pulsing LocationComponent circle.
     */
    override fun adjustPulsingCircleLayerVisibility(visible: Boolean) {
        // not supported at this time
    }

    /**
     * Adjust the the pulsing LocationComponent circle based on the set options.
     */
    override fun stylePulsingCircle(options: LocationComponentOptions) {
        // not supported at this time
    }

    /**
     * Adjust the visual appearance of the pulsing LocationComponent circle.
     */
    override fun updatePulsingUi(
        radius: Float,
        opacity: Float?,
    ) {
        // not supported at this time
    }

    private fun setImages(
        @RenderMode.Mode renderMode: Int,
        isStale: Boolean,
    ) {
        var topImage = ""
        var bearingImage = ""
        var shadowImage = ""

        when (renderMode) {
            RenderMode.COMPASS -> {
                topImage = if (isStale) FOREGROUND_STALE_ICON else FOREGROUND_ICON
                bearingImage = if (isStale) BEARING_STALE_ICON else BEARING_ICON
                shadowImage = SHADOW_ICON
            }

            RenderMode.GPS -> {
                topImage = ""
                bearingImage = if (isStale) FOREGROUND_STALE_ICON else FOREGROUND_ICON
                shadowImage = if (isStale) BACKGROUND_STALE_ICON else BACKGROUND_ICON
                setAccuracyRadius(0f)
            }

            RenderMode.NORMAL -> {
                topImage = if (isStale) FOREGROUND_STALE_ICON else FOREGROUND_ICON
                bearingImage = if (isStale) BACKGROUND_STALE_ICON else BACKGROUND_ICON
                shadowImage = SHADOW_ICON
            }
        }
        layer.setProperties(
            LocationPropertyFactory.topImage(topImage),
            LocationPropertyFactory.bearingImage(bearingImage),
            LocationPropertyFactory.shadowImage(shadowImage),
        )
    }
}
