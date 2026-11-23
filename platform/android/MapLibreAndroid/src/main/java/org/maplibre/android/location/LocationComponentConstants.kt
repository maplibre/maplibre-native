package org.maplibre.android.location

/**
 * Contains all the constants being used for the [LocationComponent].
 */
object LocationComponentConstants {
    // Controls the compass update rate in milliseconds
    internal const val COMPASS_UPDATE_RATE_MS = 500L

    // Sets the transition animation duration when switching camera modes.
    internal const val TRANSITION_ANIMATION_DURATION_MS = 750L

    // Sets the max allowed time for the location icon animation from one LatLng to another.
    internal const val MAX_ANIMATION_DURATION_MS = 2000L

    // Sets the duration of change of accuracy radius when a different value is provided.
    internal const val ACCURACY_RADIUS_ANIMATION_DURATION = 250L

    // Default animation duration for zooming while tracking.
    internal const val DEFAULT_TRACKING_ZOOM_ANIM_DURATION = 750L

    // Default animation duration for updating padding while tracking.
    internal const val DEFAULT_TRACKING_PADDING_ANIM_DURATION = 750L

    // Default animation duration for tilting while tracking.
    internal const val DEFAULT_TRACKING_TILT_ANIM_DURATION = 1250L

    // Threshold value to perform immediate camera/layer position update.
    internal const val INSTANT_LOCATION_TRANSITION_THRESHOLD = 50_000.0

    // Default interval between location updates
    internal const val DEFAULT_INTERVAL_MILLIS = 1000L

    // Default fastest acceptable interval between location updates
    internal const val DEFAULT_FASTEST_INTERVAL_MILLIS = 1000L

    // Sources

    /**
     * Source ID of the location's GeoJsonSource.
     */
    const val LOCATION_SOURCE = "mapbox-location-source"

    internal const val PROPERTY_GPS_BEARING = "mapbox-property-gps-bearing"
    internal const val PROPERTY_COMPASS_BEARING = "mapbox-property-compass-bearing"
    internal const val PROPERTY_LOCATION_STALE = "mapbox-property-location-stale"
    internal const val PROPERTY_ACCURACY_RADIUS = "mapbox-property-accuracy-radius"
    internal const val PROPERTY_ACCURACY_COLOR = "mapbox-property-accuracy-color"
    internal const val PROPERTY_ACCURACY_ALPHA = "mapbox-property-accuracy-alpha"
    internal const val PROPERTY_FOREGROUND_ICON_OFFSET = "mapbox-property-foreground-icon-offset"
    internal const val PROPERTY_SHADOW_ICON_OFFSET = "mapbox-property-shadow-icon-offset"
    internal const val PROPERTY_FOREGROUND_ICON = "mapbox-property-foreground-icon"
    internal const val PROPERTY_BACKGROUND_ICON = "mapbox-property-background-icon"
    internal const val PROPERTY_FOREGROUND_STALE_ICON = "mapbox-property-foreground-stale-icon"
    internal const val PROPERTY_BACKGROUND_STALE_ICON = "mapbox-property-background-stale-icon"
    internal const val PROPERTY_BEARING_ICON = "mapbox-property-shadow-icon"
    internal const val PROPERTY_PULSING_RADIUS = "mapbox-property-pulsing-circle-radius"
    internal const val PROPERTY_PULSING_OPACITY = "mapbox-property-pulsing-circle-opacity"

    // Layers

    /**
     * Layer ID of the location shadow.
     */
    const val SHADOW_LAYER = "mapbox-location-shadow-layer"

    /**
     * Layer ID of the location foreground icon or the only runtime layer added if
     * [LocationComponentActivationOptions.useSpecializedLocationLayer] is used.
     */
    const val FOREGROUND_LAYER = "mapbox-location-foreground-layer"

    /**
     * Layer ID of the location background icon.
     */
    const val BACKGROUND_LAYER = "mapbox-location-background-layer"

    /**
     * Layer ID of the location accuracy.
     */
    const val ACCURACY_LAYER = "mapbox-location-accuracy-layer"

    /**
     * Layer ID of the location bearing icon.
     */
    const val BEARING_LAYER = "mapbox-location-bearing-layer"

    /**
     * Layer ID of the location pulsing circle.
     */
    const val PULSING_CIRCLE_LAYER = "mapbox-location-pulsing-circle-layer"

    // Icons
    internal const val FOREGROUND_ICON = "mapbox-location-icon"
    internal const val BACKGROUND_ICON = "mapbox-location-stroke-icon"
    internal const val FOREGROUND_STALE_ICON = "mapbox-location-stale-icon"
    internal const val BACKGROUND_STALE_ICON = "mapbox-location-background-stale-icon"
    internal const val SHADOW_ICON = "mapbox-location-shadow-icon"
    internal const val BEARING_ICON = "mapbox-location-bearing-icon"
    internal const val BEARING_STALE_ICON = "mapbox-location-bearing-stale-icon"
}
