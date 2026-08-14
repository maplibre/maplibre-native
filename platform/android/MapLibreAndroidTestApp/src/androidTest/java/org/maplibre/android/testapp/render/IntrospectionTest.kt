package org.maplibre.android.testapp.render

import android.graphics.Color
import android.graphics.RectF
import android.os.Handler
import android.os.Looper
import android.view.View
import androidx.test.espresso.Espresso
import androidx.test.espresso.IdlingPolicies
import androidx.test.espresso.IdlingRegistry
import androidx.test.espresso.UiController
import androidx.test.espresso.ViewAction
import androidx.test.espresso.matcher.ViewMatchers
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.hamcrest.Matcher
import org.junit.After
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.EspressoTest
import org.maplibre.android.testapp.utils.FeatureMatchIdleResource
import org.maplibre.android.testapp.utils.MapIdleResource
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection

@RunWith(AndroidJUnit4ClassRunner::class)
class IntrospectionTest : EspressoTest() {
    @Before
    fun setup() {
        validateTestSetup()

        // This ensures that the map is "idle" before a check is made, so that, e.g.,
        // after changing the camera, the map is rendered before we query for features.
        mapIdleResource = MapIdleResource(mapView)
        IdlingRegistry.getInstance().register(mapIdleResource)

        // This allows us to wait for a specific feature/layer/source to be rendered
        featureIdleResource = FeatureMatchIdleResource(mapView)
        IdlingRegistry.getInstance().register(featureIdleResource)

        IdlingPolicies.setIdlingResourceTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
    }

    @After
    fun teardown() {
        mapIdleResource?.let {
            Assert.assertTrue(IdlingRegistry.getInstance().unregister(it))
            it.unregister()
        }
        mapIdleResource = null
        featureIdleResource?.let {
            Assert.assertTrue(IdlingRegistry.getInstance().unregister(it))
            it.unregister()
        }
        featureIdleResource = null
    }

    @Test
    fun testPOIMarkers() {
        val onView = Espresso.onView(ViewMatchers.withId(R.id.mapView))
        onView.perform(SimpleViewAction { _, _ ->
            featureIdleResource!!.addAllMatch(
                "11596182622", // https://www.openstreetmap.org/way/1159618262
                "poi_r20", "openmaptiles"
            ) { _, _, feature ->
                // should be roughly centered in the view
                feature.ndcBound.midX in -0.2..0.2 && feature.ndcBound.midY in -0.2..0.2
            }
            featureIdleResource!!.addAllMatch(
                "107519139171", // https://www.openstreetmap.org/node/10751913917
                "poi_r20", "openmaptiles"
            ) { _, _, feature ->
                // upper left
                feature.ndcBound.maxX < 0 && feature.ndcBound.minY > 0
            }

            onMainThreadSync {
                maplibreMap.cameraPosition =
                    CameraPosition.Builder().target(domkerk).zoom(19.0).build()
            }

            // wait for the features to be rendered via idle resource...
        })
    }

    @Test
    fun testHighlightFeatures() {
        val highlightLayerName = "highlighted-shapes-layer"
        val highlightSourceName = "highlighted-shapes-source"
        var source: GeoJsonSource? = null
        onMainThreadSync {
            // Set up a highlight layer in a GeoJSON source for feature overlay
            val style = maplibreMap.style!!
            val opts = GeoJsonOptions().withSynchronousUpdate(true)
            source = GeoJsonSource(highlightSourceName, opts)
            style.addSource(source)
            style.addLayer(
                FillLayer(highlightLayerName, highlightSourceName).withProperties(
                        PropertyFactory.fillColor(Color.RED), PropertyFactory.fillOpacity(0.75f)
                    )
            )

            // Wait for map idle after this action
            mapIdleResource!!.reset()

            // Move the camera to a location with 3D buildings
            maplibreMap.cameraPosition = CameraPosition.Builder().target(domkerk).zoom(17.0).build()
        }

        // query for features in the center of the map
        var features: List<Feature>? = null
        val onView = Espresso.onView(ViewMatchers.withId(R.id.mapView))
        onView.perform(SimpleViewAction { _, view ->
            val box = RectF(
                view.width * 0.25f, view.height * 0.25f, view.width * 0.75f, view.height * 0.75f
            )
            features = maplibreMap.queryRenderedFeatures(box, "building-3d").filter {
                !it.id().isNullOrBlank()
            }
            Assert.assertTrue(features.isNotEmpty())
        })

        // Highlight and check each feature
        for (feature in features!!) {
            // Add the feature to the highlight layer
            onView.perform(SimpleViewAction { _, _ ->
                featureIdleResource!!.clear()
                featureIdleResource!!.addAllMatch(
                    feature.id(), highlightLayerName, highlightSourceName
                )
                onMainThreadSync {
                    source!!.setGeoJson(FeatureCollection.fromFeature(feature))
                }

                // Wait for the feature to be rendered in the highlight layer via idle resource
            })

            onView.perform(SimpleViewAction { uiController, _ ->
                maplibreMap.getRenderedFeatureCount(feature.id(), highlightLayerName, highlightSourceName).let { count ->
                    Assert.assertEquals(1, count)
                }
                // Wait very briefly so you can see something happening on the device.
                // This is not necessary for the test to pass.
                uiController.loopMainThreadForAtLeast(100)
            })
        }
    }

    private fun onMainThreadSync(action: () -> Unit) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            action()
        } else {
            val latch = java.util.concurrent.CountDownLatch(1)
            Handler(Looper.getMainLooper()).post {
                action()
                latch.countDown()
            }
            latch.await()
        }
    }

    class SimpleViewAction(private val action: (uiController: UiController, view: View) -> Unit) :
        ViewAction {

        override fun getConstraints(): Matcher<View?> {
            return ViewMatchers.isDisplayed()
        }

        override fun getDescription(): String {
            return javaClass.simpleName
        }

        override fun perform(uiController: UiController, view: View) {
            action(uiController, view)
        }
    }

    private val domkerk = LatLng(52.090864, 5.122452)
    private var mapIdleResource: MapIdleResource? = null
    private var featureIdleResource: FeatureMatchIdleResource? = null
}
