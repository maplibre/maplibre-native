package org.maplibre.android.maps

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.platform.app.InstrumentationRegistry
import org.maplibre.android.AppCenter
import org.maplibre.android.style.layers.Layer
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4ClassRunner::class)
abstract class BaseLayerTest : AppCenter() {
    private lateinit var nativeMapView: NativeMap

    companion object {
        const val WIDTH = 500
        const val HEIGHT = WIDTH
    }

    fun before() {
        val context = InstrumentationRegistry.getInstrumentation().context
        nativeMapView = NativeMapView(context, false, null, null, NativeMapViewTest.DummyRenderer(context))
        nativeMapView.resizeView(WIDTH, HEIGHT)
    }

    fun setupLayer(layer: Layer) {
        nativeMapView.addLayer(layer)
    }
}
