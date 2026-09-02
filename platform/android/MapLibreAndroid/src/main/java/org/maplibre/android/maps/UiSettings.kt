package org.maplibre.android.maps

import android.content.Context
import android.content.res.Resources
import android.graphics.Color
import android.graphics.PointF
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.view.View
import android.widget.FrameLayout
import android.widget.ImageView
import androidx.annotation.ColorInt
import androidx.annotation.FloatRange
import androidx.annotation.Px
import androidx.annotation.UiThread
import androidx.annotation.VisibleForTesting
import androidx.core.content.ContextCompat
import androidx.core.content.res.ResourcesCompat
import org.maplibre.android.R
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.maps.widgets.CompassView
import org.maplibre.android.utils.BitmapUtils
import org.maplibre.android.utils.ColorUtils

/**
 * Settings for the user interface of a MapLibreMap. To obtain this interface, call getUiSettings().
 */
@Suppress("TooManyFunctions", "LargeClass")
class UiSettings internal constructor(
    private val projection: Projection,
    private val focalPointChangeListener: FocalPointChangeListener,
    private val pixelRatio: Float,
    private val mapView: MapView,
) {
    @VisibleForTesting
    internal var compassView: CompassView? = null
    private val compassMargins = IntArray(4)

    @VisibleForTesting
    internal var attributionsView: ImageView? = null
    private val attributionsMargins = IntArray(4)
    private var attributionDialogManager: AttributionDialogManager? = null

    @VisibleForTesting
    internal var logoView: ImageView? = null
    private val logoMargins = IntArray(4)

    private var rotateGesturesEnabled = true

    private var tiltGesturesEnabled = true

    private var zoomGesturesEnabled = true

    private var scrollGesturesEnabled = true

    private var horizontalScrollGesturesEnabled = true

    private var doubleTapGesturesEnabled = true

    private var quickZoomGesturesEnabled = true

    private var scaleVelocityAnimationEnabled = true
    private var rotateVelocityAnimationEnabled = true
    private var flingVelocityAnimationEnabled = true

    private var increaseRotateThresholdWhenScaling = true
    private var disableRotateWhenScaling = true
    private var increaseScaleThresholdWhenRotating = true

    /**
     * Zoom gesture rate, including pinch to zoom and quick scale.
     *
     * Default value is 1.0f.
     */
    @setparam:FloatRange(from = 0.0)
    var zoomRate = 1.0f

    private var deselectMarkersOnTap = true

    var flingAnimationBaseTime = MapLibreConstants.ANIMATION_DURATION_FLING_BASE
    var flingThreshold = MapLibreConstants.VELOCITY_THRESHOLD_IGNORE_FLING

    private var userProvidedFocalPoint: PointF? = null

    @VisibleForTesting
    internal var isCompassInitialized = false

    @VisibleForTesting
    internal var isAttributionInitialized = false

    @VisibleForTesting
    internal var isLogoInitialized = false
    private var clockwiseBearing = 0.0

    internal fun initialise(
        context: Context,
        options: MapLibreMapOptions,
    ) {
        val resources = context.resources
        initialiseGestures(options)
        if (options.compassEnabled) {
            initialiseCompass(options, resources)
        }
        if (options.logoEnabled) {
            initialiseLogo(options, resources)
        }
        if (options.attributionEnabled) {
            initialiseAttribution(context, options)
        }
    }

    internal fun onSaveInstanceState(outState: Bundle) {
        saveGestures(outState)
        saveCompass(outState)
        saveLogo(outState)
        saveAttribution(outState)
        saveDeselectMarkersOnTap(outState)
        saveFocalPoint(outState)
    }

    internal fun onRestoreInstanceState(savedInstanceState: Bundle) {
        restoreGestures(savedInstanceState)
        restoreCompass(savedInstanceState)
        restoreLogo(savedInstanceState)
        restoreAttribution(savedInstanceState)
        restoreDeselectMarkersOnTap(savedInstanceState)
        restoreFocalPoint(savedInstanceState)
    }

    private fun initialiseGestures(options: MapLibreMapOptions) {
        isZoomGesturesEnabled = options.zoomGesturesEnabled
        isScrollGesturesEnabled = options.scrollGesturesEnabled
        isHorizontalScrollGesturesEnabled = options.horizontalScrollGesturesEnabled
        isRotateGesturesEnabled = options.rotateGesturesEnabled
        isTiltGesturesEnabled = options.tiltGesturesEnabled
        isDoubleTapGesturesEnabled = options.doubleTapGesturesEnabled
        isQuickZoomGesturesEnabled = options.quickZoomGesturesEnabled
    }

    private fun saveGestures(outState: Bundle) {
        outState.putBoolean(MapLibreConstants.STATE_HORIZONAL_SCROLL_ENABLED, isHorizontalScrollGesturesEnabled)
        outState.putBoolean(MapLibreConstants.STATE_ZOOM_ENABLED, isZoomGesturesEnabled)
        outState.putBoolean(MapLibreConstants.STATE_SCROLL_ENABLED, isScrollGesturesEnabled)
        outState.putBoolean(MapLibreConstants.STATE_ROTATE_ENABLED, isRotateGesturesEnabled)
        outState.putBoolean(MapLibreConstants.STATE_TILT_ENABLED, isTiltGesturesEnabled)
        outState.putBoolean(MapLibreConstants.STATE_DOUBLE_TAP_ENABLED, isDoubleTapGesturesEnabled)
        outState.putBoolean(MapLibreConstants.STATE_SCALE_ANIMATION_ENABLED, isScaleVelocityAnimationEnabled)
        outState.putBoolean(MapLibreConstants.STATE_ROTATE_ANIMATION_ENABLED, isRotateVelocityAnimationEnabled)
        outState.putBoolean(MapLibreConstants.STATE_FLING_ANIMATION_ENABLED, isFlingVelocityAnimationEnabled)
        @Suppress("DEPRECATION")
        outState.putBoolean(MapLibreConstants.STATE_INCREASE_ROTATE_THRESHOLD, isIncreaseRotateThresholdWhenScaling)
        outState.putBoolean(MapLibreConstants.STATE_DISABLE_ROTATE_WHEN_SCALING, isDisableRotateWhenScaling)
        outState.putBoolean(MapLibreConstants.STATE_INCREASE_SCALE_THRESHOLD, isIncreaseScaleThresholdWhenRotating)
        outState.putBoolean(MapLibreConstants.STATE_QUICK_ZOOM_ENABLED, isQuickZoomGesturesEnabled)
        outState.putFloat(MapLibreConstants.STATE_ZOOM_RATE, zoomRate)
    }

    @Suppress("DEPRECATION")
    private fun restoreGestures(savedInstanceState: Bundle) {
        isHorizontalScrollGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_HORIZONAL_SCROLL_ENABLED)
        isZoomGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_ZOOM_ENABLED)
        isScrollGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_SCROLL_ENABLED)
        isRotateGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_ROTATE_ENABLED)
        isTiltGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_TILT_ENABLED)
        isDoubleTapGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_DOUBLE_TAP_ENABLED)
        isScaleVelocityAnimationEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_SCALE_ANIMATION_ENABLED)
        isRotateVelocityAnimationEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_ROTATE_ANIMATION_ENABLED)
        isFlingVelocityAnimationEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_FLING_ANIMATION_ENABLED)
        isIncreaseRotateThresholdWhenScaling = savedInstanceState.getBoolean(MapLibreConstants.STATE_INCREASE_ROTATE_THRESHOLD)
        isDisableRotateWhenScaling = savedInstanceState.getBoolean(MapLibreConstants.STATE_DISABLE_ROTATE_WHEN_SCALING)
        isIncreaseScaleThresholdWhenRotating = savedInstanceState.getBoolean(MapLibreConstants.STATE_INCREASE_SCALE_THRESHOLD)
        isQuickZoomGesturesEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_QUICK_ZOOM_ENABLED)
        zoomRate = savedInstanceState.getFloat(MapLibreConstants.STATE_ZOOM_RATE, 1.0f)
    }

    private fun initialiseCompass(
        options: MapLibreMapOptions,
        resources: Resources,
    ) {
        isCompassInitialized = true
        compassView = mapView.initialiseCompassView()
        isCompassEnabled = options.compassEnabled
        compassGravity = options.compassGravity
        val compassMargins = options.compassMargins
        if (compassMargins != null) {
            setCompassMargins(compassMargins[0], compassMargins[1], compassMargins[2], compassMargins[3])
        } else {
            val tenDp = resources.getDimension(R.dimen.maplibre_four_dp).toInt()
            setCompassMargins(tenDp, tenDp, tenDp, tenDp)
        }
        setCompassFadeFacingNorth(options.compassFadeFacingNorth)
        if (options.compassImage == null) {
            options.compassImage(ResourcesCompat.getDrawable(resources, R.drawable.maplibre_compass_icon, null))
        }
        compassImage = options.compassImage
    }

    private fun saveCompass(outState: Bundle) {
        outState.putBoolean(MapLibreConstants.STATE_COMPASS_ENABLED, isCompassEnabled)
        outState.putInt(MapLibreConstants.STATE_COMPASS_GRAVITY, compassGravity)
        outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_LEFT, compassMarginLeft)
        outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_TOP, compassMarginTop)
        outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_BOTTOM, compassMarginBottom)
        outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_RIGHT, compassMarginRight)
        outState.putBoolean(MapLibreConstants.STATE_COMPASS_FADE_WHEN_FACING_NORTH, isCompassFadeWhenFacingNorth)
        outState.putByteArray(
            MapLibreConstants.STATE_COMPASS_IMAGE_BITMAP,
            BitmapUtils.getByteArrayFromDrawable(compassImage),
        )
    }

    private fun restoreCompass(savedInstanceState: Bundle) {
        val compassEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_COMPASS_ENABLED)
        if (compassEnabled && !isCompassInitialized) {
            compassView = mapView.initialiseCompassView()
            isCompassInitialized = true
        }
        isCompassEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_COMPASS_ENABLED)
        compassGravity = savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_GRAVITY)
        setCompassMargins(
            savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_LEFT),
            savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_TOP),
            savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_RIGHT),
            savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_BOTTOM),
        )
        setCompassFadeFacingNorth(
            savedInstanceState.getBoolean(MapLibreConstants.STATE_COMPASS_FADE_WHEN_FACING_NORTH),
        )
        compassImage =
            BitmapUtils.getDrawableFromByteArray(
                mapView.context,
                savedInstanceState.getByteArray(MapLibreConstants.STATE_COMPASS_IMAGE_BITMAP),
            )
    }

    private fun initialiseLogo(
        options: MapLibreMapOptions,
        resources: Resources,
    ) {
        isLogoInitialized = true
        logoView = mapView.initialiseLogoView()
        isLogoEnabled = options.logoEnabled
        logoGravity = options.logoGravity
        setLogoMargins(resources, options.logoMargins)
    }

    private fun setLogoMargins(
        resources: Resources,
        logoMargins: IntArray?,
    ) {
        if (logoMargins != null) {
            setLogoMargins(logoMargins[0], logoMargins[1], logoMargins[2], logoMargins[3])
        } else {
            // user did not specify margins when programmatically creating a map
            val fourDp = resources.getDimension(R.dimen.maplibre_four_dp).toInt()
            setLogoMargins(fourDp, fourDp, fourDp, fourDp)
        }
    }

    private fun saveLogo(outState: Bundle) {
        outState.putInt(MapLibreConstants.STATE_LOGO_GRAVITY, logoGravity)
        outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_LEFT, logoMarginLeft)
        outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_TOP, logoMarginTop)
        outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_RIGHT, logoMarginRight)
        outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_BOTTOM, logoMarginBottom)
        outState.putBoolean(MapLibreConstants.STATE_LOGO_ENABLED, isLogoEnabled)
    }

    private fun restoreLogo(savedInstanceState: Bundle) {
        val logoEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_LOGO_ENABLED)
        if (logoEnabled && !isLogoInitialized) {
            logoView = mapView.initialiseLogoView()
            isLogoInitialized = true
        }
        isLogoEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_LOGO_ENABLED)
        logoGravity = savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_GRAVITY)
        setLogoMargins(
            savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_LEFT),
            savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_TOP),
            savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_RIGHT),
            savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_BOTTOM),
        )
    }

    private fun initialiseAttribution(
        context: Context,
        options: MapLibreMapOptions,
    ) {
        isAttributionInitialized = true
        attributionsView = mapView.initialiseAttributionView()
        isAttributionEnabled = options.attributionEnabled
        attributionGravity = options.attributionGravity
        setAttributionMargins(context, options.attributionMargins)
        val attributionTintColor = options.attributionTintColor
        setAttributionTintColor(
            if (attributionTintColor != -1) attributionTintColor else ColorUtils.getPrimaryColor(context),
        )
    }

    private fun setAttributionMargins(
        context: Context,
        attributionMargins: IntArray?,
    ) {
        if (attributionMargins != null) {
            setAttributionMargins(
                attributionMargins[0],
                attributionMargins[1],
                attributionMargins[2],
                attributionMargins[3],
            )
        } else {
            // user did not specify margins when programmatically creating a map
            val resources = context.resources
            val margin = resources.getDimension(R.dimen.maplibre_four_dp).toInt()
            val leftMargin = resources.getDimension(R.dimen.maplibre_ninety_two_dp).toInt()
            setAttributionMargins(leftMargin, margin, margin, margin)
        }
    }

    private fun saveAttribution(outState: Bundle) {
        outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_GRAVITY, attributionGravity)
        outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_LEFT, attributionMarginLeft)
        outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_TOP, attributionMarginTop)
        outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_RIGHT, attributionMarginRight)
        outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_BOTTOM, attributionMarginBottom)
        outState.putBoolean(MapLibreConstants.STATE_ATTRIBUTION_ENABLED, isAttributionEnabled)
    }

    private fun restoreAttribution(savedInstanceState: Bundle) {
        val attributionEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_ATTRIBUTION_ENABLED)
        if (attributionEnabled && !isAttributionInitialized) {
            attributionsView = mapView.initialiseAttributionView()
            isAttributionInitialized = true
        }
        isAttributionEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_ATTRIBUTION_ENABLED)
        attributionGravity = savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_GRAVITY)
        setAttributionMargins(
            savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_LEFT),
            savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_TOP),
            savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_RIGHT),
            savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_BOTTOM),
        )
    }

    /**
     * Enables or disables the compass. The compass is an icon on the map that indicates the
     * direction of north on the map. When a user clicks
     * the compass, the camera orients itself to its default orientation and fades away shortly
     * after. If disabled, the compass will never be displayed.
     *
     * By default, the compass is enabled.
     */
    var isCompassEnabled: Boolean
        get() = compassView?.isEnabled ?: false
        set(value) {
            if (value && !isCompassInitialized) {
                initialiseCompass(mapView.maplibreMapOptions, mapView.context.resources)
            }
            compassView?.let {
                it.isEnabled = value
                it.update(clockwiseBearing)
            }
        }

    /**
     * Enables or disables fading of the compass when facing north.
     *
     * By default this feature is enabled
     *
     * @param compassFadeFacingNorth True to enable the fading animation; false to disable it
     */
    fun setCompassFadeFacingNorth(compassFadeFacingNorth: Boolean) {
        compassView?.fadeCompassViewFacingNorth(compassFadeFacingNorth)
    }

    /**
     * Returns whether the compass performs a fading animation out when facing north.
     *
     * @return True if the compass will fade, false if it remains visible
     */
    val isCompassFadeWhenFacingNorth: Boolean
        get() = compassView?.isFadeCompassViewFacingNorth ?: false

    /**
     * Sets the gravity of the compass view. Use this to change the corner of the map view that the
     * compass is displayed in.
     *
     * By default, the compass is in the top right corner.
     */
    @set:UiThread
    var compassGravity: Int
        get() = compassView?.let { (it.layoutParams as FrameLayout.LayoutParams).gravity } ?: -1
        set(value) {
            compassView?.let { setWidgetGravity(it, value) }
        }

    /**
     * Sets the margins of the compass view in pixels. Use this to change the distance of the compass from the
     * map view edge.
     *
     * @param left   The left margin in pixels.
     * @param top    The top margin in pixels.
     * @param right  The right margin in pixels.
     * @param bottom The bottom margin in pixels.
     */
    @UiThread
    fun setCompassMargins(
        @Px left: Int,
        @Px top: Int,
        @Px right: Int,
        @Px bottom: Int,
    ) {
        compassView?.let { setWidgetMargins(it, compassMargins, left, top, right, bottom) }
    }

    /**
     * Returns the left side margin of CompassView in pixels.
     *
     * @return The left margin in pixels
     */
    @get:Px
    val compassMarginLeft: Int
        get() = compassMargins[0]

    /**
     * Returns the top side margin of CompassView in pixels.
     *
     * @return The top margin in pixels
     */
    @get:Px
    val compassMarginTop: Int
        get() = compassMargins[1]

    /**
     * Returns the right side margin of CompassView in pixels.
     *
     * @return The right margin in pixels
     */
    @get:Px
    val compassMarginRight: Int
        get() = compassMargins[2]

    /**
     * Returns the bottom side margin of CompassView in pixels.
     *
     * @return The bottom margin in pixels
     */
    @get:Px
    val compassMarginBottom: Int
        get() = compassMargins[3]

    /**
     * Specifies the CompassView image.
     *
     * By default this value is R.drawable.maplibre_compass_icon.
     */
    var compassImage: Drawable?
        get() = compassView?.getCompassImage()
        set(value) {
            compassView?.setCompassImage(value)
        }

    internal fun update(cameraPosition: CameraPosition) {
        clockwiseBearing = -cameraPosition.bearing
        compassView?.update(clockwiseBearing)
    }

    /**
     * Enables or disables the MapLibre logo.
     *
     * By default, the logo is enabled.
     */
    var isLogoEnabled: Boolean
        get() = logoView?.visibility == View.VISIBLE
        set(value) {
            if (value && !isLogoInitialized) {
                initialiseLogo(mapView.maplibreMapOptions, mapView.context.resources)
            }
            logoView?.visibility = if (value) View.VISIBLE else View.GONE
        }

    /**
     * Sets the gravity of the logo view. Use this to change the corner of the map view that the
     * MapLibre logo is displayed in.
     *
     * By default, the logo is in the bottom left corner.
     */
    var logoGravity: Int
        get() = logoView?.let { (it.layoutParams as FrameLayout.LayoutParams).gravity } ?: -1
        set(value) {
            logoView?.let { setWidgetGravity(it, value) }
        }

    /**
     * Sets the margins of the logo view in pixels. Use this to change the distance of the MapLibre logo from the
     * map view edge.
     *
     * @param left   The left margin in pixels.
     * @param top    The top margin in pixels.
     * @param right  The right margin in pixels.
     * @param bottom The bottom margin in pixels.
     */
    fun setLogoMargins(
        @Px left: Int,
        @Px top: Int,
        @Px right: Int,
        @Px bottom: Int,
    ) {
        logoView?.let { setWidgetMargins(it, logoMargins, left, top, right, bottom) }
    }

    /**
     * Returns the left side margin of the logo in pixels.
     *
     * @return The left margin in pixels
     */
    @get:Px
    val logoMarginLeft: Int
        get() = logoMargins[0]

    /**
     * Returns the top side margin of the logo in pixels.
     *
     * @return The top margin in pixels
     */
    @get:Px
    val logoMarginTop: Int
        get() = logoMargins[1]

    /**
     * Returns the right side margin of the logo in pixels.
     *
     * @return The right margin in pixels
     */
    @get:Px
    val logoMarginRight: Int
        get() = logoMargins[2]

    /**
     * Returns the bottom side margin of the logo in pixels.
     *
     * @return The bottom margin in pixels
     */
    @get:Px
    val logoMarginBottom: Int
        get() = logoMargins[3]

    /**
     * Enables or disables the attribution.
     *
     * By default, the attribution is enabled.
     */
    var isAttributionEnabled: Boolean
        get() = attributionsView?.visibility == View.VISIBLE
        set(value) {
            if (value && !isAttributionInitialized) {
                initialiseAttribution(mapView.context, mapView.maplibreMapOptions)
            }
            attributionsView?.visibility = if (value) View.VISIBLE else View.GONE
        }

    /**
     * Set a custom attribution dialog manager.
     *
     * Set to null to reset to default behaviour.
     *
     * @param attributionDialogManager the manager class used for showing attribution
     */
    fun setAttributionDialogManager(attributionDialogManager: AttributionDialogManager) {
        this.attributionDialogManager = attributionDialogManager
    }

    /**
     * Get the custom attribution dialog manager.
     *
     * @return the active manager class used for showing attribution
     */
    fun getAttributionDialogManager(): AttributionDialogManager? = attributionDialogManager

    /**
     * Sets the gravity of the attribution.
     *
     * By default, the attribution is in the bottom left corner next to the MapLibre logo.
     */
    var attributionGravity: Int
        get() = attributionsView?.let { (it.layoutParams as FrameLayout.LayoutParams).gravity } ?: -1
        set(value) {
            attributionsView?.let { setWidgetGravity(it, value) }
        }

    /**
     * Sets the margins of the attribution view in pixels.
     *
     * @param left   The left margin in pixels.
     * @param top    The top margin in pixels.
     * @param right  The right margin in pixels.
     * @param bottom The bottom margin in pixels.
     */
    fun setAttributionMargins(
        @Px left: Int,
        @Px top: Int,
        @Px right: Int,
        @Px bottom: Int,
    ) {
        attributionsView?.let { setWidgetMargins(it, attributionsMargins, left, top, right, bottom) }
    }

    /**
     * Sets the tint of the attribution view. Use this to change the color of the attribution.
     *
     * @param tintColor Color to tint the attribution.
     */
    fun setAttributionTintColor(
        @ColorInt tintColor: Int,
    ) {
        // Check that the tint color being passed in isn't transparent.
        val attributionsView = this.attributionsView ?: return
        if (Color.alpha(tintColor) == 0) {
            ColorUtils.setTintList(
                attributionsView,
                ContextCompat.getColor(attributionsView.context, R.color.maplibre_blue),
            )
        } else {
            ColorUtils.setTintList(attributionsView, tintColor)
        }
    }

    /**
     * Returns the left side margin of the attribution view in pixels.
     *
     * @return The left margin in pixels
     */
    @get:Px
    val attributionMarginLeft: Int
        get() = attributionsMargins[0]

    /**
     * Returns the top side margin of the attribution view in pixels.
     *
     * @return The top margin in pixels
     */
    @get:Px
    val attributionMarginTop: Int
        get() = attributionsMargins[1]

    /**
     * Returns the right side margin of the attribution view in pixels.
     *
     * @return The right margin in pixels
     */
    @get:Px
    val attributionMarginRight: Int
        get() = attributionsMargins[2]

    /**
     * Returns the bottom side margin of the logo in pixels.
     *
     * @return The bottom margin in pixels
     */
    @get:Px
    val attributionMarginBottom: Int
        get() = attributionsMargins[3]

    /**
     * Changes whether the user may rotate the map.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isRotateGesturesEnabled: Boolean
        get() = rotateGesturesEnabled
        set(value) {
            this.rotateGesturesEnabled = value
        }

    /**
     * Changes whether the user may tilt the map.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isTiltGesturesEnabled: Boolean
        get() = tiltGesturesEnabled
        set(value) {
            this.tiltGesturesEnabled = value
        }

    /**
     * Changes whether the user may zoom the map.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isZoomGesturesEnabled: Boolean
        get() = zoomGesturesEnabled
        set(value) {
            this.zoomGesturesEnabled = value
        }

    /**
     * Changes whether the user may zoom the map with a double tap.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isDoubleTapGesturesEnabled: Boolean
        get() = doubleTapGesturesEnabled
        set(value) {
            this.doubleTapGesturesEnabled = value
        }

    /**
     * Changes whether the user may zoom the map by tapping twice, holding and moving the pointer up and down.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isQuickZoomGesturesEnabled: Boolean
        get() = quickZoomGesturesEnabled
        set(value) {
            this.quickZoomGesturesEnabled = value
        }

    private fun restoreDeselectMarkersOnTap(savedInstanceState: Bundle) {
        isDeselectMarkersOnTap = savedInstanceState.getBoolean(MapLibreConstants.STATE_DESELECT_MARKER_ON_TAP)
    }

    private fun saveDeselectMarkersOnTap(outState: Bundle) {
        outState.putBoolean(MapLibreConstants.STATE_DESELECT_MARKER_ON_TAP, isDeselectMarkersOnTap)
    }

    /**
     * Sets whether the markers are automatically deselected (and therefore, their infowindows
     * closed) when a map tap is detected.
     *
     * Gets whether the markers are automatically deselected (and therefore, their infowindows
     * closed) when a map tap is detected.
     */
    var isDeselectMarkersOnTap: Boolean
        get() = deselectMarkersOnTap
        set(value) {
            this.deselectMarkersOnTap = value
        }

    /**
     * Changes whether the user may scroll around the map.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isScrollGesturesEnabled: Boolean
        get() = scrollGesturesEnabled
        set(value) {
            this.scrollGesturesEnabled = value
        }

    /**
     * Changes whether the user may scroll horizontally around the map.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     */
    var isHorizontalScrollGesturesEnabled: Boolean
        get() = horizontalScrollGesturesEnabled
        set(value) {
            this.horizontalScrollGesturesEnabled = value
        }

    /**
     * Set whether scale velocity animation should execute after users finishes a gesture. True by default.
     */
    var isScaleVelocityAnimationEnabled: Boolean
        get() = scaleVelocityAnimationEnabled
        set(value) {
            this.scaleVelocityAnimationEnabled = value
        }

    /**
     * Set whether rotate velocity animation should execute after users finishes a gesture. True by default.
     */
    var isRotateVelocityAnimationEnabled: Boolean
        get() = rotateVelocityAnimationEnabled
        set(value) {
            this.rotateVelocityAnimationEnabled = value
        }

    /**
     * Set whether fling velocity animation should execute after users finishes a gesture. True by default.
     */
    var isFlingVelocityAnimationEnabled: Boolean
        get() = flingVelocityAnimationEnabled
        set(value) {
            this.flingVelocityAnimationEnabled = value
        }

    /**
     * Set whether all velocity animations should execute after users finishes a gesture.
     *
     * @param allVelocityAnimationsEnabled If true, all velocity animations will be enabled.
     */
    fun setAllVelocityAnimationsEnabled(allVelocityAnimationsEnabled: Boolean) {
        isScaleVelocityAnimationEnabled = allVelocityAnimationsEnabled
        isRotateVelocityAnimationEnabled = allVelocityAnimationsEnabled
        isFlingVelocityAnimationEnabled = allVelocityAnimationsEnabled
    }

    /**
     * Whether rotation threshold should be increase whenever scale is detected.
     *
     * @deprecated unused, see [isDisableRotateWhenScaling] instead
     */
    @Deprecated("unused, see isDisableRotateWhenScaling instead", ReplaceWith("isDisableRotateWhenScaling"))
    var isIncreaseRotateThresholdWhenScaling: Boolean
        get() = increaseRotateThresholdWhenScaling
        set(value) {
            this.increaseRotateThresholdWhenScaling = value
        }

    /**
     * Set whether rotation gesture detector should be disabled when scale is detected first.
     */
    var isDisableRotateWhenScaling: Boolean
        get() = disableRotateWhenScaling
        set(value) {
            this.disableRotateWhenScaling = value
        }

    /**
     * set whether scale threshold should be increase whenever rotation is detected.
     */
    var isIncreaseScaleThresholdWhenRotating: Boolean
        get() = increaseScaleThresholdWhenRotating
        set(value) {
            this.increaseScaleThresholdWhenRotating = value
        }

    /**
     * Sets the preference for whether all gestures should be enabled or disabled.
     *
     * This setting controls only user interactions with the map. If you set the value to false,
     * you may still change the map location programmatically.
     *
     * The default value is true.
     *
     * @param enabled If true, all gestures are available; otherwise, all gestures are disabled.
     * @see isZoomGesturesEnabled
     * @see isScrollGesturesEnabled
     * @see isRotateGesturesEnabled
     * @see isTiltGesturesEnabled
     * @see isDoubleTapGesturesEnabled
     * @see isQuickZoomGesturesEnabled
     */
    fun setAllGesturesEnabled(enabled: Boolean) {
        isScrollGesturesEnabled = enabled
        isRotateGesturesEnabled = enabled
        isTiltGesturesEnabled = enabled
        isZoomGesturesEnabled = enabled
        isDoubleTapGesturesEnabled = enabled
        isQuickZoomGesturesEnabled = enabled
    }

    /**
     * Retrieves the current status of whether all gestures are enabled.
     *
     * @return If true, all gestures are enabled.
     */
    fun areAllGesturesEnabled(): Boolean =
        rotateGesturesEnabled && tiltGesturesEnabled && zoomGesturesEnabled &&
            scrollGesturesEnabled && doubleTapGesturesEnabled && quickZoomGesturesEnabled

    private fun saveFocalPoint(outState: Bundle) {
        outState.putParcelable(MapLibreConstants.STATE_USER_FOCAL_POINT, focalPoint)
    }

    private fun restoreFocalPoint(savedInstanceState: Bundle) {
        @Suppress("DEPRECATION")
        val pointF = savedInstanceState.getParcelable<PointF>(MapLibreConstants.STATE_USER_FOCAL_POINT)
        if (pointF != null) {
            focalPoint = pointF
        }
    }

    /**
     * Sets the focal point used as center for a gesture
     */
    var focalPoint: PointF?
        get() = userProvidedFocalPoint
        set(value) {
            userProvidedFocalPoint = value
            focalPointChangeListener.onFocalPointChanged(value)
        }

    /**
     * Returns the measured height of the MapView
     *
     * @return height in pixels
     */
    val height: Float
        get() = projection.getHeight()

    /**
     * Returns the measured width of the MapView
     *
     * @return widht in pixels
     */
    val width: Float
        get() = projection.getWidth()

    internal fun getPixelRatio(): Float = pixelRatio

    /**
     * Invalidates the ViewSettings instances shown on top of the MapView
     */
    fun invalidate() {
        setLogoMargins(logoMarginLeft, logoMarginTop, logoMarginRight, logoMarginBottom)
        isCompassEnabled = isCompassEnabled
        setCompassMargins(
            compassMarginLeft,
            compassMarginTop,
            compassMarginRight,
            compassMarginBottom,
        )
        setAttributionMargins(
            attributionMarginLeft,
            attributionMarginTop,
            attributionMarginRight,
            attributionMarginBottom,
        )
    }

    private fun setWidgetGravity(
        view: View,
        gravity: Int,
    ) {
        val layoutParams = view.layoutParams as FrameLayout.LayoutParams
        layoutParams.gravity = gravity
        view.layoutParams = layoutParams
    }

    @Suppress("LongParameterList")
    private fun setWidgetMargins(
        view: View,
        initMargins: IntArray,
        left: Int,
        top: Int,
        right: Int,
        bottom: Int,
    ) {
        // keep state of initially set margins
        initMargins[0] = left
        initMargins[1] = top
        initMargins[2] = right
        initMargins[3] = bottom

        // convert initial margins with padding
        val layoutParams = view.layoutParams as FrameLayout.LayoutParams
        layoutParams.setMargins(left, top, right, bottom)

        // support RTL
        layoutParams.marginStart = left
        layoutParams.marginEnd = right

        view.layoutParams = layoutParams
    }
}
