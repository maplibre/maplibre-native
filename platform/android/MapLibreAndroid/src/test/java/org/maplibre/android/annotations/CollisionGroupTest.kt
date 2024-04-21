package org.maplibre.android.annotations

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.maplibre.android.annotations.data.Alignment
import org.maplibre.android.annotations.data.Text
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.mockito.InjectMocks
import org.mockito.Mockito

class CollisionGroupTest {
    @InjectMocks
    private val maplibreMap = Mockito.mock(MapLibreMap::class.java)
    @InjectMocks
    private val mapView = Mockito.mock(MapView::class.java)
    @InjectMocks
    private val style = Mockito.mock(Style::class.java)
    private val symbolElementProvider: CoreElementProvider<SymbolLayer> = Mockito.mock(
        SymbolElementProvider::class.java
    )
    private val symbolLayer: SymbolLayer = Mockito.mock(SymbolLayer::class.java)

    private val layerId = "annotation_layer"
    private val geoJsonSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)
    private val geoJsonOptions: GeoJsonOptions = Mockito.mock(GeoJsonOptions::class.java)

    private val optionedGeoJsonSource: GeoJsonSource = Mockito.mock(GeoJsonSource::class.java)

    @Before
    fun beforeTest() {
        Mockito.`when`(symbolLayer.id).thenReturn(layerId)
        Mockito.`when`(symbolElementProvider.layer).thenReturn(symbolLayer)
        Mockito.`when`(symbolElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(symbolElementProvider.getSource(geoJsonOptions))
            .thenReturn(optionedGeoJsonSource)

        Mockito.`when`(style.isFullyLoaded).thenReturn(true)
    }

    @InjectMocks
    private val draggableAnnotationController = Mockito.mock(
        DraggableAnnotationController::class.java
    )

    private fun getMockContainer() = KAnnotationContainer(
        maplibreMap,
        mapView,
        style,
        draggableAnnotationController,
        { symbolElementProvider }
    )

    @Test
    fun `add CollisionGroup`() {
        val container = getMockContainer()
        val collisionGroup = CollisionGroup()
        container.add(collisionGroup)

        assertEquals(0, container.size)
        assertEquals(1, container.managerCount)
    }

    @Test
    fun `add and remove CollisionGroup`() {
        val container = getMockContainer()
        val collisionGroup = CollisionGroup()

        container.add(collisionGroup)
        container.remove(collisionGroup)

        assertEquals(0, container.size)
        assertEquals(0, container.managerCount)
    }

    @Test
    fun `add CollisionGroup with Symbols`() {
        val container = getMockContainer()
        val collisionGroup = CollisionGroup(
            symbols = listOf(
                Symbol(LatLng(0.0, 0.0), text = Text("custom", lineHeight = 2.0f))
            )
        )

        container.add(collisionGroup)

        assertEquals(1, container.size)
        assertEquals(1, container.managerCount)

        assertTrue(collisionGroup.manager is SymbolManager)
        Mockito.verify(symbolLayer).setProperties(PropertyFactory.textLineHeight(2f))
    }

    @Test
    fun `add CollisionGroup, then add Symbols`() {
        val container = getMockContainer()
        val collisionGroup = CollisionGroup()

        container.add(collisionGroup)

        collisionGroup.symbols = listOf(
            Symbol(LatLng(0.0, 0.0))
        )

        assertEquals(1, collisionGroup.symbols.size)
        assertEquals(1, container.size)
        assertEquals(1, container.managerCount)

        assertTrue(collisionGroup.manager is SymbolManager)
        assertEquals(1, (collisionGroup.manager as SymbolManager).annotations.size)
    }

    @Test
    fun `add CollisionGroup with properties`() {
        val container = getMockContainer()
        val collisionGroup = CollisionGroup(
            textOptional = true
        )

        container.add(collisionGroup)

        assertEquals(0, container.size)
        assertEquals(1, container.managerCount)

        assertTrue(collisionGroup.manager is SymbolManager)
        Mockito.verify(symbolLayer).setProperties(PropertyFactory.textOptional(true))
    }

    @Test
    fun `add CollisionGroup, then set properties`() {
        val container = getMockContainer()
        val collisionGroup = CollisionGroup()

        container.add(collisionGroup)

        assertTrue(collisionGroup.manager is SymbolManager)
        Mockito.verify(symbolLayer, Mockito.never()).setProperties(PropertyFactory.textOptional(true))

        assertEquals(0, container.size)
        assertEquals(1, container.managerCount)

        collisionGroup.textOptional = true

        Mockito.verify(symbolLayer).setProperties(PropertyFactory.textOptional(true))
    }

    @Test(expected = IllegalArgumentException::class)
    fun `throws on adding values with inconsistent NDD properties`() {
        CollisionGroup(
            listOf(
                Symbol(LatLng(0.0, 0.0), text = Text("hello", pitchAlignment = Alignment.MAP)),
                Symbol(LatLng(0.0, 0.0), text = Text("world", pitchAlignment = Alignment.VIEWPORT))
            )
        )
    }

    @Test(expected = IllegalArgumentException::class)
    fun `throws on adding empty text-variable-anchor array`() {
        CollisionGroup(
            emptyList(),
            textVariableAnchor = emptyArray()
        )
    }

}