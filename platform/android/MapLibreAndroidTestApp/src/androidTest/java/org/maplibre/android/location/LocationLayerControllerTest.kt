package org.maplibre.android.location

import android.Manifest
import android.content.Context
import android.graphics.Color
import android.location.Location
import androidx.test.annotation.UiThreadTest
import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.IdlingRegistry
import androidx.test.espresso.UiController
import androidx.test.espresso.assertion.ViewAssertions.matches
import androidx.test.espresso.matcher.ViewMatchers.isDisplayed
import androidx.test.espresso.matcher.ViewMatchers.withId
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.GrantPermissionRule
import androidx.test.rule.GrantPermissionRule.grant
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.*
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.location.utils.*
import org.maplibre.android.location.utils.MapLibreTestingUtils.Companion.MAPBOX_HEAVY_STYLE
import org.maplibre.android.location.utils.MapLibreTestingUtils.Companion.pushSourceUpdates
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.EspressoTest
import org.maplibre.android.testapp.utils.TestingAsyncUtils
import org.maplibre.android.utils.BitmapUtils
import org.hamcrest.CoreMatchers.`is`
import org.hamcrest.CoreMatchers.notNullValue
import org.hamcrest.Matchers.equalTo
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThat
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.testapp.styles.TestStyles
import kotlin.math.abs

@RunWith(AndroidJUnit4ClassRunner::class)
class LocationLayerControllerTest : EspressoTest() {

    @Rule
    @JvmField
    val permissionRule: GrantPermissionRule = grant(Manifest.permission.ACCESS_FINE_LOCATION)

    private lateinit var styleChangeIdlingResource: StyleChangeIdlingResource
    private val location: Location by lazy {
        val initLocation = Location("")
        initLocation.latitude = 15.0
        initLocation.longitude = 17.0
        initLocation.bearing = 10f
        initLocation.accuracy = 150f
        initLocation
    }

    @Before
    override fun beforeTest() {
        super.beforeTest()
        styleChangeIdlingResource = StyleChangeIdlingResource()
        IdlingRegistry.getInstance().register(styleChangeIdlingResource)
    }

    //
    // Location Source
    //

    @Test
    fun renderModeNormal_sourceDoesGetAdded() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.NORMAL
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(style.getSource(LOCATION_SOURCE), notNullValue())
            }
        }
        executeComponentTest(componentAction)
    }

    //
    // Location Layers
    //

    @Test
    fun renderModeNormal_trackingNormalLayersDoGetAdded() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.NORMAL
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(PULSING_CIRCLE_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun onMapChange_locationComponentPulsingCircleLayerGetsRedrawn() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .locationComponentOptions(
                            LocationComponentOptions.builder(context)
                                .pulseEnabled(true)
                                .build()
                        )
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.NORMAL
                component.forceLocationUpdate(location)
                styleChangeIdlingResource.waitForStyle(maplibreMap, TestStyles.getPredefinedStyleWithFallback("Bright"))
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(component.renderMode, `is`(equalTo(RenderMode.NORMAL)))

                // Check that the Source has been re-added to the new map style
                val source: GeoJsonSource? = maplibreMap.style!!.getSourceAs(LOCATION_SOURCE)
                assertThat(source, notNullValue())

                // Check that the pulsing circle layer visibilities is set to visible
                assertThat(maplibreMap.isLayerVisible(PULSING_CIRCLE_LAYER), `is`(true))
            }
        }
    }

    @Test
    fun renderModeCompass_bearingLayersDoGetAdded() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.COMPASS
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun pulsingCircle_enableLocationComponent_pulsingLayerVisibility() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                component.applyStyle(
                    LocationComponentOptions.builder(context)
                        .pulseEnabled(true).build()
                )

                assertThat(maplibreMap.isLayerVisible(PULSING_CIRCLE_LAYER), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun pulsingCircle_disableLocationComponent_pulsingLayerVisibility() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = false
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.isLayerVisible(PULSING_CIRCLE_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun pulsingCircle_changeColorCheck() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                component.applyStyle(
                    LocationComponentOptions.builder(context)
                        .pulseEnabled(true)
                        .pulseColor(Color.RED)
                        .build()
                )

                component.applyStyle(
                    LocationComponentOptions.builder(context)
                        .pulseEnabled(true)
                        .pulseColor(Color.BLUE)
                        .build()
                )

                maplibreMap.style.apply {
                    assertThat(component.locationComponentOptions.pulseColor(), `is`(Color.BLUE))
                }
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun pulsingCircle_changeSpeedCheck() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                component.applyStyle(
                    LocationComponentOptions.builder(context)
                        .pulseEnabled(true)
                        .pulseSingleDuration(8000f)
                        .build()
                )

                component.applyStyle(
                    LocationComponentOptions.builder(context)
                        .pulseEnabled(true)
                        .pulseSingleDuration(400f)
                        .build()
                )

                maplibreMap.style.apply {
                    assertThat(component.locationComponentOptions.pulseSingleDuration(), `is`(400f))
                }
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun renderModeGps_navigationLayersDoGetAdded() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.GPS
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun dontShowPuckWhenRenderModeSetAndComponentDisabled() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)
                component.isLocationComponentEnabled = false
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                component.renderMode = RenderMode.GPS

                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun whenLocationComponentDisabled_doesSetAllLayersToVisibilityNone() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.NORMAL
                component.forceLocationUpdate(location)
                component.isLocationComponentEnabled = false
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                // Check that all layers visibilities are set to none
                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(false))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun onMapChange_locationComponentLayersDoGetRedrawn() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.renderMode = RenderMode.NORMAL
                component.forceLocationUpdate(location)
                styleChangeIdlingResource.waitForStyle(maplibreMap, TestStyles.getPredefinedStyleWithFallback("Bright"))
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(component.renderMode, `is`(equalTo(RenderMode.NORMAL)))

                // Check that the Source has been re-added to the new map style
                val source: GeoJsonSource? = maplibreMap.style!!.getSourceAs(LOCATION_SOURCE)
                assertThat(source, notNullValue())

                // Check that all layers visibilities are set to visible
                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun whenStyleChanged_continuesUsingStaleIcons() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.applyStyle(LocationComponentOptions.builder(context).staleStateTimeout(100).build())
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(150)

                assertThat(
                    maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getBooleanProperty(PROPERTY_LOCATION_STALE),
                    `is`(true)
                )

                maplibreMap.setStyle(Style.Builder().fromUrl(TestStyles.getPredefinedStyleWithFallback("Bright")))
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(
                    maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getBooleanProperty(PROPERTY_LOCATION_STALE),
                    `is`(true)
                )
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun whenStyleChanged_isDisabled_hasLayerBelow_staysHidden() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                component.isLocationComponentEnabled = false
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                assertThat(maplibreMap.queryRenderedFeatures(location, FOREGROUND_LAYER).isEmpty(), `is`(true))

                val options = component.locationComponentOptions
                    .toBuilder()
                    .layerBelow("road-label")
                    .build()

                component.applyStyle(options)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                assertThat(maplibreMap.queryRenderedFeatures(location, FOREGROUND_LAYER).isEmpty(), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun whenStyleChanged_staleStateChanges() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.applyStyle(LocationComponentOptions.builder(context).staleStateTimeout(1).build())
                styleChangeIdlingResource.waitForStyle(maplibreMap, MAPBOX_HEAVY_STYLE)
                pushSourceUpdates(styleChangeIdlingResource) {
                    component.forceLocationUpdate(location)
                }
            }
        }
        executeComponentTest(componentAction)

        // Waiting for style to finish loading while pushing updates
        onView(withId(android.R.id.content)).check(matches(isDisplayed()))
    }

    @Test
    fun whenStyleChanged_layerVisibilityUpdates() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                styleChangeIdlingResource.waitForStyle(maplibreMap, MAPBOX_HEAVY_STYLE)
                uiController.loopMainThreadForAtLeast(100)
                var show = true
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, maplibreMap.style!!)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                pushSourceUpdates(styleChangeIdlingResource) {
                    component.isLocationComponentEnabled = show
                    show = !show
                }

                TestingAsyncUtils.waitForLayer(uiController, mapView)
            }
        }
        executeComponentTest(componentAction)

        // Waiting for style to finish loading while pushing updates
        onView(withId(android.R.id.content)).check(matches(isDisplayed()))
    }

    @Test
    fun accuracy_visibleWithNewLocation() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(location), 16.0))
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(ACCURACY_RADIUS_ANIMATION_DURATION)

                assertEquals(
                    Utils.calculateZoomLevelRadius(maplibreMap, location) /*meters projected to radius on zoom 16*/,
                    maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0]
                        .getNumberProperty(PROPERTY_ACCURACY_RADIUS).toFloat(),
                    0.1f
                )
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun accuracy_visibleWhenCameraEased() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)

                val target = LatLng(location)
                val zoom = 16.0
                maplibreMap.easeCamera(CameraUpdateFactory.newLatLngZoom(target, zoom), 300)
                uiController.loopMainThreadForAtLeast(300)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(
                    Math.abs(zoom - maplibreMap.cameraPosition.zoom) < 0.1 &&
                        Math.abs(target.latitude - maplibreMap.cameraPosition.target!!.latitude) < 0.1 &&
                        Math.abs(target!!.longitude - maplibreMap.cameraPosition.target!!.longitude) < 0.1,
                    `is`(true)
                )

                val expectedRadius =
                    Utils.calculateZoomLevelRadius(maplibreMap, location) /*meters projected to radius on zoom 16*/
                assertThat(
                    Math.abs(
                        expectedRadius - maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getNumberProperty(
                            PROPERTY_ACCURACY_RADIUS
                        ).toFloat()
                    ) < 0.1,
                    `is`(true)
                )
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun accuracy_visibleWhenCameraMoved() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)

                val target = LatLng(location)
                val zoom = 16.0
                maplibreMap.moveCamera(CameraUpdateFactory.newLatLngZoom(target, zoom))
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(
                    abs(zoom - maplibreMap.cameraPosition.zoom) < 0.1 &&
                        abs(target.latitude - maplibreMap.cameraPosition.target!!.latitude) < 0.1 &&
                        abs(target!!.longitude - maplibreMap.cameraPosition.target!!.longitude) < 0.1,
                    `is`(true)
                )

                val expectedRadius =
                    Utils.calculateZoomLevelRadius(maplibreMap, location) /*meters projected to radius on zoom 16*/
                assertThat(
                    Math.abs(
                        expectedRadius - maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getNumberProperty(
                            PROPERTY_ACCURACY_RADIUS
                        ).toFloat()
                    ) < 0.1,
                    `is`(true)
                )
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun applyStyle_layerBelow_restoreLayerVisibility() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                component.applyStyle(LocationComponentOptions.builder(context).layerBelow("road-label").build())

                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BACKGROUND_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(SHADOW_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(true))
                assertThat(maplibreMap.isLayerVisible(BEARING_LAYER), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    @UiThreadTest
    fun test_15026_missingShadowGradientRadius() {
        // test for https://github.com/mapbox/mapbox-gl-native/issues/15026
        val shadowDrawable = BitmapUtils.getDrawableFromRes(context, R.drawable.mapbox_user_icon_shadow_0px_test)
        Utils.generateShadow(shadowDrawable, 0f)
    }

    @After
    override fun afterTest() {
        super.afterTest()
        IdlingRegistry.getInstance().unregister(styleChangeIdlingResource)
    }

    private fun executeComponentTest(listener: LocationComponentAction.OnPerformLocationComponentAction) {
        onView(withId(android.R.id.content)).perform(LocationComponentAction(maplibreMap, listener))
    }
}
