package org.maplibre.android.camera

import org.maplibre.android.maps.MapLibreMap

/**
 * Interface definition for camera updates.
 */
interface CameraUpdate {
    /**
     * Get the camera position from the camera update.
     *
     * @param maplibreMap Map object to build the position from
     * @return the camera position from the implementing camera update
     */
    fun getCameraPosition(maplibreMap: MapLibreMap): CameraPosition?
}
