package org.maplibre.android.annotations

import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.PointF
import com.google.gson.JsonPrimitive
import com.mapbox.geojson.Point
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.annotations.data.Anchor
import org.maplibre.android.annotations.data.Halo
import org.maplibre.android.annotations.data.Icon
import org.maplibre.android.annotations.data.SdfIcon
import org.maplibre.android.annotations.data.Text
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.get
import org.maplibre.android.style.layers.PropertyFactory.iconAnchor
import org.maplibre.android.style.layers.PropertyFactory.iconColor
import org.maplibre.android.style.layers.PropertyFactory.iconHaloBlur
import org.maplibre.android.style.layers.PropertyFactory.iconHaloColor
import org.maplibre.android.style.layers.PropertyFactory.iconHaloWidth
import org.maplibre.android.style.layers.PropertyFactory.iconImage
import org.maplibre.android.style.layers.PropertyFactory.iconOffset
import org.maplibre.android.style.layers.PropertyFactory.iconOpacity
import org.maplibre.android.style.layers.PropertyFactory.iconRotate
import org.maplibre.android.style.layers.PropertyFactory.iconSize
import org.maplibre.android.style.layers.PropertyFactory.symbolSortKey
import org.maplibre.android.style.layers.PropertyFactory.textAnchor
import org.maplibre.android.style.layers.PropertyFactory.textColor
import org.maplibre.android.style.layers.PropertyFactory.textField
import org.maplibre.android.style.layers.PropertyFactory.textFont
import org.maplibre.android.style.layers.PropertyFactory.textHaloBlur
import org.maplibre.android.style.layers.PropertyFactory.textHaloColor
import org.maplibre.android.style.layers.PropertyFactory.textHaloWidth
import org.maplibre.android.style.layers.PropertyFactory.textJustify
import org.maplibre.android.style.layers.PropertyFactory.textLetterSpacing
import org.maplibre.android.style.layers.PropertyFactory.textMaxWidth
import org.maplibre.android.style.layers.PropertyFactory.textOffset
import org.maplibre.android.style.layers.PropertyFactory.textOpacity
import org.maplibre.android.style.layers.PropertyFactory.textRadialOffset
import org.maplibre.android.style.layers.PropertyFactory.textRotate
import org.maplibre.android.style.layers.PropertyFactory.textSize
import org.maplibre.android.style.layers.PropertyFactory.textTransform
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers
import org.mockito.Mockito

class SymbolManagerTest {
    private val draggableAnnotationController = Mockito.mock(
        DraggableAnnotationController::class.java
    )
    private val mapView: MapView = Mockito.mock(MapView::class.java)
    private val maplibreMap: MapLibreMap = Mockito.mock(MapLibreMap::class.java)
    private val style: Style = Mockito.mock(Style::class.java)
    private val geoJsonSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
    private val optionedGeoJsonSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
    private val layerId = "annotation_layer"
    private val symbolLayer: SymbolLayer = Mockito.mock(SymbolLayer::class.java)
    private lateinit var symbolManager: SymbolManager
    private val coreElementProvider: CoreElementProvider<SymbolLayer> = Mockito.mock(
        SymbolElementProvider::class.java
    )
    private val geoJsonOptions: GeoJsonOptions = Mockito.mock(GeoJsonOptions::class.java)

    @Before
    fun beforeTest() {
        Mockito.`when`(symbolLayer.id).thenReturn(layerId)
        Mockito.`when`(coreElementProvider.layer).thenReturn(symbolLayer)
        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(coreElementProvider.getSource(geoJsonOptions))
            .thenReturn(optionedGeoJsonSource)
        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
    }

    @Test
    fun testInitialization() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(style).addSource(geoJsonSource)
        Mockito.verify(style).addLayer(symbolLayer)
        Assert.assertTrue(symbolManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(symbolLayer).setProperties(
            *symbolManager.constantPropertyUsageMap.values.toTypedArray()
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setFilter(
            ArgumentMatchers.any(
                Expression::class.java
            )
        )
    }

    @Test
    fun testInitializationOnStyleReload() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(style).addSource(geoJsonSource)
        Mockito.verify(style).addLayer(symbolLayer)
        Assert.assertTrue(symbolManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(symbolLayer).setProperties(
            *symbolManager.constantPropertyUsageMap.values.toTypedArray()
        )
        val filter: Expression = Expression.literal(false)
        symbolManager.setFilter(filter)
        val loadingArgumentCaptor: ArgumentCaptor<MapView.OnDidFinishLoadingStyleListener> =
            ArgumentCaptor.forClass(
                MapView.OnDidFinishLoadingStyleListener::class.java
            )
        Mockito.verify(mapView).addOnDidFinishLoadingStyleListener(loadingArgumentCaptor.capture())
        loadingArgumentCaptor.value.onDidFinishLoadingStyle()
        val styleLoadedArgumentCaptor: ArgumentCaptor<Style.OnStyleLoaded> =
            ArgumentCaptor.forClass(
                Style.OnStyleLoaded::class.java
            )
        Mockito.verify(maplibreMap).getStyle(styleLoadedArgumentCaptor.capture())
        val newStyle: Style = Mockito.mock(Style::class.java)
        Mockito.`when`(newStyle.isFullyLoaded).thenReturn(true)
        val newSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(newSource)
        val newLayer: SymbolLayer = Mockito.mock(SymbolLayer::class.java)
        Mockito.`when`<Any>(coreElementProvider.layer).thenReturn(newLayer)
        styleLoadedArgumentCaptor.value.onStyleLoaded(newStyle)
        Mockito.verify(newStyle).addSource(newSource)
        Mockito.verify(newStyle).addLayer(newLayer)
        Assert.assertTrue(symbolManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(newLayer).setProperties(
            *symbolManager.constantPropertyUsageMap.values.toTypedArray()
        )
        Mockito.verify(symbolLayer).setFilter(filter)
    }

    @Test
    fun testLayerBelowInitialization() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            "test_layer",
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(style).addSource(geoJsonSource)
        Mockito.verify(style).addLayerBelow(symbolLayer, "test_layer")
        Assert.assertTrue(symbolManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(symbolLayer).setProperties(
            *symbolManager.constantPropertyUsageMap.values.toTypedArray()
        )
    }

    @Test
    fun testGeoJsonOptionsInitialization() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            geoJsonOptions,
            draggableAnnotationController
        )
        Mockito.verify(style).addSource(optionedGeoJsonSource)
        Mockito.verify(style).addLayer(symbolLayer)
        Assert.assertTrue(symbolManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(symbolLayer).setProperties(
            *symbolManager.constantPropertyUsageMap.values.toTypedArray()
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setFilter(
            ArgumentMatchers.any(
                Expression::class.java
            )
        )
    }

    @Test
    fun testLayerId() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertEquals(layerId, symbolManager.layerId)
    }

    @Test
    fun testAddSymbol() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val symbol = Symbol(LatLng())
        symbolManager.add(symbol)
        Assert.assertSame(symbol, symbolManager.annotations.values.first())
    }

    @Test
    fun addSymbols() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        ).map { Symbol(it) }.forEach { symbolManager.add(it) }
        Assert.assertTrue("Amount of annotations should match", symbolManager.annotations.size == 2)
    }

    @Test
    fun testDeleteSymbol() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val symbol = Symbol(LatLng())
        symbolManager.add(symbol)
        Assert.assertEquals(
            "After adding a symbol, one symbol should be present",
            1,
            symbolManager.annotations.size
        )
        symbolManager.delete(symbol)
        Assert.assertEquals(
            "After removing the only symbol, no symbols should be present anymore",
            0,
            symbolManager.annotations.size
        )
    }

    @Test
    fun testGeometrySymbol() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLng = LatLng(12.0, 34.0)
        val symbol = Symbol(latLng)
        Assert.assertEquals(symbol.position, latLng)
        Assert.assertEquals(symbol.geometry, Point.fromLngLat(34.0, 12.0))
    }

    @Test
    fun testSymbolId() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        (0 until 10).map {
            Symbol(LatLng()).also { symbolManager.add(it) }
        }.forEachIndexed { index, symbol ->
            Assert.assertEquals(
                "Symbol ID should be generated and assigned automatically, starting with -1",
                symbol.id,
                -index.toLong() - 1L
            )
        }
    }

    @Test
    fun testSymbolDraggableFlag() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val symbol = Symbol(LatLng())
        symbolManager.add(symbol)
        Assert.assertFalse(symbol.draggable)
        symbol.draggable = true
        Assert.assertTrue(symbol.draggable)
        symbol.draggable = false
        Assert.assertFalse(symbol.draggable)
        // Verify that map update was posted
        Mockito.verify(mapView).post(Mockito.any())
    }

    @Test
    fun testSymbolSortKeyLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(symbolSortKey(get("symbol-sort-key")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng()).apply {
                zLayer = 2
            }
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(symbolSortKey(get("symbol-sort-key")))
                )
            )
        }
    }

    @Test
    fun testIconSizeLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconSize(get("icon-size"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                Icon(Mockito.mock(Bitmap::class.java), size = 2f)
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconSize(get("icon-size"))))
            )
        }
    }

    @Test
    fun testIconImageLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconImage(get("icon-image"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                Icon(Mockito.mock(Bitmap::class.java))
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconImage(get("icon-image"))))
            )
        }
    }

    @Test
    fun testIconRotateLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconRotate(get("icon-rotate"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                Icon(Mockito.mock(Bitmap::class.java), rotate = 2f)
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconRotate(get("icon-rotate"))))
            )
        }
    }

    @Test
    fun testIconOffsetLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconOffset(get("icon-offset"))))
        )
        for (i in 0 until 2) {
            val symbol =
                Symbol(
                    LatLng(),
                    Icon(
                        Mockito.mock(Bitmap::class.java),
                        offset = PointF(1f, 0f)
                    )
                )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconOffset(get("icon-offset"))))
            )
        }
    }

    @Test
    fun testIconAnchorLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconAnchor(get("icon-anchor"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                Icon(Mockito.mock(Bitmap::class.java), anchor = Anchor.LEFT)
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconAnchor(get("icon-anchor"))))
            )
        }
    }

    @Test
    fun testTextFieldLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textField(get("text-field"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text"))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textField(get("text-field"))))
            )
        }
    }

    @Test
    fun testTextFontLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textFont(get("text-font"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", font = listOf("Open Sans Regular")))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textFont(get("text-font"))))
            )
        }
    }

    @Test
    fun testTextSizeLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textSize(get("text-size"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", size = 2f))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textSize(get("text-size"))))
            )
        }
    }

    @Test
    fun testTextMaxWidthLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textMaxWidth(get("text-max-width"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", maxWidth = 2f))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textMaxWidth(get("text-max-width"))))
            )
        }
    }

    @Test
    fun testTextLetterSpacingLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textLetterSpacing(get("text-letter-spacing")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", letterSpacing = 2f))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(textLetterSpacing(get("text-letter-spacing")))
                )
            )
        }
    }

    @Test
    fun testTextJustifyLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textJustify(get("text-justify"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", justify = Text.Justify.AUTO))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textJustify(get("text-justify"))))
            )
        }
    }

    @Test
    fun testTextRadialOffsetLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textRadialOffset(get("text-radial-offset")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", offset = Text.RadialOffset(2f)))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(textRadialOffset(get("text-radial-offset")))
                )
            )
        }
    }

    @Test
    fun testTextAnchorLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textAnchor(get("text-anchor"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", anchor = Anchor.LEFT))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textAnchor(get("text-anchor"))))
            )
        }
    }

    @Test
    fun testTextRotateLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textRotate(get("text-rotate"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", rotate = 2f))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textRotate(get("text-rotate"))))
            )
        }
    }

    @Test
    fun testTextTransformLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textTransform(get("text-transform"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                text = Text("text", transform = Text.Transform.LOWERCASE)
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textTransform(get("text-transform"))))
            )
        }
    }

    @Test
    fun testTextOffsetLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textOffset(get("text-offset"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                text = Text("text", offset = Text.AbsoluteOffset(PointF(1f, 0f)))
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textOffset(get("text-offset"))))
            )
        }
    }

    @Test
    fun testIconOpacityLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconOpacity(get("icon-opacity"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), Icon(Mockito.mock(Bitmap::class.java), opacity = 0.5f))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconOpacity(get("icon-opacity"))))
            )
        }
    }

    @Test
    fun testIconColorLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconColor(get("icon-color"))))
        )
        for (i in 0 until 2) {
            val symbol =
                Symbol(
                    LatLng(),
                    SdfIcon(
                        Mockito.mock(Bitmap::class.java),
                        color = Color.WHITE
                    )
                )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconColor(get("icon-color"))))
            )
        }
    }

    @Test
    fun testIconHaloColorLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(iconHaloColor(get("icon-halo-color")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                SdfIcon(
                    Mockito.mock(Bitmap::class.java),
                    Color.WHITE,
                    Halo(2f, Color.BLUE)
                )
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(iconHaloColor(get("icon-halo-color")))
                )
            )
        }
    }

    @Test
    fun testIconHaloWidthLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(iconHaloWidth(get("icon-halo-width")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                SdfIcon(
                    Mockito.mock(Bitmap::class.java),
                    Color.WHITE,
                    Halo(2f, Color.BLUE)
                )
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(iconHaloWidth(get("icon-halo-width")))
                )
            )
        }
    }

    @Test
    fun testIconHaloBlurLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconHaloBlur(get("icon-halo-blur"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(
                LatLng(),
                SdfIcon(
                    Mockito.mock(Bitmap::class.java),
                    Color.WHITE,
                    Halo(2f, Color.BLUE, 2f)
                )
            )
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(iconHaloBlur(get("icon-halo-blur"))))
            )
        }
    }

    @Test
    fun testTextOpacityLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textOpacity(get("text-opacity"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", opacity = 0.5f))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textOpacity(get("text-opacity"))))
            )
        }
    }

    @Test
    fun testTextColorLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textColor(get("text-color"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", color = Color.CYAN))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textColor(get("text-color"))))
            )
        }
    }

    @Test
    fun testTextHaloColorLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textHaloColor(get("text-halo-color")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", halo = Halo(2f, Color.GREEN)))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(textHaloColor(get("text-halo-color")))
                )
            )
        }
    }

    @Test
    fun testTextHaloWidthLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textHaloWidth(get("text-halo-width")))
            )
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", halo = Halo(2f, Color.GREEN)))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(textHaloWidth(get("text-halo-width")))
                )
            )
        }
    }

    @Test
    fun testTextHaloBlurLayerProperty() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(symbolLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textHaloBlur(get("text-halo-blur"))))
        )
        for (i in 0 until 2) {
            val symbol = Symbol(LatLng(), text = Text("text", halo = Halo(2f, Color.GREEN, blur = 0.5f)))
            symbolManager.add(symbol)
            symbolManager.updateSourceNow()
            Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(textHaloBlur(get("text-halo-blur"))))
            )
        }
    }

    @Test
    fun testSymbolLayerFilter() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val expression: Expression = Expression.eq(Expression.get("test"), "selected")
        Mockito.verify(symbolLayer, Mockito.times(0)).setFilter(expression)
        symbolManager.setFilter(expression)
        Mockito.verify(symbolLayer, Mockito.times(1)).setFilter(expression)
        Mockito.`when`(symbolLayer.filter).thenReturn(expression)
        Assert.assertEquals(expression, symbolManager.filter)
        Assert.assertEquals(expression, symbolManager.layerFilter)
    }

    @Test
    fun testClickListener() {
        val listener = object : OnSymbolClickListener {
            override fun onAnnotationClick(t: Symbol) = false
        }
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(symbolManager.getClickListeners().isEmpty())
        symbolManager.addClickListener(listener)
        Assert.assertTrue(symbolManager.getClickListeners().contains(listener))
        symbolManager.removeClickListener(listener)
        Assert.assertTrue(symbolManager.getClickListeners().isEmpty())
    }

    @Test
    fun testLongClickListener() {
        val listener = object : OnSymbolLongClickListener {
            override fun onAnnotationLongClick(t: Symbol) = false
        }
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(symbolManager.getLongClickListeners().isEmpty())
        symbolManager.addLongClickListener(listener)
        Assert.assertTrue(symbolManager.getLongClickListeners().contains(listener))
        symbolManager.removeLongClickListener(listener)
        Assert.assertTrue(symbolManager.getLongClickListeners().isEmpty())
    }

    @Test
    fun testDragListener() {
        val listener = object : OnSymbolDragListener {
            override fun onAnnotationDragStarted(annotation: Symbol) = Unit
            override fun onAnnotationDrag(annotation: Symbol) = Unit
            override fun onAnnotationDragFinished(annotation: Symbol) = Unit
        }
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(symbolManager.getDragListeners().isEmpty())
        symbolManager.addDragListener(listener)
        Assert.assertTrue(symbolManager.getDragListeners().contains(listener))
        symbolManager.removeDragListener(listener)
        Assert.assertTrue(symbolManager.getDragListeners().isEmpty())
    }

    @Test
    fun testCustomData() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val symbol = Symbol(LatLng()).apply {
            data = JsonPrimitive("hello")
        }
        symbolManager.add(symbol)
        Assert.assertEquals(JsonPrimitive("hello"), symbol.data)
    }

    @Test
    fun testClearAll() {
        symbolManager = SymbolManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val symbol = Symbol(LatLng())
        symbolManager.add(symbol)
        Assert.assertEquals(1, symbolManager.annotations.size)
        symbolManager.deleteAll()
        Assert.assertEquals(0, symbolManager.annotations.size)
    }
}
