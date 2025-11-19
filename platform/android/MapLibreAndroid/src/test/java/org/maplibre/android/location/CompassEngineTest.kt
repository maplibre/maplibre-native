package org.maplibre.android.location

import android.hardware.Sensor
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.view.WindowManager
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.ArgumentMatchers
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.junit.MockitoJUnitRunner

@RunWith(MockitoJUnitRunner::class)
class CompassEngineTest : BaseTest() {
    private var compassEngine: LocationComponentCompassEngine? = null

    @Mock
    private val windowManager: WindowManager? = null

    @Mock
    private val sensorManager: SensorManager? = null

    @Mock
    private val compassSensor: Sensor? = null

    @Mock
    private val compassListener: CompassListener? = null

    @Before
    @Throws(Exception::class)
    fun setUp() {
        Mockito.`when`(sensorManager!!.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR))
            .thenReturn(compassSensor)
        compassEngine = LocationComponentCompassEngine(windowManager!!, sensorManager)
    }

    @Test
    fun lastKnownCompassBearingAccuracyDefault() {
        Assert.assertEquals(
            "Last accuracy should match",
            compassEngine!!.lastAccuracySensorStatus,
            0
        )
    }

    @Test
    fun lastKnownCompassAccuracyStatusValue() {
        val sensor = Mockito.mock(
            Sensor::class.java
        )
        compassEngine!!.onAccuracyChanged(sensor, 2)
        Assert.assertEquals(
            "Last accuracy should match",
            compassEngine!!.lastAccuracySensorStatus,
            2
        )
    }

    @Test
    fun whenGyroscopeIsNull_fallbackToGravity() {
        val sensorManager = Mockito.mock(SensorManager::class.java)
        LocationComponentCompassEngine(windowManager!!, sensorManager)
        Mockito.verify(sensorManager, Mockito.times(1)).getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
    }

    @Test
    fun whenGyroscopeIsNull_fallbackToMagneticField() {
        val sensorManager = Mockito.mock(SensorManager::class.java)
        LocationComponentCompassEngine(windowManager!!, sensorManager)
        Mockito.verify(sensorManager, Mockito.times(1)).getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD)
    }

    @Test
    fun listener_registerOnAdd() {
        compassEngine!!.addCompassListener(compassListener!!)
        Mockito.verify(sensorManager)
            ?.registerListener(
                ArgumentMatchers.any(SensorEventListener::class.java),
                ArgumentMatchers.eq(compassSensor),
                ArgumentMatchers.eq(LocationComponentCompassEngine.SENSOR_DELAY_MICROS)
            )
    }

    @Test
    fun listener_unregisterOnRemove() {
        compassEngine!!.addCompassListener(compassListener!!)
        compassEngine!!.removeCompassListener(compassListener)
        Mockito.verify(sensorManager)?.unregisterListener(
            ArgumentMatchers.any(
                SensorEventListener::class.java
            ),
            ArgumentMatchers.eq(compassSensor)
        )
    }
}
