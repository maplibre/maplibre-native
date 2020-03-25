package com.mapbox.mapboxsdk.snapshotter

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.rule.ActivityTestRule
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.layers.BackgroundLayer
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.testapp.activity.FeatureOverviewActivity
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeoutException
import junit.framework.Assert.assertNotNull
import org.junit.Assert
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Integration test that validates if a snapshotter creation
 */
@RunWith(AndroidJUnit4::class)
class MapSnapshotterTest {

  @Rule
  @JvmField
  var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

  private val countDownLatch = CountDownLatch(1)

  @Test
  fun mapSnapshotter() {
    var mapSnapshotter: MapSnapshotter?
    rule.activity.runOnUiThread {
      val bg = BackgroundLayer("rand_tint")
      bg.setProperties(PropertyFactory.backgroundColor("rgba(255,128,0,0.7)"))
      val options = MapSnapshotter.Options(512, 512)
        .withPixelRatio(1.0f)
        .withStyleBuilder(Style.Builder().fromUri(Style.SATELLITE_STREETS)
          .withLayerAbove(bg, "country-label"))
        .withCameraPosition(
          CameraPosition.Builder()
            .zoom(12.0)
            .target(LatLng(51.145495, 5.742234))
            .build()
        )
      mapSnapshotter = MapSnapshotter(rule.activity, options)
      mapSnapshotter!!.start({
        assertNotNull(it)
        assertNotNull(it.bitmap)
        countDownLatch.countDown()
      }, {
        Assert.fail(it)
      })
    }
    if (!countDownLatch.await(30, TimeUnit.SECONDS)) {
      throw TimeoutException()
    }
  }
}