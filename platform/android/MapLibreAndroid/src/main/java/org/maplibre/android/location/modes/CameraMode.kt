package org.maplibre.android.location.modes

import android.location.Location
import androidx.annotation.IntDef
import org.maplibre.android.location.LocationComponent

/**
 * Contains the variety of camera modes which determine how the camera will track
 * the user location.
 */
object CameraMode {
    /**
     * Determine the camera tracking behavior in the [LocationComponent].
     */
    @IntDef(NONE, NONE_COMPASS, NONE_GPS, TRACKING, TRACKING_COMPASS, TRACKING_GPS, TRACKING_GPS_NORTH)
    @Retention(AnnotationRetention.SOURCE)
    annotation class Mode

    /**
     * No camera tracking.
     */
    const val NONE = 0x00000008

    /**
     * Camera does not track location, but does track compass bearing.
     */
    const val NONE_COMPASS = 0x00000010

    /**
     * Camera does not track location, but does track GPS [Location] bearing.
     */
    const val NONE_GPS = 0x00000016

    /**
     * Camera tracks the user location.
     */
    const val TRACKING = 0x00000018

    /**
     * Camera tracks the user location, with bearing
     * provided by a compass.
     */
    const val TRACKING_COMPASS = 0x00000020

    /**
     * Camera tracks the user location, with bearing
     * provided by a normalized [Location.getBearing].
     */
    const val TRACKING_GPS = 0x00000022

    /**
     * Camera tracks the user location, with bearing
     * always set to north (0).
     */
    const val TRACKING_GPS_NORTH = 0x00000024
}
