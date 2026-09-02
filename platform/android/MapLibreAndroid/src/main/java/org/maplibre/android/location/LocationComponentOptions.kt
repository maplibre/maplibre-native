package org.maplibre.android.location

import android.content.Context
import android.graphics.RectF
import android.os.Parcel
import android.os.Parcelable
import android.view.animation.Interpolator
import androidx.annotation.ColorInt
import androidx.annotation.Dimension
import androidx.annotation.DrawableRes
import androidx.annotation.StyleRes
import org.maplibre.android.R
import java.util.Arrays

/**
 * This class exposes options for the Location Component. The options can be set by defining a
 * style in your apps style.xml file and passing in directly into the [LocationComponent]
 * class. Alternatively, if properties need to be changed at runtime depending on a specific state,
 * you can build an instance of this class, setting the values you desire, and then passing it into
 * either the [LocationComponent] activation method (if it isn't initialized yet) or
 * [LocationComponent.applyStyle].
 *
 * When the [createFromAttributes] methods called, any attributes not found
 * inside the style will revert back to using their default set values. Likewise, when building a
 * new [LocationComponentOptions] class using the builder, any options neglecting to be set will
 * reset to their default values.
 *
 * If you would like to keep your custom style changes while modifying a single attribute, you can
 * get the currently used options object using [LocationComponent.locationComponentOptions]
 * and it's [toBuilder] method to modify a single entry while also maintaining the other
 * settings. Once your modifications have been made, you'll need to pass it back into the location
 * component using [LocationComponent.applyStyle].
 */
class LocationComponentOptions : Parcelable {
    private val accuracyAlpha: Float
    private val accuracyColor: Int
    private val backgroundDrawableStale: Int
    private val backgroundStaleName: String?
    private val foregroundDrawableStale: Int
    private val foregroundStaleName: String?
    private val gpsDrawable: Int
    private val gpsName: String?
    private val foregroundDrawable: Int
    private val foregroundName: String?
    private val backgroundDrawable: Int
    private val backgroundName: String?
    private val bearingDrawable: Int
    private val bearingName: String?
    private val bearingTintColor: Int?
    private val foregroundTintColor: Int?
    private val backgroundTintColor: Int?
    private val foregroundStaleTintColor: Int?
    private val backgroundStaleTintColor: Int?
    private val elevation: Float
    private val enableStaleState: Boolean
    private val staleStateTimeout: Long
    private val padding: IntArray?
    private val maxZoomIconScale: Float
    private val minZoomIconScale: Float
    private val trackingGesturesManagement: Boolean
    private val trackingInitialMoveThreshold: Float
    private val trackingMultiFingerMoveThreshold: Float
    private val trackingMultiFingerProtectedMoveArea: RectF?
    private val layerAbove: String?
    private val layerBelow: String?
    private val bearingOnTop: Boolean
    private val trackingAnimationDurationMultiplier: Float
    private val compassAnimationEnabled: Boolean
    private val accuracyAnimationEnabled: Boolean
    private val pulseEnabled: Boolean?
    private val pulseFadeEnabled: Boolean?
    private val pulseColor: Int?
    private val pulseSingleDuration: Float
    private val pulseMaxRadius: Float
    private val pulseAlpha: Float
    private val pulseInterpolator: Interpolator?

    @Suppress("LongParameterList")
    constructor(
        accuracyAlpha: Float,
        accuracyColor: Int,
        backgroundDrawableStale: Int,
        backgroundStaleName: String?,
        foregroundDrawableStale: Int,
        foregroundStaleName: String?,
        gpsDrawable: Int,
        gpsName: String?,
        foregroundDrawable: Int,
        foregroundName: String?,
        backgroundDrawable: Int,
        backgroundName: String?,
        bearingDrawable: Int,
        bearingName: String?,
        bearingTintColor: Int?,
        foregroundTintColor: Int?,
        backgroundTintColor: Int?,
        foregroundStaleTintColor: Int?,
        backgroundStaleTintColor: Int?,
        elevation: Float,
        enableStaleState: Boolean,
        staleStateTimeout: Long,
        padding: IntArray?,
        maxZoomIconScale: Float,
        minZoomIconScale: Float,
        trackingGesturesManagement: Boolean,
        trackingInitialMoveThreshold: Float,
        trackingMultiFingerMoveThreshold: Float,
        trackingMultiFingerProtectedMoveArea: RectF?,
        layerAbove: String?,
        layerBelow: String?,
        bearingOnTop: Boolean,
        trackingAnimationDurationMultiplier: Float,
        compassAnimationEnabled: Boolean,
        accuracyAnimationEnabled: Boolean,
        pulseEnabled: Boolean?,
        pulseFadeEnabled: Boolean?,
        pulseColor: Int?,
        pulseSingleDuration: Float,
        pulseMaxRadius: Float,
        pulseAlpha: Float,
        pulseInterpolator: Interpolator?,
    ) {
        this.accuracyAlpha = accuracyAlpha
        this.accuracyColor = accuracyColor
        this.backgroundDrawableStale = backgroundDrawableStale
        this.backgroundStaleName = backgroundStaleName
        this.foregroundDrawableStale = foregroundDrawableStale
        this.foregroundStaleName = foregroundStaleName
        this.gpsDrawable = gpsDrawable
        this.gpsName = gpsName
        this.foregroundDrawable = foregroundDrawable
        this.foregroundName = foregroundName
        this.backgroundDrawable = backgroundDrawable
        this.backgroundName = backgroundName
        this.bearingDrawable = bearingDrawable
        this.bearingName = bearingName
        this.bearingTintColor = bearingTintColor
        this.foregroundTintColor = foregroundTintColor
        this.backgroundTintColor = backgroundTintColor
        this.foregroundStaleTintColor = foregroundStaleTintColor
        this.backgroundStaleTintColor = backgroundStaleTintColor
        this.elevation = elevation
        this.enableStaleState = enableStaleState
        this.staleStateTimeout = staleStateTimeout
        if (padding == null) {
            throw NullPointerException("Null padding")
        }
        this.padding = padding
        this.maxZoomIconScale = maxZoomIconScale
        this.minZoomIconScale = minZoomIconScale
        this.trackingGesturesManagement = trackingGesturesManagement
        this.trackingInitialMoveThreshold = trackingInitialMoveThreshold
        this.trackingMultiFingerMoveThreshold = trackingMultiFingerMoveThreshold
        this.trackingMultiFingerProtectedMoveArea = trackingMultiFingerProtectedMoveArea
        this.layerAbove = layerAbove
        this.layerBelow = layerBelow
        this.bearingOnTop = bearingOnTop
        this.trackingAnimationDurationMultiplier = trackingAnimationDurationMultiplier
        this.compassAnimationEnabled = compassAnimationEnabled
        this.accuracyAnimationEnabled = accuracyAnimationEnabled
        this.pulseEnabled = pulseEnabled
        this.pulseFadeEnabled = pulseFadeEnabled
        this.pulseColor = pulseColor
        this.pulseSingleDuration = pulseSingleDuration
        this.pulseMaxRadius = pulseMaxRadius
        this.pulseAlpha = pulseAlpha
        this.pulseInterpolator = pulseInterpolator
    }

    internal constructor(parcel: Parcel) {
        this.accuracyAlpha = parcel.readFloat()
        this.accuracyColor = parcel.readInt()
        this.backgroundDrawableStale = parcel.readInt()
        this.backgroundStaleName = parcel.readString()
        this.foregroundDrawableStale = parcel.readInt()
        this.foregroundStaleName = parcel.readString()
        this.gpsDrawable = parcel.readInt()
        this.gpsName = parcel.readString()
        this.foregroundDrawable = parcel.readInt()
        this.foregroundName = parcel.readString()
        this.backgroundDrawable = parcel.readInt()
        this.backgroundName = parcel.readString()
        this.bearingDrawable = parcel.readInt()
        this.bearingName = parcel.readString()
        this.bearingTintColor = parcel.readValue(Int::class.java.classLoader) as Int?
        this.foregroundTintColor = parcel.readValue(Int::class.java.classLoader) as Int?
        this.backgroundTintColor = parcel.readValue(Int::class.java.classLoader) as Int?
        this.foregroundStaleTintColor = parcel.readValue(Int::class.java.classLoader) as Int?
        this.backgroundStaleTintColor = parcel.readValue(Int::class.java.classLoader) as Int?
        this.elevation = parcel.readFloat()
        this.enableStaleState = parcel.readByte().toInt() != 0
        this.staleStateTimeout = parcel.readLong()
        this.padding = parcel.createIntArray()
        this.maxZoomIconScale = parcel.readFloat()
        this.minZoomIconScale = parcel.readFloat()
        this.trackingGesturesManagement = parcel.readByte().toInt() != 0
        this.trackingInitialMoveThreshold = parcel.readFloat()
        this.trackingMultiFingerMoveThreshold = parcel.readFloat()
        @Suppress("DEPRECATION")
        this.trackingMultiFingerProtectedMoveArea = parcel.readParcelable(RectF::class.java.classLoader)
        this.layerAbove = parcel.readString()
        this.layerBelow = parcel.readString()
        this.bearingOnTop = parcel.readByte().toInt() != 0
        this.trackingAnimationDurationMultiplier = parcel.readFloat()
        this.compassAnimationEnabled = parcel.readByte().toInt() != 0
        this.accuracyAnimationEnabled = parcel.readByte().toInt() != 0
        this.pulseEnabled = parcel.readValue(Boolean::class.java.classLoader) as Boolean?
        this.pulseFadeEnabled = parcel.readValue(Boolean::class.java.classLoader) as Boolean?
        this.pulseColor = parcel.readValue(Int::class.java.classLoader) as Int?
        this.pulseSingleDuration = parcel.readFloat()
        this.pulseMaxRadius = parcel.readFloat()
        this.pulseAlpha = parcel.readFloat()
        this.pulseInterpolator = null
    }

    /**
     * Takes the currently constructed [LocationComponentOptions] object and provides it's builder
     * with all the values set matching the values in this instance. This allows you to modify a
     * single attribute and then rebuild the object.
     *
     * @return the builder which contains the values defined in this current instance as defaults.
     */
    fun toBuilder(): Builder = Builder(this)

    /**
     * Set the opacity of the accuracy view to a value from 0 to 1, where 0 means the accuracy view is
     * completely transparent and 1 means the view is completely opaque.
     * References style attribute R.styleable#LocationComponent_accuracyAlpha
     *
     * @return the opacity of the accuracy view
     */
    fun accuracyAlpha(): Float = accuracyAlpha

    /**
     * Solid color to use as the accuracy view color property.
     * References style attribute R.styleable#LocationComponent_accuracyColor
     *
     * @return the color of the accuracy view
     */
    @ColorInt
    fun accuracyColor(): Int = accuracyColor

    /**
     * Defines the drawable used for the stale background icon.
     * References style attribute R.styleable#LocationComponent_backgroundDrawableStale
     *
     * @return the drawable resource ID
     */
    @DrawableRes
    fun backgroundDrawableStale(): Int = backgroundDrawableStale

    /**
     * String image name, identical to one used in
     * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
     * component, will use this image in place of the provided or default maplibre_foregroundDrawableStale.
     *
     * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
     * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
     *
     * @return String icon or maki-icon name
     */
    fun backgroundStaleName(): String? = backgroundStaleName

    /**
     * Defines the drawable used for the stale foreground icon.
     * References style attribute R.styleable#LocationComponent_foregroundDrawableStale
     *
     * @return the drawable resource ID
     */
    @DrawableRes
    fun foregroundDrawableStale(): Int = foregroundDrawableStale

    /**
     * String image name, identical to one used in
     * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
     * component, will used this image in place of the provided or default maplibre_foregroundDrawableStale.
     *
     * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
     * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
     *
     * @return String icon or maki-icon name
     */
    fun foregroundStaleName(): String? = foregroundStaleName

    /**
     * Defines the drawable used for the navigation state icon.
     * References style attribute R.styleable#LocationComponent_gpsDrawable
     *
     * @return the drawable resource ID
     */
    @DrawableRes
    fun gpsDrawable(): Int = gpsDrawable

    /**
     * String image name, identical to one used in
     * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
     * component, will used this image in place of the provided or default maplibre_gpsDrawable.
     *
     * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
     * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
     *
     * @return String icon or maki-icon name
     */
    fun gpsName(): String? = gpsName

    /**
     * Supply a Drawable that is to be rendered on top of all of the content in the Location LayerComponent layer stack.
     * References style attribute R.styleable#LocationComponent_foregroundDrawable
     *
     * @return the drawable resource used for the foreground layer
     */
    @DrawableRes
    fun foregroundDrawable(): Int = foregroundDrawable

    /**
     * String image name, identical to one used in
     * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
     * component, will used this image in place of the provided or default maplibre_foregroundDrawable.
     *
     * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
     * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
     *
     * @return String icon or maki-icon name
     */
    fun foregroundName(): String? = foregroundName

    /**
     * Defines the drawable used for the background state icon.
     * References style attribute R.styleable#LocationComponent_backgroundDrawable
     *
     * @return the drawable resource ID
     */
    @DrawableRes
    fun backgroundDrawable(): Int = backgroundDrawable

    /**
     * String image name, identical to one used in
     * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
     * component, will used this image in place of the provided or default maplibre_backgroundDrawable.
     *
     * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
     * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
     *
     * @return String icon or maki-icon name
     */
    fun backgroundName(): String? = backgroundName

    /**
     * Defines the drawable used for the bearing icon.
     * References style attribute R.styleable#LocationComponent_bearingDrawable
     *
     * @return the drawable resource ID
     */
    @DrawableRes
    fun bearingDrawable(): Int = bearingDrawable

    /**
     * String image name, identical to one used in
     * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
     * component, will used this image in place of the provided or default maplibre_bearingDrawable.
     *
     * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
     * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
     *
     * @return String icon or maki-icon name
     */
    fun bearingName(): String? = bearingName

    /**
     * Defines the bearing icon color as an integer.
     * References style attribute R.styleable#LocationComponent_bearingTintColor
     *
     * @return the color integer resource
     */
    @ColorInt
    fun bearingTintColor(): Int? = bearingTintColor

    /**
     * Defines the foreground color as an integer.
     * References style attribute R.styleable#LocationComponent_foregroundTintColor
     *
     * @return the color integer resource
     */
    @ColorInt
    fun foregroundTintColor(): Int? = foregroundTintColor

    /**
     * Defines the background color as an integer.
     * References style attribute R.styleable#LocationComponent_backgroundTintColor
     *
     * @return the color integer resource
     */
    @ColorInt
    fun backgroundTintColor(): Int? = backgroundTintColor

    /**
     * Defines the foreground stale color as an integer.
     * References style attribute R.styleable#LocationComponent_foregroundStaleTintColor
     *
     * @return the color integer resource
     */
    @ColorInt
    fun foregroundStaleTintColor(): Int? = foregroundStaleTintColor

    /**
     * Defines the background stale color as an integer.
     * References style attribute R.styleable#LocationComponent_backgroundStaleTintColor
     *
     * @return the color integer resource
     */
    @ColorInt
    fun backgroundStaleTintColor(): Int? = backgroundStaleTintColor

    /**
     * Sets the base elevation of this view, in pixels. To turn off the shadow that appears under
     * the location icon, set the elevation to 0.
     * References style attribute R.styleable#LocationComponent_elevation
     *
     * @return the elevation currently set for the location component icon
     */
    @Dimension
    fun elevation(): Float = elevation

    /**
     * Enable or disable to stale state mode. This mode indicates to the user that the location being
     * displayed on the map hasn't been updated in a specific amount of time.
     * References style attribute R.styleable#LocationComponent_enableStaleState
     *
     * @return whether the stale state mode is enabled or not
     */
    fun enableStaleState(): Boolean = enableStaleState

    /**
     * Set the delay before the location icon becomes stale. The timer begins approximately when a new
     * location update comes in and using this defined time, if an update hasn't occured by the end,
     * the location is considered stale.
     * References style attribute R.styleable#LocationComponent_staleStateDelay
     *
     * @return the duration in milliseconds which it should take before the location is
     * considered stale
     */
    fun staleStateTimeout(): Long = staleStateTimeout

    /**
     * Sets the distance from the edges of the map view’s frame to the edges of the map
     * view’s logical viewport.
     *
     * When the value of this property is equal to {0,0,0,0}, viewport
     * properties such as `centerCoordinate` assume a viewport that matches the map
     * view’s frame. Otherwise, those properties are inset, excluding part of the
     * frame from the viewport. For instance, if the only the top edge is inset, the
     * map center is effectively shifted downward.
     *
     * @return integer array of padding values
     */
    fun padding(): IntArray? = padding

    /**
     * The scale factor of the location icon when the map is zoomed in.
     * Scaling is linear.
     *
     * @return icon scale factor
     */
    fun maxZoomIconScale(): Float = maxZoomIconScale

    /**
     * The scale factor of the location icon when the map is zoomed out.
     * Scaling is linear.
     *
     * @return icon scale factor
     */
    fun minZoomIconScale(): Float = minZoomIconScale

    /**
     * Returns whether gesture threshold should be adjusted when camera is in one of the tracking modes.
     * This will adjust the focal point and increase thresholds to enable camera manipulation,
     * like zooming in and out, without breaking tracking.
     *
     * **Note**: If set to true, this can overwrite some of the gesture thresholds
     * and the custom [org.maplibre.android.gestures.AndroidGesturesManager] that was set with
     * [org.maplibre.android.maps.MapLibreMap.setGesturesManager].
     *
     * @return true if gestures are adjusted when in one of the camera tracking modes, false otherwise
     * @see Builder.trackingInitialMoveThreshold
     * @see Builder.trackingMultiFingerMoveThreshold
     * @see Builder.trackingMultiFingerProtectedMoveArea
     */
    fun trackingGesturesManagement(): Boolean = trackingGesturesManagement

    /**
     * Minimum single pointer movement in pixels required to break camera tracking.
     *
     * @return the minimum movement
     */
    fun trackingInitialMoveThreshold(): Float = trackingInitialMoveThreshold

    /**
     * Minimum multi pointer movement in pixels required to break camera tracking (for example during scale gesture).
     *
     * @return the minimum movement
     */
    fun trackingMultiFingerMoveThreshold(): Float = trackingMultiFingerMoveThreshold

    /**
     * Protected multi pointer gesture area. When the camera is in a tracking mode, any multi finger gesture with focal
     * point inside the provided screen coordinate rectangle is not going to break the tracking.
     *
     * Best paired with the [Builder.trackingMultiFingerMoveThreshold]
     * set to 0 or a relatively small value to not interfere with gestures outside of the defined rectangle.
     *
     * @return the protected multi finger area while camera is tracking
     */
    fun trackingMultiFingerProtectedMoveArea(): RectF? = trackingMultiFingerProtectedMoveArea

    /**
     * Gets the id of the layer that's referenced when placing the component on the map using
     * [org.maplibre.android.maps.Style.addLayerAbove].
     *
     * The component is going to placed directly above this layer.
     *
     * @return layerAbove the id of the layer the component is going to placed directly above.
     */
    fun layerAbove(): String? = layerAbove

    /**
     * Gets the id of the layer that's referenced when placing the component on the map using
     * [org.maplibre.android.maps.Style.addLayerBelow].
     *
     * The component is going to placed directly below this layer.
     *
     * @return layerBelow the id of the layer the component is going to placed directly below.
     */
    fun layerBelow(): String? = layerBelow

    /**
     * Whether the bearing icon is rendered on top of the foreground icon in the location layer stack.
     *
     * @return true if the bearing icon is rendered above the foreground icon, false otherwise.
     */
    fun bearingOnTop(): Boolean = bearingOnTop

    /**
     * Get the tracking animation duration multiplier.
     *
     * @return tracking animation duration multiplier
     */
    fun trackingAnimationDurationMultiplier(): Float = trackingAnimationDurationMultiplier

    /**
     * Enable or disable smooth animation of compass values for [org.maplibre.android.location.modes.CameraMode]
     * and [org.maplibre.android.location.modes.RenderMode].
     *
     * @return whether smooth compass animation is enabled
     */
    fun compassAnimationEnabled(): Boolean = compassAnimationEnabled

    /**
     * Enable or disable smooth animation of the accuracy circle around the user's position.
     *
     * @return whether smooth animation of the accuracy circle is enabled
     */
    fun accuracyAnimationEnabled(): Boolean = accuracyAnimationEnabled

    /**
     * Enable or disable the LocationComponent's pulsing circle.
     *
     * @return whether the LocationComponent's pulsing circle is enabled
     */
    fun pulseEnabled(): Boolean? = pulseEnabled

    /**
     * Enable or disable fading of the LocationComponent's pulsing circle. If it fades, the circle's
     * opacity decreases as its radius increases.
     *
     * @return whether fading of the LocationComponent's pulsing circle is enabled
     */
    fun pulseFadeEnabled(): Boolean? = pulseFadeEnabled

    /**
     * Color of the LocationComponent's pulsing circle as it pulses.
     *
     * @return the current set color of the circle
     */
    fun pulseColor(): Int? = pulseColor

    /**
     * The number of milliseconds it takes for a single pulse of the LocationComponent's pulsing circle.
     *
     * @return the current set length of time for a single pulse
     */
    fun pulseSingleDuration(): Float = pulseSingleDuration

    /**
     * The maximum radius that a single pulse should expand the LocationComponent's pulsing circle to.
     *
     * @return the maximum radius that the pulsing circle will expand to.
     */
    fun pulseMaxRadius(): Float = pulseMaxRadius

    /**
     * The opacity of the LocationComponent's circle as it pulses. The expected range is
     * 0 to 1. An opacity of 1 makes the layer fully visible.
     *
     * @return the current opacity of the LocationComponent's pulsing circle
     */
    fun pulseAlpha(): Float = pulseAlpha

    /**
     * The interpolator type of animation for the movement of the LocationComponent's circle
     *
     * @return the current set type of animation interpolator for the pulsing circle
     */
    fun pulseInterpolator(): Interpolator? = pulseInterpolator

    override fun toString(): String =
        "LocationComponentOptions{" +
            "accuracyAlpha=" + accuracyAlpha + ", " +
            "accuracyColor=" + accuracyColor + ", " +
            "backgroundDrawableStale=" + backgroundDrawableStale + ", " +
            "backgroundStaleName=" + backgroundStaleName + ", " +
            "foregroundDrawableStale=" + foregroundDrawableStale + ", " +
            "foregroundStaleName=" + foregroundStaleName + ", " +
            "gpsDrawable=" + gpsDrawable + ", " +
            "gpsName=" + gpsName + ", " +
            "foregroundDrawable=" + foregroundDrawable + ", " +
            "foregroundName=" + foregroundName + ", " +
            "backgroundDrawable=" + backgroundDrawable + ", " +
            "backgroundName=" + backgroundName + ", " +
            "bearingDrawable=" + bearingDrawable + ", " +
            "bearingName=" + bearingName + ", " +
            "bearingTintColor=" + bearingTintColor + ", " +
            "foregroundTintColor=" + foregroundTintColor + ", " +
            "backgroundTintColor=" + backgroundTintColor + ", " +
            "foregroundStaleTintColor=" + foregroundStaleTintColor + ", " +
            "backgroundStaleTintColor=" + backgroundStaleTintColor + ", " +
            "elevation=" + elevation + ", " +
            "enableStaleState=" + enableStaleState + ", " +
            "staleStateTimeout=" + staleStateTimeout + ", " +
            "padding=" + Arrays.toString(padding) + ", " +
            "maxZoomIconScale=" + maxZoomIconScale + ", " +
            "minZoomIconScale=" + minZoomIconScale + ", " +
            "trackingGesturesManagement=" + trackingGesturesManagement + ", " +
            "trackingInitialMoveThreshold=" + trackingInitialMoveThreshold + ", " +
            "trackingMultiFingerMoveThreshold=" + trackingMultiFingerMoveThreshold + ", " +
            "trackingMultiFingerProtectedMoveArea=" + trackingMultiFingerProtectedMoveArea + ", " +
            "layerAbove=" + layerAbove +
            "layerBelow=" + layerBelow +
            "bearingOnTop=" + bearingOnTop +
            "trackingAnimationDurationMultiplier=" + trackingAnimationDurationMultiplier +
            "pulseEnabled=" + pulseEnabled +
            "pulseFadeEnabled=" + pulseFadeEnabled +
            "pulseColor=" + pulseColor +
            "pulseSingleDuration=" + pulseSingleDuration +
            "pulseMaxRadius=" + pulseMaxRadius +
            "pulseAlpha=" + pulseAlpha +
            "}"

    @Suppress("CyclomaticComplexMethod", "ReturnCount")
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        val options = other as LocationComponentOptions

        if (java.lang.Float.compare(options.accuracyAlpha, accuracyAlpha) != 0) {
            return false
        }
        if (accuracyColor != options.accuracyColor) {
            return false
        }
        if (backgroundDrawableStale != options.backgroundDrawableStale) {
            return false
        }
        if (foregroundDrawableStale != options.foregroundDrawableStale) {
            return false
        }
        if (gpsDrawable != options.gpsDrawable) {
            return false
        }
        if (foregroundDrawable != options.foregroundDrawable) {
            return false
        }
        if (backgroundDrawable != options.backgroundDrawable) {
            return false
        }
        if (bearingDrawable != options.bearingDrawable) {
            return false
        }
        if (java.lang.Float.compare(options.elevation, elevation) != 0) {
            return false
        }
        if (enableStaleState != options.enableStaleState) {
            return false
        }
        if (staleStateTimeout != options.staleStateTimeout) {
            return false
        }
        if (java.lang.Float.compare(options.maxZoomIconScale, maxZoomIconScale) != 0) {
            return false
        }
        if (java.lang.Float.compare(options.minZoomIconScale, minZoomIconScale) != 0) {
            return false
        }
        if (trackingGesturesManagement != options.trackingGesturesManagement) {
            return false
        }
        if (java.lang.Float.compare(options.trackingInitialMoveThreshold, trackingInitialMoveThreshold) != 0) {
            return false
        }
        if (java.lang.Float.compare(options.trackingMultiFingerMoveThreshold, trackingMultiFingerMoveThreshold) != 0) {
            return false
        }
        if (java.lang.Float.compare(
                options.trackingAnimationDurationMultiplier,
                trackingAnimationDurationMultiplier,
            ) != 0
        ) {
            return false
        }
        if (trackingMultiFingerProtectedMoveArea != options.trackingMultiFingerProtectedMoveArea) {
            return false
        }
        if (compassAnimationEnabled != options.compassAnimationEnabled) {
            return false
        }
        if (accuracyAnimationEnabled != options.accuracyAnimationEnabled) {
            return false
        }
        if (backgroundStaleName != options.backgroundStaleName) {
            return false
        }
        if (foregroundStaleName != options.foregroundStaleName) {
            return false
        }
        if (gpsName != options.gpsName) {
            return false
        }
        if (foregroundName != options.foregroundName) {
            return false
        }
        if (backgroundName != options.backgroundName) {
            return false
        }
        if (bearingName != options.bearingName) {
            return false
        }
        if (bearingTintColor != options.bearingTintColor) {
            return false
        }
        if (foregroundTintColor != options.foregroundTintColor) {
            return false
        }
        if (backgroundTintColor != options.backgroundTintColor) {
            return false
        }
        if (foregroundStaleTintColor != options.foregroundStaleTintColor) {
            return false
        }
        if (backgroundStaleTintColor != options.backgroundStaleTintColor) {
            return false
        }
        if (!Arrays.equals(padding, options.padding)) {
            return false
        }
        if (layerAbove != options.layerAbove) {
            return false
        }
        if (bearingOnTop != options.bearingOnTop) {
            return false
        }
        if (pulseEnabled != options.pulseEnabled) {
            return false
        }
        if (pulseFadeEnabled != options.pulseFadeEnabled) {
            return false
        }
        if (pulseColor != options.pulseColor) {
            return false
        }
        if (java.lang.Float.compare(options.pulseSingleDuration, pulseSingleDuration) != 0) {
            return false
        }
        if (java.lang.Float.compare(options.pulseMaxRadius, pulseMaxRadius) != 0) {
            return false
        }
        if (java.lang.Float.compare(options.pulseAlpha, pulseAlpha) != 0) {
            return false
        }

        return layerBelow == options.layerBelow
    }

    @Suppress("LongMethod")
    override fun hashCode(): Int {
        var result = if (accuracyAlpha != +0.0f) accuracyAlpha.toBits() else 0
        result = 31 * result + accuracyColor
        result = 31 * result + backgroundDrawableStale
        result = 31 * result + (backgroundStaleName?.hashCode() ?: 0)
        result = 31 * result + foregroundDrawableStale
        result = 31 * result + (foregroundStaleName?.hashCode() ?: 0)
        result = 31 * result + gpsDrawable
        result = 31 * result + (gpsName?.hashCode() ?: 0)
        result = 31 * result + foregroundDrawable
        result = 31 * result + (foregroundName?.hashCode() ?: 0)
        result = 31 * result + backgroundDrawable
        result = 31 * result + (backgroundName?.hashCode() ?: 0)
        result = 31 * result + bearingDrawable
        result = 31 * result + (bearingName?.hashCode() ?: 0)
        result = 31 * result + (bearingTintColor?.hashCode() ?: 0)
        result = 31 * result + (foregroundTintColor?.hashCode() ?: 0)
        result = 31 * result + (backgroundTintColor?.hashCode() ?: 0)
        result = 31 * result + (foregroundStaleTintColor?.hashCode() ?: 0)
        result = 31 * result + (backgroundStaleTintColor?.hashCode() ?: 0)
        result = 31 * result + if (elevation != +0.0f) elevation.toBits() else 0
        result = 31 * result + if (enableStaleState) 1 else 0
        result = 31 * result + (staleStateTimeout xor (staleStateTimeout ushr 32)).toInt()
        result = 31 * result + Arrays.hashCode(padding)
        result = 31 * result + if (maxZoomIconScale != +0.0f) maxZoomIconScale.toBits() else 0
        result = 31 * result + if (minZoomIconScale != +0.0f) minZoomIconScale.toBits() else 0
        result = 31 * result + if (trackingGesturesManagement) 1 else 0
        result = 31 * result + if (trackingInitialMoveThreshold != +0.0f) trackingInitialMoveThreshold.toBits() else 0
        result = 31 * result +
            if (trackingMultiFingerMoveThreshold != +0.0f) trackingMultiFingerMoveThreshold.toBits() else 0
        result = 31 * result + (trackingMultiFingerProtectedMoveArea?.hashCode() ?: 0)
        result = 31 * result + (layerAbove?.hashCode() ?: 0)
        result = 31 * result + (layerBelow?.hashCode() ?: 0)
        result = 31 * result + if (bearingOnTop) 1 else 0
        result = 31 * result +
            if (trackingAnimationDurationMultiplier != +0.0f) trackingAnimationDurationMultiplier.toBits() else 0
        result = 31 * result + if (compassAnimationEnabled) 1 else 0
        result = 31 * result + if (accuracyAnimationEnabled) 1 else 0
        result = 31 * result + if (pulseEnabled == true) 1 else 0
        result = 31 * result + if (pulseFadeEnabled == true) 1 else 0
        result = 31 * result + (pulseColor?.hashCode() ?: 0)
        result = 31 * result + if (pulseSingleDuration != +0.0f) pulseSingleDuration.toBits() else 0
        result = 31 * result + if (pulseMaxRadius != +0.0f) pulseMaxRadius.toBits() else 0
        result = 31 * result + if (pulseAlpha != +0.0f) pulseAlpha.toBits() else 0
        return result
    }

    override fun describeContents(): Int = 0

    override fun writeToParcel(
        dest: Parcel,
        flags: Int,
    ) {
        dest.writeFloat(this.accuracyAlpha)
        dest.writeInt(this.accuracyColor)
        dest.writeInt(this.backgroundDrawableStale)
        dest.writeString(this.backgroundStaleName)
        dest.writeInt(this.foregroundDrawableStale)
        dest.writeString(this.foregroundStaleName)
        dest.writeInt(this.gpsDrawable)
        dest.writeString(this.gpsName)
        dest.writeInt(this.foregroundDrawable)
        dest.writeString(this.foregroundName)
        dest.writeInt(this.backgroundDrawable)
        dest.writeString(this.backgroundName)
        dest.writeInt(this.bearingDrawable)
        dest.writeString(this.bearingName)
        dest.writeValue(this.bearingTintColor)
        dest.writeValue(this.foregroundTintColor)
        dest.writeValue(this.backgroundTintColor)
        dest.writeValue(this.foregroundStaleTintColor)
        dest.writeValue(this.backgroundStaleTintColor)
        dest.writeFloat(this.elevation)
        dest.writeByte(if (this.enableStaleState) 1 else 0)
        dest.writeLong(this.staleStateTimeout)
        dest.writeIntArray(this.padding)
        dest.writeFloat(this.maxZoomIconScale)
        dest.writeFloat(this.minZoomIconScale)
        dest.writeByte(if (this.trackingGesturesManagement) 1 else 0)
        dest.writeFloat(this.trackingInitialMoveThreshold)
        dest.writeFloat(this.trackingMultiFingerMoveThreshold)
        dest.writeParcelable(this.trackingMultiFingerProtectedMoveArea, flags)
        dest.writeString(this.layerAbove)
        dest.writeString(this.layerBelow)
        dest.writeByte(if (this.bearingOnTop) 1 else 0)
        dest.writeFloat(this.trackingAnimationDurationMultiplier)
        dest.writeByte(if (this.compassAnimationEnabled) 1 else 0)
        dest.writeByte(if (this.accuracyAnimationEnabled) 1 else 0)
        dest.writeValue(this.pulseEnabled)
        dest.writeValue(this.pulseFadeEnabled)
        dest.writeValue(this.pulseColor)
        dest.writeFloat(this.pulseSingleDuration)
        dest.writeFloat(this.pulseMaxRadius)
        dest.writeFloat(this.pulseAlpha)
    }

    /**
     * Builder class for constructing a new instance of [LocationComponentOptions].
     */
    class Builder {
        private var accuracyAlpha: Float? = null
        private var accuracyColor: Int? = null
        private var backgroundDrawableStale: Int? = null
        private var backgroundStaleName: String? = null
        private var foregroundDrawableStale: Int? = null
        private var foregroundStaleName: String? = null
        private var gpsDrawable: Int? = null
        private var gpsName: String? = null
        private var foregroundDrawable: Int? = null
        private var foregroundName: String? = null
        private var backgroundDrawable: Int? = null
        private var backgroundName: String? = null
        private var bearingDrawable: Int? = null
        private var bearingName: String? = null
        private var bearingTintColor: Int? = null
        private var foregroundTintColor: Int? = null
        private var backgroundTintColor: Int? = null
        private var foregroundStaleTintColor: Int? = null
        private var backgroundStaleTintColor: Int? = null
        private var elevation: Float? = null
        private var enableStaleState: Boolean? = null
        private var staleStateTimeout: Long? = null
        private var padding: IntArray? = null
        private var maxZoomIconScale: Float? = null
        private var minZoomIconScale: Float? = null
        private var trackingGesturesManagement: Boolean? = null
        private var trackingInitialMoveThreshold: Float? = null
        private var trackingMultiFingerMoveThreshold: Float? = null
        private var trackingMultiFingerProtectedMoveArea: RectF? = null
        private var layerAbove: String? = null
        private var layerBelow: String? = null
        private var bearingOnTop = false
        private var trackingAnimationDurationMultiplier: Float? = null
        private var compassAnimationEnabled: Boolean? = null
        private var accuracyAnimationEnabled: Boolean? = null
        private var pulseEnabled: Boolean? = null
        private var pulseFadeEnabled: Boolean? = null
        private var pulseColor = 0
        private var pulseSingleDuration = 0f
        private var pulseMaxRadius = 0f
        private var pulseAlpha = 0f
        private var pulseInterpolator: Interpolator? = null

        internal constructor()

        internal constructor(source: LocationComponentOptions) {
            this.accuracyAlpha = source.accuracyAlpha()
            this.accuracyColor = source.accuracyColor()
            this.backgroundDrawableStale = source.backgroundDrawableStale()
            this.backgroundStaleName = source.backgroundStaleName()
            this.foregroundDrawableStale = source.foregroundDrawableStale()
            this.foregroundStaleName = source.foregroundStaleName()
            this.gpsDrawable = source.gpsDrawable()
            this.gpsName = source.gpsName()
            this.foregroundDrawable = source.foregroundDrawable()
            this.foregroundName = source.foregroundName()
            this.backgroundDrawable = source.backgroundDrawable()
            this.backgroundName = source.backgroundName()
            this.bearingDrawable = source.bearingDrawable()
            this.bearingName = source.bearingName()
            this.bearingTintColor = source.bearingTintColor()
            this.foregroundTintColor = source.foregroundTintColor()
            this.backgroundTintColor = source.backgroundTintColor()
            this.foregroundStaleTintColor = source.foregroundStaleTintColor()
            this.backgroundStaleTintColor = source.backgroundStaleTintColor()
            this.elevation = source.elevation()
            this.enableStaleState = source.enableStaleState()
            this.staleStateTimeout = source.staleStateTimeout()
            this.padding = source.padding()
            this.maxZoomIconScale = source.maxZoomIconScale()
            this.minZoomIconScale = source.minZoomIconScale()
            this.trackingGesturesManagement = source.trackingGesturesManagement()
            this.trackingInitialMoveThreshold = source.trackingInitialMoveThreshold()
            this.trackingMultiFingerMoveThreshold = source.trackingMultiFingerMoveThreshold()
            this.trackingMultiFingerProtectedMoveArea = source.trackingMultiFingerProtectedMoveArea()
            this.layerAbove = source.layerAbove()
            this.layerBelow = source.layerBelow()
            this.bearingOnTop = source.bearingOnTop()
            this.trackingAnimationDurationMultiplier = source.trackingAnimationDurationMultiplier()
            this.compassAnimationEnabled = source.compassAnimationEnabled()
            this.accuracyAnimationEnabled = source.accuracyAnimationEnabled()
            this.pulseEnabled = source.pulseEnabled
            this.pulseFadeEnabled = source.pulseFadeEnabled
            this.pulseColor = source.pulseColor!!
            this.pulseSingleDuration = source.pulseSingleDuration
            this.pulseMaxRadius = source.pulseMaxRadius
            this.pulseAlpha = source.pulseAlpha
            this.pulseInterpolator = source.pulseInterpolator
        }

        /**
         * Build a new instance of this [LocationComponentOptions] class.
         *
         * @return a new instance of [LocationComponentOptions]
         */
        fun build(): LocationComponentOptions {
            val locationComponentOptions = autoBuild()
            require(locationComponentOptions.accuracyAlpha() >= 0 && locationComponentOptions.accuracyAlpha() <= 1) {
                "Accuracy alpha value must be between 0.0 and 1.0."
            }

            require(locationComponentOptions.elevation() >= 0f) {
                "Invalid shadow size " + locationComponentOptions.elevation() + ". Must be >= 0"
            }

            require(locationComponentOptions.layerAbove() == null || locationComponentOptions.layerBelow() == null) {
                "You cannot set both layerAbove and layerBelow options. Choose one or the other."
            }

            if (locationComponentOptions.pulseEnabled() == null) {
                var pulsingSetupError = ""
                if (locationComponentOptions.pulseFadeEnabled() != null) {
                    pulsingSetupError += " pulseFadeEnabled"
                }
                if (locationComponentOptions.pulseColor() != null) {
                    pulsingSetupError += " pulseColor"
                }
                if (locationComponentOptions.pulseSingleDuration() > 0) {
                    pulsingSetupError += " pulseSingleDuration"
                }
                if (locationComponentOptions.pulseMaxRadius() > 0) {
                    pulsingSetupError += " pulseMaxRadius"
                }
                if (locationComponentOptions.pulseAlpha() >= 0 && locationComponentOptions.pulseAlpha() <= 1) {
                    pulsingSetupError += " pulseAlpha"
                }
                if (locationComponentOptions.pulseInterpolator() != null) {
                    pulsingSetupError += " pulseInterpolator"
                }
                check(pulsingSetupError.isEmpty()) {
                    "You've set up the following pulsing circle options but have not enabled the pulsing circle " +
                        "via the LocationComponentOptions builder:" + pulsingSetupError +
                        ". Enable the pulsing circle if you're going to set pulsing options."
                }
            }
            return locationComponentOptions
        }

        /**
         * Set the opacity of the accuracy view to a value from 0 to 1, where 0 means the accuracy view
         * is completely transparent and 1 means the view is completely opaque.
         * References style attribute R.styleable#LocationComponent_accuracyAlpha
         *
         * @param accuracyAlpha the opacity of the accuracy view
         * @return this builder for chaining options together
         */
        fun accuracyAlpha(accuracyAlpha: Float): Builder {
            this.accuracyAlpha = accuracyAlpha
            return this
        }

        /**
         * Solid color to use as the accuracy view color property.
         * References style attribute R.styleable#LocationComponent_accuracyColor
         *
         * @param accuracyColor the color of the accuracy view
         * @return this builder for chaining options together
         */
        fun accuracyColor(accuracyColor: Int): Builder {
            this.accuracyColor = accuracyColor
            return this
        }

        /**
         * Defines the drawable used for the stale background icon.
         * References style attribute R.styleable#LocationComponent_backgroundDrawableStale
         *
         * @param backgroundDrawableStale the drawable resource ID
         * @return this builder for chaining options together
         */
        fun backgroundDrawableStale(backgroundDrawableStale: Int): Builder {
            this.backgroundDrawableStale = backgroundDrawableStale
            return this
        }

        /**
         * Given a String image name, identical to one used in
         * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
         * component, will used this image in place of the provided or default maplibre_backgroundDrawableStale.
         *
         * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
         * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
         *
         * @param backgroundStaleName String icon or maki-icon name
         * @return this builder for chaining options together
         */
        fun backgroundStaleName(backgroundStaleName: String?): Builder {
            this.backgroundStaleName = backgroundStaleName
            return this
        }

        /**
         * Defines the drawable used for the stale foreground icon.
         * References style attribute R.styleable#LocationComponent_foregroundDrawableStale
         *
         * @param foregroundDrawableStale the drawable resource ID
         * @return this builder for chaining options together
         */
        fun foregroundDrawableStale(foregroundDrawableStale: Int): Builder {
            this.foregroundDrawableStale = foregroundDrawableStale
            return this
        }

        /**
         * Given a String image name, identical to one used in
         * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
         * component, will used this image in place of the provided or default maplibre_foregroundDrawableStale.
         *
         * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
         * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
         *
         * @param foregroundStaleName String icon or maki-icon name
         * @return this builder for chaining options together
         */
        fun foregroundStaleName(foregroundStaleName: String?): Builder {
            this.foregroundStaleName = foregroundStaleName
            return this
        }

        /**
         * Defines the drawable used for the navigation state icon.
         * References style attribute R.styleable#LocationComponent_gpsDrawable
         *
         * @param gpsDrawable the drawable resource ID
         * @return this builder for chaining options together
         */
        fun gpsDrawable(gpsDrawable: Int): Builder {
            this.gpsDrawable = gpsDrawable
            return this
        }

        /**
         * Given a String image name, identical to one used in
         * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
         * component, will used this image in place of the provided or default maplibre_gpsDrawable.
         *
         * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
         * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
         *
         * @param gpsName String icon or maki-icon name
         * @return this builder for chaining options together
         */
        fun gpsName(gpsName: String?): Builder {
            this.gpsName = gpsName
            return this
        }

        /**
         * Supply a Drawable that is to be rendered on top of all of the content in the Location Component layer stack.
         * References style attribute R.styleable#LocationComponent_foregroundDrawable
         *
         * @param foregroundDrawable the drawable resource used for the foreground layer
         * @return this builder for chaining options together
         */
        fun foregroundDrawable(foregroundDrawable: Int): Builder {
            this.foregroundDrawable = foregroundDrawable
            return this
        }

        /**
         * Given a String image name, identical to one used in
         * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
         * component, will used this image in place of the provided or default maplibre_foregroundDrawable.
         *
         * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
         * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
         *
         * @param foregroundName String icon or maki-icon name
         * @return this builder for chaining options together
         */
        fun foregroundName(foregroundName: String?): Builder {
            this.foregroundName = foregroundName
            return this
        }

        /**
         * Defines the drawable used for the background state icon.
         * References style attribute R.styleable#LocationComponent_backgroundDrawable
         *
         * @param backgroundDrawable the drawable resource ID
         * @return this builder for chaining options together
         */
        fun backgroundDrawable(backgroundDrawable: Int): Builder {
            this.backgroundDrawable = backgroundDrawable
            return this
        }

        /**
         * Given a String image name, identical to one used in
         * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
         * component, will used this image in place of the provided or default maplibre_backgroundDrawable.
         *
         * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
         * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
         *
         * @param backgroundName String icon or maki-icon name
         * @return this builder for chaining options together
         */
        fun backgroundName(backgroundName: String?): Builder {
            this.backgroundName = backgroundName
            return this
        }

        /**
         * Defines the drawable used for the bearing icon.
         * References style attribute R.styleable#LocationComponent_bearingDrawable
         *
         * @param bearingDrawable the drawable resource ID
         * @return this builder for chaining options together
         */
        fun bearingDrawable(bearingDrawable: Int): Builder {
            this.bearingDrawable = bearingDrawable
            return this
        }

        /**
         * Given a String image name, identical to one used in
         * the first parameter of [org.maplibre.android.maps.Style.Builder.addImage], the
         * component, will used this image in place of the provided or default maplibre_bearingDrawable.
         *
         * A maki-icon name (example: "circle-15") may also be provided. These are images that can be loaded
         * with certain styles. Note, this will fail if the provided icon name is not provided by the loaded map style.
         *
         * @param bearingName String icon or maki-icon name
         * @return this builder for chaining options together
         */
        fun bearingName(bearingName: String?): Builder {
            this.bearingName = bearingName
            return this
        }

        /**
         * Defines the bearing icon color as an integer.
         * References style attribute R.styleable#LocationComponent_bearingTintColor
         *
         * @param bearingTintColor the color integer resource
         * @return this builder for chaining options together
         */
        fun bearingTintColor(bearingTintColor: Int?): Builder {
            this.bearingTintColor = bearingTintColor
            return this
        }

        /**
         * Defines the foreground color as an integer.
         * References style attribute R.styleable#LocationComponent_foregroundTintColor
         *
         * @param foregroundTintColor the color integer resource
         * @return this builder for chaining options together
         */
        fun foregroundTintColor(foregroundTintColor: Int?): Builder {
            this.foregroundTintColor = foregroundTintColor
            return this
        }

        /**
         * Defines the background color as an integer.
         * References style attribute R.styleable#LocationComponent_backgroundTintColor
         *
         * @param backgroundTintColor the color integer resource
         * @return this builder for chaining options together
         */
        fun backgroundTintColor(backgroundTintColor: Int?): Builder {
            this.backgroundTintColor = backgroundTintColor
            return this
        }

        /**
         * Defines the foreground stale color as an integer.
         * References style attribute R.styleable#LocationComponent_foregroundStaleTintColor
         *
         * @param foregroundStaleTintColor the color integer resource
         * @return this builder for chaining options together
         */
        fun foregroundStaleTintColor(foregroundStaleTintColor: Int?): Builder {
            this.foregroundStaleTintColor = foregroundStaleTintColor
            return this
        }

        /**
         * Defines the background stale color as an integer.
         * References style attribute R.styleable#LocationComponent_backgroundStaleTintColor
         *
         * @param backgroundStaleTintColor the color integer resource
         * @return this builder for chaining options together
         */
        fun backgroundStaleTintColor(backgroundStaleTintColor: Int?): Builder {
            this.backgroundStaleTintColor = backgroundStaleTintColor
            return this
        }

        /**
         * Sets the base elevation of this view, in pixels.
         * References style attribute R.styleable#LocationComponent_elevation
         *
         * @param elevation the elevation currently set for the location icon
         * @return this builder for chaining options together
         */
        fun elevation(elevation: Float): Builder {
            this.elevation = elevation
            return this
        }

        /**
         * Enable or disable to stale state mode. This mode indicates to the user that the location
         * being displayed on the map hasn't been updated in a specific amount of time.
         * References style attribute R.styleable#LocationComponent_enableStaleState
         *
         * @param enabled whether the stale state mode is enabled or not
         * @return this builder for chaining options together
         */
        fun enableStaleState(enabled: Boolean): Builder {
            this.enableStaleState = enabled
            return this
        }

        /**
         * Set the timeout before the location icon becomes stale. The timer begins approximately when a
         * new location update comes in and using this defined time, if an update hasn't occurred by the
         * end, the location is considered stale.
         * References style attribute R.styleable#LocationComponent_staleStateTimeout
         *
         * @param timeout the duration in milliseconds which it should take before the location is
         *                considered stale
         * @return this builder for chaining options together
         */
        fun staleStateTimeout(timeout: Long): Builder {
            this.staleStateTimeout = timeout
            return this
        }

        /**
         * Sets the distance from the edges of the map view’s frame to the edges of the map
         * view’s logical viewport.
         *
         * When the value of this property is equal to {0,0,0,0}, viewport
         * properties such as `centerCoordinate` assume a viewport that matches the map
         * view’s frame. Otherwise, those properties are inset, excluding part of the
         * frame from the viewport. For instance, if the only the top edge is inset, the
         * map center is effectively shifted downward.
         *
         * @param padding The margins for the map in pixels (left, top, right, bottom).
         */
        @Deprecated(
            "Use org.maplibre.android.camera.CameraPosition.Builder.padding or " +
                "org.maplibre.android.camera.CameraUpdateFactory.paddingTo instead.",
        )
        fun padding(padding: IntArray?): Builder {
            if (padding == null) {
                throw NullPointerException("Null padding")
            }
            this.padding = padding
            return this
        }

        /**
         * Sets the scale factor of the location icon when the map is zoomed in.
         * Scaling is linear and the new pixel size of the image will be the original pixel size multiplied
         * by the argument.
         *
         * Set both this and [minZoomIconScale] to 1f to disable location icon scaling.
         *
         * Scaling is based on the maps minimum and maximum zoom levels in time of component's style application.
         *
         * @param maxZoomIconScale icon scale factor
         */
        fun maxZoomIconScale(maxZoomIconScale: Float): Builder {
            this.maxZoomIconScale = maxZoomIconScale
            return this
        }

        /**
         * Sets the scale factor of the location icon when the map is zoomed out.
         * Scaling is linear and the new pixel size of the image will be the original pixel size multiplied
         * by the argument.
         *
         * Set both this and [maxZoomIconScale] to 1f to disable location icon scaling.
         *
         * Scaling is based on the maps minimum and maximum zoom levels in time of component's style application.
         *
         * @param minZoomIconScale icon scale factor
         */
        fun minZoomIconScale(minZoomIconScale: Float): Builder {
            this.minZoomIconScale = minZoomIconScale
            return this
        }

        /**
         * Set whether gesture threshold should be adjusted when camera is in one of the tracking modes.
         * This will adjust the focal point and increase thresholds to enable camera manipulation,
         * like zooming in and out, without breaking tracking.
         *
         * **Note**: This can overwrite some of the gesture thresholds
         * and the custom [org.maplibre.android.gestures.AndroidGesturesManager] that was set with
         * [org.maplibre.android.maps.MapLibreMap.setGesturesManager].
         *
         * @param trackingGesturesManagement true if gestures should be adjusted when in one of the camera
         *                                   tracking modes, false otherwise
         * @see Builder.trackingInitialMoveThreshold
         * @see Builder.trackingMultiFingerMoveThreshold
         * @see Builder.trackingMultiFingerProtectedMoveArea
         */
        fun trackingGesturesManagement(trackingGesturesManagement: Boolean): Builder {
            this.trackingGesturesManagement = trackingGesturesManagement
            return this
        }

        /**
         * Sets minimum single pointer movement (map pan) in pixels required to break camera tracking.
         *
         * @param moveThreshold the minimum movement
         */
        fun trackingInitialMoveThreshold(moveThreshold: Float): Builder {
            this.trackingInitialMoveThreshold = moveThreshold
            return this
        }

        /**
         * Sets minimum multi pointer movement (map pan) in pixels required to break camera tracking
         * (for example during scale gesture).
         *
         * @param moveThreshold the minimum movement
         */
        fun trackingMultiFingerMoveThreshold(moveThreshold: Float): Builder {
            this.trackingMultiFingerMoveThreshold = moveThreshold
            return this
        }

        /**
         * Sets protected multi pointer gesture area.
         * When the camera is in a tracking mode, any multi finger gesture with focal
         * point inside the provided screen coordinate rectangle is not going to break the tracking.
         *
         * Best paired with the [Builder.trackingMultiFingerMoveThreshold]
         * set to 0 or a relatively small value to not interfere with gestures outside of the defined rectangle.
         *
         * @param rect the protected multi finger area while camera is tracking
         */
        fun trackingMultiFingerProtectedMoveArea(rect: RectF?): Builder {
            this.trackingMultiFingerProtectedMoveArea = rect
            return this
        }

        /**
         * Sets the id of the layer that's referenced when placing the component on the map using
         * [org.maplibre.android.maps.Style.addLayerAbove].
         *
         * The component is going to placed directly above this layer.
         *
         * @param layerAbove the id of the layer the component is going to placed directly above.
         */
        fun layerAbove(layerAbove: String?): Builder {
            this.layerAbove = layerAbove
            return this
        }

        /**
         * Sets the id of the layer that's referenced when placing the component on the map using
         * [org.maplibre.android.maps.Style.addLayerBelow].
         *
         * The component is going to placed directly below this layer.
         *
         * @param layerBelow the id of the layer the component is going to placed directly below.
         */
        fun layerBelow(layerBelow: String?): Builder {
            this.layerBelow = layerBelow
            return this
        }

        /**
         * Sets whether the bearing icon is rendered on top of the foreground icon in the location
         * layer stack. When true, the bearing layer is placed above the foreground layer; when false,
         * it is rendered below.
         *
         * @param bearingOnTop true to render the bearing icon above the foreground icon, false for below.
         */
        fun bearingOnTop(bearingOnTop: Boolean): Builder {
            this.bearingOnTop = bearingOnTop
            return this
        }

        /**
         * Sets the tracking animation duration multiplier.
         *
         * @param trackingAnimationDurationMultiplier the tracking animation duration multiplier
         */
        fun trackingAnimationDurationMultiplier(trackingAnimationDurationMultiplier: Float): Builder {
            this.trackingAnimationDurationMultiplier = trackingAnimationDurationMultiplier
            return this
        }

        /**
         * Enable or disable smooth animation of compass values for [org.maplibre.android.location.modes.CameraMode]
         * and [org.maplibre.android.location.modes.RenderMode].
         *
         * @return whether smooth compass animation is enabled
         */
        fun compassAnimationEnabled(compassAnimationEnabled: Boolean?): Builder {
            this.compassAnimationEnabled = compassAnimationEnabled
            return this
        }

        /**
         * Enable or disable smooth animation of the accuracy circle around the user's position.
         *
         * @return whether smooth animation of the accuracy circle is enabled
         */
        fun accuracyAnimationEnabled(accuracyAnimationEnabled: Boolean): Builder {
            this.accuracyAnimationEnabled = accuracyAnimationEnabled
            return this
        }

        /**
         * Enable or disable the LocationComponent's pulsing circle.
         *
         * @return whether the LocationComponent's pulsing circle is enabled
         */
        fun pulseEnabled(pulseEnabled: Boolean): Builder {
            this.pulseEnabled = pulseEnabled
            return this
        }

        /**
         * Enable or disable fading of the LocationComponent's pulsing circle. If it fades, the circle's
         * opacity decreases as its radius increases.
         *
         * @return whether fading of the LocationComponent's pulsing circle is enabled
         */
        fun pulseFadeEnabled(pulseFadeEnabled: Boolean): Builder {
            this.pulseFadeEnabled = pulseFadeEnabled
            return this
        }

        /**
         * Sets the color of the LocationComponent's pulsing circle.
         *
         * @return the current set color of the circle
         */
        fun pulseColor(
            @ColorInt pulseColor: Int,
        ): Builder {
            this.pulseColor = pulseColor
            return this
        }

        /**
         * Sets the number of milliseconds it takes for a single pulse of the LocationComponent's pulsing circle.
         *
         * @return the current set length of time for a single pulse
         */
        fun pulseSingleDuration(pulseSingleDuration: Float): Builder {
            this.pulseSingleDuration = pulseSingleDuration
            return this
        }

        /**
         * The maximum radius that a single pulse should expand the LocationComponent's pulsing circle to.
         *
         * @return the maximum radius that the pulsing circle will expand to.
         */
        fun pulseMaxRadius(pulseMaxRadius: Float): Builder {
            this.pulseMaxRadius = pulseMaxRadius
            return this
        }

        /**
         * Sets the opacity of the LocationComponent's pulsing circle. The expected range is
         * 0 to 1. An opacity of 1 makes the layer fully visible.
         *
         * @return the current opacity of the LocationComponent's pulsing circle
         */
        fun pulseAlpha(pulseAlpha: Float): Builder {
            this.pulseAlpha = pulseAlpha
            return this
        }

        /**
         * Sets the pulsing circle's interpolator animation.
         *
         * @param pulseInterpolator the type of Android-system interpolator to use when
         *                          creating the pulsing animation
         * @return a String which represents the interpolator animation that the pulsing circle will use.
         */
        fun pulseInterpolator(pulseInterpolator: Interpolator?): Builder {
            this.pulseInterpolator = pulseInterpolator
            return this
        }

        @Suppress("CyclomaticComplexMethod", "LongMethod")
        internal fun autoBuild(): LocationComponentOptions {
            var missing = ""
            if (this.accuracyAlpha == null) {
                missing += " accuracyAlpha"
            }
            if (this.accuracyColor == null) {
                missing += " accuracyColor"
            }
            if (this.backgroundDrawableStale == null) {
                missing += " backgroundDrawableStale"
            }
            if (this.foregroundDrawableStale == null) {
                missing += " foregroundDrawableStale"
            }
            if (this.gpsDrawable == null) {
                missing += " gpsDrawable"
            }
            if (this.foregroundDrawable == null) {
                missing += " foregroundDrawable"
            }
            if (this.backgroundDrawable == null) {
                missing += " backgroundDrawable"
            }
            if (this.bearingDrawable == null) {
                missing += " bearingDrawable"
            }
            if (this.elevation == null) {
                missing += " elevation"
            }
            if (this.enableStaleState == null) {
                missing += " enableStaleState"
            }
            if (this.staleStateTimeout == null) {
                missing += " staleStateTimeout"
            }
            if (this.padding == null) {
                missing += " padding"
            }
            if (this.maxZoomIconScale == null) {
                missing += " maxZoomIconScale"
            }
            if (this.minZoomIconScale == null) {
                missing += " minZoomIconScale"
            }
            if (this.trackingGesturesManagement == null) {
                missing += " trackingGesturesManagement"
            }
            if (this.trackingInitialMoveThreshold == null) {
                missing += " trackingInitialMoveThreshold"
            }
            if (this.trackingMultiFingerMoveThreshold == null) {
                missing += " trackingMultiFingerMoveThreshold"
            }
            if (this.trackingAnimationDurationMultiplier == null) {
                missing += " trackingAnimationDurationMultiplier"
            }
            check(missing.isEmpty()) { "Missing required properties:$missing" }
            return LocationComponentOptions(
                this.accuracyAlpha!!,
                this.accuracyColor!!,
                this.backgroundDrawableStale!!,
                this.backgroundStaleName,
                this.foregroundDrawableStale!!,
                this.foregroundStaleName,
                this.gpsDrawable!!,
                this.gpsName,
                this.foregroundDrawable!!,
                this.foregroundName,
                this.backgroundDrawable!!,
                this.backgroundName,
                this.bearingDrawable!!,
                this.bearingName,
                this.bearingTintColor,
                this.foregroundTintColor,
                this.backgroundTintColor,
                this.foregroundStaleTintColor,
                this.backgroundStaleTintColor,
                this.elevation!!,
                this.enableStaleState!!,
                this.staleStateTimeout!!,
                this.padding,
                this.maxZoomIconScale!!,
                this.minZoomIconScale!!,
                this.trackingGesturesManagement!!,
                this.trackingInitialMoveThreshold!!,
                this.trackingMultiFingerMoveThreshold!!,
                this.trackingMultiFingerProtectedMoveArea,
                this.layerAbove,
                this.layerBelow,
                this.bearingOnTop,
                this.trackingAnimationDurationMultiplier!!,
                this.compassAnimationEnabled!!,
                this.accuracyAnimationEnabled!!,
                this.pulseEnabled,
                this.pulseFadeEnabled,
                this.pulseColor,
                this.pulseSingleDuration,
                this.pulseMaxRadius,
                this.pulseAlpha,
                this.pulseInterpolator,
            )
        }
    }

    companion object {
        /**
         * Default accuracy alpha
         */
        private const val ACCURACY_ALPHA_DEFAULT = 0.15f

        /**
         * Default icon scale factor when the map is zoomed out
         */
        private const val MIN_ZOOM_ICON_SCALE_DEFAULT = 0.6f

        /**
         * Default icon scale factor when the map is zoomed in
         */
        private const val MAX_ZOOM_ICON_SCALE_DEFAULT = 1f

        /**
         * Default map padding
         */
        private val PADDING_DEFAULT = intArrayOf(0, 0, 0, 0)

        /**
         * The default value which is used when the stale state is enabled
         */
        private const val STALE_STATE_DELAY_MS = 30_000L

        /**
         * Default animation duration multiplier
         */
        private const val TRACKING_ANIMATION_DURATION_MULTIPLIER_DEFAULT = 1.1f

        /**
         * Default duration of a single LocationComponent circle pulse.
         */
        private const val CIRCLE_PULSING_DURATION_DEFAULT_MS = 2300f

        /**
         * Default opacity of the LocationComponent circle when it ends a single pulse.
         */
        private const val CIRCLE_PULSING_ALPHA_DEFAULT = 1f

        /**
         * Default maximum radius of the LocationComponent circle when it's pulsing.
         */
        const val CIRCLE_PULSING_MAX_RADIUS_DEFAULT = 35f

        @JvmField
        val CREATOR: Parcelable.Creator<LocationComponentOptions> =
            object : Parcelable.Creator<LocationComponentOptions> {
                override fun createFromParcel(source: Parcel): LocationComponentOptions = LocationComponentOptions(source)

                override fun newArray(size: Int): Array<LocationComponentOptions?> = arrayOfNulls(size)
            }

        /**
         * Construct a new Location Component Options class using the attributes found within a style
         * resource. It's important to note that you only need to define the attributes you plan to
         * change and can safely ignore the other attributes which will be set to their default value.
         *
         * @param context  your activity's context used for acquiring resources
         * @param styleRes the style id where your custom attributes are defined
         * @return a new [LocationComponentOptions] object with the settings you defined in your style
         * resource
         */
        @JvmStatic
        @Suppress("LongMethod", "DEPRECATION")
        fun createFromAttributes(
            context: Context,
            @StyleRes styleRes: Int,
        ): LocationComponentOptions {
            val typedArray = context.obtainStyledAttributes(styleRes, R.styleable.maplibre_LocationComponent)

            val builder =
                Builder()
                    .enableStaleState(true)
                    .staleStateTimeout(STALE_STATE_DELAY_MS)
                    .maxZoomIconScale(MAX_ZOOM_ICON_SCALE_DEFAULT)
                    .minZoomIconScale(MIN_ZOOM_ICON_SCALE_DEFAULT)
                    .padding(PADDING_DEFAULT)

            builder.foregroundDrawable(
                typedArray.getResourceId(R.styleable.maplibre_LocationComponent_maplibre_foregroundDrawable, -1),
            )
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_foregroundTintColor)) {
                builder.foregroundTintColor(
                    typedArray.getColor(R.styleable.maplibre_LocationComponent_maplibre_foregroundTintColor, -1),
                )
            }
            builder.backgroundDrawable(
                typedArray.getResourceId(R.styleable.maplibre_LocationComponent_maplibre_backgroundDrawable, -1),
            )
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_backgroundTintColor)) {
                builder.backgroundTintColor(
                    typedArray.getColor(R.styleable.maplibre_LocationComponent_maplibre_backgroundTintColor, -1),
                )
            }
            builder.foregroundDrawableStale(
                typedArray.getResourceId(R.styleable.maplibre_LocationComponent_maplibre_foregroundDrawableStale, -1),
            )
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_foregroundStaleTintColor)) {
                builder.foregroundStaleTintColor(
                    typedArray.getColor(R.styleable.maplibre_LocationComponent_maplibre_foregroundStaleTintColor, -1),
                )
            }
            builder.backgroundDrawableStale(
                typedArray.getResourceId(R.styleable.maplibre_LocationComponent_maplibre_backgroundDrawableStale, -1),
            )
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_backgroundStaleTintColor)) {
                builder.backgroundStaleTintColor(
                    typedArray.getColor(R.styleable.maplibre_LocationComponent_maplibre_backgroundStaleTintColor, -1),
                )
            }
            builder.bearingDrawable(
                typedArray.getResourceId(R.styleable.maplibre_LocationComponent_maplibre_bearingDrawable, -1),
            )
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_bearingTintColor)) {
                builder.bearingTintColor(
                    typedArray.getColor(R.styleable.maplibre_LocationComponent_maplibre_bearingTintColor, -1),
                )
            }
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_enableStaleState)) {
                builder.enableStaleState(
                    typedArray.getBoolean(R.styleable.maplibre_LocationComponent_maplibre_enableStaleState, true),
                )
            }
            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_staleStateTimeout)) {
                builder.staleStateTimeout(
                    typedArray
                        .getInteger(
                            R.styleable.maplibre_LocationComponent_maplibre_staleStateTimeout,
                            STALE_STATE_DELAY_MS.toInt(),
                        ).toLong(),
                )
            }
            builder.gpsDrawable(typedArray.getResourceId(R.styleable.maplibre_LocationComponent_maplibre_gpsDrawable, -1))
            val elevation = typedArray.getDimension(R.styleable.maplibre_LocationComponent_maplibre_elevation, 0f)
            builder.accuracyColor(typedArray.getColor(R.styleable.maplibre_LocationComponent_maplibre_accuracyColor, -1))
            builder.accuracyAlpha(
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_accuracyAlpha,
                    ACCURACY_ALPHA_DEFAULT,
                ),
            )
            builder.elevation(elevation)

            builder.trackingGesturesManagement(
                typedArray.getBoolean(
                    R.styleable.maplibre_LocationComponent_maplibre_trackingGesturesManagement,
                    false,
                ),
            )
            builder.trackingInitialMoveThreshold(
                typedArray.getDimension(
                    R.styleable.maplibre_LocationComponent_maplibre_trackingInitialMoveThreshold,
                    context.resources.getDimension(R.dimen.maplibre_locationComponentTrackingInitialMoveThreshold),
                ),
            )
            builder.trackingMultiFingerMoveThreshold(
                typedArray.getDimension(
                    R.styleable.maplibre_LocationComponent_maplibre_trackingMultiFingerMoveThreshold,
                    context.resources.getDimension(R.dimen.maplibre_locationComponentTrackingMultiFingerMoveThreshold),
                ),
            )

            builder.padding(
                intArrayOf(
                    typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingLeft, 0),
                    typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingTop, 0),
                    typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingRight, 0),
                    typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingBottom, 0),
                ),
            )

            builder.layerAbove(typedArray.getString(R.styleable.maplibre_LocationComponent_maplibre_layer_above))

            builder.layerBelow(typedArray.getString(R.styleable.maplibre_LocationComponent_maplibre_layer_below))

            builder.bearingOnTop(
                typedArray.getBoolean(R.styleable.maplibre_LocationComponent_maplibre_bearing_on_top, true),
            )

            val minScale =
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_minZoomIconScale,
                    MIN_ZOOM_ICON_SCALE_DEFAULT,
                )
            val maxScale =
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_maxZoomIconScale,
                    MAX_ZOOM_ICON_SCALE_DEFAULT,
                )
            builder.minZoomIconScale(minScale)
            builder.maxZoomIconScale(maxScale)

            val trackingAnimationDurationMultiplier =
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_trackingAnimationDurationMultiplier,
                    TRACKING_ANIMATION_DURATION_MULTIPLIER_DEFAULT,
                )
            builder.trackingAnimationDurationMultiplier(trackingAnimationDurationMultiplier)

            builder.compassAnimationEnabled(
                typedArray.getBoolean(
                    R.styleable.maplibre_LocationComponent_maplibre_compassAnimationEnabled,
                    true,
                ),
            )

            builder.accuracyAnimationEnabled(
                typedArray.getBoolean(
                    R.styleable.maplibre_LocationComponent_maplibre_accuracyAnimationEnabled,
                    true,
                ),
            )

            builder.pulseEnabled(
                typedArray.getBoolean(
                    R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleEnabled,
                    false,
                ),
            )

            builder.pulseFadeEnabled(
                typedArray.getBoolean(
                    R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleFadeEnabled,
                    true,
                ),
            )

            if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleColor)) {
                builder.pulseColor(
                    typedArray.getColor(
                        R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleColor,
                        -1,
                    ),
                )
            }

            builder.pulseSingleDuration(
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleDuration,
                    CIRCLE_PULSING_DURATION_DEFAULT_MS,
                ),
            )

            builder.pulseMaxRadius(
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleRadius,
                    CIRCLE_PULSING_MAX_RADIUS_DEFAULT,
                ),
            )

            builder.pulseAlpha(
                typedArray.getFloat(
                    R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleAlpha,
                    CIRCLE_PULSING_ALPHA_DEFAULT,
                ),
            )

            typedArray.recycle()

            return builder.build()
        }

        /**
         * Build a new instance of the [LocationComponentOptions] class with all the attributes set
         * automatically to their defined defaults in this library. This allows you to adjust a few
         * attributes while leaving the rest alone and maintaining their default behavior.
         *
         * @param context your activities context used to acquire the style resource
         * @return the builder which contains the default values defined by the style resource
         */
        @JvmStatic
        fun builder(context: Context): Builder = createFromAttributes(context, R.style.maplibre_LocationComponent).toBuilder()
    }
}
