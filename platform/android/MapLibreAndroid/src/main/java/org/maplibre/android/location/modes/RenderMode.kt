package org.maplibre.android.location.modes

import androidx.annotation.IntDef
import org.maplibre.android.location.CompassEngine
import org.maplibre.android.location.LocationComponent

/**
 * Contains the variety of ways the user location can be rendered on the map.
 */
object RenderMode {
    /**
     * One of these constants should be used with [LocationComponent.setRenderMode].
     * Mode can be switched at anytime by calling the `setLocationLayerMode` method passing
     * in the new mode you'd like the location layer to be in.
     */
    @IntDef(COMPASS, GPS, NORMAL)
    @Retention(AnnotationRetention.SOURCE)
    annotation class Mode

    /**
     * Basic tracking is enabled, bearing ignored.
     */
    const val NORMAL = 0x00000012

    /**
     * Tracking the user location with bearing considered
     * from a [CompassEngine].
     */
    const val COMPASS = 0x00000004

    /**
     * Tracking the user location with bearing considered from [android.location.Location].
     */
    const val GPS = 0x00000008
}
