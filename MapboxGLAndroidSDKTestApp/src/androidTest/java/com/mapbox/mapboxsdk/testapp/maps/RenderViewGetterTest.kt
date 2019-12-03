package com.mapbox.mapboxsdk.testapp.maps

import android.opengl.GLSurfaceView
import android.support.test.annotation.UiThreadTest
import android.support.test.rule.ActivityTestRule
import android.support.test.runner.AndroidJUnit4
import android.view.TextureView
import android.view.ViewGroup
import com.mapbox.mapboxsdk.AppCenter
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMapOptions
import com.mapbox.mapboxsdk.testapp.activity.FeatureOverviewActivity
import java.util.concurrent.CountDownLatch
import junit.framework.Assert.assertNotNull
import junit.framework.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
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
    assertTrue(mapView.renderView is GLSurfaceView)
  }

  @Test
  @UiThreadTest
  fun testTextureView() {
    rootView = rule.activity.findViewById(android.R.id.content)
    mapView = MapView(rule.activity,
      MapboxMapOptions.createFromAttributes(rule.activity, null)
        .textureMode(true)
    )
    assertNotNull(mapView.renderView)
    assertTrue(mapView.renderView is TextureView)
  }
}