package org.maplibre.android.location

/**
 * Callbacks related to the compass
 */
interface CompassListener {
    /**
     * Callback's invoked when a new compass update occurs. You can listen into the compass updates
     * using [CompassEngine.addCompassListener] and implementing these
     * callbacks. Note that this interface is also used internally to to update the UI chevron/arrow.
     *
     * @param userHeading the new compass heading
     */
    fun onCompassChanged(userHeading: Float)

    /**
     * This gets invoked when the compass accuracy status changes from one value to another. It
     * provides an integer value which is identical to the `SensorManager` class constants:
     *
     * - [android.hardware.SensorManager.SENSOR_STATUS_NO_CONTACT]
     * - [android.hardware.SensorManager.SENSOR_STATUS_UNRELIABLE]
     * - [android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_LOW]
     * - [android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM]
     * - [android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_HIGH]
     *
     * @param compassStatus the new accuracy of this sensor, one of
     *                      `SensorManager.SENSOR_STATUS_*`
     */
    fun onCompassAccuracyChange(compassStatus: Int)
}
