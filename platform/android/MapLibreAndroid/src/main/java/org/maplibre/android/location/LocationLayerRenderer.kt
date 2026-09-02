package org.maplibre.android.location

import android.graphics.Bitmap
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression

internal interface LocationLayerRenderer {
    fun initializeComponents(style: Style)

    fun addLayers(positionManager: LocationComponentPositionManager)

    fun removeLayers()

    fun hide()

    fun cameraTiltUpdated(tilt: Double)

    fun cameraBearingUpdated(bearing: Double)

    fun show(
        @RenderMode.Mode renderMode: Int,
        isStale: Boolean,
    )

    fun styleAccuracy(
        accuracyAlpha: Float,
        accuracyColor: Int,
    )

    fun setLatLng(latLng: LatLng)

    fun setGpsBearing(gpsBearing: Float)

    fun setCompassBearing(compassBearing: Float)

    fun setAccuracyRadius(accuracy: Float)

    fun styleScaling(scaleExpression: Expression)

    fun setLocationStale(
        isStale: Boolean,
        renderMode: Int,
    )

    fun adjustPulsingCircleLayerVisibility(visible: Boolean)

    fun stylePulsingCircle(options: LocationComponentOptions)

    fun updatePulsingUi(
        radius: Float,
        opacity: Float?,
    )

    fun updateIconIds(
        foregroundIconString: String,
        foregroundStaleIconString: String,
        backgroundIconString: String,
        backgroundStaleIconString: String,
        bearingIconString: String,
    )

    fun addBitmaps(
        @RenderMode.Mode renderMode: Int,
        shadowBitmap: Bitmap?,
        backgroundBitmap: Bitmap?,
        backgroundStaleBitmap: Bitmap?,
        bearingBitmap: Bitmap?,
        foregroundBitmap: Bitmap?,
        foregroundStaleBitmap: Bitmap?,
    )
}
