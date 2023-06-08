package org.maplibre.android.testapp.maps

import android.view.TextureView
import android.view.ViewGroup
import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.AppCenter
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.renderer.glsurfaceview.MapLibreGLSurfaceView
import org.maplibre.android.testapp.activity.FeatureOverviewActivity
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.CountDownLatch

@RunWith(AndroidJUnit4ClassRunner::class)
class RenderViewGetterTest : AppCenter() {

    @Rule
    @JvmField
    var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

    private lateinit var rootView: ViewGroup
    private lateinit var mapView: MapView
    private val latch: CountDownLatch = CountDownLatch(1)

    @Test
    @UiThreadTest
    fun testGLSurfaceView() {
        rootView = rule.activity.findViewById(android.R.id.content)
        mapView = MapView(rule.activity)
        assertNotNull(mapView.renderView)
        assertTrue(mapView.renderView is MapLibreGLSurfaceView)
    }

    @Test
    @UiThreadTest
    fun testTextureView() {
        rootView = rule.activity.findViewById(android.R.id.content)
        mapView = MapView(
            rule.activity,
            MapLibreMapOptions.createFromAttributes(rule.activity, null)
                .textureMode(true)
        )
        assertNotNull(mapView.renderView)
        assertTrue(mapView.renderView is TextureView)
    }
}
