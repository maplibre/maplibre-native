package org.maplibre.android.annotations

import android.graphics.PointF
import com.google.gson.JsonPrimitive
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.Geometry
import com.mapbox.geojson.Point
import java.util.Arrays
import junit.framework.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.annotations.ConvertUtils.convertArray
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.get
import org.maplibre.android.style.layers.Property.ICON_ANCHOR_CENTER
import org.maplibre.android.style.layers.Property.TEXT_ANCHOR_CENTER
import org.maplibre.android.style.layers.Property.TEXT_JUSTIFY_AUTO
import org.maplibre.android.style.layers.Property.TEXT_TRANSFORM_NONE
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
        Assert.assertTrue(symbolManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in symbolManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
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
        Assert.assertTrue(symbolManager.dataDrivenPropertyUsageMap.size > 0)
        for (value in symbolManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
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
        Assert.assertTrue(symbolManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in symbolManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
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
        Assert.assertTrue(symbolManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in symbolManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
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
        Assert.assertTrue(symbolManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in symbolManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
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
        val symbol = symbolManager.create(SymbolOptions().withLatLng(LatLng()))
        Assert.assertEquals(symbolManager.annotations[0], symbol)
    }

    @Test
    fun addSymbolFromFeatureCollection() {
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
        val geometry: Geometry = Point.fromLngLat(10.0, 10.0)
        val feature = Feature.fromGeometry(geometry).apply {
            addNumberProperty("symbol-sort-key", 2.0f)
            addNumberProperty("icon-size", 2.0f)
            addStringProperty("icon-image", "undefined")
            addNumberProperty("icon-rotate", 2.0f)
            addProperty("icon-offset", convertArray(arrayOf(0f, 0f)))
            addStringProperty("icon-anchor", ICON_ANCHOR_CENTER)
            addStringProperty("text-field", "")
            addProperty(
                "text-font",
                convertArray(arrayOf("Open Sans Regular", "Arial Unicode MS Regular"))
            )
            addNumberProperty("text-size", 2.0f)
            addNumberProperty("text-max-width", 2.0f)
            addNumberProperty("text-letter-spacing", 2.0f)
            addStringProperty("text-justify", TEXT_JUSTIFY_AUTO)
            addNumberProperty("text-radial-offset", 2.0f)
            addStringProperty("text-anchor", TEXT_ANCHOR_CENTER)
            addNumberProperty("text-rotate", 2.0f)
            addStringProperty("text-transform", TEXT_TRANSFORM_NONE)
            addProperty("text-offset", convertArray(arrayOf<Float>(0f, 0f)))
            addNumberProperty("icon-opacity", 2.0f)
            addStringProperty("icon-color", "rgba(0, 0, 0, 1)")
            addStringProperty("icon-halo-color", "rgba(0, 0, 0, 1)")
            addNumberProperty("icon-halo-width", 2.0f)
            addNumberProperty("icon-halo-blur", 2.0f)
            addNumberProperty("text-opacity", 2.0f)
            addStringProperty("text-color", "rgba(0, 0, 0, 1)")
            addStringProperty("text-halo-color", "rgba(0, 0, 0, 1)")
            addNumberProperty("text-halo-width", 2.0f)
            addNumberProperty("text-halo-blur", 2.0f)
            addBooleanProperty("is-draggable", true)
        }

        val symbols: List<Symbol> = symbolManager.create(FeatureCollection.fromFeature(feature))
        val symbol = symbols[0]
        Assert.assertEquals(symbol.geometry, geometry)
        Assert.assertEquals(symbol.symbolSortKey, 2.0f)
        Assert.assertEquals(symbol.iconSize, 2.0f)
        Assert.assertEquals(symbol.iconImage, "undefined")
        Assert.assertEquals(symbol.iconRotate, 2.0f)
        val iconOffsetExpected = PointF(0f, 0f)
        Assert.assertEquals(iconOffsetExpected.x, symbol.iconOffset.x)
        Assert.assertEquals(iconOffsetExpected.y, symbol.iconOffset.y)
        Assert.assertEquals(symbol.iconAnchor, ICON_ANCHOR_CENTER)
        Assert.assertEquals(symbol.textField, "")
        Assert.assertTrue(
            Arrays.equals(
                symbol.textFont,
                arrayOf("Open Sans Regular", "Arial Unicode MS Regular")
            )
        )
        Assert.assertEquals(symbol.textSize, 2.0f)
        Assert.assertEquals(symbol.textMaxWidth, 2.0f)
        Assert.assertEquals(symbol.textLetterSpacing, 2.0f)
        Assert.assertEquals(symbol.textJustify, TEXT_JUSTIFY_AUTO)
        Assert.assertEquals(symbol.textRadialOffset, 2.0f)
        Assert.assertEquals(symbol.textAnchor, TEXT_ANCHOR_CENTER)
        Assert.assertEquals(symbol.textRotate, 2.0f)
        Assert.assertEquals(symbol.textTransform, TEXT_TRANSFORM_NONE)
        val textOffsetExpected = PointF(0f, 0f)
        Assert.assertEquals(textOffsetExpected.x, symbol.textOffset.x)
        Assert.assertEquals(textOffsetExpected.y, symbol.textOffset.y)
        Assert.assertEquals(symbol.iconOpacity, 2.0f)
        Assert.assertEquals(symbol.iconColor, "rgba(0, 0, 0, 1)")
        Assert.assertEquals(symbol.iconHaloColor, "rgba(0, 0, 0, 1)")
        Assert.assertEquals(symbol.iconHaloWidth, 2.0f)
        Assert.assertEquals(symbol.iconHaloBlur, 2.0f)
        Assert.assertEquals(symbol.textOpacity, 2.0f)
        Assert.assertEquals(symbol.textColor, "rgba(0, 0, 0, 1)")
        Assert.assertEquals(symbol.textHaloColor, "rgba(0, 0, 0, 1)")
        Assert.assertEquals(symbol.textHaloWidth, 2.0f)
        Assert.assertEquals(symbol.textHaloBlur, 2.0f)
        Assert.assertTrue(symbol.isDraggable)
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
        val options = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        ).map { SymbolOptions().withLatLng(it) }
        val symbols = symbolManager.create(options)
        Assert.assertTrue("Returned value size should match", symbols.size == 2)
        Assert.assertTrue("Annotations size should match", symbolManager.annotations.size() == 2)
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
        val symbol = symbolManager.create(SymbolOptions().withLatLng(LatLng()))
        symbolManager.delete(symbol)
        Assert.assertTrue(symbolManager.annotations.size() == 0)
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
        val options = SymbolOptions().withLatLng(latLng)
        val symbol = symbolManager.create(options)
        Assert.assertEquals(options.latLng, latLng)
        Assert.assertEquals(symbol.latLng, latLng)
        Assert.assertEquals(options.geometry, Point.fromLngLat(34.0, 12.0))
        Assert.assertEquals(symbol.getGeometry(), Point.fromLngLat(34.0, 12.0))
    }

    @Test
    fun testFeatureIdSymbol() {
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
        val symbolZero = symbolManager.create(SymbolOptions().withLatLng(LatLng()))
        val symbolOne = symbolManager.create(SymbolOptions().withLatLng(LatLng()))
        Assert.assertEquals(symbolZero.feature[Symbol.ID_KEY].asLong, 0)
        Assert.assertEquals(symbolOne.feature[Symbol.ID_KEY].asLong, 1)
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
        val symbolZero = symbolManager.create(SymbolOptions().withLatLng(LatLng()))
        Assert.assertFalse(symbolZero.isDraggable)
        symbolZero.isDraggable = true
        Assert.assertTrue(symbolZero.isDraggable)
        symbolZero.isDraggable = false
        Assert.assertFalse(symbolZero.isDraggable)
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
        val options = SymbolOptions().withLatLng(LatLng()).withSymbolSortKey(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(symbolSortKey(get("symbol-sort-key")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(symbolSortKey(get("symbol-sort-key")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconSize(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconSize(get("icon-size"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconSize(get("icon-size"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconImage("undefined")
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconImage(get("icon-image"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconImage(get("icon-image"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconRotate(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconRotate(get("icon-rotate"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconRotate(get("icon-rotate"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconOffset(arrayOf(0f, 0f))
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconOffset(get("icon-offset"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconOffset(get("icon-offset"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconAnchor(ICON_ANCHOR_CENTER)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconAnchor(get("icon-anchor"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconAnchor(get("icon-anchor"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextField("")
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textField(get("text-field"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textField(get("text-field"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng())
            .withTextFont(arrayOf<String>("Open Sans Regular", "Arial Unicode MS Regular"))
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textFont(get("text-font"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textFont(get("text-font"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextSize(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textSize(get("text-size"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textSize(get("text-size"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextMaxWidth(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textMaxWidth(get("text-max-width"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textMaxWidth(get("text-max-width"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextLetterSpacing(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textLetterSpacing(get("text-letter-spacing")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textLetterSpacing(get("text-letter-spacing")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextJustify(TEXT_JUSTIFY_AUTO)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textJustify(get("text-justify"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textJustify(get("text-justify"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextRadialOffset(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textRadialOffset(get("text-radial-offset")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textRadialOffset(get("text-radial-offset")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextAnchor(TEXT_ANCHOR_CENTER)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textAnchor(get("text-anchor"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textAnchor(get("text-anchor"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextRotate(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textRotate(get("text-rotate"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textRotate(get("text-rotate"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextTransform(TEXT_TRANSFORM_NONE)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textTransform(get("text-transform"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textTransform(get("text-transform"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextOffset(arrayOf(0f, 0f))
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textOffset(get("text-offset"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textOffset(get("text-offset"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconOpacity(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconOpacity(get("icon-opacity"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconOpacity(get("icon-opacity"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconColor("rgba(0, 0, 0, 1)")
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconColor(get("icon-color"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconColor(get("icon-color"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconHaloColor("rgba(0, 0, 0, 1)")
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(iconHaloColor(get("icon-halo-color")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(iconHaloColor(get("icon-halo-color")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconHaloWidth(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(iconHaloWidth(get("icon-halo-width")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(iconHaloWidth(get("icon-halo-width")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withIconHaloBlur(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconHaloBlur(get("icon-halo-blur"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(iconHaloBlur(get("icon-halo-blur"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextOpacity(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textOpacity(get("text-opacity"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textOpacity(get("text-opacity"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextColor("rgba(0, 0, 0, 1)")
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textColor(get("text-color"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textColor(get("text-color"))))
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextHaloColor("rgba(0, 0, 0, 1)")
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textHaloColor(get("text-halo-color")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textHaloColor(get("text-halo-color")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextHaloWidth(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textHaloWidth(get("text-halo-width")))
            )
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(textHaloWidth(get("text-halo-width")))
            )
        )
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
        val options = SymbolOptions().withLatLng(LatLng()).withTextHaloBlur(2.0f)
        symbolManager.create(options)
        symbolManager.updateSourceNow()
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textHaloBlur(get("text-halo-blur"))))
        )
        symbolManager.create(options)
        Mockito.verify(symbolLayer, Mockito.times(1)).setProperties(
            ArgumentMatchers.argThat(PropertyValueMatcher(textHaloBlur(get("text-halo-blur"))))
        )
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
        val listener = Mockito.mock(
            OnSymbolClickListener::class.java
        )
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
        val listener = Mockito.mock(
            OnSymbolLongClickListener::class.java
        )
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
        val listener = Mockito.mock(
            OnSymbolDragListener::class.java
        )
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
        val options = SymbolOptions().withLatLng(LatLng())
        options.withData(JsonPrimitive("hello"))
        val symbol = symbolManager.create(options)
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
        val options = SymbolOptions().withLatLng(LatLng())
        symbolManager.create(options)
        Assert.assertEquals(1, symbolManager.annotations.size())
        symbolManager.deleteAll()
        Assert.assertEquals(0, symbolManager.annotations.size())
    }

    @Test
    fun testIgnoreClearedAnnotations() {
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
        val options = SymbolOptions().withLatLng(LatLng())
        val symbol = symbolManager.create(options)
        Assert.assertEquals(1, symbolManager.annotations.size())
        symbolManager.annotations.clear()
        symbolManager.updateSource()
        Assert.assertTrue(symbolManager.annotations.isEmpty)
        symbolManager.update(symbol)
        Assert.assertTrue(symbolManager.annotations.isEmpty)
    }
}
