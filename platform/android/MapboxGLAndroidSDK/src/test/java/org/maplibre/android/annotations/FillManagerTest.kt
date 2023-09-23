//package org.maplibre.android.annotations
//
//import com.google.gson.JsonPrimitive
//import com.mapbox.geojson.Feature
//import com.mapbox.geojson.FeatureCollection
//import com.mapbox.geojson.Geometry
//import com.mapbox.geojson.Point
//import com.mapbox.geojson.Polygon
//import junit.framework.Assert
//import org.junit.Before
//import org.junit.Test
//import org.maplibre.android.geometry.LatLng
//import org.maplibre.android.maps.MapLibreMap
//import org.maplibre.android.maps.MapView
//import org.maplibre.android.maps.Style
//import org.maplibre.android.style.expressions.Expression
//import org.maplibre.android.style.expressions.Expression.get
//import org.maplibre.android.style.layers.FillLayer
//import org.maplibre.android.style.layers.PropertyFactory.fillColor
//import org.maplibre.android.style.layers.PropertyFactory.fillOpacity
//import org.maplibre.android.style.layers.PropertyFactory.fillOutlineColor
//import org.maplibre.android.style.layers.PropertyFactory.fillPattern
//import org.maplibre.android.style.sources.GeoJsonOptions
//import org.maplibre.android.style.sources.GeoJsonSource
//import org.mockito.ArgumentCaptor
//import org.mockito.ArgumentMatchers
//import org.mockito.Mockito
//
//class FillManagerTest {
//    private val draggableAnnotationController = Mockito.mock(
//        DraggableAnnotationController::class.java
//    )
//    private val mapView: MapView = Mockito.mock(MapView::class.java)
//    private val maplibreMap: MapLibreMap = Mockito.mock(MapLibreMap::class.java)
//    private val style: Style = Mockito.mock(Style::class.java)
//    private val geoJsonSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
//    private val optionedGeoJsonSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
//    private val layerId = "annotation_layer"
//    private val fillLayer: FillLayer = Mockito.mock(FillLayer::class.java)
//    private lateinit var fillManager: FillManager
//    private val coreElementProvider: CoreElementProvider<FillLayer> = Mockito.mock(
//        FillElementProvider::class.java
//    )
//    private val geoJsonOptions: GeoJsonOptions = Mockito.mock(GeoJsonOptions::class.java)
//
//    @Before
//    fun beforeTest() {
//        Mockito.`when`(fillLayer.id).thenReturn(layerId)
//        Mockito.`when`<Any>(coreElementProvider.layer).thenReturn(fillLayer)
//        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(geoJsonSource)
//        Mockito.`when`(coreElementProvider.getSource(geoJsonOptions))
//            .thenReturn(optionedGeoJsonSource)
//        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
//    }
//
//    @Test
//    fun testInitialization() {
//        fillManager = FillManager(
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
//        Mockito.verify(style).addLayer(fillLayer)
//        Assert.assertTrue(fillManager!!.dataDrivenPropertyUsageMap.size > 0)
//        for (value in fillManager!!.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(fillLayer).setProperties(
//            *fillManager!!.constantPropertyUsageMap.values.toTypedArray()
//        )
//        Mockito.verify(fillLayer, Mockito.times(0)).setFilter(
//            ArgumentMatchers.any(
//                Expression::class.java
//            )
//        )
//    }
//
//    @Test
//    fun testInitializationOnStyleReload() {
//        fillManager = FillManager(
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
//        Mockito.verify(style).addLayer(fillLayer)
//        Assert.assertTrue(fillManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in fillManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(fillLayer).setProperties(
//            *fillManager.constantPropertyUsageMap.values.toTypedArray()
//        )
//        val filter: Expression = Expression.literal(false)
//        fillManager.setFilter(filter)
//        val loadingArgumentCaptor: ArgumentCaptor<MapView.OnDidFinishLoadingStyleListener> =
//            ArgumentCaptor.forClass(
//                MapView.OnDidFinishLoadingStyleListener::class.java
//            )
//        Mockito.verify(mapView).addOnDidFinishLoadingStyleListener(loadingArgumentCaptor.capture())
//        loadingArgumentCaptor.value.onDidFinishLoadingStyle()
//        val styleLoadedArgumentCaptor: ArgumentCaptor<Style.OnStyleLoaded> =
//            ArgumentCaptor.forClass(
//                Style.OnStyleLoaded::class.java
//            )
//        Mockito.verify(maplibreMap).getStyle(styleLoadedArgumentCaptor.capture())
//        val newStyle: Style = Mockito.mock(Style::class.java)
//        Mockito.`when`(newStyle.isFullyLoaded).thenReturn(true)
//
//        val newSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
//        Mockito.`when`(coreElementProvider.getSource(null)).thenReturn(newSource)
//
//        val newLayer: FillLayer = Mockito.mock(FillLayer::class.java)
//        Mockito.`when`<Any>(coreElementProvider.layer).thenReturn(newLayer)
//
//        styleLoadedArgumentCaptor.value.onStyleLoaded(newStyle)
//        Mockito.verify(newStyle).addSource(newSource)
//        Mockito.verify(newStyle).addLayer(newLayer)
//        Assert.assertTrue(fillManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in fillManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(newLayer).setProperties(
//            *fillManager.constantPropertyUsageMap.values.toTypedArray()
//        )
//        Mockito.verify(fillLayer).setFilter(filter)
//    }
//
//    @Test
//    fun testLayerBelowInitialization() {
//        fillManager = FillManager(
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
//        Mockito.verify(style).addLayerBelow(fillLayer, "test_layer")
//        Assert.assertTrue(fillManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in fillManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(fillLayer).setProperties(
//            *fillManager.constantPropertyUsageMap.values.toTypedArray()
//        )
//    }
//
//    @Test
//    fun testGeoJsonOptionsInitialization() {
//        fillManager = FillManager(
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
//        Mockito.verify(style).addLayer(fillLayer)
//        Assert.assertTrue(fillManager.dataDrivenPropertyUsageMap.isNotEmpty())
//        for (value in fillManager.dataDrivenPropertyUsageMap.values) {
//            Assert.assertFalse(value)
//        }
//        Mockito.verify(fillLayer).setProperties(
//            *fillManager.constantPropertyUsageMap.values.toTypedArray()
//        )
//        Mockito.verify(fillLayer, Mockito.times(0)).setFilter(
//            ArgumentMatchers.any(
//                Expression::class.java
//            )
//        )
//    }
//
//    @Test
//    fun testLayerId() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertEquals(layerId, fillManager.layerId)
//    }
//
//    @Test
//    fun testAddFill() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0),
//            LatLng()
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val fill = fillManager.create(FillOptions().withLatLngs(latLngs))
//        Assert.assertEquals(fillManager.annotations[0], fill)
//    }
//
//    @Test
//    fun addFillFromFeatureCollection() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//
//        val innerPoints: List<Point> = listOf(
//            Point.fromLngLat(0.0, 0.0),
//            Point.fromLngLat(1.0, 1.0),
//            Point.fromLngLat(-1.0, -1.0),
//            Point.fromLngLat(0.0, 0.0)
//        )
//        val points: List<List<Point>> = listOf(innerPoints)
//
//        val geometry: Geometry = Polygon.fromLngLats(points)
//        val feature = Feature.fromGeometry(geometry)
//        feature.addNumberProperty("fill-opacity", 2.0f)
//        feature.addStringProperty("fill-color", "rgba(0, 0, 0, 1)")
//        feature.addStringProperty("fill-outline-color", "rgba(0, 0, 0, 1)")
//        feature.addStringProperty("fill-pattern", "pedestrian-polygon")
//        feature.addBooleanProperty("is-draggable", true)
//
//        val fills: List<Fill> = fillManager.create(FeatureCollection.fromFeature(feature))
//        val fill = fills[0]
//
//        Assert.assertEquals(fill.geometry, geometry)
//        Assert.assertEquals(fill.fillOpacity, 2.0f)
//        Assert.assertEquals(fill.fillColor, "rgba(0, 0, 0, 1)")
//        Assert.assertEquals(fill.fillOutlineColor, "rgba(0, 0, 0, 1)")
//        Assert.assertEquals(fill.fillPattern, "pedestrian-polygon")
//        Assert.assertTrue(fill.isDraggable)
//    }
//
//    @Test
//    fun addFills() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val latLngListOne: List<List<LatLng>> = listOf(
//            listOf(
//                LatLng(2.0, 2.0),
//                LatLng(2.0, 3.0)
//            ),
//            listOf(
//                LatLng(1.0, 1.0),
//                LatLng(2.0, 3.0)
//            )
//        )
//        val latLngListTwo: List<List<LatLng>> = listOf(
//            listOf(
//                LatLng(5.0, 7.0),
//                LatLng(2.0, 3.0)
//            ),
//            listOf(
//                LatLng(1.0, 1.0),
//                LatLng(3.0, 9.0)
//            )
//        )
//
//        val latLngList: List<List<List<LatLng>>> = listOf(
//            latLngListOne,
//            latLngListTwo
//        )
//
//        val options: MutableList<FillOptions> = ArrayList()
//        for (lists in latLngList) {
//            options.add(FillOptions().withLatLngs(lists))
//        }
//        val fills = fillManager.create(options)
//        Assert.assertTrue("Returned value size should match", fills.size == 2)
//        Assert.assertTrue("Annotations size should match", fillManager.annotations.size() == 2)
//    }
//
//    @Test
//    fun testDeleteFill() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: MutableList<LatLng> = ArrayList()
//        innerLatLngs.add(LatLng())
//        innerLatLngs.add(LatLng(1.0, 1.0))
//        innerLatLngs.add(LatLng(-1.0, -1.0))
//        val latLngs: MutableList<List<LatLng>> = ArrayList()
//        latLngs.add(innerLatLngs)
//        val fill = fillManager.create(FillOptions().withLatLngs(latLngs))
//        fillManager.delete(fill)
//        Assert.assertTrue(fillManager.annotations.size() == 0)
//    }
//
//    @Test
//    fun testGeometryFill() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs)
//        val fill = fillManager.create(options)
//        Assert.assertEquals(options.latLngs, latLngs)
//        Assert.assertEquals(fill.latLngs, latLngs)
//        Assert.assertEquals(
//            options.geometry,
//            Polygon.fromLngLats(
//                listOf(
//                    listOf(
//                        Point.fromLngLat(0.0, 0.0),
//                        Point.fromLngLat(1.0, 1.0),
//                        Point.fromLngLat(-1.0, -1.0)
//                    )
//                )
//            )
//        )
//        Assert.assertEquals(
//            fill.getGeometry(),
//            Polygon.fromLngLats(
//                listOf(
//                    listOf(
//                        Point.fromLngLat(0.0, 0.0),
//                        Point.fromLngLat(1.0, 1.0),
//                        Point.fromLngLat(-1.0, -1.0)
//                    )
//                )
//            )
//        )
//    }
//
//    @Test
//    fun testFeatureIdFill() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val fillZero = fillManager.create(FillOptions().withLatLngs(latLngs))
//        val fillOne = fillManager.create(FillOptions().withLatLngs(latLngs))
//        Assert.assertEquals(fillZero.feature[Fill.ID_KEY].asLong, 0)
//        Assert.assertEquals(fillOne.feature[Fill.ID_KEY].asLong, 1)
//    }
//
//    @Test
//    fun testFillDraggableFlag() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val fillZero = fillManager.create(FillOptions().withLatLngs(latLngs))
//        Assert.assertFalse(fillZero.isDraggable)
//        fillZero.isDraggable = true
//        Assert.assertTrue(fillZero.isDraggable)
//        fillZero.isDraggable = false
//        Assert.assertFalse(fillZero.isDraggable)
//    }
//
//    @Test
//    fun testFillOpacityLayerProperty() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(fillLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillOpacity(get("fill-opacity"))))
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs).withFillOpacity(2.0f)
//        fillManager.create(options)
//        fillManager.updateSourceNow()
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillOpacity(get("fill-opacity"))))
//        )
//        fillManager.create(options)
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillOpacity(get("fill-opacity"))))
//        )
//    }
//
//    @Test
//    fun testFillColorLayerProperty() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(fillLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillColor(get("fill-color"))))
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs).withFillColor("rgba(0, 0, 0, 1)")
//        fillManager.create(options)
//        fillManager.updateSourceNow()
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillColor(get("fill-color"))))
//        )
//        fillManager.create(options)
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillColor(get("fill-color"))))
//        )
//    }
//
//    @Test
//    fun testFillOutlineColorLayerProperty() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(fillLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(fillOutlineColor(get("fill-outline-color")))
//            )
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs).withFillOutlineColor("rgba(0, 0, 0, 1)")
//        fillManager.create(options)
//        fillManager.updateSourceNow()
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(fillOutlineColor(get("fill-outline-color")))
//            )
//        )
//        fillManager.create(options)
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(
//                PropertyValueMatcher(fillOutlineColor(get("fill-outline-color")))
//            )
//        )
//    }
//
//    @Test
//    fun testFillPatternLayerProperty() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Mockito.verify(fillLayer, Mockito.times(0)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillPattern(get("fill-pattern"))))
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs).withFillPattern("pedestrian-polygon")
//        fillManager.create(options)
//        fillManager.updateSourceNow()
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillPattern(get("fill-pattern"))))
//        )
//        fillManager.create(options)
//        Mockito.verify(fillLayer, Mockito.times(1)).setProperties(
//            ArgumentMatchers.argThat(PropertyValueMatcher(fillPattern(get("fill-pattern"))))
//        )
//    }
//
//    @Test
//    fun testFillLayerFilter() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val expression: Expression = Expression.eq(Expression.get("test"), "selected")
//        Mockito.verify(fillLayer, Mockito.times(0)).setFilter(expression)
//        fillManager.setFilter(expression)
//        Mockito.verify(fillLayer, Mockito.times(1)).setFilter(expression)
//        Mockito.`when`(fillLayer.filter).thenReturn(expression)
//        Assert.assertEquals(expression, fillManager.filter)
//        Assert.assertEquals(expression, fillManager.layerFilter)
//    }
//
//    @Test
//    fun testClickListener() {
//        val listener = Mockito.mock(
//            OnFillClickListener::class.java
//        )
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertTrue(fillManager.getClickListeners().isEmpty())
//        fillManager.addClickListener(listener)
//        Assert.assertTrue(fillManager.getClickListeners().contains(listener))
//        fillManager.removeClickListener(listener)
//        Assert.assertTrue(fillManager.getClickListeners().isEmpty())
//    }
//
//    @Test
//    fun testLongClickListener() {
//        val listener = Mockito.mock(
//            OnFillLongClickListener::class.java
//        )
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertTrue(fillManager.getLongClickListeners().isEmpty())
//        fillManager.addLongClickListener(listener)
//        Assert.assertTrue(fillManager.getLongClickListeners().contains(listener))
//        fillManager.removeLongClickListener(listener)
//        Assert.assertTrue(fillManager.getLongClickListeners().isEmpty())
//    }
//
//    @Test
//    fun testDragListener() {
//        val listener = Mockito.mock(
//            OnFillDragListener::class.java
//        )
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        Assert.assertTrue(fillManager.getDragListeners().isEmpty())
//        fillManager.addDragListener(listener)
//        Assert.assertTrue(fillManager.getDragListeners().contains(listener))
//        fillManager.removeDragListener(listener)
//        Assert.assertTrue(fillManager.getDragListeners().isEmpty())
//    }
//
//    @Test
//    fun testCustomData() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs)
//        options.withData(JsonPrimitive("hello"))
//        val fill = fillManager.create(options)
//        Assert.assertEquals(JsonPrimitive("hello"), fill.data)
//    }
//
//    @Test
//    fun testClearAll() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs)
//        fillManager.create(options)
//        Assert.assertEquals(1, fillManager.annotations.size())
//        fillManager.deleteAll()
//        Assert.assertEquals(0, fillManager.annotations.size())
//    }
//
//    @Test
//    fun testIgnoreClearedAnnotations() {
//        fillManager = FillManager(
//            mapView,
//            maplibreMap,
//            style,
//            coreElementProvider,
//            null,
//            null,
//            null,
//            draggableAnnotationController
//        )
//        val innerLatLngs: List<LatLng> = listOf(
//            LatLng(),
//            LatLng(1.0, 1.0),
//            LatLng(-1.0, -1.0)
//        )
//        val latLngs: List<List<LatLng>> = listOf(innerLatLngs)
//        val options = FillOptions().withLatLngs(latLngs)
//        val fill = fillManager.create(options)
//        Assert.assertEquals(1, fillManager.annotations.size())
//        fillManager.annotations.clear()
//        fillManager.updateSource()
//        Assert.assertTrue(fillManager.annotations.isEmpty)
//        fillManager.update(fill)
//        Assert.assertTrue(fillManager.annotations.isEmpty)
//    }
//}
