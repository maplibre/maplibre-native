package org.maplibre.android.location

import android.graphics.Bitmap
import com.google.gson.JsonElement
import org.maplibre.geojson.Feature
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.MapLibreAnimator.AnimationsValueChangeListener
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.sources.GeoJsonSource
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.ArgumentMatchers
import org.mockito.Mockito

class LocationLayerControllerTest : BaseTest() {
    private val maplibreMap = Mockito.mock(MapLibreMap::class.java)
    private val style = Mockito.mock(
        Style::class.java
    )
    private val indicatorRenderer = Mockito.mock(
        LocationLayerRenderer::class.java
    )

    @Before
    fun before() {
        Mockito.`when`(maplibreMap.style).thenReturn(style)
    }

    @Test
    fun onInitialization_locationSourceIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addSource(locationSource)
    }

    @Test
    fun onInitialization_shadowLayerIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val shadowLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.SHADOW_LAYER))
            .thenReturn(shadowLayer)
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style)
            .addLayerBelow(shadowLayer, LocationComponentConstants.BACKGROUND_LAYER)
    }

    @Test
    fun onInitialization_backgroundLayerIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val backgroundLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.BACKGROUND_LAYER))
            .thenReturn(backgroundLayer)
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style)
            .addLayerBelow(backgroundLayer, LocationComponentConstants.FOREGROUND_LAYER)
    }

    @Test
    fun onInitialization_foregroundLayerIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val foregroundLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.FOREGROUND_LAYER))
            .thenReturn(foregroundLayer)
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style)
            .addLayerBelow(foregroundLayer, LocationComponentConstants.BEARING_LAYER)
    }

    @Test
    fun onInitialization_bearingLayerIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val bearingLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.BEARING_LAYER))
            .thenReturn(bearingLayer)
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val layerBelow = "layer-below"
        Mockito.`when`(options.layerBelow()).thenReturn(layerBelow)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addLayerBelow(bearingLayer, layerBelow)
    }

    @Test
    fun onInitialization_accuracyLayerIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val accuracyLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateAccuracyLayer()).thenReturn(accuracyLayer)
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style)
            .addLayerBelow(accuracyLayer, LocationComponentConstants.BACKGROUND_LAYER)
    }

    @Test
    fun onInitialization_pulsingCircleLayerIsAdded() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val pulsingCircleLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generatePulsingCircleLayer()).thenReturn(pulsingCircleLayer)
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style)
            .addLayerBelow(pulsingCircleLayer, LocationComponentConstants.ACCURACY_LAYER)
    }

    @Test
    fun onInitialization_numberOfCachedLayerIdsIsConstant() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val layerSet: Set<String> = HashSet()
        Mockito.`when`(sourceProvider.emptyLayerSet).thenReturn(layerSet)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val controller = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        controller.initializeComponents(maplibreMap.style, options)
        Assert.assertEquals(6, layerSet.size)
    }

    @Test
    fun applyStyle_styleShadowWithValidElevation() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(
            bitmapProvider.generateShadowBitmap(
                ArgumentMatchers.any(
                    LocationComponentOptions::class.java
                )
            )
        ).thenReturn(bitmap)
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        Mockito.`when`(options.elevation()).thenReturn(2f)

        // Style is applied on initialization
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.SHADOW_ICON, bitmap)
    }

    @Test
    fun applyStyle_ignoreStyleShadowWithInvalidElevation() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(
            bitmapProvider.generateShadowBitmap(
                ArgumentMatchers.any(
                    LocationComponentOptions::class.java
                )
            )
        ).thenReturn(bitmap)
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        Mockito.`when`(options.elevation()).thenReturn(0f)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style, Mockito.times(0))
            .addImage(LocationComponentConstants.SHADOW_ICON, bitmap)
    }

    @Test
    fun applyStyle_styleForegroundFromOptions() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val drawableResId = 123
        val tintColor = 456
        Mockito.`when`(options.foregroundDrawable()).thenReturn(drawableResId)
        Mockito.`when`(options.foregroundTintColor()).thenReturn(tintColor)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableResId, tintColor)).thenReturn(bitmap)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.FOREGROUND_ICON, bitmap)
    }

    @Test
    fun applyStyle_styleForegroundStaleFromOptions() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val drawableResId = 123
        val tintColor = 456
        Mockito.`when`(options.foregroundDrawableStale()).thenReturn(drawableResId)
        Mockito.`when`(options.foregroundStaleTintColor()).thenReturn(tintColor)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableResId, tintColor)).thenReturn(bitmap)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.FOREGROUND_STALE_ICON, bitmap)
    }

    @Test
    fun applyStyle_styleBackgroundFromOptions() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val drawableResId = 123
        val tintColor = 456
        Mockito.`when`(options.backgroundDrawable()).thenReturn(drawableResId)
        Mockito.`when`(options.backgroundTintColor()).thenReturn(tintColor)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableResId, tintColor)).thenReturn(bitmap)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.BACKGROUND_ICON, bitmap)
    }

    @Test
    fun applyStyle_styleBackgroundStaleFromOptions() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val drawableResId = 123
        val tintColor = 456
        Mockito.`when`(options.backgroundDrawableStale()).thenReturn(drawableResId)
        Mockito.`when`(options.backgroundStaleTintColor()).thenReturn(tintColor)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableResId, tintColor)).thenReturn(bitmap)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.BACKGROUND_STALE_ICON, bitmap)
    }

    @Test
    fun applyStyle_styleBearingFromOptions() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val drawableResId = 123
        val tintColor = 456
        Mockito.`when`(options.bearingDrawable()).thenReturn(drawableResId)
        Mockito.`when`(options.bearingTintColor()).thenReturn(tintColor)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableResId, tintColor)).thenReturn(bitmap)
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.BEARING_ICON, bitmap)
    }

    @Test
    fun applyStyle_specializedLayer_ignoreBitmapNames() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        Mockito.`when`(options.foregroundName()).thenReturn("new_name")
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            true
        )
        Mockito.verify(indicatorRenderer).updateIconIds(
            ArgumentMatchers.eq(LocationComponentConstants.FOREGROUND_ICON),
            ArgumentMatchers.anyString(),
            ArgumentMatchers.anyString(),
            ArgumentMatchers.anyString(),
            ArgumentMatchers.anyString()
        )
    }

    @Test
    fun applyStyle_layerBelowChanged() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(
            bitmapProvider.generateShadowBitmap(
                ArgumentMatchers.any(
                    LocationComponentOptions::class.java
                )
            )
        ).thenReturn(bitmap)
        val layerController = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        val bearingLayer2 = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.BEARING_LAYER))
            .thenReturn(bearingLayer2)
        val foregroundLayer2 = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.FOREGROUND_LAYER))
            .thenReturn(foregroundLayer2)
        val backgroundLayer2 = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.BACKGROUND_LAYER))
            .thenReturn(backgroundLayer2)
        val shadowLayer2 = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateLayer(LocationComponentConstants.SHADOW_LAYER))
            .thenReturn(shadowLayer2)
        val accuracyLayer2 = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(sourceProvider.generateAccuracyLayer()).thenReturn(accuracyLayer2)
        val layerBelow = "layer-below"
        Mockito.`when`(options.layerBelow()).thenReturn(layerBelow)
        layerController.applyStyle(options)
        Mockito.verify(style).removeLayer(LocationComponentConstants.BEARING_LAYER)
        Mockito.verify(style).removeLayer(LocationComponentConstants.FOREGROUND_LAYER)
        Mockito.verify(style).removeLayer(LocationComponentConstants.BACKGROUND_LAYER)
        Mockito.verify(style).removeLayer(LocationComponentConstants.SHADOW_LAYER)
        Mockito.verify(style).removeLayer(LocationComponentConstants.ACCURACY_LAYER)
        Mockito.verify(style).addLayerBelow(bearingLayer2, layerBelow)
        Mockito.verify(style)
            .addLayerBelow(foregroundLayer2, LocationComponentConstants.BEARING_LAYER)
        Mockito.verify(style)
            .addLayerBelow(backgroundLayer2, LocationComponentConstants.FOREGROUND_LAYER)
        Mockito.verify(style)
            .addLayerBelow(shadowLayer2, LocationComponentConstants.BACKGROUND_LAYER)
        Mockito.verify(style)
            .addLayerBelow(accuracyLayer2, LocationComponentConstants.BACKGROUND_LAYER)
    }

    @Test
    fun applyStyle_layerBelowNotChanged() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        var options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(
            bitmapProvider.generateShadowBitmap(
                ArgumentMatchers.any(
                    LocationComponentOptions::class.java
                )
            )
        ).thenReturn(bitmap)
        var layerBelow = "layer-below"
        Mockito.`when`(options.layerBelow()).thenReturn(layerBelow)
        val layerController = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        options = Mockito.mock(LocationComponentOptions::class.java)
        layerBelow = "layer-below"
        Mockito.`when`(options.layerBelow()).thenReturn(layerBelow)
        layerController.applyStyle(options)
        Mockito.verify(style, Mockito.times(0)).removeLayer(
            ArgumentMatchers.any(
                String::class.java
            )
        )
        Mockito.verify(style, Mockito.times(6)).addLayerBelow(
            ArgumentMatchers.any(
                Layer::class.java
            ),
            ArgumentMatchers.any(
                String::class.java
            )
        )
    }

    @Test
    fun applyStyle_layerBelowNotChangedNull() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        var options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(
            bitmapProvider.generateShadowBitmap(
                ArgumentMatchers.any(
                    LocationComponentOptions::class.java
                )
            )
        ).thenReturn(bitmap)
        val layerController = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        options = Mockito.mock(LocationComponentOptions::class.java)
        layerController.applyStyle(options)
        Mockito.verify(style, Mockito.times(0)).removeLayer(
            ArgumentMatchers.any(
                String::class.java
            )
        )
        Mockito.verify(style, Mockito.times(1)).addLayer(
            ArgumentMatchers.any(
                Layer::class.java
            )
        )
        Mockito.verify(style, Mockito.times(5)).addLayerBelow(
            ArgumentMatchers.any(
                Layer::class.java
            ),
            Mockito.any()
        )
    }

    @Test
    fun updateForegroundOffset_foregroundIconPropertyIsUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.cameraTiltUpdated(2.0)
        Mockito.verify(locationFeature).addProperty(
            ArgumentMatchers.eq(LocationComponentConstants.PROPERTY_FOREGROUND_ICON_OFFSET),
            ArgumentMatchers.any(
                JsonElement::class.java
            )
        )
    }

    @Test
    fun updateForegroundOffset_shadowPropertyIsUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.cameraTiltUpdated(2.0)
        Mockito.verify(locationFeature).addProperty(
            ArgumentMatchers.eq(LocationComponentConstants.PROPERTY_SHADOW_ICON_OFFSET),
            ArgumentMatchers.any(
                JsonElement::class.java
            )
        )
    }

    @Test
    fun onNewLatLngValue_locationFeatureIsUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        getAnimationListener<Any>(
            MapLibreAnimator.ANIMATOR_LAYER_LATLNG,
            layer.animationListeners
        )!!.onNewAnimationValue(LatLng())

        // wanted twice (once for initialization)
        Mockito.verify(locationSource, Mockito.times(2)).setGeoJson(locationFeature)
    }

    @Test
    fun onNewGpsBearingValue_locationFeatureIsUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.renderMode = RenderMode.GPS
        val gpsBearing = 2f
        getAnimationListener<Any>(
            MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING,
            layer.animationListeners
        )!!.onNewAnimationValue(gpsBearing)
        Mockito.verify(locationFeature)
            .addNumberProperty(LocationComponentConstants.PROPERTY_GPS_BEARING, gpsBearing)
    }

    @Test
    fun onNewGpsBearingValue_updateIgnoredWithInvalidRenderMode() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.renderMode = RenderMode.COMPASS
        val gpsBearing = 2f
        Assert.assertNull(
            getAnimationListener<Any>(
                MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING,
                layer.animationListeners
            )
        )
        Mockito.verify(locationFeature, Mockito.times(0))
            .addNumberProperty(LocationComponentConstants.PROPERTY_GPS_BEARING, gpsBearing)
    }

    @Test
    fun onNewCompassBearingValue_locationFeatureIsUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.renderMode = RenderMode.COMPASS
        val compassBearing = 2f
        getAnimationListener<Any>(
            MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
            layer.animationListeners
        )
            ?.onNewAnimationValue(compassBearing)
        Mockito.verify(locationFeature)
            .addNumberProperty(LocationComponentConstants.PROPERTY_COMPASS_BEARING, compassBearing)
    }

    @Test
    fun onNewCompassBearingValue_updateIgnoredWithInvalidRenderMode() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.renderMode = RenderMode.GPS
        val compassBearing = 2f
        Assert.assertNull(
            getAnimationListener<Any>(
                MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING,
                layer.animationListeners
            )
        )
        Mockito.verify(locationFeature, Mockito.times(0))
            .addNumberProperty(LocationComponentConstants.PROPERTY_COMPASS_BEARING, compassBearing)
    }

    @Test
    fun onNewAccuracyRadiusValue_locationFeatureIsUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.renderMode = RenderMode.NORMAL
        val accuracyRadiusValue = 2f
        getAnimationListener<Any>(MapLibreAnimator.ANIMATOR_LAYER_ACCURACY, layer.animationListeners)
            ?.onNewAnimationValue(accuracyRadiusValue)
        Mockito.verify(locationFeature).addNumberProperty(
            LocationComponentConstants.PROPERTY_ACCURACY_RADIUS,
            accuracyRadiusValue
        )
    }

    @Test
    fun onNewAccuracyRadiusValue_updateIgnoredWithInvalidRenderMode() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`<Any?>(style.getSourceAs(LocationComponentConstants.LOCATION_SOURCE))
            .thenReturn(locationSource)
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val locationFeature = Mockito.mock(
            Feature::class.java
        )
        val layer = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(locationFeature, options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        layer.renderMode = RenderMode.GPS
        val accuracyRadiusValue = 2f
        Assert.assertNull(
            getAnimationListener<Any>(
                MapLibreAnimator.ANIMATOR_LAYER_ACCURACY,
                layer.animationListeners
            )
        )
        Mockito.verify(locationFeature, Mockito.times(0))
            .addNumberProperty(
                LocationComponentConstants.PROPERTY_ACCURACY_RADIUS,
                accuracyRadiusValue
            )
    }

    @Test
    fun renderModeChanged_doNotNotifyAboutDuplicates_NORMAL() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val controller = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        controller.renderMode = RenderMode.NORMAL
        controller.renderMode = RenderMode.NORMAL
        Mockito.verify(internalRenderModeChangedListener, Mockito.times(1)).onRenderModeChanged(
            RenderMode.NORMAL
        )
    }

    @Test
    fun renderModeChanged_doNotNotifyAboutDuplicates_GPS() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        val locationSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(locationSource)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val controller = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        controller.renderMode = RenderMode.GPS
        controller.renderMode = RenderMode.GPS
        Mockito.verify(internalRenderModeChangedListener, Mockito.times(1)).onRenderModeChanged(
            RenderMode.GPS
        )
    }

    @Test
    fun layerHidden_renderModeChanged_layerShown_foregroundIconUpdated() {
        val internalRenderModeChangedListener = Mockito.mock(
            OnRenderModeChangedListener::class.java
        )
        val sourceProvider = buildLayerProvider()
        Mockito.`when`(
            sourceProvider.generateSource(
                ArgumentMatchers.any(
                    Feature::class.java
                ),
                ArgumentMatchers.eq(true)
            )
        ).thenReturn(
            Mockito.mock(
                GeoJsonSource::class.java
            )
        )
        val options = Mockito.mock(
            LocationComponentOptions::class.java
        )
        val drawableResId = 123
        val tintColor = 456
        Mockito.`when`(options.foregroundDrawable()).thenReturn(drawableResId)
        Mockito.`when`(options.foregroundTintColor()).thenReturn(tintColor)
        val bitmapProvider = Mockito.mock(
            LayerBitmapProvider::class.java
        )
        val bitmap = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableResId, tintColor)).thenReturn(bitmap)
        val controller = LocationLayerController(
            maplibreMap,
            maplibreMap.style,
            sourceProvider,
            buildFeatureProvider(options),
            bitmapProvider,
            options,
            internalRenderModeChangedListener,
            false
        )
        Mockito.verify(style).addImage(LocationComponentConstants.FOREGROUND_ICON, bitmap)
        val drawableGpsResId = 789
        Mockito.`when`(options.gpsDrawable()).thenReturn(drawableGpsResId)
        val bitmapGps = Mockito.mock(Bitmap::class.java)
        Mockito.`when`(bitmapProvider.generateBitmap(drawableGpsResId, tintColor))
            .thenReturn(bitmapGps)
        controller.hide()
        controller.renderMode = RenderMode.GPS
        controller.show()
        Mockito.verify(style).addImage(LocationComponentConstants.FOREGROUND_ICON, bitmapGps)
    }

    private fun buildFeatureProvider(options: LocationComponentOptions): LayerFeatureProvider {
        val provider = Mockito.mock(
            LayerFeatureProvider::class.java
        )
        Mockito.`when`(provider.generateLocationFeature(null, options.enableStaleState()))
            .thenReturn(
                Mockito.mock(
                    Feature::class.java
                )
            )
        return provider
    }

    private fun buildFeatureProvider(
        feature: Feature,
        options: LocationComponentOptions
    ): LayerFeatureProvider {
        val provider = Mockito.mock(
            LayerFeatureProvider::class.java
        )
        Mockito.`when`(provider.generateLocationFeature(null, options.enableStaleState()))
            .thenReturn(feature)
        return provider
    }

    private fun buildLayerProvider(): LayerSourceProvider {
        val layerSourceProvider = Mockito.mock(
            LayerSourceProvider::class.java
        )
        Mockito.`when`(layerSourceProvider.indicatorLocationLayerRenderer)
            .thenReturn(indicatorRenderer)
        Mockito.`when`(
            layerSourceProvider.getSymbolLocationLayerRenderer(
                ArgumentMatchers.any(
                    LayerFeatureProvider::class.java
                ),
                ArgumentMatchers.anyBoolean()
            )
        ).thenAnswer { invocation ->
            val featureProvider = invocation.getArgument<LayerFeatureProvider>(0)
            val isStale = invocation.getArgument<Boolean>(1)
            SymbolLocationLayerRenderer(layerSourceProvider, featureProvider, isStale)
        }
        val shadowLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(shadowLayer.id).thenReturn(LocationComponentConstants.SHADOW_LAYER)
        Mockito.`when`(layerSourceProvider.generateLayer(LocationComponentConstants.SHADOW_LAYER))
            .thenReturn(shadowLayer)
        val backgroundLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(backgroundLayer.id).thenReturn(LocationComponentConstants.BACKGROUND_LAYER)
        Mockito.`when`(layerSourceProvider.generateLayer(LocationComponentConstants.BACKGROUND_LAYER))
            .thenReturn(backgroundLayer)
        val foregroundLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(foregroundLayer.id).thenReturn(LocationComponentConstants.FOREGROUND_LAYER)
        Mockito.`when`(layerSourceProvider.generateLayer(LocationComponentConstants.FOREGROUND_LAYER))
            .thenReturn(foregroundLayer)
        val bearingLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(bearingLayer.id).thenReturn(LocationComponentConstants.BEARING_LAYER)
        Mockito.`when`(layerSourceProvider.generateLayer(LocationComponentConstants.BEARING_LAYER))
            .thenReturn(bearingLayer)
        val accuracyLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(accuracyLayer.id).thenReturn(LocationComponentConstants.ACCURACY_LAYER)
        Mockito.`when`(layerSourceProvider.generateAccuracyLayer()).thenReturn(accuracyLayer)
        val pulsingCircleLayer = Mockito.mock(
            Layer::class.java
        )
        Mockito.`when`(pulsingCircleLayer.id)
            .thenReturn(LocationComponentConstants.PULSING_CIRCLE_LAYER)
        Mockito.`when`(layerSourceProvider.generatePulsingCircleLayer())
            .thenReturn(pulsingCircleLayer)
        return layerSourceProvider
    }

    private fun <T> getAnimationListener(
        @MapLibreAnimator.Type animatorType: Int,
        holders: Set<AnimatorListenerHolder>
    ): AnimationsValueChangeListener<Any>? {
        for (holder in holders) {
            @MapLibreAnimator.Type val type = holder.animatorType
            if (type == animatorType) {
                return holder.listener
            }
        }
        return null
    }
}
