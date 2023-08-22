package org.maplibre.android.annotations

import com.google.gson.JsonPrimitive
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.Geometry
import com.mapbox.geojson.LineString
import com.mapbox.geojson.Point
import junit.framework.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.Style.OnStyleLoaded
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.get
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.Property.LINE_JOIN_BEVEL
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyFactory.lineBlur
import org.maplibre.android.style.layers.PropertyFactory.lineColor
import org.maplibre.android.style.layers.PropertyFactory.lineGapWidth
import org.maplibre.android.style.layers.PropertyFactory.lineJoin
import org.maplibre.android.style.layers.PropertyFactory.lineOffset
import org.maplibre.android.style.layers.PropertyFactory.lineOpacity
import org.maplibre.android.style.layers.PropertyFactory.linePattern
import org.maplibre.android.style.layers.PropertyFactory.lineWidth
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers
import org.mockito.Mockito

class LineManagerTest {
    private val draggableAnnotationController = Mockito.mock(
        DraggableAnnotationController::class.java
    )
    private val mapView: MapView = Mockito.mock(MapView::class.java)
    private val maplibreMap: MapLibreMap = Mockito.mock(
        MapLibreMap::class.java
    )
    private val style: Style = Mockito.mock(Style::class.java)
    private val geoJsonSource: GeoJsonSource = Mockito.mock(
        GeoJsonSource::class.java
    )
    private val optionedGeoJsonSource: GeoJsonSource = Mockito.mock(
        GeoJsonSource::class.java
    )
    private val layerId = "annotation_layer"
    private val lineLayer: LineLayer = Mockito.mock(
        LineLayer::class.java
    )
    private lateinit var lineManager: LineManager
    private val coreElementProvider: CoreElementProvider<LineLayer> = Mockito.mock(
        LineElementProvider::class.java
    )
    private val geoJsonOptions: GeoJsonOptions = Mockito.mock(
        GeoJsonOptions::class.java
    )

    @Before
    fun beforeTest() {
        Mockito.`when`(lineLayer.id).thenReturn(layerId)
        Mockito.`when`(coreElementProvider.layer).thenReturn(lineLayer)
        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(coreElementProvider.getSource(geoJsonOptions))
            .thenReturn(optionedGeoJsonSource)
        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
    }

    @Test
    fun testInitialization() {
        lineManager = LineManager(
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
        Mockito.verify(style).addLayer(lineLayer)
        Assert.assertTrue(lineManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in lineManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
        Mockito.verify(lineLayer).setProperties(
            *lineManager.constantPropertyUsageMap.values.toTypedArray()
        )
        Mockito.verify(lineLayer, Mockito.times(0)).setFilter(
            ArgumentMatchers.any(
                Expression::class.java
            )
        )
    }

    @Test
    fun testInitializationOnStyleReload() {
        lineManager = LineManager(
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
        Mockito.verify(style).addLayer(lineLayer)
        Assert.assertTrue(lineManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in lineManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
        Mockito.verify(lineLayer).setProperties(
            *lineManager.constantPropertyUsageMap.values.toTypedArray()
        )
        val filter: Expression = Expression.literal(false)
        lineManager.setFilter(filter)
        val loadingArgumentCaptor: ArgumentCaptor<MapView.OnDidFinishLoadingStyleListener> =
            ArgumentCaptor.forClass(
                MapView.OnDidFinishLoadingStyleListener::class.java
            )
        Mockito.verify(mapView)
            .addOnDidFinishLoadingStyleListener(loadingArgumentCaptor.capture())
        loadingArgumentCaptor.value.onDidFinishLoadingStyle()
        val styleLoadedArgumentCaptor: ArgumentCaptor<OnStyleLoaded> = ArgumentCaptor.forClass(
            OnStyleLoaded::class.java
        )
        Mockito.verify(maplibreMap).getStyle(styleLoadedArgumentCaptor.capture())
        val newStyle: Style = Mockito.mock(Style::class.java)
        Mockito.`when`(newStyle.isFullyLoaded).thenReturn(true)
        val newSource: GeoJsonSource = Mockito.mock(
            GeoJsonSource::class.java
        )
        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(newSource)
        val newLayer: LineLayer = Mockito.mock(LineLayer::class.java)
        Mockito.`when`<Any>(coreElementProvider.layer).thenReturn(newLayer)
        styleLoadedArgumentCaptor.value.onStyleLoaded(newStyle)
        Mockito.verify(newStyle).addSource(newSource)
        Mockito.verify(newStyle).addLayer(newLayer)
        Assert.assertTrue(lineManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in lineManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
        Mockito.verify(newLayer).setProperties(
            *lineManager.constantPropertyUsageMap.values.toTypedArray()
        )
        Mockito.verify(lineLayer).setFilter(filter)
    }

    @Test
    fun testLayerBelowInitialization() {
        lineManager = LineManager(
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
        Mockito.verify(style).addLayerBelow(lineLayer, "test_layer")
        Assert.assertTrue(lineManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in lineManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
        Mockito.verify(lineLayer).setProperties(
            *lineManager.constantPropertyUsageMap.values.toTypedArray()
        )
    }

    @Test
    fun testGeoJsonOptionsInitialization() {
        lineManager = LineManager(
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
        Mockito.verify(style).addLayer(lineLayer)
        Assert.assertTrue(lineManager.dataDrivenPropertyUsageMap.isNotEmpty())
        for (value in lineManager.dataDrivenPropertyUsageMap.values) {
            Assert.assertFalse(value)
        }
        Mockito.verify(lineLayer).setProperties(
            *lineManager.constantPropertyUsageMap.values.toTypedArray()
        )
        Mockito.verify(lineLayer, Mockito.times(0)).setFilter(
            ArgumentMatchers.any(
                Expression::class.java
            )
        )
    }

    @Test
    fun testLayerId() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertEquals(layerId, lineManager.layerId)
    }

    @Test
    fun testAddLine() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(LatLng(), LatLng(1.0, 1.0))
        val line = lineManager.create(LineOptions().withLatLngs(latLngs))
        Assert.assertEquals(lineManager.annotations[0], line)
    }

    @Test
    fun addLineFromFeatureCollection() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val points = listOf(
            Point.fromLngLat(0.0, 0.0),
            Point.fromLngLat(1.0, 1.0)
        )
        val geometry: Geometry = LineString.fromLngLats(points)
        val feature = Feature.fromGeometry(geometry)
        feature.addStringProperty("line-join", LINE_JOIN_BEVEL)
        feature.addNumberProperty("line-opacity", 2.0f)
        feature.addStringProperty("line-color", "rgba(0, 0, 0, 1)")
        feature.addNumberProperty("line-width", 2.0f)
        feature.addNumberProperty("line-gap-width", 2.0f)
        feature.addNumberProperty("line-offset", 2.0f)
        feature.addNumberProperty("line-blur", 2.0f)
        feature.addStringProperty("line-pattern", "pedestrian-polygon")
        feature.addBooleanProperty("is-draggable", true)
        val lines: List<Line> = lineManager.create(FeatureCollection.fromFeature(feature))
        val line = lines[0]
        Assert.assertEquals(line.geometry, geometry)
        Assert.assertEquals(line.lineJoin, LINE_JOIN_BEVEL)
        Assert.assertEquals(line.lineOpacity, 2.0f)
        Assert.assertEquals(line.lineColor, "rgba(0, 0, 0, 1)")
        Assert.assertEquals(line.lineWidth, 2.0f)
        Assert.assertEquals(line.lineGapWidth, 2.0f)
        Assert.assertEquals(line.lineOffset, 2.0f)
        Assert.assertEquals(line.lineBlur, 2.0f)
        Assert.assertEquals(line.linePattern, "pedestrian-polygon")
        Assert.assertTrue(line.isDraggable)
    }

    @Test
    fun addLines() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val lines = listOf(
            listOf(
                LatLng(2.0, 2.0),
                LatLng(2.0, 3.0)
            ),
            listOf(
                LatLng(1.0, 1.0),
                LatLng(2.0, 3.0)
            )
        ).map { LineOptions().withLatLngs(it) }
            .let { lineManager.create(it) }
        Assert.assertTrue("Returned value size should match", lines.size == 2)
        Assert.assertTrue("Annotations size should match", lineManager.annotations.size() == 2)
    }

    @Test
    fun testDeleteLine() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val line = lineManager.create(LineOptions().withLatLngs(latLngs))
        lineManager.delete(line)
        Assert.assertTrue(lineManager.annotations.size() == 0)
    }

    @Test
    fun testGeometryLine() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs)
        val line = lineManager.create(options)
        Assert.assertEquals(options.latLngs, latLngs)
        Assert.assertEquals(line.latLngs, latLngs)
        Assert.assertEquals(
            options.geometry as Any,
            LineString.fromLngLats(
                listOf(
                    Point.fromLngLat(0.0, 0.0),
                    Point.fromLngLat(1.0, 1.0)
                )
            )
        )
        Assert.assertEquals(
            line.getGeometry(),
            LineString.fromLngLats(
                listOf(
                    Point.fromLngLat(0.0, 0.0),
                    Point.fromLngLat(1.0, 1.0)
                )
            )
        )
    }

    @Test
    fun testFeatureIdLine() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val lineZero = lineManager.create(LineOptions().withLatLngs(latLngs))
        val lineOne = lineManager.create(LineOptions().withLatLngs(latLngs))
        Assert.assertEquals(lineZero.feature[Line.ID_KEY].asLong, 0)
        Assert.assertEquals(lineOne.feature[Line.ID_KEY].asLong, 1)
    }

    @Test
    fun testLineDraggableFlag() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val lineZero = lineManager.create(LineOptions().withLatLngs(latLngs))
        Assert.assertFalse(lineZero.isDraggable)
        lineZero.isDraggable = true
        Assert.assertTrue(lineZero.isDraggable)
        lineZero.isDraggable = false
        Assert.assertFalse(lineZero.isDraggable)
    }

    @Test
    fun testLineJoinLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineJoin(get("line-join"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineJoin(LINE_JOIN_BEVEL)
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineJoin(get("line-join"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineJoin(get("line-join"))))
            )
    }

    @Test
    fun testLineOpacityLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineOpacity(get("line-opacity"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineOpacity(2.0f)
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineOpacity(get("line-opacity"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineOpacity(get("line-opacity"))))
            )
    }

    @Test
    fun testLineColorLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineColor(get("line-color"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineColor("rgba(0, 0, 0, 1)")
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineColor(get("line-color"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineColor(get("line-color"))))
            )
    }

    @Test
    fun testLineWidthLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineWidth(get("line-width"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineWidth(2.0f)
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineWidth(get("line-width"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineWidth(get("line-width"))))
            )
    }

    @Test
    fun testLineGapWidthLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineGapWidth(get("line-gap-width"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineGapWidth(2.0f)
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineGapWidth(get("line-gap-width"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineGapWidth(get("line-gap-width"))))
            )
    }

    @Test
    fun testLineOffsetLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineOffset(get("line-offset"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineOffset(2.0f)
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineOffset(get("line-offset"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineOffset(get("line-offset"))))
            )
    }

    @Test
    fun testLineBlurLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(PropertyFactory.lineBlur(get("line-blur")))
                )
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLineBlur(2.0f)
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineBlur(get("line-blur"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(lineBlur(get("line-blur"))))
            )
    }

    @Test
    fun testLinePatternLayerProperty() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(lineLayer, Mockito.times(0))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(linePattern(get("line-pattern"))))
            )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs).withLinePattern("pedestrian-polygon")
        lineManager.create(options)
        lineManager.updateSourceNow()
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(linePattern(get("line-pattern"))))
            )
        lineManager.create(options)
        Mockito.verify(lineLayer, Mockito.times(1))
            .setProperties(
                ArgumentMatchers.argThat(PropertyValueMatcher(linePattern(get("line-pattern"))))
            )
    }

    @Test
    fun testLineLayerFilter() {
        lineManager = LineManager(
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
        Mockito.verify(lineLayer, Mockito.times(0)).setFilter(expression)
        lineManager.setFilter(expression)
        Mockito.verify(lineLayer, Mockito.times(1)).setFilter(expression)
        Mockito.`when`(lineLayer.filter).thenReturn(expression)
        Assert.assertEquals(expression, lineManager.filter)
        Assert.assertEquals(expression, lineManager.layerFilter)
    }

    @Test
    fun testClickListener() {
        val listener = Mockito.mock(
            OnLineClickListener::class.java
        )
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(lineManager.getClickListeners().isEmpty())
        lineManager.addClickListener(listener)
        Assert.assertTrue(lineManager.getClickListeners().contains(listener))
        lineManager.removeClickListener(listener)
        Assert.assertTrue(lineManager.getClickListeners().isEmpty())
    }

    @Test
    fun testLongClickListener() {
        val listener = Mockito.mock(
            OnLineLongClickListener::class.java
        )
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(lineManager.getLongClickListeners().isEmpty())
        lineManager.addLongClickListener(listener)
        Assert.assertTrue(lineManager.getLongClickListeners().contains(listener))
        lineManager.removeLongClickListener(listener)
        Assert.assertTrue(lineManager.getLongClickListeners().isEmpty())
    }

    @Test
    fun testDragListener() {
        val listener = Mockito.mock(
            OnLineDragListener::class.java
        )
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(lineManager.getDragListeners().isEmpty())
        lineManager.addDragListener(listener)
        Assert.assertTrue(lineManager.getDragListeners().contains(listener))
        lineManager.removeDragListener(listener)
        Assert.assertTrue(lineManager.getDragListeners().isEmpty())
    }

    @Test
    fun testCustomData() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs)
        options.withData(JsonPrimitive("hello"))
        val line = lineManager.create(options)
        Assert.assertEquals(JsonPrimitive("hello"), line.data)
    }

    @Test
    fun testClearAll() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs)
        lineManager.create(options)
        Assert.assertEquals(1, lineManager.annotations.size())
        lineManager.deleteAll()
        Assert.assertEquals(0, lineManager.annotations.size())
    }

    @Test
    fun testIgnoreClearedAnnotations() {
        lineManager = LineManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val latLngs = listOf(
            LatLng(),
            LatLng(1.0, 1.0)
        )
        val options = LineOptions().withLatLngs(latLngs)
        val line = lineManager.create(options)
        Assert.assertEquals(1, lineManager.annotations.size())
        lineManager.annotations.clear()
        lineManager.updateSource()
        Assert.assertTrue(lineManager.annotations.isEmpty)
        lineManager.update(line)
        Assert.assertTrue(lineManager.annotations.isEmpty)
    }
}
