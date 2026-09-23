package org.maplibre.android.testapp.style

import android.view.View
import androidx.test.espresso.UiController
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.action.MapLibreMapAction
import org.maplibre.android.testapp.activity.EspressoTest
import org.maplibre.android.testapp.utils.ResourceUtils.readRawResource
import org.maplibre.android.testapp.utils.TestingAsyncUtils
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
        MapLibreMapAction.invoke(
            maplibreMap
        ) { uiController: UiController?, maplibreMap: MapLibreMap ->
            try {
                val expected =
                    readRawResource(
                        rule.activity,
                        R.raw.local_style
                    )
                maplibreMap.setStyle(Style.Builder().fromJson(expected))
                val actual = maplibreMap.style!!.json
                Assert.assertEquals("Style json should match", expected, actual)
            } catch (exception: IOException) {
                exception.printStackTrace()
            }
        }
    }

    @Test
    fun testDefaultStyleLoadWithActivityLifecycleChange() {
        validateTestSetup()
        MapLibreMapAction.invoke(
            maplibreMap
        ) { uiController: UiController?, maplibreMap: MapLibreMap ->
            try {
                val expected =
                    readRawResource(
                        rule.activity,
                        R.raw.local_style
                    )
                maplibreMap.setStyle(Style.Builder().fromJson(expected))

                // fake activity stop/start
                val mapView =
                    rule.activity.findViewById<View>(R.id.mapView) as MapView
                mapView.onPause()
                mapView.onStop()
                mapView.onStart()
                mapView.onResume()
                val actual = maplibreMap.style!!.json
                Assert.assertEquals(
                    "Style URL should be empty",
                    "",
                    maplibreMap.style!!.uri
                )
                Assert.assertEquals("Style json should match", expected, actual)
            } catch (exception: IOException) {
                exception.printStackTrace()
            }
        }
    }

    @Test
    fun testLoadStyleFromAndroidAssetFileUri() {
        validateTestSetup()
        MapLibreMapAction.invoke(
            maplibreMap
        ) { uiController: UiController, maplibreMap: MapLibreMap ->
            val expected = rule.activity.assets.open("fill_color_style.json").bufferedReader().use { it.readText() }
            val uri = "file:///android_asset/fill_color_style.json"
            maplibreMap.setStyle(Style.Builder().fromUri(uri))
            TestingAsyncUtils.waitForLayer(uiController, mapView)
            Assert.assertEquals("Style URI should match", uri, maplibreMap.style!!.uri)
            Assert.assertEquals("Style json should match", expected, maplibreMap.style!!.json)
        }
    }
}
