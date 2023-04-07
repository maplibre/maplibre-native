package com.mapbox.mapboxsdk.testapp.style

import android.view.View
import androidx.test.espresso.UiController
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.testapp.action.MapboxMapAction
import com.mapbox.mapboxsdk.testapp.activity.EspressoTest
import com.mapbox.mapboxsdk.testapp.utils.ResourceUtils.readRawResource
import org.junit.Assert
import org.junit.Test
import java.io.IOException

/**
 * Tests around style loading
 */
class StyleLoaderTest : EspressoTest() {
    @Test
    fun testSetGetStyleJsonString() {
        validateTestSetup()
        MapboxMapAction.invoke(
            mapboxMap
        ) { uiController: UiController?, mapboxMap: MapboxMap ->
            try {
                val expected =
                    readRawResource(
                        rule.activity,
                        R.raw.local_style
                    )
                mapboxMap.setStyle(Style.Builder().fromJson(expected))
                val actual = mapboxMap.style!!.json
                Assert.assertEquals("Style json should match", expected, actual)
            } catch (exception: IOException) {
                exception.printStackTrace()
            }
        }
    }

    @Test
    fun testDefaultStyleLoadWithActivityLifecycleChange() {
        validateTestSetup()
        MapboxMapAction.invoke(
            mapboxMap
        ) { uiController: UiController?, mapboxMap: MapboxMap ->
            try {
                val expected =
                    readRawResource(
                        rule.activity,
                        R.raw.local_style
                    )
                mapboxMap.setStyle(Style.Builder().fromJson(expected))

                // fake activity stop/start
                val mapView =
                    rule.activity.findViewById<View>(R.id.mapView) as MapView
                mapView.onPause()
                mapView.onStop()
                mapView.onStart()
                mapView.onResume()
                val actual = mapboxMap.style!!.json
                Assert.assertEquals(
                    "Style URL should be empty",
                    "",
                    mapboxMap.style!!.uri
                )
                Assert.assertEquals("Style json should match", expected, actual)
            } catch (exception: IOException) {
                exception.printStackTrace()
            }
        }
    }
}
