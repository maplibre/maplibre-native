package org.maplibre.android.style.sources

import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4ClassRunner::class)
@UiThreadTest
class RasterDemSourceTest {

    private val pmtiles = "pmtiles://https://demotiles.maplibre.org/pmtiles/raster/terrain.pmtiles"

    // A URL cannot carry the encoding, so a Terrarium tileset behind a pmtiles:// URL decoded
    // as Mapbox until the constructor took it (maplibre-native#4357).
    @Test
    fun uriSourceTakesTheEncoding() {
        val source = RasterDemSource("terrain", pmtiles, 512, RasterDemSource.ENCODING_TERRARIUM)
        assertEquals(RasterDemSource.ENCODING_TERRARIUM, source.encoding)
        assertEquals(pmtiles, source.uri)

        val mapbox = RasterDemSource("terrain-mapbox", pmtiles, 512, RasterDemSource.ENCODING_MAPBOX)
        assertEquals(RasterDemSource.ENCODING_MAPBOX, mapbox.encoding)
    }

    @Test
    fun encodingDefaultsToMapbox() {
        assertEquals(RasterDemSource.ENCODING_MAPBOX, RasterDemSource("a", pmtiles).encoding)
        assertEquals(RasterDemSource.ENCODING_MAPBOX, RasterDemSource("b", pmtiles, 256).encoding)
        assertEquals(RasterDemSource.ENCODING_MAPBOX, RasterDemSource("c", pmtiles, 256, null).encoding)
    }

    @Test
    fun tileSetEncodingIsReported() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        tileSet.encoding = RasterDemSource.ENCODING_TERRARIUM
        assertEquals(RasterDemSource.ENCODING_TERRARIUM, RasterDemSource("tiles", tileSet).encoding)
    }

    @Test
    fun unsupportedEncodingIsRejected() {
        assertThrows(IllegalArgumentException::class.java) {
            RasterDemSource("terrain", pmtiles, 512, "mapzen")
        }
    }
}
