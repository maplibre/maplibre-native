package com.mapbox.mapboxsdk.snapshotter

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.layers.BackgroundLayer
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.style.layers.RasterLayer
import com.mapbox.mapboxsdk.style.sources.RasterSource
import com.mapbox.mapboxsdk.style.sources.Source
import com.mapbox.mapboxsdk.testapp.activity.FeatureOverviewActivity
import org.junit.Assert
import org.junit.Assert.assertNotNull
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeoutException

fun commonOptions(): MapSnapshotter.Options  {
    return MapSnapshotter.Options(512, 512)
        .withPixelRatio(1.0f)
        .withCameraPosition(
            CameraPosition.Builder()
                .zoom(12.0)
                .target(LatLng(51.145495, 5.742234))
                .build()
        )
}

/**
 * Integration test that validates if a snapshotter creation
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class MapSnapshotterTest {

    @Rule
    @JvmField
    var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

    private val optionsVariants: Array<() -> MapSnapshotter.Options> = arrayOf(
        {
            val options = commonOptions()
            val bg = BackgroundLayer("rand_tint")
            bg.setProperties(PropertyFactory.backgroundColor("rgba(255,128,0,0.7)"))
            val styleBuilder = Style.Builder().fromUri(Style.getPredefinedStyle("Satellite Hybrid"))
                .withLayerAbove(bg, "country-label")
            options.withStyleBuilder(styleBuilder)
        },
        {
            val options = commonOptions()
            val styleBuilder = Style.Builder().fromJson(
                rule.activity.assets.open("demotiles.json").bufferedReader().readText()
            )
            val source: Source =
                RasterSource("my-raster-source", "maptiler://sources/satellite", 512)
            styleBuilder.withLayerAbove(
                RasterLayer("satellite-layer", "my-raster-source"),
                "country_1"
            )
            styleBuilder.withSource(source)
            options.withStyleBuilder(styleBuilder)
        }
    )
    private val countDownLatch = CountDownLatch(optionsVariants.size)

    @Test
    fun mapSnapshotter() {
        for (options in optionsVariants) {
            var mapSnapshotter: MapSnapshotter?
            rule.activity.runOnUiThread {
                mapSnapshotter = MapSnapshotter(rule.activity, options())
                mapSnapshotter!!.start(
                    {
                        assertNotNull(it)
                        assertNotNull(it.bitmap)
                        countDownLatch.countDown()
                    },
                    {
                        Assert.fail(it)
                    }
                )
            }
        }

        if (!countDownLatch.await(30, TimeUnit.SECONDS)) {
            throw TimeoutException()
        }
    }
}
