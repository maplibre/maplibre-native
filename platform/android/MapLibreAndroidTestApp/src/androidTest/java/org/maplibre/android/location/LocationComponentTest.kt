package org.maplibre.android.location

import android.Manifest
import android.R
import android.content.Context
import android.graphics.Color
import android.graphics.RectF
import android.location.Location
import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.IdlingRegistry
import androidx.test.espresso.UiController
import androidx.test.espresso.assertion.ViewAssertions.matches
import androidx.test.espresso.matcher.ViewMatchers.*
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.GrantPermissionRule
import org.maplibre.geojson.Point
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.*
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.location.utils.*
import org.maplibre.android.location.utils.MapLibreTestingUtils.Companion.MAPBOX_HEAVY_STYLE
import org.maplibre.android.location.utils.MapLibreTestingUtils.Companion.pushSourceUpdates
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.activity.EspressoTest
import org.maplibre.android.testapp.activity.espresso.EspressoTestActivity
import org.maplibre.android.testapp.utils.TestingAsyncUtils
import org.maplibre.android.utils.BitmapUtils
import org.maplibre.android.utils.ColorUtils
import org.hamcrest.CoreMatchers.*
import org.junit.*
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.runner.RunWith
import org.maplibre.android.testapp.styles.TestStyles

@RunWith(AndroidJUnit4ClassRunner::class)
class LocationComponentTest : EspressoTest() {

    @Rule
    @JvmField
    val permissionRule: GrantPermissionRule = GrantPermissionRule.grant(Manifest.permission.ACCESS_FINE_LOCATION)

    private lateinit var styleChangeIdlingResource: StyleChangeIdlingResource
    private val location: Location by lazy {
        val initLocation = Location("")
        initLocation.latitude = 15.0
        initLocation.longitude = 17.0
        initLocation.bearing = 10f
        initLocation.accuracy = 150f
        initLocation
    }

    private lateinit var locationComponentActivationOptions: LocationComponentActivationOptions

    override fun validateTestSetup() {
        super.validateTestSetup()
        assertThat(maplibreMap.style, notNullValue())
    }

    @Before
    override fun beforeTest() {
        super.beforeTest()
        styleChangeIdlingResource = StyleChangeIdlingResource()
        IdlingRegistry.getInstance().register(styleChangeIdlingResource)
    }

    @Test
    fun locationComponent_initializesLocationEngineCorrectlyWhenOnesNotProvided() {
        validateTestSetup()
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
                        .build()
                )
                component.isLocationComponentEnabled = true

                val locationEngine = component.locationEngine
                assertThat(locationEngine, notNullValue())

                TestingAsyncUtils.waitForLayer(uiController, mapView)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponent_initializesLocationEngineCorrectlyWhenOnesNotProvidedButHasOptions() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .staleStateTimeout(200)
                            .enableStaleState(false)
                            .accuracyAlpha(.5f)
                            .accuracyColor(Color.BLUE)
                            .build()
                    )
                    .build()

                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                val locationEngine = component.locationEngine
                val componentOptions = component.locationComponentOptions

                assertThat(locationEngine, notNullValue())
                assertThat(componentOptions, notNullValue())

                TestingAsyncUtils.waitForLayer(uiController, mapView)
                assertThat(componentOptions?.accuracyAlpha(), `is`(.5f))
                assertThat(componentOptions?.accuracyColor(), `is`(Color.BLUE))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponent_doesntInitializeEngineWhenNullProvided() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .locationEngine(null)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .staleStateTimeout(200)
                            .enableStaleState(false)
                            .accuracyAlpha(.5f)
                            .accuracyColor(Color.BLUE)
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                val locationEngine = component.locationEngine
                val componentOptions = component.locationComponentOptions

                assertThat(locationEngine, nullValue())
                assertThat(componentOptions, notNullValue())

                TestingAsyncUtils.waitForLayer(uiController, mapView)
                assertThat(componentOptions?.accuracyAlpha(), `is`(.5f))
                assertThat(componentOptions?.accuracyColor(), `is`(Color.BLUE))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test(expected = IllegalArgumentException::class)
    fun settingMapStyleImmediatelyBeforeLoadingComponent_throwsInvalidStyle() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                maplibreMap.setStyle(Style.Builder().fromUrl(TestStyles.getPredefinedStyleWithFallback("Bright")))

                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponent_doesntShowUntilFirstLocationFix() {
        validateTestSetup()
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

                // Source should be present but empty
                val mapView = (rule.activity as EspressoTestActivity).mapView
                assertThat(
                    maplibreMap.queryRenderedFeatures(
                        RectF(0f, 0f, mapView.width.toFloat(), mapView.height.toFloat()),
                        FOREGROUND_LAYER
                    )
                        .isEmpty(),
                    `is`(true)
                )

                // Force the first location update
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                // Check if the puck is visible
                assertThat(maplibreMap.queryRenderedFeatures(location, FOREGROUND_LAYER).isEmpty(), `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    //
    // Location Layer Options
    //

    @Test
    fun locationComponentOptions_disablingStaleStateDoesWorkCorrectly() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .staleStateTimeout(200)
                            .enableStaleState(false)
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(300) // waiting for stale state

                maplibreMap.querySourceFeatures(LOCATION_SOURCE).also { feature ->
                    feature.forEach {
                        assertThat(it.getBooleanProperty(PROPERTY_LOCATION_STALE), `is`(false))
                    }
                }
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponentOptions_loadsForegroundBitmapFromNameOption() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .foregroundName("custom-foreground-bitmap")
                            .backgroundName("custom-background-bitmap")
                            .foregroundStaleName("custom-foreground-stale-bitmap")
                            .backgroundStaleName("custom-background-stale-bitmap")
                            .bearingName("custom-bearing-bitmap")
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                val foregroundDrawable = BitmapUtils.getDrawableFromRes(context, R.drawable.ic_media_play)
                foregroundDrawable?.let {
                    maplibreMap.addImageFromDrawable("custom-foreground-bitmap", it)
                    maplibreMap.addImageFromDrawable("custom-background-bitmap", it)
                    maplibreMap.addImageFromDrawable("custom-foreground-stale-bitmap", it)
                    maplibreMap.addImageFromDrawable("custom-background-stale-bitmap", it)
                    maplibreMap.addImageFromDrawable("custom-bearing-bitmap", it)
                }

                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                val feature = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0]

                assertThat(maplibreMap.queryRenderedFeatures(location, FOREGROUND_LAYER).isEmpty(), `is`(false))
                assertThat(feature.getStringProperty(PROPERTY_FOREGROUND_ICON), `is`(equalTo("custom-foreground-bitmap")))
                assertThat(feature.getStringProperty(PROPERTY_BACKGROUND_ICON), `is`(equalTo("custom-background-bitmap")))
                assertThat(feature.getStringProperty(PROPERTY_FOREGROUND_STALE_ICON), `is`(equalTo("custom-foreground-stale-bitmap")))
                assertThat(feature.getStringProperty(PROPERTY_BACKGROUND_STALE_ICON), `is`(equalTo("custom-background-stale-bitmap")))
                assertThat(feature.getStringProperty(PROPERTY_BEARING_ICON), `is`(equalTo("custom-bearing-bitmap")))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponentOptions_loadsGpsNameWithGpsRenderMode() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .foregroundName("custom-foreground-bitmap")
                            .gpsName("custom-gps-bitmap")
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                component.renderMode = RenderMode.GPS
                component.forceLocationUpdate(location)
                val foregroundDrawable = BitmapUtils.getDrawableFromRes(context, R.drawable.ic_media_play)
                foregroundDrawable?.let {
                    maplibreMap.addImageFromDrawable("custom-foreground-bitmap", it)
                    maplibreMap.addImageFromDrawable("custom-gps-bitmap", it)
                }
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                val foregroundId = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getStringProperty(PROPERTY_FOREGROUND_ICON)

                assertThat(foregroundId, `is`(equalTo("custom-gps-bitmap")))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponentOptions_customIconNameRevertsToDefault() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                val foregroundDrawable = BitmapUtils.getDrawableFromRes(context, R.drawable.ic_media_play)
                foregroundDrawable?.let {
                    maplibreMap.addImageFromDrawable("custom-gps-bitmap", it)
                }
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .foregroundName("custom-foreground-bitmap")
                            .gpsName("custom-gps-bitmap")
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)
                component.isLocationComponentEnabled = true

                component.renderMode = RenderMode.GPS
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                val foregroundId = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getStringProperty(PROPERTY_FOREGROUND_ICON)
                assertThat(foregroundId, `is`(equalTo("custom-gps-bitmap")))

                component.applyStyle(LocationComponentOptions.builder(context).build())
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(FOREGROUND_ICON, maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getStringProperty(PROPERTY_FOREGROUND_ICON))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponentOptions_customGpsIconNameChangeBackWithMode() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                val foregroundDrawable = BitmapUtils.getDrawableFromRes(context, R.drawable.ic_media_play)
                foregroundDrawable?.let {
                    maplibreMap.addImageFromDrawable("custom-gps-bitmap", it)
                }
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .gpsName("custom-gps-bitmap")
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)
                component.isLocationComponentEnabled = true

                component.renderMode = RenderMode.GPS
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                val foregroundId = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getStringProperty(PROPERTY_FOREGROUND_ICON)
                assertThat(foregroundId, `is`(equalTo("custom-gps-bitmap")))

                component.renderMode = RenderMode.NORMAL
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(FOREGROUND_ICON, maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getStringProperty(PROPERTY_FOREGROUND_ICON))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun stillStaleAfterResuming() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .staleStateTimeout(200)
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(250) // engaging stale state

                assertThat(maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getBooleanProperty(PROPERTY_LOCATION_STALE), `is`(true))

                component.onStop()
                component.onStart()
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getBooleanProperty(PROPERTY_LOCATION_STALE), `is`(true))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(false))
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun stillNotStaleAfterResuming() {
        validateTestSetup()
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

                assertThat(maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getBooleanProperty(PROPERTY_LOCATION_STALE), `is`(false))

                component.onStop()
                component.onStart()
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getBooleanProperty(PROPERTY_LOCATION_STALE), `is`(false))
                assertThat(maplibreMap.isLayerVisible(ACCURACY_LAYER), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun locationComponentOptions_accuracyRingWithColor() {
        validateTestSetup()
        val color = Color.parseColor("#4A90E2")
        val rgbaColor = ColorUtils.colorToRgbaString(color)

        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .locationComponentOptions(
                        LocationComponentOptions.builder(context)
                            .accuracyColor(color)
                            .build()
                    )
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)

                component.isLocationComponentEnabled = true

                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                maplibreMap.querySourceFeatures(LOCATION_SOURCE).also { feature ->
                    feature.forEach {
                        assertThat(it.getStringProperty(PROPERTY_ACCURACY_COLOR), `is`(equalTo(rgbaColor)))
                    }
                }
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun forceLocationUpdate_doesMoveLocationLayerIconToCorrectPosition() {
        validateTestSetup()
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

                val point: Point = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].geometry() as Point
                assertThat(component.locationEngine, nullValue())
                assertEquals(point.latitude(), location.latitude, 0.1)
                assertEquals(point.longitude(), location.longitude, 0.1)
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun disablingComponentHidesPuck() {
        validateTestSetup()
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

                val point: Point = maplibreMap.queryRenderedFeatures(location, FOREGROUND_LAYER)[0].geometry() as Point
                assertEquals(point.latitude(), location.latitude, 0.1)
                assertEquals(point.longitude(), location.longitude, 0.1)

                component.isLocationComponentEnabled = false
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                assertThat(maplibreMap.queryRenderedFeatures(location, FOREGROUND_LAYER).isEmpty(), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun disablingComponentAndChangingStyleAllowsToEnableAgain() {
        validateTestSetup()
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
                maplibreMap.setStyle(Style.Builder().fromUrl(TestStyles.getPredefinedStyleWithFallback("Bright")))
                component.isLocationComponentEnabled = true
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.isLayerVisible(FOREGROUND_LAYER), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_isDisabledOnStart() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.onStop()
                component.onStart()

                component.activateLocationComponent(
                    LocationComponentActivationOptions
                        .builder(context, style)
                        .useDefaultLocationEngine(false)
                        .build()
                )

                assertThat(component.isLocationComponentEnabled, `is`(false))
                component.onStop()
                component.onStart()
                assertThat(component.isLocationComponentEnabled, `is`(false))

                component.isLocationComponentEnabled = true
                assertThat(component.isLocationComponentEnabled, `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_keepsEnabledWhenStoppedAndStarted() {
        validateTestSetup()
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
                assertThat(component.isLocationComponentEnabled, `is`(true))
                component.onStop()
                component.onStart()
                assertThat(component.isLocationComponentEnabled, `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_keepsDisabledWhenStoppedAndStarted() {
        validateTestSetup()
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
                component.isLocationComponentEnabled = false
                assertThat(component.isLocationComponentEnabled, `is`(false))
                component.onStop()
                component.onStart()
                assertThat(component.isLocationComponentEnabled, `is`(false))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_ableToChangeStyleAfterResuming() {
        validateTestSetup()
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

                component.onStop()
                component.onStart()

                maplibreMap.setStyle(Style.Builder().fromUri(TestStyles.getPredefinedStyleWithFallback("Bright")))
                TestingAsyncUtils.waitForLayer(uiController, mapView)
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_interruptedDuringStyleChange() {
        validateTestSetup()
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
                maplibreMap.setStyle(Style.Builder().fromUrl(TestStyles.getPredefinedStyleWithFallback("Bright")))
                component.onStop()
                component.onStart()
                TestingAsyncUtils.waitForLayer(uiController, mapView)
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_forceLocationUpdateAfterStopped() {
        validateTestSetup()
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
                component.onStop()
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.querySourceFeatures(LOCATION_SOURCE).isEmpty(), `is`(true))
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_acceptAndReuseLocationUpdatesBeforeLayerStarted() {
        validateTestSetup()
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
                component.onStop()
                component.forceLocationUpdate(location)
                component.onStart()
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                val point: Point = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].geometry() as Point
                assertEquals(point.latitude(), location.latitude, 0.1)
                assertEquals(point.longitude(), location.longitude, 0.1)
            }
        }
        executeComponentTest(componentAction)
    }

    @Test
    fun lifecycle_lifecycleChangeRightAfterStyleReload() {
        validateTestSetup()
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
                maplibreMap.setStyle(Style.Builder().fromUrl(TestStyles.getPredefinedStyleWithFallback("Bright")))
                component.onStop()
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                component.onStart()
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                val point: Point = maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].geometry() as Point
                assertEquals(point.latitude(), location.latitude, 0.1)
                assertEquals(point.longitude(), location.longitude, 0.1)
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
    fun mapChange_settingComponentStyle() {
        validateTestSetup()
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
                styleChangeIdlingResource.waitForStyle(maplibreMap, MAPBOX_HEAVY_STYLE)
                val options = LocationComponentOptions.builder(context)
                    .accuracyColor(Color.RED)
                    .build()

                pushSourceUpdates(styleChangeIdlingResource) {
                    component.applyStyle(options)
                }

                TestingAsyncUtils.waitForLayer(uiController, mapView)
            }
        }
        executeComponentTest(componentAction)

        // Waiting for style to finish loading while pushing updates
        onView(withId(R.id.content)).check(matches(isDisplayed()))
    }

    @Test
    fun mapChange_forcingLocation() {
        validateTestSetup()
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
                styleChangeIdlingResource.waitForStyle(maplibreMap, MAPBOX_HEAVY_STYLE)

                pushSourceUpdates(styleChangeIdlingResource) {
                    component.forceLocationUpdate(location)
                }

                TestingAsyncUtils.waitForLayer(uiController, mapView)
            }
        }
        executeComponentTest(componentAction)

        // Waiting for style to finish loading while pushing updates
        onView(withId(R.id.content)).check(matches(isDisplayed()))
    }

    @Test
    fun mapChange_settingMapStyleBeforeComponentCreation() {
        validateTestSetup()
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                styleChangeIdlingResource.waitForStyle(maplibreMap, MAPBOX_HEAVY_STYLE)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                locationComponentActivationOptions = LocationComponentActivationOptions
                    .builder(context, maplibreMap.style!!)
                    .useDefaultLocationEngine(false)
                    .build()
                component.activateLocationComponent(locationComponentActivationOptions)
                component.isLocationComponentEnabled = true

                val options = LocationComponentOptions.builder(context)
                    .accuracyColor(Color.RED)
                    .build()

                pushSourceUpdates(styleChangeIdlingResource) {
                    component.forceLocationUpdate(location)
                    component.applyStyle(options)
                }
            }
        }
        executeComponentTest(componentAction)

        // Waiting for style to finish loading while pushing updates
        onView(withId(R.id.content)).check(matches(isDisplayed()))
    }

    @Test
    fun animators_layerBearingCorrect() {
        validateTestSetup()
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
                location.bearing = 77f
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)
                assertEquals(77f, maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getNumberProperty(PROPERTY_GPS_BEARING).toFloat(), 0.1f)

                location.bearing = 92f
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS) // Waiting for the animation to finish
                assertEquals(92.0f, maplibreMap.querySourceFeatures(LOCATION_SOURCE)[0].getNumberProperty(PROPERTY_GPS_BEARING).toFloat(), 0.5f)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_cameraLatLngBearingCorrect() {
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
                component.cameraMode = CameraMode.TRACKING_GPS
                location.bearing = 77f
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)

                assertEquals(77.0, maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(location.latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(location.longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)

                location.bearing = 92f
                location.latitude = 30.0
                location.longitude = 35.0
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS) // Waiting for the animation to finish

                assertEquals(92.0, maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(location.latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(location.longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_cameraBearingCorrect() {
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
                component.cameraMode = CameraMode.NONE_GPS
                val latitude = maplibreMap.cameraPosition.target!!.latitude
                val longitude = maplibreMap.cameraPosition.target!!.longitude

                location.bearing = 77f
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)

                assertEquals(77.0, maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)

                location.bearing = 92f
                location.latitude = 30.0
                location.longitude = 35.0
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)

                assertEquals(92.0, maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_cameraNoneCorrect() {
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
                component.cameraMode = CameraMode.NONE
                val latitude = maplibreMap.cameraPosition.target!!.latitude
                val longitude = maplibreMap.cameraPosition.target!!.longitude
                val bearing = maplibreMap.cameraPosition.bearing

                location.bearing = 77f
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)

                assertEquals(bearing, maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)

                location.bearing = 92f
                location.latitude = 30.0
                location.longitude = 35.0
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)

                assertEquals(bearing, maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_focalPointAdjustment() {
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
                component.cameraMode = CameraMode.TRACKING
                component.cameraMode = CameraMode.NONE
                component.forceLocationUpdate(location)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertThat(maplibreMap.uiSettings.focalPoint, nullValue())
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontZoomWhileNotTracking() {
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
                component.cameraMode = CameraMode.NONE
                val zoom = maplibreMap.cameraPosition.zoom
                component.zoomWhileTracking(10.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(zoom, maplibreMap.cameraPosition.zoom, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_zoomWhileTracking() {
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
                component.cameraMode = CameraMode.TRACKING
                component.zoomWhileTracking(10.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(10.0, maplibreMap.cameraPosition.zoom, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    @Ignore("No explanation")
    fun animators_zoomWhileTrackingCanceledOnModeChange() {
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
                component.cameraMode = CameraMode.TRACKING
                component.zoomWhileTracking(15.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION / 2)
                component.cameraMode = CameraMode.NONE
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION / 2)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(15.0 / 2.0, maplibreMap.cameraPosition.zoom, 3.0)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontZoomWhileStopped() {
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

                component.cameraMode = CameraMode.TRACKING
                val zoom = maplibreMap.cameraPosition.zoom

                component.onStop()
                component.zoomWhileTracking(10.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(zoom, maplibreMap.cameraPosition.zoom, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontZoomWhileTransitioning() {
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

                val zoom = maplibreMap.cameraPosition.zoom
                component.setCameraMode(CameraMode.TRACKING_GPS, 500L, null, null, null, null)
                component.zoomWhileTracking(16.0, 1000)
                uiController.loopMainThreadForAtLeast(1000)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(zoom, maplibreMap.cameraPosition.zoom, 0.0001)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    @Ignore("No explanation")
    fun animators_cancelZoomWhileTracking() {
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
                component.cameraMode = CameraMode.TRACKING
                component.zoomWhileTracking(15.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION / 2)
                component.cancelZoomWhileTrackingAnimation()
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_ZOOM_ANIM_DURATION / 2)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(15.0 / 2.0, maplibreMap.cameraPosition.zoom, 3.0)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontTiltWhileNotTracking() {
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
                component.cameraMode = CameraMode.NONE
                val tilt = maplibreMap.cameraPosition.tilt
                component.tiltWhileTracking(30.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(tilt, maplibreMap.cameraPosition.tilt, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_tiltWhileTracking() {
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
                component.cameraMode = CameraMode.TRACKING
                component.tiltWhileTracking(30.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(30.0, maplibreMap.cameraPosition.tilt, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    @Ignore("No explanation")
    fun animators_tiltWhileTrackingCanceledOnModeChange() {
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
                component.cameraMode = CameraMode.TRACKING
                component.tiltWhileTracking(30.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION / 2)
                component.cameraMode = CameraMode.NONE
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION / 2)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(30.0 / 2.0, maplibreMap.cameraPosition.tilt, 3.0)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontTiltWhileStopped() {
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
                component.cameraMode = CameraMode.TRACKING
                val tilt = maplibreMap.cameraPosition.tilt

                component.onStop()
                component.tiltWhileTracking(30.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(tilt, maplibreMap.cameraPosition.tilt, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontTiltWhileTransitioning() {
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

                val tilt = maplibreMap.cameraPosition.tilt
                component.setCameraMode(CameraMode.TRACKING_GPS, 500L, null, null, null, null)
                component.tiltWhileTracking(30.0, 1000)
                uiController.loopMainThreadForAtLeast(1000)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertEquals(tilt, maplibreMap.cameraPosition.tilt, 0.0001)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    @Ignore("No explanation")
    fun animators_cancelTiltWhileTracking() {
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
                component.cameraMode = CameraMode.TRACKING
                component.tiltWhileTracking(30.0)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION / 2)
                component.cancelTiltWhileTrackingAnimation()
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_TILT_ANIM_DURATION / 2)

                assertEquals(30.0 / 2.0, maplibreMap.cameraPosition.tilt, 3.0)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontPaddingWhileNotTracking() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .build())
                component.isLocationComponentEnabled = true
                component.cameraMode = CameraMode.NONE
                val padding = maplibreMap.cameraPosition.padding
                component.paddingWhileTracking(doubleArrayOf(100.0, 200.0, 300.0, 400.0))
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_PADDING_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertArrayEquals(padding, maplibreMap.cameraPosition.padding, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontPaddingWhileStopped() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .build())
                component.isLocationComponentEnabled = true
                component.cameraMode = CameraMode.TRACKING
                val padding = maplibreMap.cameraPosition.padding

                component.onStop()
                component.paddingWhileTracking(doubleArrayOf(100.0, 200.0, 300.0, 400.0))
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_PADDING_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertArrayEquals(padding, maplibreMap.cameraPosition.padding, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_dontPaddingWhileTransitioning() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .build())
                component.isLocationComponentEnabled = true
                component.forceLocationUpdate(location)

                val padding = maplibreMap.cameraPosition.padding
                component.setCameraMode(CameraMode.TRACKING_GPS, 500L, null, null, null, null)
                component.paddingWhileTracking(doubleArrayOf(100.0, 200.0, 300.0, 400.0), 1000L)
                uiController.loopMainThreadForAtLeast(1000L)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertArrayEquals(padding, maplibreMap.cameraPosition.padding, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun animators_paddingWhileTracking() {
        val componentAction = object : LocationComponentAction.OnPerformLocationComponentAction {
            override fun onLocationComponentAction(
                component: LocationComponent,
                maplibreMap: MapLibreMap,
                style: Style,
                uiController: UiController,
                context: Context
            ) {
                component.activateLocationComponent(LocationComponentActivationOptions
                    .builder(context, style)
                    .useDefaultLocationEngine(false)
                    .build())
                component.isLocationComponentEnabled = true
                component.cameraMode = CameraMode.TRACKING
                val padding = doubleArrayOf(100.0, 200.0, 300.0, 400.0)
                component.paddingWhileTracking(padding)
                uiController.loopMainThreadForAtLeast(DEFAULT_TRACKING_PADDING_ANIM_DURATION)
                TestingAsyncUtils.waitForLayer(uiController, mapView)

                assertArrayEquals(padding, maplibreMap.cameraPosition.padding, 0.01)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun cameraPositionAdjustedToTrackingModeWhenComponentEnabled() {
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
                component.cameraMode = CameraMode.TRACKING_GPS
                component.forceLocationUpdate(location)
                component.isLocationComponentEnabled = false
                maplibreMap.moveCamera(CameraUpdateFactory.newLatLng(LatLng(51.0, 17.0)))
                maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(90.0))
                component.isLocationComponentEnabled = true
                TestingAsyncUtils.waitForLayer(uiController, mapView)
                uiController.loopMainThreadForAtLeast(MAX_ANIMATION_DURATION_MS)

                assertEquals(location.bearing.toDouble(), maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(location.latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(location.longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun cameraPositionSnappedToTargetIfExceedsThreshold() {
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
                component.cameraMode = CameraMode.NONE
                component.forceLocationUpdate(location)

                val target = LatLng(51.0, 17.0)
                maplibreMap.moveCamera(CameraUpdateFactory.newLatLng(target))
                maplibreMap.moveCamera(CameraUpdateFactory.zoomTo(15.0))
                maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(90.0))
                assertTrue(Utils.immediateAnimation(maplibreMap.projection, maplibreMap.cameraPosition.target!!, LatLng(location)))

                component.cameraMode = CameraMode.TRACKING_GPS
                assertEquals(location.bearing.toDouble(), maplibreMap.cameraPosition.bearing, 0.1)
                assertEquals(location.latitude, maplibreMap.cameraPosition.target!!.latitude, 0.1)
                assertEquals(location.longitude, maplibreMap.cameraPosition.target!!.longitude, 0.1)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun compassEngine_onComponentInitializedDefaultIsProvided() {
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
                assertTrue(component.compassEngine is LocationComponentCompassEngine)
            }
        }

        executeComponentTest(componentAction)
    }

    @Test
    fun compassEngine_changesWhenNewProvided() {
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
                val engine: CompassEngine = object : CompassEngine {
                    override fun addCompassListener(compassListener: CompassListener) {
                    }

                    override fun removeCompassListener(compassListener: CompassListener) {
                    }

                    override fun getLastHeading(): Float {
                        return 0f
                    }

                    override fun getLastAccuracySensorStatus(): Int {
                        return 0
                    }
                }

                component.compassEngine = engine
                assertThat(component.compassEngine, notNullValue())
                assertThat(component.compassEngine, `is`(equalTo(engine)))
            }
        }

        executeComponentTest(componentAction)
    }

    @After
    override fun afterTest() {
        super.afterTest()
        IdlingRegistry.getInstance().unregister(styleChangeIdlingResource)
    }

    private fun executeComponentTest(listener: LocationComponentAction.OnPerformLocationComponentAction) {
        onView(withId(R.id.content)).perform(LocationComponentAction(maplibreMap, listener))
    }
}
