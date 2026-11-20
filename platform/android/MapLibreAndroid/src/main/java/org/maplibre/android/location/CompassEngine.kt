package org.maplibre.android.location

import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode

/**
 * Interface defining the source of compass heading data that is
 * consumed by the [LocationComponent] when in compass related
 * [RenderMode] or [CameraMode]s.
 */
interface CompassEngine {
    /**
     * The last heading value produced and pushed via a compass listener.
     */
    val lastHeading: Float

    /**
     * The last know accuracy status from the sensor manager.
     *
     * An integer value which is identical to the `SensorManager` class constants:
     * - [android.hardware.SensorManager.SENSOR_STATUS_NO_CONTACT]
     * - [android.hardware.SensorManager.SENSOR_STATUS_UNRELIABLE]
     * - [android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_LOW]
     * - [android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM]
     * - [android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_HIGH]
     */
    val lastAccuracySensorStatus: Int

    /**
     * Adds a [CompassListener] that can be used to receive heading and state changes.
     *
     * @param compassListener to be added
     */
    fun addCompassListener(compassListener: CompassListener)

    /**
     * Removes a [CompassListener] that can be used to receive heading and state changes.
     *
     * @param compassListener to be removed
     */
    fun removeCompassListener(compassListener: CompassListener)
}
