package org.maplibre.android.location

import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.SystemClock
import android.view.Surface
import android.view.WindowManager
import org.maplibre.android.log.Logger

/**
 * This manager class handles compass events such as starting the tracking of device bearing, or
 * when a new compass update occurs.
 *
 * Construct a new instance of the this class. A internal compass listeners needed to separate it
 * from the cleared list of public listeners.
 */
internal class LocationComponentCompassEngine(
    private val windowManager: WindowManager,
    private val sensorManager: SensorManager,
) : CompassEngine,
    SensorEventListener {
    private val compassListeners = mutableListOf<CompassListener>()

    // Not all devices have a compassSensor
    private var compassSensor: Sensor? = null
    private var gravitySensor: Sensor? = null
    private var magneticFieldSensor: Sensor? = null

    private val truncatedRotationVectorValue = FloatArray(4)
    private val rotationMatrix = FloatArray(9)
    private var rotationVectorValue: FloatArray? = null

    override var lastHeading = 0f
        private set

    override var lastAccuracySensorStatus = 0
        private set

    private var compassUpdateNextTimestamp: Long = 0
    private var gravityValues: FloatArray? = FloatArray(3)
    private var magneticValues: FloatArray? = FloatArray(3)

    init {
        compassSensor = sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR)
        if (compassSensor == null) {
            Logger.d(
                TAG,
                "Rotation vector sensor not supported on device, falling back to accelerometer and magnetic field.",
            )
            gravitySensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
            magneticFieldSensor = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD)
        }
    }

    override fun addCompassListener(compassListener: CompassListener) {
        if (compassListeners.isEmpty()) {
            registerSensorListeners()
        }
        compassListeners.add(compassListener)
    }

    override fun removeCompassListener(compassListener: CompassListener) {
        compassListeners.remove(compassListener)
        if (compassListeners.isEmpty()) {
            unregisterSensorListeners()
        }
    }

    override fun onSensorChanged(event: SensorEvent) {
        if (lastAccuracySensorStatus == SensorManager.SENSOR_STATUS_UNRELIABLE) {
            Logger.d(TAG, "Compass sensor is unreliable, device calibration is needed.")
            // Update the heading, even if the sensor is unreliable.
            // This makes it possible to use a different indicator for the unreliable case,
            // instead of just changing the RenderMode to NORMAL.
        }
        when (event.sensor.type) {
            Sensor.TYPE_ROTATION_VECTOR -> {
                rotationVectorValue = getRotationVectorFromSensorEvent(event)
                updateOrientation()
            }

            Sensor.TYPE_ACCELEROMETER -> {
                gravityValues = lowPassFilter(getRotationVectorFromSensorEvent(event), gravityValues)
                updateOrientation()
            }

            Sensor.TYPE_MAGNETIC_FIELD -> {
                magneticValues = lowPassFilter(getRotationVectorFromSensorEvent(event), magneticValues)
                updateOrientation()
            }
        }
    }

    override fun onAccuracyChanged(
        sensor: Sensor?,
        accuracy: Int,
    ) {
        if (lastAccuracySensorStatus != accuracy) {
            for (compassListener in compassListeners) {
                compassListener.onCompassAccuracyChange(accuracy)
            }
            lastAccuracySensorStatus = accuracy
        }
    }

    @Suppress("SuspiciousNameCombination")
    private fun updateOrientation() {
        // check when the last time the compass was updated, return if too soon.
        val currentTime = SystemClock.elapsedRealtime()
        if (currentTime < compassUpdateNextTimestamp) {
            return
        }

        val currentRotationVectorValue = rotationVectorValue
        if (currentRotationVectorValue != null) {
            SensorManager.getRotationMatrixFromVector(rotationMatrix, currentRotationVectorValue)
        } else {
            // Get rotation matrix given the gravity and geomagnetic matrices
            SensorManager.getRotationMatrix(rotationMatrix, null, gravityValues, magneticValues)
        }

        var worldAxisForDeviceAxisX: Int
        var worldAxisForDeviceAxisY: Int

        // Assume the device screen was parallel to the ground,
        // and adjust the rotation matrix for the device orientation.
        when (windowManager.defaultDisplay.rotation) {
            Surface.ROTATION_90 -> {
                worldAxisForDeviceAxisX = SensorManager.AXIS_Y
                worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_X
            }

            Surface.ROTATION_180 -> {
                worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_X
                worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_Y
            }

            Surface.ROTATION_270 -> {
                worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_Y
                worldAxisForDeviceAxisY = SensorManager.AXIS_X
            }

            else -> {
                worldAxisForDeviceAxisX = SensorManager.AXIS_X
                worldAxisForDeviceAxisY = SensorManager.AXIS_Y
            }
        }

        val adjustedRotationMatrix = FloatArray(9)
        SensorManager.remapCoordinateSystem(
            rotationMatrix,
            worldAxisForDeviceAxisX,
            worldAxisForDeviceAxisY,
            adjustedRotationMatrix,
        )

        // Transform rotation matrix into azimuth/pitch/roll
        val orientation = FloatArray(3)
        SensorManager.getOrientation(adjustedRotationMatrix, orientation)

        if (orientation[1] < -Math.PI / 4) {
            // The pitch is less than -45 degrees.
            // Remap the axes as if the device screen was the instrument panel.
            when (windowManager.defaultDisplay.rotation) {
                Surface.ROTATION_90 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_Z
                    worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_X
                }

                Surface.ROTATION_180 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_X
                    worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_Z
                }

                Surface.ROTATION_270 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_Z
                    worldAxisForDeviceAxisY = SensorManager.AXIS_X
                }

                else -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_X
                    worldAxisForDeviceAxisY = SensorManager.AXIS_Z
                }
            }
        } else if (orientation[1] > Math.PI / 4) {
            // The pitch is larger than 45 degrees.
            // Remap the axes as if the device screen was upside down and facing back.
            when (windowManager.defaultDisplay.rotation) {
                Surface.ROTATION_90 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_Z
                    worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_X
                }

                Surface.ROTATION_180 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_X
                    worldAxisForDeviceAxisY = SensorManager.AXIS_Z
                }

                Surface.ROTATION_270 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_Z
                    worldAxisForDeviceAxisY = SensorManager.AXIS_X
                }

                else -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_X
                    worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_Z
                }
            }
        } else if (Math.abs(orientation[2]) > Math.PI / 2) {
            // The roll is less than -90 degrees, or is larger than 90 degrees.
            // Remap the axes as if the device screen was face down.
            when (windowManager.defaultDisplay.rotation) {
                Surface.ROTATION_90 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_Y
                    worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_X
                }

                Surface.ROTATION_180 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_MINUS_X
                    worldAxisForDeviceAxisY = SensorManager.AXIS_Y
                }

                Surface.ROTATION_270 -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_Y
                    worldAxisForDeviceAxisY = SensorManager.AXIS_X
                }

                else -> {
                    worldAxisForDeviceAxisX = SensorManager.AXIS_X
                    worldAxisForDeviceAxisY = SensorManager.AXIS_MINUS_Y
                }
            }
        }

        SensorManager.remapCoordinateSystem(
            rotationMatrix,
            worldAxisForDeviceAxisX,
            worldAxisForDeviceAxisY,
            adjustedRotationMatrix,
        )

        // Transform rotation matrix into azimuth/pitch/roll
        SensorManager.getOrientation(adjustedRotationMatrix, orientation)

        // The x-axis is all we care about here.
        notifyCompassChangeListeners(Math.toDegrees(orientation[0].toDouble()).toFloat())

        // Update the compassUpdateNextTimestamp
        compassUpdateNextTimestamp = currentTime + LocationComponentConstants.COMPASS_UPDATE_RATE_MS
    }

    private fun notifyCompassChangeListeners(heading: Float) {
        for (compassListener in compassListeners) {
            compassListener.onCompassChanged(heading)
        }
        lastHeading = heading
    }

    private fun registerSensorListeners() {
        if (isCompassSensorAvailable()) {
            // Does nothing if the sensors already registered.
            sensorManager.registerListener(this, compassSensor, SENSOR_DELAY_MICROS)
        } else {
            sensorManager.registerListener(this, gravitySensor, SENSOR_DELAY_MICROS)
            sensorManager.registerListener(this, magneticFieldSensor, SENSOR_DELAY_MICROS)
        }
    }

    private fun unregisterSensorListeners() {
        if (isCompassSensorAvailable()) {
            sensorManager.unregisterListener(this, compassSensor)
        } else {
            sensorManager.unregisterListener(this, gravitySensor)
            sensorManager.unregisterListener(this, magneticFieldSensor)
        }
    }

    private fun isCompassSensorAvailable(): Boolean = compassSensor != null

    /**
     * Helper function, that filters newValues, considering previous values
     *
     * @param newValues      array of float, that contains new data
     * @param smoothedValues array of float, that contains previous state
     * @return float filtered array of float
     */
    private fun lowPassFilter(
        newValues: FloatArray,
        smoothedValues: FloatArray?,
    ): FloatArray {
        if (smoothedValues == null) {
            return newValues
        }
        for (i in newValues.indices) {
            smoothedValues[i] = smoothedValues[i] + ALPHA * (newValues[i] - smoothedValues[i])
        }
        return smoothedValues
    }

    /**
     * Pulls out the rotation vector from a SensorEvent, with a maximum length
     * vector of four elements to avoid potential compatibility issues.
     *
     * @param event the sensor event
     * @return the events rotation vector, potentially truncated
     */
    private fun getRotationVectorFromSensorEvent(event: SensorEvent): FloatArray =
        if (event.values.size > 4) {
            // On some Samsung devices SensorManager.getRotationMatrixFromVector
            // appears to throw an exception if rotation vector has length > 4.
            // For the purposes of this class the first 4 values of the
            // rotation vector are sufficient (see crbug.com/335298 for details).
            // Only affects Android 4.3
            System.arraycopy(event.values, 0, truncatedRotationVectorValue, 0, 4)
            truncatedRotationVectorValue
        } else {
            event.values
        }

    companion object {
        private const val TAG = "Mbgl-LocationComponentCompassEngine"

        // The rate sensor events will be delivered at. As the Android documentation states, this is only
        // a hint to the system and the events might actually be received faster or slower then this
        // specified rate. Since the minimum Android API levels about 9, we are able to set this value
        // ourselves rather than using one of the provided constants which deliver updates too quickly for
        // our use case. The default is set to 100ms
        internal const val SENSOR_DELAY_MICROS = 100 * 1000

        // Filtering coefficient 0 < ALPHA < 1
        private const val ALPHA = 0.45f
    }
}
