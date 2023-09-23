//package org.maplibre.android.annotations
//
//import com.google.gson.JsonPrimitive
//import com.mapbox.geojson.Feature
//import com.mapbox.geojson.FeatureCollection
//import com.mapbox.geojson.Geometry
//import com.mapbox.geojson.Point
//import junit.framework.Assert
//import org.junit.Before
//import org.junit.Test
//import org.maplibre.android.geometry.LatLng
//import org.maplibre.android.maps.MapLibreMap
//import org.maplibre.android.maps.MapView
//import org.maplibre.android.maps.MapView.OnDidFinishLoadingStyleListener
//import org.maplibre.android.maps.Style
//import org.maplibre.android.maps.Style.OnStyleLoaded
//import org.maplibre.android.style.expressions.Expression
//import org.maplibre.android.style.layers.CircleLayer
//import org.maplibre.android.style.layers.PropertyFactory
//import org.maplibre.android.style.sources.GeoJsonOptions
//import org.maplibre.android.style.sources.GeoJsonSource
//import org.mockito.ArgumentCaptor
//import org.mockito.ArgumentMatchers
//import org.mockito.Mockito
//
//class CircleManagerTest {
//    private val draggableAnnotationController = Mockito.mock(
//        DraggableAnnotationController::class.java
//    )
//    private val mapView = Mockito.mock(MapView::class.java)
//    private val maplibreMap = Mockito.mock(MapLibreMap::class.java)
//    private val style = Mockito.mock(Style::class.java)
//    private val geoJsonSource = Mockito.mock(GeoJsonSource::class.java)
//    private val optionedGeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
//    private val layerId = "annotation_layer"
//    private val circleLayer = Mockito.mock(CircleLayer::class.java)
//    private lateinit var circleManager: CircleManager
//    private val coreElementProvider: CoreElementProvider<CircleLayer> = Mockito.mock(
//        CircleElementProvider::class.java
//    )
//    private val geoJsonOptions = Mockito.mock(GeoJsonOptions::class.java)
//
//    @Before
//    fun beforeTest() {
//        Mockito.`when`(circleLayer.id).thenReturn(layerId)
//        Mockito.`when`(coreElementProvider.layer).thenReturn(circleLayer)
//        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(geoJsonSource)
//        Mockito.`when`(coreElementProvider.getSource(geoJsonOptions))
//            .thenReturn(optionedGeoJsonSource)
//        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
//    }
//
//    @Test
//    fun testInitialization() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(style).addSource(geoJsonSource)
//        Mockito.verify(style).addLayer(circleLayer)
//        Assert.assertTrue(circleManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in circleManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(circleLayer)
//            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
//        Mockito.verify(circleLayer, Mockito.times(0)).setFilter(
//            ArgumentMatchers.any(
//                Expression::class.java
//            )
//        )
//    }
//
//    @Test
//    fun testInitializationOnStyleReload() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(style).addSource(geoJsonSource)
//        Mockito.verify(style).addLayer(circleLayer)
//        Assert.assertTrue(circleManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in circleManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(circleLayer)
//            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
//        val filter = Expression.literal(false)
//        circleManager.setFilter(filter)
//        val loadingArgumentCaptor = ArgumentCaptor.forClass(
//            OnDidFinishLoadingStyleListener::class.java
//        )
//        Mockito.verify(mapView).addOnDidFinishLoadingStyleListener(loadingArgumentCaptor.capture())
//        loadingArgumentCaptor.value.onDidFinishLoadingStyle()
//        val styleLoadedArgumentCaptor = ArgumentCaptor.forClass(
//            OnStyleLoaded::class.java
//        )
//        Mockito.verify(maplibreMap).getStyle(styleLoadedArgumentCaptor.capture())
//        val newStyle = Mockito.mock(
//            Style::class.java
//        )
//        Mockito.`when`(newStyle.isFullyLoaded).thenReturn(true)
//        val newSource = Mockito.mock(GeoJsonSource::class.java)
//        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(newSource)
//        val newLayer = Mockito.mock(CircleLayer::class.java)
//        Mockito.`when`(coreElementProvider.layer).thenReturn(newLayer)
//        styleLoadedArgumentCaptor.value.onStyleLoaded(newStyle)
//        Mockito.verify(newStyle).addSource(newSource)
//        Mockito.verify(newStyle).addLayer(newLayer)
//        Assert.assertTrue(circleManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in circleManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(newLayer)
//            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
//        Mockito.verify(circleLayer).setFilter(filter)
//    }
//
//    @Test
//    fun testLayerBelowInitialization() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            "test_layer",
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(style).addSource(geoJsonSource)
//        Mockito.verify(style).addLayerBelow(circleLayer, "test_layer")
//        Assert.assertTrue(circleManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in circleManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(circleLayer)
//            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
//    }
//
//    @Test
//    fun testGeoJsonOptionsInitialization() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            geoJsonOptions,
//            draggableAnnotationController
//        )
//        Mockito.verify(style).addSource(optionedGeoJsonSource)
//        Mockito.verify(style).addLayer(circleLayer)
//        Assert.assertTrue(circleManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in circleManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(circleLayer)
//            .setProperties(*circleManager.constantPropertyUsageMap.values.toTypedArray())
//        Mockito.verify(circleLayer, Mockito.times(0)).setFilter(
//            ArgumentMatchers.any(
//                Expression::class.java
//            )
//        )
//    }
//
//    @Test
//    fun testLayerId() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertEquals(layerId, circleManager.layerId)
//    }
//
//    @Test
//    fun testAddCircle() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val circle = circleManager.create(CircleOptions().withLatLng(LatLng()))
//        Assert.assertEquals(circleManager.annotations[0], circle)
//    }
//
//    @Test
//    fun addCircleFromFeatureCollection() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val geometry: Geometry = Point.fromLngLat(10.0, 10.0)
//        val feature = Feature.fromGeometry(geometry)
//        feature.addNumberProperty("circle-radius", 2.0f)
//        feature.addStringProperty("circle-color", "rgba(0, 0, 0, 1)")
//        feature.addNumberProperty("circle-blur", 2.0f)
//        feature.addNumberProperty("circle-opacity", 2.0f)
//        feature.addNumberProperty("circle-stroke-width", 2.0f)
//        feature.addStringProperty("circle-stroke-color", "rgba(0, 0, 0, 1)")
//        feature.addNumberProperty("circle-stroke-opacity", 2.0f)
//        feature.addBooleanProperty("is-draggable", true)
//        val circles = circleManager.create(FeatureCollection.fromFeature(feature))
//        val circle = circles[0]
//        Assert.assertEquals(circle.geometry, geometry)
//        Assert.assertEquals(circle.circleRadius, 2.0f)
//        Assert.assertEquals(circle.circleColor, "rgba(0, 0, 0, 1)")
//        Assert.assertEquals(circle.circleBlur, 2.0f)
//        Assert.assertEquals(circle.circleOpacity, 2.0f)
//        Assert.assertEquals(circle.circleStrokeWidth, 2.0f)
//        Assert.assertEquals(circle.circleStrokeColor, "rgba(0, 0, 0, 1)")
//        Assert.assertEquals(circle.circleStrokeOpacity, 2.0f)
//        Assert.assertTrue(circle.isDraggable)
//    }
//
//    @Test
//    fun addCircles() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val latLngList: MutableList<LatLng> = ArrayList()
//        latLngList.add(LatLng())
//        latLngList.add(LatLng(1.0, 1.0))
//        val options: MutableList<CircleOptions> = ArrayList()
//        for (latLng in latLngList) {
//            options.add(CircleOptions().withLatLng(latLng))
//        }
//        val circles = circleManager.create(options)
//        Assert.assertTrue("Returned value size should match", circles.size == 2)
//        Assert.assertTrue("Annotations size should match", circleManager.annotations.size() == 2)
//    }
//
//    @Test
//    fun testDeleteCircle() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val circle = circleManager.create(CircleOptions().withLatLng(LatLng()))
//        circleManager.delete(circle)
//        Assert.assertTrue(circleManager.annotations.size() == 0)
//    }
//
//    @Test
//    fun testGeometryCircle() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val latLng = LatLng(12.0, 34.0)
//        val options = CircleOptions().withLatLng(latLng)
//        val circle = circleManager.create(options)
//        Assert.assertEquals(options.latLng, latLng)
//        Assert.assertEquals(circle.latLng, latLng)
//        Assert.assertEquals(options.geometry, Point.fromLngLat(34.0, 12.0))
//        Assert.assertEquals(circle.getGeometry(), Point.fromLngLat(34.0, 12.0))
//    }
//
//    @Test
//    fun testFeatureIdCircle() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val circleZero = circleManager.create(CircleOptions().withLatLng(LatLng()))
//        val circleOne = circleManager.create(CircleOptions().withLatLng(LatLng()))
//        Assert.assertEquals(circleZero.feature[Circle.ID_KEY].asLong, 0)
//        Assert.assertEquals(circleOne.feature[Circle.ID_KEY].asLong, 1)
//    }
//
//    @Test
//    fun testCircleDraggableFlag() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val circleZero = circleManager.create(CircleOptions().withLatLng(LatLng()))
//        Assert.assertFalse(circleZero.isDraggable)
//        circleZero.isDraggable = true
//        Assert.assertTrue(circleZero.isDraggable)
//        circleZero.isDraggable = false
//        Assert.assertFalse(circleZero.isDraggable)
//    }
//
//    @Test
//    fun testCircleRadiusLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleRadius(Expression.get("circle-radius"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleRadius(2.0f)
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleRadius(Expression.get("circle-radius"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleRadius(Expression.get("circle-radius"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleColorLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleColor(Expression.get("circle-color"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleColor("rgba(0, 0, 0, 1)")
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleColor(Expression.get("circle-color"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleColor(Expression.get("circle-color"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleBlurLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleBlur(Expression.get("circle-blur"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleBlur(2.0f)
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleBlur(Expression.get("circle-blur"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleBlur(Expression.get("circle-blur"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleOpacityLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleOpacity(Expression.get("circle-opacity"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleOpacity(2.0f)
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleOpacity(Expression.get("circle-opacity"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleOpacity(Expression.get("circle-opacity"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleStrokeWidthLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeWidth(Expression.get("circle-stroke-width"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleStrokeWidth(2.0f)
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeWidth(Expression.get("circle-stroke-width"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeWidth(Expression.get("circle-stroke-width"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleStrokeColorLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeColor(Expression.get("circle-stroke-color"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleStrokeColor("rgba(0, 0, 0, 1)")
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeColor(Expression.get("circle-stroke-color"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeColor(Expression.get("circle-stroke-color"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleStrokeOpacityLayerProperty() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(circleLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeOpacity(Expression.get("circle-stroke-opacity"))
//                )
//            )
//        )
//        val options = CircleOptions().withLatLng(LatLng()).withCircleStrokeOpacity(2.0f)
//        circleManager.create(options)
//        circleManager.updateSourceNow()
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeOpacity(Expression.get("circle-stroke-opacity"))
//                )
//            )
//        )
//        circleManager.create(options)
//        Mockito.verify(circleLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(
//                    PropertyFactory.circleStrokeOpacity(Expression.get("circle-stroke-opacity"))
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testCircleLayerFilter() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val expression = Expression.eq(Expression.get("test"), "selected")
//        Mockito.verify(circleLayer, Mockito.times(0)).setFilter(expression)
//        circleManager.setFilter(expression)
//        Mockito.verify(circleLayer, Mockito.times(1)).setFilter(expression)
//        Mockito.`when`(circleLayer.filter).thenReturn(expression)
//        Assert.assertEquals(expression, circleManager.filter)
//        Assert.assertEquals(expression, circleManager.layerFilter)
//    }
//
//    @Test
//    fun testClickListener() {
//        val listener = Mockito.mock(
//            OnCircleClickListener::class.java
//        )
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertTrue(circleManager.getClickListeners().isEmpty())
//        circleManager.addClickListener(listener)
//        Assert.assertTrue(circleManager.getClickListeners().contains(listener))
//        circleManager.removeClickListener(listener)
//        Assert.assertTrue(circleManager.getClickListeners().isEmpty())
//    }
//
//    @Test
//    fun testLongClickListener() {
//        val listener = Mockito.mock(
//            OnCircleLongClickListener::class.java
//        )
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertTrue(circleManager.getLongClickListeners().isEmpty())
//        circleManager.addLongClickListener(listener)
//        Assert.assertTrue(circleManager.getLongClickListeners().contains(listener))
//        circleManager.removeLongClickListener(listener)
//        Assert.assertTrue(circleManager.getLongClickListeners().isEmpty())
//    }
//
//    @Test
//    fun testDragListener() {
//        val listener = Mockito.mock(
//            OnCircleDragListener::class.java
//        )
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertTrue(circleManager.getDragListeners().isEmpty())
//        circleManager.addDragListener(listener)
//        Assert.assertTrue(circleManager.getDragListeners().contains(listener))
//        circleManager.removeDragListener(listener)
//        Assert.assertTrue(circleManager.getDragListeners().isEmpty())
//    }
//
//    @Test
//    fun testCustomData() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val options = CircleOptions().withLatLng(LatLng())
//        options.withData(JsonPrimitive("hello"))
//        val circle = circleManager.create(options)
//        Assert.assertEquals(JsonPrimitive("hello"), circle.data)
//    }
//
//    @Test
//    fun testClearAll() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val options = CircleOptions().withLatLng(LatLng())
//        circleManager.create(options)
//        Assert.assertEquals(1, circleManager.annotations.size())
//        circleManager.deleteAll()
//        Assert.assertEquals(0, circleManager.annotations.size())
//    }
//
//    @Test
//    fun testIgnoreClearedAnnotations() {
//        circleManager = CircleManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val options = CircleOptions().withLatLng(LatLng())
//        val circle = circleManager.create(options)
//        Assert.assertEquals(1, circleManager.annotations.size())
//        circleManager.annotations.clear()
//        circleManager.updateSource()
//        Assert.assertTrue(circleManager.annotations.isEmpty)
//        circleManager.update(circle)
//        Assert.assertTrue(circleManager.annotations.isEmpty)
//    }
//}
