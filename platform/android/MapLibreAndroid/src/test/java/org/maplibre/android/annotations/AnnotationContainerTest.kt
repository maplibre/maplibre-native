package org.maplibre.android.annotations

import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.mockito.InjectMocks
import org.mockito.Mockito

class AnnotationContainerTest {
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

    private val lineElementProvider: CoreElementProvider<LineLayer> = Mockito.mock(
        LineElementProvider::class.java
    )
    private val lineLayer: LineLayer = Mockito.mock(LineLayer::class.java)

    private val circleElementProvider: CoreElementProvider<CircleLayer> = Mockito.mock(
        CircleElementProvider::class.java
    )
    private val circleLayer: CircleLayer = Mockito.mock(CircleLayer::class.java)

    private val fillElementProvider: CoreElementProvider<FillLayer> = Mockito.mock(
        FillElementProvider::class.java
    )
    private val fillLayer: FillLayer = Mockito.mock(FillLayer::class.java)

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

        Mockito.`when`(lineLayer.id).thenReturn(layerId)
        Mockito.`when`(lineElementProvider.layer).thenReturn(lineLayer)
        Mockito.`when`(lineElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(lineElementProvider.getSource(geoJsonOptions))
            .thenReturn(optionedGeoJsonSource)


        Mockito.`when`(circleLayer.id).thenReturn(layerId)
        Mockito.`when`(circleElementProvider.layer).thenReturn(circleLayer)
        Mockito.`when`(circleElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(circleElementProvider.getSource(geoJsonOptions))
            .thenReturn(optionedGeoJsonSource)


        Mockito.`when`(fillLayer.id).thenReturn(layerId)
        Mockito.`when`(fillElementProvider.layer).thenReturn(fillLayer)
        Mockito.`when`(fillElementProvider.getSource(null)).thenReturn(geoJsonSource)
        Mockito.`when`(fillElementProvider.getSource(geoJsonOptions))
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
        { symbolElementProvider },
        { lineElementProvider },
        { circleElementProvider },
        { fillElementProvider }
    )

    @Test
    fun `add one of each type`() {
        val container = getMockContainer()
        container.add(
            Symbol(LatLng(0.0, 0.0))
        )
        container.add(
            Line(listOf(LatLng(2.0, -1.0), LatLng(1.0, -2.0)))
        )
        container.add(
            Circle(LatLng(1.0, 2.0))
        )
        container.add(
            Fill(listOf(listOf(LatLng(1.0, 0.0), LatLng(0.0, 1.0))))
        )
        assertEquals(4, container.size)
        assertEquals(4, container.managerCount)
    }

    @Test
    fun `add two of same type`() {
        val container = getMockContainer()
        container.add(
            Symbol(LatLng(0.0, 0.0))
        )
        container.add(
            Symbol(LatLng(1.0, 2.0))
        )
        assertEquals(2, container.size)
        assertEquals(1, container.managerCount)
    }

    @Test
    fun `add two of same with different z layers`() {
        val container = getMockContainer()
        container.add(
            Symbol(LatLng(0.0, 0.0)).apply { zLayer = 1 }
        )
        container.add(
            Symbol(LatLng(1.0, 2.0)).apply { zLayer = 2 }
        )
        assertEquals(2, container.size)
        assertEquals(2, container.managerCount)
    }

    @Test
    fun `add two of same kind with different NDD properties`() {
        val container = getMockContainer()
        container.add(
            Fill(
                paths = listOf(listOf(LatLng(1.0, 0.0), LatLng(0.0, 1.0))),
                antialias = true
            )
        )
        container.add(
            Fill(
                paths = listOf(listOf(LatLng(1.0, 0.0), LatLng(0.0, 1.0))),
                antialias = false
            )
        )
        assertEquals(2, container.size)
        assertEquals(2, container.managerCount)
    }
}