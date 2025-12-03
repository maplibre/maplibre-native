package org.maplibre.android.maps

import android.content.Context
import android.graphics.Bitmap
import android.graphics.drawable.ShapeDrawable
import org.maplibre.android.MapLibreInjector
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.style.layers.CannotAddLayerException
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.sources.CannotAddSourceException
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.utils.ConfigUtils
import io.mockk.every
import io.mockk.mockk
import io.mockk.spyk
import io.mockk.verify
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.Mock
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class StyleTest : BaseTest() {

    private lateinit var maplibreMap: MapLibreMap

    private lateinit var nativeMapView: NativeMap

    @Mock
    private lateinit var context: Context

    @Mock
    private lateinit var appContext: Context

    @Before
    fun setup() {
        MockitoAnnotations.initMocks(this)
        MapLibreInjector.inject(context, "abcdef", ConfigUtils.getMockedOptions())
        nativeMapView = mockk(relaxed = true)
        maplibreMap = MapLibreMap(
            nativeMapView,
            null,
            null,
            null,
            null,
            null,
            null
        )
        every { nativeMapView.isDestroyed } returns false
        maplibreMap.injectLocationComponent(spyk())
    }

    @Test
    fun testFromUrl() {
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
    }

    @Test
    fun testFromJson() {
        val builder = Style.Builder().fromJson("{}")
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleJson = "{}" }
    }

    @Test
    fun testEmptyBuilder() {
        val builder = Style.Builder()
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleJson = Style.EMPTY_JSON }
    }

    @Test
    fun testWithLayer() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().withLayer(layer)
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) {
            nativeMapView.addLayerBelow(
                layer,
                MapLibreConstants.LAYER_ID_ANNOTATIONS
            )
        }
    }

    @Test
    fun testWithLayerAbove() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().withLayerAbove(layer, "id")
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.addLayerAbove(layer, "id") }
    }

    @Test
    fun testWithLayerBelow() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().withLayerBelow(layer, "id")
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.addLayerBelow(layer, "id") }
    }

    @Test
    fun testWithLayerAt() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().withLayerAt(layer, 1)
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.addLayerAt(layer, 1) }
    }

    @Test
    fun testWithSource() {
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder = Style.Builder().withSource(source)
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.addSource(source) }
    }

    @Test
    fun testWithTransitionOptions() {
        val transitionOptions = TransitionOptions(100, 200)
        val builder = Style.Builder().withTransition(transitionOptions)
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.transitionOptions = transitionOptions }
    }

    @Test
    fun testWithFromLoadingSource() {
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder =
            Style.Builder().fromUri(Style.getPredefinedStyle("Streets")).withSource(source)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addSource(source) }
    }

    @Test
    fun testWithFromLoadingLayer() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets")).withLayer(layer)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) {
            nativeMapView.addLayerBelow(
                layer,
                MapLibreConstants.LAYER_ID_ANNOTATIONS
            )
        }
    }

    @Test
    fun testWithFromLoadingLayerAt() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder =
            Style.Builder().fromUri(Style.getPredefinedStyle("Streets")).withLayerAt(layer, 1)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addLayerAt(layer, 1) }
    }

    @Test
    fun testWithFromLoadingLayerBelow() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
            .withLayerBelow(layer, "below")
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addLayerBelow(layer, "below") }
    }

    @Test
    fun testWithFromLoadingLayerAbove() {
        val layer = mockk<SymbolLayer>()
        every { layer.id } returns "1"
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
            .withLayerBelow(layer, "below")
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addLayerBelow(layer, "below") }
    }

    @Test
    fun testWithFromLoadingTransitionOptions() {
        val transitionOptions = TransitionOptions(100, 200)
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
            .withTransition(transitionOptions)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.transitionOptions = transitionOptions }
    }

    @Test
    fun testFromCallback() {
        val callback = mockk<Style.OnStyleLoaded>()
        every { callback.onStyleLoaded(any()) } answers {}
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
        maplibreMap.setStyle(builder, callback)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { callback.onStyleLoaded(any()) }
    }

    @Test
    fun testWithCallback() {
        val callback = mockk<Style.OnStyleLoaded>()
        every { callback.onStyleLoaded(any()) } answers {}
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder = Style.Builder().withSource(source)
        maplibreMap.setStyle(builder, callback)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.addSource(source) }
        verify(exactly = 1) { callback.onStyleLoaded(any()) }
    }

    @Test
    fun testGetAsyncWith() {
        val callback = mockk<Style.OnStyleLoaded>()
        every { callback.onStyleLoaded(any()) } answers {}
        maplibreMap.getStyle(callback)
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder = Style.Builder().withSource(source)
        maplibreMap.setStyle(builder)
        maplibreMap.onFinishLoadingStyle()
        verify(exactly = 1) { nativeMapView.addSource(source) }
        verify(exactly = 1) { callback.onStyleLoaded(any()) }
    }

    @Test
    fun testGetAsyncFrom() {
        val callback = mockk<Style.OnStyleLoaded>()
        every { callback.onStyleLoaded(any()) } answers {}
        maplibreMap.getStyle(callback)
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder = Style.Builder().fromJson("{}")
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleJson = "{}" }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { callback.onStyleLoaded(any()) }
    }

    @Test
    fun testGetAsyncWithFrom() {
        val callback = mockk<Style.OnStyleLoaded>()
        every { callback.onStyleLoaded(any()) } answers {}
        maplibreMap.getStyle(callback)
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder =
            Style.Builder().fromUri(Style.getPredefinedStyle("Streets")).withSource(source)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Streets") }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addSource(source) }
        verify(exactly = 1) { callback.onStyleLoaded(any()) }
    }

    @Test
    fun testGetNullStyle() {
        Assert.assertNull(maplibreMap.style)
    }

    @Test
    fun testGetNullWhileLoading() {
        val transitionOptions = TransitionOptions(100, 200)
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
            .withTransition(transitionOptions)
        maplibreMap.setStyle(builder)
        Assert.assertNull(maplibreMap.style)
        maplibreMap.notifyStyleLoaded()
        Assert.assertNotNull(maplibreMap.style)
    }

    @Test
    fun testNotReinvokeSameListener() {
        val callback = mockk<Style.OnStyleLoaded>()
        every { callback.onStyleLoaded(any()) } answers {}
        maplibreMap.getStyle(callback)
        val source = mockk<GeoJsonSource>()
        every { source.id } returns "1"
        val builder = Style.Builder().fromJson("{}")
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleJson = "{}" }
        maplibreMap.notifyStyleLoaded()
        maplibreMap.setStyle(Style.getPredefinedStyle("Streets"))
        verify(exactly = 1) { callback.onStyleLoaded(any()) }
    }

    @Test(expected = IllegalStateException::class)
    fun testIllegalStateExceptionWithStyleReload() {
        val builder = Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
        maplibreMap.setStyle(builder)
        maplibreMap.notifyStyleLoaded()
        val style = maplibreMap.style
        maplibreMap.setStyle(Style.Builder().fromUri(Style.getPredefinedStyle("Bright")))
        style!!.addLayer(mockk<SymbolLayer>())
    }

    @Test
    fun testAddImage() {
        val bitmap = Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888)
        val builder =
            Style.Builder().fromUri(Style.getPredefinedStyle("Satellite Hybrid")).withImage("id", bitmap)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Satellite Hybrid") }
        verify(exactly = 0) { nativeMapView.addImages(any()) }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addImages(any()) }
    }

    @Test
    fun testAddDrawable() {
        val drawable = ShapeDrawable()
        drawable.intrinsicHeight = 10
        drawable.intrinsicWidth = 10
        val builder =
            Style.Builder().fromUri(Style.getPredefinedStyle("Satellite Hybrid")).withImage("id", drawable)
        maplibreMap.setStyle(builder)
        verify(exactly = 1) { nativeMapView.styleUri = Style.getPredefinedStyle("Satellite Hybrid") }
        verify(exactly = 0) { nativeMapView.addImages(any()) }
        maplibreMap.notifyStyleLoaded()
        verify(exactly = 1) { nativeMapView.addImages(any()) }
    }

    @Test
    fun testSourceSkippedIfAdditionFails() {
        val source1 = mockk<GeoJsonSource>(relaxed = true)
        every { source1.id } returns "source1"
        val source2 = mockk<GeoJsonSource>(relaxed = true)
        every { source2.id } returns "source1" // same ID

        val builder = Style.Builder().withSource(source1)
        maplibreMap.setStyle(builder)
        maplibreMap.notifyStyleLoaded()

        every { nativeMapView.addSource(any()) } throws CannotAddSourceException("Duplicate ID")

        try {
            maplibreMap.style!!.addSource(source2)
        } catch (ex: Exception) {
            Assert.assertEquals(
                "Source that failed to be added shouldn't be cached",
                source1,
                maplibreMap.style!!.getSource("source1")
            )
        }
    }

    @Test
    fun testLayerSkippedIfAdditionFails() {
        val layer1 = mockk<SymbolLayer>(relaxed = true)
        every { layer1.id } returns "layer1"
        val layer2 = mockk<SymbolLayer>(relaxed = true)
        every { layer2.id } returns "layer1" // same ID

        val builder = Style.Builder().withLayer(layer1)
        maplibreMap.setStyle(builder)
        maplibreMap.notifyStyleLoaded()

        every { nativeMapView.addLayer(any()) } throws CannotAddLayerException("Duplicate ID")

        try {
            maplibreMap.style!!.addLayer(layer2)
        } catch (ex: Exception) {
            Assert.assertEquals(
                "Layer that failed to be added shouldn't be cached",
                layer1,
                maplibreMap.style!!.getLayer("layer1")
            )
        }
    }

    @Test
    fun testLayerSkippedIfAdditionBelowFails() {
        val layer1 = mockk<SymbolLayer>(relaxed = true)
        every { layer1.id } returns "layer1"
        val layer2 = mockk<SymbolLayer>(relaxed = true)
        every { layer2.id } returns "layer1" // same ID

        val builder = Style.Builder().withLayer(layer1)
        maplibreMap.setStyle(builder)
        maplibreMap.notifyStyleLoaded()

        every {
            nativeMapView.addLayerBelow(
                any(),
                ""
            )
        } throws CannotAddLayerException("Duplicate ID")

        try {
            maplibreMap.style!!.addLayerBelow(layer2, "")
        } catch (ex: Exception) {
            Assert.assertEquals(
                "Layer that failed to be added shouldn't be cached",
                layer1,
                maplibreMap.style!!.getLayer("layer1")
            )
        }
    }

    @Test
    fun testLayerSkippedIfAdditionAboveFails() {
        val layer1 = mockk<SymbolLayer>(relaxed = true)
        every { layer1.id } returns "layer1"
        val layer2 = mockk<SymbolLayer>(relaxed = true)
        every { layer2.id } returns "layer1" // same ID

        val builder = Style.Builder().withLayer(layer1)
        maplibreMap.setStyle(builder)
        maplibreMap.notifyStyleLoaded()

        every {
            nativeMapView.addLayerAbove(
                any(),
                ""
            )
        } throws CannotAddLayerException("Duplicate ID")

        try {
            maplibreMap.style!!.addLayerAbove(layer2, "")
        } catch (ex: Exception) {
            Assert.assertEquals(
                "Layer that failed to be added shouldn't be cached",
                layer1,
                maplibreMap.style!!.getLayer("layer1")
            )
        }
    }

    @Test
    fun testLayerSkippedIfAdditionAtFails() {
        val layer1 = mockk<SymbolLayer>(relaxed = true)
        every { layer1.id } returns "layer1"
        val layer2 = mockk<SymbolLayer>(relaxed = true)
        every { layer2.id } returns "layer1" // same ID

        val builder = Style.Builder().withLayer(layer1)
        maplibreMap.setStyle(builder)
        maplibreMap.notifyStyleLoaded()

        every { nativeMapView.addLayerAt(any(), 5) } throws CannotAddLayerException("Duplicate ID")

        try {
            maplibreMap.style!!.addLayerAt(layer2, 5)
        } catch (ex: Exception) {
            Assert.assertEquals(
                "Layer that failed to be added shouldn't be cached",
                layer1,
                maplibreMap.style!!.getLayer("layer1")
            )
        }
    }
}
