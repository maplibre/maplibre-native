package org.maplibre.android.style.sources

import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4ClassRunner::class)
@UiThreadTest
class RasterSourceTest {
    @Test
    fun rasterSource_constructor() {
        val tiles = arrayOf("https://example.com")
        val tileSet = TileSet("2.1.0", *tiles)

        // set bounds on tile set
        val bounds = floatArrayOf(-180.0f, -85.05113f, 180.0f, 85.05113f)
        tileSet.setBounds(*bounds)
        // create RasterSource
        RasterSource("source_id", tileSet)
    }
}
