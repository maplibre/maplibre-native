package org.maplibre.android.constants

import java.util.Locale

/**
 * MapLibreConstants exposes MapLibre related constants
 */
object MapLibreConstants {
    /**
     * Default Locale for data processing (ex: String.toLowerCase(MAPBOX_LOCALE, "foo"))
     */
    @JvmField
    val MAPLIBRE_LOCALE: Locale = Locale.US

    /**
     * The name of the desired preferences file for Android's SharedPreferences.
     */
    const val MAPLIBRE_SHARED_PREFERENCES: String = "MapboxSharedPreferences"

    /**
     * Key used to switch storage to external in AndroidManifest.xml
     */
    const val KEY_META_DATA_SET_STORAGE_EXTERNAL: String = "com.mapbox.SetStorageExternal"

    /**
     * Default value for KEY_META_DATA_SET_STORAGE_EXTERNAL (default is internal storage)
     */
    const val DEFAULT_SET_STORAGE_EXTERNAL: Boolean = false

    /**
     * Key used to switch Tile Download Measuring on/off in AndroidManifest.xml
     */
    const val KEY_META_DATA_MEASURE_TILE_DOWNLOAD_ON: String = "com.mapbox.MeasureTileDownloadOn"

    /**
     * Default value for KEY_META_DATA_MEASURE_TILE_DOWNLOAD_ON (default is off)
     */
    const val DEFAULT_MEASURE_TILE_DOWNLOAD_ON: Boolean = false

    /**
     * Default value for font fallback for local ideograph fonts
     */
    const val DEFAULT_FONT: String = "sans-serif"

    /**
     * Unmeasured state
     */
    const val UNMEASURED: Float = -1f

    /**
     * Default animation time
     */
    const val ANIMATION_DURATION: Int = 300

    /**
     * Default short animation time
     */
    const val ANIMATION_DURATION_SHORT: Int = 150

    /**
     * Animation time of a fling gesture, matches [ANIMATION_DURATION_SHORT]
     */
    const val ANIMATION_DURATION_FLING_BASE: Long = 150

    /**
     * Velocity threshold for a fling gesture
     */
    const val VELOCITY_THRESHOLD_IGNORE_FLING: Long = 1000

    /**
     * Vertical angle threshold for a horizontal disabled fling gesture
     */
    const val ANGLE_THRESHOLD_IGNORE_VERTICAL_FLING: Long = 75

    /**
     * Value by which the default rotation threshold will be increased when scaling
     */
    @Deprecated("unused, see org.maplibre.android.maps.UiSettings.setDisableRotateWhenScaling")
    const val ROTATION_THRESHOLD_INCREASE_WHEN_SCALING: Float = 25f

    /**
     * Maximum absolute zoom change for multi-pointer scale velocity animation
     */
    const val MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE: Double = 2.5

    /**
     * Maximum possible zoom change during the quick zoom gesture executed across the whole screen
     */
    const val QUICK_ZOOM_MAX_ZOOM_CHANGE: Double = 4.0

    /**
     * Scale velocity animation duration multiplier.
     */
    const val SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER: Double = 150.0

    /**
     * Minimum angular velocity for rotation animation
     */
    @Deprecated("unused, see [ROTATE_VELOCITY_RATIO_THRESHOLD]")
    const val MINIMUM_ANGULAR_VELOCITY: Float = 1.5f

    /**
     * Last scale span delta to XY velocity ratio required to execute scale velocity animation.
     */
    const val SCALE_VELOCITY_RATIO_THRESHOLD: Double = 4 * 1e-3

    /**
     * Last rotation delta to XY velocity ratio required to execute rotation velocity animation.
     */
    const val ROTATE_VELOCITY_RATIO_THRESHOLD: Double = 2.2 * 1e-4

    /**
     * Time within which user needs to lift fingers for velocity animation to start.
     */
    const val SCHEDULED_ANIMATION_TIMEOUT: Long = 150L

    /**
     * Maximum angular velocity for rotation animation
     */
    const val MAXIMUM_ANGULAR_VELOCITY: Float = 30f

    /**
     * Factor to calculate tilt change based on pixel change during shove gesture.
     */
    const val SHOVE_PIXEL_CHANGE_FACTOR: Float = 0.1f

    /**
     * The currently supported minimum zoom level.
     */
    const val MINIMUM_ZOOM: Float = 0.0f

    /**
     * The currently supported maximum zoom level.
     */
    const val MAXIMUM_ZOOM: Float = 25.5f

    /**
     * The currently supported minimum pitch level.
     */
    const val MINIMUM_PITCH: Float = 0.0f

    /**
     * The currently supported maximum pitch level.
     */
    const val MAXIMUM_PITCH: Float = 60.0f

    /**
     * The currently supported maximum tilt value.
     */
    const val MAXIMUM_TILT: Double = 60.0

    /**
     * The currently supported minimum tilt value.
     */
    const val MINIMUM_TILT: Double = 0.0

    /**
     * The currently supported maximum direction
     */
    const val MAXIMUM_DIRECTION: Double = 360.0

    /**
     * The currently supported minimum direction
     */
    const val MINIMUM_DIRECTION: Double = 0.0

    /**
     * The current default vertical field of view (matches mln::util::DEFAULT_FOV)
     */
    const val DEFAULT_FOV: Double = 36.86989764584402

    /**
     * The currently used minimum scale factor to clamp to when a quick zoom gesture occurs
     */
    @Deprecated("unused")
    const val MINIMUM_SCALE_FACTOR_CLAMP: Float = 0.00f

    /**
     * The currently used maximum scale factor to clamp to when a quick zoom gesture occurs
     */
    @Deprecated("unused")
    const val MAXIMUM_SCALE_FACTOR_CLAMP: Float = 0.15f

    /**
     * Zoom value multiplier for scale gestures.
     */
    const val ZOOM_RATE: Float = 0.65f

    /**
     * Fragment Argument Key for MapLibreMapOptions
     */
    const val FRAG_ARG_MAPLIBREMAPOPTIONS: String = "MapLibreMapOptions"

    /**
     * Layer Id of annotations layer
     */
    const val LAYER_ID_ANNOTATIONS: String = "org.maplibre.annotations.points"

    // Save instance state keys
    const val STATE_HAS_SAVED_STATE: String = "maplibre_savedState"
    const val STATE_CAMERA_POSITION: String = "maplibre_cameraPosition"
    const val STATE_ZOOM_ENABLED: String = "maplibre_zoomEnabled"
    const val STATE_SCROLL_ENABLED: String = "maplibre_scrollEnabled"
    const val STATE_HORIZONAL_SCROLL_ENABLED: String = "maplibre_horizontalScrollEnabled"
    const val STATE_ROTATE_ENABLED: String = "maplibre_rotateEnabled"
    const val STATE_TILT_ENABLED: String = "maplibre_tiltEnabled"
    const val STATE_DOUBLE_TAP_ENABLED: String = "maplibre_doubleTapEnabled"
    const val STATE_QUICK_ZOOM_ENABLED: String = "maplibre_quickZoom"
    const val STATE_ZOOM_RATE: String = "maplibre_zoomRate"
    const val STATE_DEBUG_ACTIVE: String = "maplibre_debugActive"
    const val STATE_COMPASS_ENABLED: String = "maplibre_compassEnabled"
    const val STATE_COMPASS_GRAVITY: String = "maplibre_compassGravity"
    const val STATE_COMPASS_MARGIN_LEFT: String = "maplibre_compassMarginLeft"
    const val STATE_COMPASS_MARGIN_TOP: String = "maplibre_compassMarginTop"
    const val STATE_COMPASS_MARGIN_RIGHT: String = "maplibre_compassMarginRight"
    const val STATE_COMPASS_MARGIN_BOTTOM: String = "maplibre_compassMarginBottom"
    const val STATE_COMPASS_FADE_WHEN_FACING_NORTH: String = "maplibre_compassFade"
    const val STATE_COMPASS_IMAGE_BITMAP: String = "maplibre_compassImage"
    const val STATE_LOGO_GRAVITY: String = "maplibre_logoGravity"
    const val STATE_LOGO_MARGIN_LEFT: String = "maplibre_logoMarginLeft"
    const val STATE_LOGO_MARGIN_TOP: String = "maplibre_logoMarginTop"
    const val STATE_LOGO_MARGIN_RIGHT: String = "maplibre_logoMarginRight"
    const val STATE_LOGO_MARGIN_BOTTOM: String = "maplibre_logoMarginBottom"
    const val STATE_LOGO_ENABLED: String = "maplibre_logoEnabled"
    const val STATE_ATTRIBUTION_GRAVITY: String = "maplibre_attrGravity"
    const val STATE_ATTRIBUTION_MARGIN_LEFT: String = "maplibre_attrMarginLeft"
    const val STATE_ATTRIBUTION_MARGIN_TOP: String = "maplibre_attrMarginTop"
    const val STATE_ATTRIBUTION_MARGIN_RIGHT: String = "maplibre_attrMarginRight"
    const val STATE_ATTRIBUTION_MARGIN_BOTTOM: String = "maplibre_atrrMarginBottom"
    const val STATE_ATTRIBUTION_ENABLED: String = "maplibre_atrrEnabled"
    const val STATE_DESELECT_MARKER_ON_TAP: String = "maplibre_deselectMarkerOnTap"
    const val STATE_USER_FOCAL_POINT: String = "maplibre_userFocalPoint"
    const val STATE_SCALE_ANIMATION_ENABLED: String = "maplibre_scaleAnimationEnabled"
    const val STATE_ROTATE_ANIMATION_ENABLED: String = "maplibre_rotateAnimationEnabled"
    const val STATE_FLING_ANIMATION_ENABLED: String = "maplibre_flingAnimationEnabled"
    const val STATE_INCREASE_ROTATE_THRESHOLD: String = "maplibre_increaseRotateThreshold"
    const val STATE_DISABLE_ROTATE_WHEN_SCALING: String = "maplibre_disableRotateWhenScaling"
    const val STATE_INCREASE_SCALE_THRESHOLD: String = "maplibre_increaseScaleThreshold"
}
