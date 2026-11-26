package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.CustomVectorSource
import org.maplibre.android.style.sources.TileData
import org.maplibre.android.style.sources.CustomVectorTileProvider
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import java.io.ByteArrayOutputStream

/**
 * Demo activity that uses CustomVectorSource with a fake MVT tile provider.
 * Generates a diagonal cross pattern per tile as synthetic MVT data.
 * Toggle the source on/off with the FAB.
 */
class CustomVectorSourceActivity : AppCompatActivity(), OnMapReadyCallback {

    private lateinit var mapView: MapView
    private var maplibreMap: MapLibreMap? = null
    private var sourceActive = false
    private val tileScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_custom_vector_source)

        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)

        findViewById<FloatingActionButton>(R.id.fabToggle).setOnClickListener {
            toggleSource()
        }
    }

    override fun onMapReady(map: MapLibreMap) {
        maplibreMap = map
        map.setStyle(Style.Builder().fromUri(TestStyles.DEMOTILES))
    }

    private fun toggleSource() {
        val map = maplibreMap ?: return
        val style = map.style ?: return

        if (sourceActive) {
            style.removeLayer(LAYER_ID)
            style.removeSource(SOURCE_ID)
            sourceActive = false
            Toast.makeText(this, "CustomVectorSource removed", Toast.LENGTH_SHORT).show()
        } else {
            val source = CustomVectorSource(SOURCE_ID, DiagonalTileProvider(), tileScope, minZoom = 0, maxZoom = 14)
            val layer = LineLayer(LAYER_ID, SOURCE_ID).apply {
                setSourceLayer(SOURCE_LAYER_NAME)
                setProperties(
                    PropertyFactory.lineColor(Color.RED),
                    PropertyFactory.lineWidth(2f),
                    PropertyFactory.lineOpacity(0.8f)
                )
            }
            style.addSource(source)
            style.addLayer(layer)
            sourceActive = true
            Toast.makeText(this, "CustomVectorSource added", Toast.LENGTH_SHORT).show()
        }
    }

    /**
     * A fake tile provider that generates MVT tiles containing diagonal lines.
     * Simulates async work with a small delay.
     */
    private class DiagonalTileProvider : CustomVectorTileProvider {
        override suspend fun fetchTile(z: Int, x: Int, y: Int): TileData {
            // Simulate network latency
            delay(50)
            val mvtBytes = buildDiagonalMvtTile(z, x, y)
            return TileData.Mvt(mvtBytes)
        }
    }

    override fun onStart() { super.onStart(); mapView.onStart() }
    override fun onResume() { super.onResume(); mapView.onResume() }
    override fun onPause() { super.onPause(); mapView.onPause() }
    override fun onStop() { super.onStop(); mapView.onStop() }
    override fun onDestroy() { tileScope.cancel(); super.onDestroy(); mapView.onDestroy() }
    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    companion object {
        const val SOURCE_ID = "custom-vector-source"
        const val LAYER_ID = "custom-vector-layer"
        const val SOURCE_LAYER_NAME = "diagonal"
        private const val EXTENT = 4096

        /**
         * Builds a minimal MVT tile with two diagonal lines forming an X.
         * MVT protobuf encoding done by hand (no protobuf library needed).
         */
        private fun buildDiagonalMvtTile(z: Int, x: Int, y: Int): ByteArray {
            // Geometry: two lines forming an X across the tile
            // Line 1: (0,0) -> (4096,4096)
            // Line 2: (4096,0) -> (0,4096)
            val geom1 = encodeMvtGeometry(
                listOf(Pair(0, 0), Pair(EXTENT, EXTENT))
            )
            val geom2 = encodeMvtGeometry(
                listOf(Pair(EXTENT, 0), Pair(0, EXTENT))
            )

            // Also add a border rectangle
            val border = encodeMvtGeometry(
                listOf(Pair(0, 0), Pair(EXTENT, 0), Pair(EXTENT, EXTENT), Pair(0, EXTENT), Pair(0, 0))
            )

            val feature1 = encodeMvtFeature(id = 1, geometry = geom1, geomType = 2)
            val feature2 = encodeMvtFeature(id = 2, geometry = geom2, geomType = 2)
            val feature3 = encodeMvtFeature(id = 3, geometry = border, geomType = 2)

            val layer = encodeMvtLayer(
                name = SOURCE_LAYER_NAME,
                extent = EXTENT,
                features = listOf(feature1, feature2, feature3)
            )

            // Tile: repeated Layer layers = 3;
            val tile = ByteArrayOutputStream()
            writeTag(tile, 3, 2) // field 3, wire type 2 (length-delimited)
            writeBytes(tile, layer)
            return tile.toByteArray()
        }

        private fun encodeMvtGeometry(points: List<Pair<Int, Int>>): ByteArray {
            val commands = mutableListOf<Int>()
            for (i in points.indices) {
                if (i == 0) {
                    // MoveTo command: id=1, count=1
                    commands.add(commandInteger(1, 1))
                    commands.add(zigzag(points[0].first))
                    commands.add(zigzag(points[0].second))
                } else {
                    if (i == 1) {
                        // LineTo command: id=2, count=remaining points
                        commands.add(commandInteger(2, points.size - 1))
                    }
                    val dx = points[i].first - points[i - 1].first
                    val dy = points[i].second - points[i - 1].second
                    commands.add(zigzag(dx))
                    commands.add(zigzag(dy))
                }
            }
            return commands.flatMap { encodeVarint(it) }.toByteArray()
        }

        private fun encodeMvtFeature(id: Long, geometry: ByteArray, geomType: Int): ByteArray {
            val out = ByteArrayOutputStream()
            // optional uint64 id = 1
            writeTag(out, 1, 0)
            writeVarint(out, id)
            // optional GeomType type = 3
            writeTag(out, 3, 0)
            writeVarint(out, geomType.toLong())
            // repeated uint32 geometry = 4 (packed)
            writeTag(out, 4, 2)
            writeBytes(out, geometry)
            return out.toByteArray()
        }

        private fun encodeMvtLayer(name: String, extent: Int, features: List<ByteArray>): ByteArray {
            val out = ByteArrayOutputStream()
            // required uint32 version = 15
            writeTag(out, 15, 0)
            writeVarint(out, 2)
            // required string name = 1
            writeTag(out, 1, 2)
            writeBytes(out, name.toByteArray(Charsets.UTF_8))
            // repeated Feature features = 2
            for (feature in features) {
                writeTag(out, 2, 2)
                writeBytes(out, feature)
            }
            // optional uint32 extent = 5
            writeTag(out, 5, 0)
            writeVarint(out, extent.toLong())
            return out.toByteArray()
        }

        private fun commandInteger(id: Int, count: Int): Int = (id and 0x7) or (count shl 3)

        private fun zigzag(value: Int): Int = (value shl 1) xor (value shr 31)

        private fun encodeVarint(value: Int): List<Byte> {
            val bytes = mutableListOf<Byte>()
            var v = value.toLong() and 0xFFFFFFFFL
            while (v > 0x7F) {
                bytes.add(((v and 0x7F) or 0x80).toByte())
                v = v ushr 7
            }
            bytes.add((v and 0x7F).toByte())
            return bytes
        }

        private fun writeTag(out: ByteArrayOutputStream, fieldNumber: Int, wireType: Int) {
            writeVarint(out, ((fieldNumber shl 3) or wireType).toLong())
        }

        private fun writeVarint(out: ByteArrayOutputStream, value: Long) {
            var v = value
            while (v > 0x7F) {
                out.write(((v and 0x7F) or 0x80).toInt())
                v = v ushr 7
            }
            out.write((v and 0x7F).toInt())
        }

        private fun writeBytes(out: ByteArrayOutputStream, data: ByteArray) {
            writeVarint(out, data.size.toLong())
            out.write(data)
        }

        private fun List<Byte>.toByteArray(): ByteArray = ByteArray(size) { this[it] }
    }
}
