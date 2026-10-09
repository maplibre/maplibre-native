package org.maplibre.android.testapp.activity.style

import androidx.test.core.app.ActivityScenario
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.maps.MapView
import org.maplibre.android.testapp.R
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

@RunWith(AndroidJUnit4::class)
class GlobeActivityTest {

    @Test
    fun recreatingTheActivityRestoresTheMapState() {
        ActivityScenario.launch(GlobeActivity::class.java).use { scenario ->
            val ready = CountDownLatch(1)
            scenario.onActivity { activity ->
                activity.findViewById<MapView>(R.id.mapView).getMapAsync {
                    it.uiSettings.isRotateGesturesEnabled = false
                    ready.countDown()
                }
            }
            assertTrue(ready.await(30, TimeUnit.SECONDS))

            scenario.recreate()

            val restored = CountDownLatch(1)
            var rotateGesturesEnabled = true
            scenario.onActivity { activity ->
                activity.findViewById<MapView>(R.id.mapView).getMapAsync {
                    rotateGesturesEnabled = it.uiSettings.isRotateGesturesEnabled
                    restored.countDown()
                }
            }
            assertTrue(restored.await(30, TimeUnit.SECONDS))
            assertFalse(rotateGesturesEnabled)
        }
    }
}
