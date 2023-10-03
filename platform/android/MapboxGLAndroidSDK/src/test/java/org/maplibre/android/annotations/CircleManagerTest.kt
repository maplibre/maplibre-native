package org.maplibre.android.annotations

import android.graphics.Color
import com.google.gson.JsonPrimitive
import com.mapbox.geojson.Point
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapView.OnDidFinishLoadingStyleListener
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.Style.OnStyleLoaded
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers
import org.mockito.Mockito

class CircleManagerTest {
    private val draggableAnnotationController = Mockito.mock(
        DraggableAnnotationController::class.java
    )
    private val mapView = Mockito.mock(MapView::class.java)
    private val maplibreMap = Mockito.mock(MapLibreMap::class.java)
    private val style = Mockito.mock(Style::class.java)
    private val geoJsonSource = Mockito.mock(GeoJsonSource::class.java)
    private val optionedGeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
    private val layerId = "annotation_layer"
    private val circleLayer = Mockito.mock(CircleLayer::class.java)
    private lateinit var circleManager: CircleManager
    private val coreElementProvider: CoreElementProvider<CircleLayer> = Mockito.mock(
        CircleElementProvider::class.java
    )
    private val geoJsonOptions = Mockito.mock(GeoJsonOptions::class.java)

    @Before
    fun beforeTest() {
        Mockito.`when`(circleLayer.id).thenReturn(layerId)
        Mockito.`when`(coreElementProvider.layer).thenReturn(circleLayer)
        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(coreElementProvider.getSource(geoJsonOptions))
            .thenReturn(optionedGeoJsonSource)
        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
    }

    @Test
    fun testInitialization() {
        circleManager = CircleManager(
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
        Mockito.verify(style).addLayer(circleLayer)
        Assert.assertTrue(circleManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(circleLayer)
            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
        Mockito.verify(circleLayer, Mockito.times(0)).setFilter(
            ArgumentMatchers.any(
                Expression::class.java
            )
        )
    }

    @Test
    fun testInitializationOnStyleReload() {
        circleManager = CircleManager(
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
        Mockito.verify(style).addLayer(circleLayer)
        Assert.assertTrue(circleManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(circleLayer)
            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
        val filter = Expression.literal(false)
        circleManager.setFilter(filter)
        val loadingArgumentCaptor = ArgumentCaptor.forClass(
            OnDidFinishLoadingStyleListener::class.java
        )
        Mockito.verify(mapView).addOnDidFinishLoadingStyleListener(loadingArgumentCaptor.capture())
        loadingArgumentCaptor.value.onDidFinishLoadingStyle()
        val styleLoadedArgumentCaptor = ArgumentCaptor.forClass(
            OnStyleLoaded::class.java
        )
        Mockito.verify(maplibreMap).getStyle(styleLoadedArgumentCaptor.capture())
        val newStyle = Mockito.mock(
            Style::class.java
        )
        Mockito.`when`(newStyle.isFullyLoaded).thenReturn(true)
        val newSource = Mockito.mock(GeoJsonSource::class.java)
        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(newSource)
        val newLayer = Mockito.mock(CircleLayer::class.java)
        Mockito.`when`(coreElementProvider.layer).thenReturn(newLayer)
        styleLoadedArgumentCaptor.value.onStyleLoaded(newStyle)
        Mockito.verify(newStyle).addSource(newSource)
        Mockito.verify(newStyle).addLayer(newLayer)
        Assert.assertTrue(circleManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(newLayer)
            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
        Mockito.verify(circleLayer).setFilter(filter)
    }

    @Test
    fun testLayerBelowInitialization() {
        circleManager = CircleManager(
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
        Mockito.verify(style).addLayerBelow(circleLayer, "test_layer")
        Assert.assertTrue(circleManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(circleLayer)
            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
    }

    @Test
    fun testGeoJsonOptionsInitialization() {
        circleManager = CircleManager(
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
        Mockito.verify(style).addLayer(circleLayer)
        Assert.assertTrue(circleManager.usedDataDrivenProperties.isEmpty())
        Mockito.verify(circleLayer)
            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
        Mockito.verify(circleLayer, Mockito.times(0)).setFilter(
            ArgumentMatchers.any(
                Expression::class.java
            )
        )
    }

    @Test
    fun testLayerId() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertEquals(layerId, circleManager.layerId)
    }

    @Test
    fun testAddCircle() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val circle = Circle(LatLng())
        circleManager.add(circle)
        Assert.assertSame(circle, circleManager.annotations.values.first())
    }

    @Test
    fun addCircles() {
        circleManager = CircleManager(
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
        ).map { Circle(it) }.forEach { circleManager.add(it) }
        Assert.assertTrue("Amount of annotations should match", circleManager.annotations.size == 2)
    }

    @Test
    fun testDeleteCircle() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val circle = Circle(LatLng())
        circleManager.add(circle)
        Assert.assertEquals(
            "After adding a circle, one circle should be present",
            1,
            circleManager.annotations.size
        )
        circleManager.delete(circle)
        Assert.assertEquals(
            "After removing the only circle, no circles should be present anymore",
            0,
            circleManager.annotations.size
        )
    }

    @Test
    fun testGeometryCircle() {
        circleManager = CircleManager(
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
        val circle = Circle(latLng)
        circleManager.add(circle)
        Assert.assertEquals(circle.center, latLng)
        Assert.assertEquals(circle.geometry, Point.fromLngLat(34.0, 12.0))
    }

    @Test
    fun testCircleDraggableFlag() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val circle = Circle(LatLng())
        circleManager.add(circle)
        Assert.assertFalse(circle.draggable)
        circle.draggable = true
        Assert.assertTrue(circle.draggable)
        circle.draggable = false
        Assert.assertFalse(circle.draggable)
    }

    @Test
    fun testCircleRadiusLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleRadius(Expression.get("circle-radius"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng()).apply {
                radius = 2f
            }
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleRadius(Expression.get("circle-radius"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleColorLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleColor(Expression.get("circle-color"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng()).apply {
                color = Color.RED
            }
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleColor(Expression.get("circle-color"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleBlurLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleBlur(Expression.get("circle-blur"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng()).apply {
                blur = 2f
            }
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleBlur(Expression.get("circle-blur"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleOpacityLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleOpacity(Expression.get("circle-opacity"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng()).apply {
                opacity = 0.5f
            }
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleOpacity(Expression.get("circle-opacity"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleStrokeWidthLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleStrokeWidth(Expression.get("circle-stroke-width"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng(), stroke = Circle.Stroke(width = 2f))
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleStrokeWidth(Expression.get("circle-stroke-width"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleStrokeColorLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleStrokeColor(Expression.get("circle-stroke-color"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng(), stroke = Circle.Stroke(width = 2f, color = Color.CYAN))
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleStrokeColor(Expression.get("circle-stroke-color"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleStrokeOpacityLayerProperty() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
            ArgumentMatchers.argThat(
                PropertyValueMatcher(
                    PropertyFactory.circleStrokeOpacity(Expression.get("circle-stroke-opacity"))
                )
            )
        )
        for (i in 0 until 2) {
            val circle = Circle(LatLng(), stroke = Circle.Stroke(width = 2f, opacity = 0.5f))
            circleManager.add(circle)
            circleManager.updateSourceNow()
            Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
                ArgumentMatchers.argThat(
                    PropertyValueMatcher(
                        PropertyFactory.circleStrokeOpacity(Expression.get("circle-stroke-opacity"))
                    )
                )
            )
        }
    }

    @Test
    fun testCircleLayerFilter() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val expression = Expression.eq(Expression.get("test"), "selected")
        Mockito.verify(circleLayer, Mockito.times(0)).setFilter(expression)
        circleManager.setFilter(expression)
        Mockito.verify(circleLayer, Mockito.times(1)).setFilter(expression)
        Mockito.`when`(circleLayer.filter).thenReturn(expression)
        Assert.assertEquals(expression, circleManager.filter)
        Assert.assertEquals(expression, circleManager.layerFilter)
    }

    @Test
    fun testClickListener() {
        val listener = object : OnCircleClickListener {
            override fun onAnnotationClick(t: Circle): Boolean = false
        }
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(circleManager.getClickListeners().isEmpty())
        circleManager.addClickListener(listener)
        Assert.assertTrue(circleManager.getClickListeners().contains(listener))
        circleManager.removeClickListener(listener)
        Assert.assertTrue(circleManager.getClickListeners().isEmpty())
    }

    @Test
    fun testLongClickListener() {
        val listener = object : OnCircleLongClickListener {
            override fun onAnnotationLongClick(t: Circle): Boolean = false
        }
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(circleManager.getLongClickListeners().isEmpty())
        circleManager.addLongClickListener(listener)
        Assert.assertTrue(circleManager.getLongClickListeners().contains(listener))
        circleManager.removeLongClickListener(listener)
        Assert.assertTrue(circleManager.getLongClickListeners().isEmpty())
    }

    @Test
    fun testDragListener() {
        val listener = object : OnCircleDragListener {
            override fun onAnnotationDragStarted(annotation: Circle) = Unit
            override fun onAnnotationDrag(annotation: Circle) = Unit
            override fun onAnnotationDragFinished(annotation: Circle) = Unit
        }
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        Assert.assertTrue(circleManager.getDragListeners().isEmpty())
        circleManager.addDragListener(listener)
        Assert.assertTrue(circleManager.getDragListeners().contains(listener))
        circleManager.removeDragListener(listener)
        Assert.assertTrue(circleManager.getDragListeners().isEmpty())
    }

    @Test
    fun testCustomData() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val circle = Circle(LatLng()).apply {
            data = JsonPrimitive("hello")
        }
        circleManager.add(circle)
        Assert.assertEquals(JsonPrimitive("hello"), circle.data)
    }

    @Test
    fun testClearAll() {
        circleManager = CircleManager(
            mapView,
            maplibreMap,
            style,
            coreElementProvider,
            null,
            null,
            null,
            draggableAnnotationController
        )
        val circle = Circle(LatLng())
        circleManager.add(circle)
        Assert.assertEquals(1, circleManager.annotations.size)
        circleManager.deleteAll()
        Assert.assertEquals(0, circleManager.annotations.size)
    }
}
