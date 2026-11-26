package org.maplibre.android.testapp.maps

import android.graphics.Bitmap
import android.graphics.Color
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.rule.ActivityTestRule
import org.json.JSONArray
import org.json.JSONObject
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.AppCenter
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.activity.espresso.DeviceIndependentTestActivity
import java.io.File
import java.io.FileOutputStream
import java.util.UUID
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * Tests MapLibreMap.getTerrainElevation, the Android counterpart of iOS terrainElevation(at:).
 *
 * The map shows a style with a raster-dem source over local tiles that are one flat plateau, so
 * the height under any coordinate is known.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class TerrainElevationTest : AppCenter() {

    @Rule
    @JvmField
    var rule = ActivityTestRule(DeviceIndependentTestActivity::class.java)

    private lateinit var directory: File
    private lateinit var map: MapLibreMap

    @Before
    fun setup() {
        directory = File(
            InstrumentationRegistry.getInstrumentation().targetContext.cacheDir,
            "terrain-elevation-" + UUID.randomUUID()
        )
        assertTrue(directory.mkdirs())
        val latch = CountDownLatch(1)
        rule.runOnUiThread {
            rule.activity.mapView.getMapAsync {
                map = it
                latch.countDown()
            }
        }
        assertTrue(latch.await(30, TimeUnit.SECONDS))
    }

    @After
    fun tearDownDirectory() {
        directory.deleteRecursively()
    }

    // A flat DEM tile in Terrain-RGB ("mapbox") encoding: -10000 + (R * 65536 + G * 256 + B) * 0.1
    private fun writePlateauTile(name: String, meters: Double) {
        val encoded = Math.round((meters + 10000.0) * 10.0).toInt()
        val color = Color.rgb((encoded shr 16) and 0xFF, (encoded shr 8) and 0xFF, encoded and 0xFF)
        val bitmap = Bitmap.createBitmap(DEM_TILE_SIZE, DEM_TILE_SIZE, Bitmap.Config.ARGB_8888)
        bitmap.eraseColor(color)
        FileOutputStream(File(directory, name)).use { bitmap.compress(Bitmap.CompressFormat.PNG, 100, it) }
    }

    // With `servesTiles` false the tile URLs point at files that do not exist, so every DEM tile
    // fails to load.
    private fun terrainStyle(exaggeration: Double, servesTiles: Boolean): String {
        if (servesTiles) {
            for (z in 0..2) {
                for (x in 0 until (1 shl z)) {
                    for (y in 0 until (1 shl z)) {
                        writePlateauTile("$z-$x-$y.png", PLATEAU_METERS)
                    }
                }
            }
        }
        return JSONObject()
            .put("version", 8)
            .put(
                "sources",
                JSONObject().put(
                    "dem",
                    JSONObject()
                        .put("type", "raster-dem")
                        .put("tiles", JSONArray().put("file://" + directory.absolutePath + "/{z}-{x}-{y}.png"))
                        .put("encoding", "mapbox")
                        .put("maxzoom", 2)
                        .put("tileSize", DEM_TILE_SIZE)
                )
            )
            .put("terrain", JSONObject().put("source", "dem").put("exaggeration", exaggeration))
            .put("layers", JSONArray())
            .toString()
    }

    private fun loadStyle(json: String) {
        val latch = CountDownLatch(1)
        rule.runOnUiThread {
            map.cameraPosition = CameraPosition.Builder().target(INNSBRUCK).zoom(2.0).build()
            map.setStyle(Style.Builder().fromJson(json)) { latch.countDown() }
        }
        assertTrue("The style should load.", latch.await(30, TimeUnit.SECONDS))
    }

    private fun elevationAt(latLng: LatLng): Double? {
        var elevation: Double? = null
        rule.runOnUiThread { elevation = map.getTerrainElevation(latLng) }
        return elevation
    }

    // Waits until the map reports a height at the coordinate, or the timeout.
    private fun waitForElevationAt(latLng: LatLng): Double? {
        val deadline = System.currentTimeMillis() + 20_000
        var elevation = elevationAt(latLng)
        while (elevation == null && System.currentTimeMillis() < deadline) {
            Thread.sleep(50)
            elevation = elevationAt(latLng)
        }
        return elevation
    }

    @Test
    fun testElevationMatchesTheDemTile() {
        loadStyle(terrainStyle(1.0, servesTiles = true))

        val elevation = waitForElevationAt(INNSBRUCK)
        assertNotNull("The DEM tile should load and answer.", elevation)
        assertEquals(PLATEAU_METERS, elevation!!, 1.0)
    }

    @Test
    fun testElevationIncludesTheExaggeration() {
        loadStyle(terrainStyle(2.0, servesTiles = true))

        val elevation = waitForElevationAt(INNSBRUCK)
        assertNotNull("The DEM tile should load and answer.", elevation)
        assertEquals(PLATEAU_METERS * 2, elevation!!, 2.0)
    }

    @Test
    fun testNullWithoutTerrain() {
        loadStyle(JSONObject().put("version", 8).put("sources", JSONObject()).put("layers", JSONArray()).toString())
        Thread.sleep(1000)

        assertNull(elevationAt(INNSBRUCK))
        assertNull(elevationAt(LatLng(0.0, 0.0)))
    }

    @Test
    fun testNullWhenTheDemTileHasNotLoaded() {
        loadStyle(terrainStyle(1.0, servesTiles = false))
        Thread.sleep(3000)

        assertNull(elevationAt(INNSBRUCK))
    }

    @Test
    fun testNullForInvalidLongitude() {
        loadStyle(terrainStyle(1.0, servesTiles = true))
        // Terrain is up and answering, so a null below comes from the coordinate and not from the map.
        assertNotNull(waitForElevationAt(INNSBRUCK))

        assertNull(elevationAt(LatLng(47.0, 181.0)))
        assertNull(elevationAt(LatLng(47.0, -181.0)))
        assertNotNull(elevationAt(INNSBRUCK))
    }

    @Test
    fun testNullAfterTerrainIsRemoved() {
        loadStyle(terrainStyle(1.0, servesTiles = true))
        assertNotNull(waitForElevationAt(INNSBRUCK))

        loadStyle(JSONObject().put("version", 8).put("sources", JSONObject()).put("layers", JSONArray()).toString())

        val deadline = System.currentTimeMillis() + 15_000
        while (elevationAt(INNSBRUCK) != null && System.currentTimeMillis() < deadline) {
            Thread.sleep(50)
        }
        assertNull(elevationAt(INNSBRUCK))
    }

    companion object {
        private const val DEM_TILE_SIZE = 64
        private const val PLATEAU_METERS = 1000.0

        // Innsbruck, inside the one DEM tile the test styles serve.
        private val INNSBRUCK = LatLng(47.2692, 11.4041)
    }
}
