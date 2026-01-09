package org.maplibre.android.testapp.activity.events

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.tile.TileOperation
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import java.util.*
import kotlin.time.TimeMark
import kotlin.time.TimeSource
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.RenderingStats
import org.maplibre.android.maps.MapLibreMap

// # --8<-- [start:ObserverActivity]
/**
 * Test activity showcasing logging observer actions from the core
 */
class ObserverActivity : AppCompatActivity(),
    MapView.OnPreCompileShaderListener,
    MapView.OnPostCompileShaderListener,
    MapView.OnTileActionListener,
    MapView.OnGlyphsLoadedListener,
    MapView.OnGlyphsRequestedListener,
    MapView.OnSpriteLoadedListener,
    MapView.OnSpriteRequestedListener,
    MapView.OnDidFinishRenderingFrameWithStatsListener {
    // # --8<-- [end:ObserverActivity]

    private lateinit var mapView: MapView
    private val renderStatsTracker = RenderStatsTracker()

    companion object {
        const val TAG = "ObserverActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_events)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)

        // # --8<-- [start:addListeners]
        mapView.addOnPreCompileShaderListener(this)
        mapView.addOnPostCompileShaderListener(this)
        mapView.addOnTileActionListener(this)
        mapView.addOnGlyphsLoadedListener(this)
        mapView.addOnGlyphsRequestedListener(this)
        mapView.addOnSpriteLoadedListener(this)
        mapView.addOnSpriteRequestedListener(this)
        mapView.addOnDidFinishRenderingFrameListener(this)
        // # --8<-- [end:addListeners]

        // # --8<-- [start:renderStatsTracker]
        renderStatsTracker.setReportFields(listOf(
            "encodingTime",
            "renderingTime",
            "numDrawCalls",
            "numActiveTextures",
            "numBuffers",
            "memTextures",
            "memBuffers"
        ))

        renderStatsTracker.setReportListener { _, _, avg ->
            Logger.i(TAG, "RenderStatsReport - avg - ${avg.nonZeroValuesString()}")
        }

        renderStatsTracker.setThresholds(hashMapOf(
            "numDrawCalls" to 1000,
            "totalBuffers" to 1000L
        ))

        renderStatsTracker.setThresholdExceededListener { exceededValues, _ ->
            Logger.i(TAG, "Exceeded render values $exceededValues")
        }

        renderStatsTracker.startReports(10L)
        // # --8<-- [end:renderStatsTracker]

        mapView.getMapAsync {
            it.setStyle(
                Style.Builder().fromUri(TestStyles.DEMOTILES)
            )
            it.enableRenderingStatsView(true)
        }

        // print and clear action journal
        val fab = findViewById<View>(R.id.fab)
        fab?.setOnClickListener { _: View? ->
            mapView.getMapAsync {
                printActionJournal(it)
            }
        }
    }

    // # --8<-- [start:printActionJournal]
    private fun printActionJournal(map: MapLibreMap) {
        // configure using `MapLibreMapOptions.actionJournal*` methods

        Logger.i(TAG,"ActionJournal files: \n${map.actionJournalLogFiles.joinToString("\n")}")
        Logger.i(TAG,"ActionJournal : \n${map.actionJournalLog.joinToString("\n")}")

        // print only the newest events on each call
        map.clearActionJournalLog()
    }
    // # --8<-- [end:printActionJournal]

    private val shaderTimes: HashMap<String, TimeMark> = HashMap()

    // # --8<-- [start:mapEvents]
    override fun onPreCompileShader(id: Int, type: Int, defines: String) {
        shaderTimes["${id}-${type}-${defines}"] = TimeSource.Monotonic.markNow()
        Logger.i(TAG, "A new shader is being compiled, shaderID:${id}, backend type:${type}, program configuration:${defines}")
    }

    override fun onPostCompileShader(id: Int, type: Int, defines: String) {
        val startTime = shaderTimes.get("${id}-${type}-${defines}")
        if (startTime != null) {
            Logger.i(TAG, "A shader has been compiled in ${startTime.elapsedNow()}, shaderID:${id}, backend type:${type}, program configuration:${defines}")
        }
    }

    override fun onGlyphsRequested(fontStack: Array<String>, rangeStart: Int, rangeEnd: Int) {
        Logger.i(TAG, "Glyphs are being requested for the font stack $fontStack, ranging from $rangeStart to $rangeEnd")
    }

    override fun onGlyphsLoaded(fontStack: Array<String>, rangeStart: Int, rangeEnd: Int) {
        Logger.i(TAG, "Glyphs have been loaded for the font stack $fontStack, ranging from $rangeStart to $rangeEnd")
    }

    override fun onSpriteRequested(id: String, url: String) {
        Logger.i(TAG, "The sprite $id has been requested from $url")
    }

    override fun onSpriteLoaded(id: String, url: String) {
        Logger.i(TAG, "The sprite $id has been loaded from $url")
    }

    override fun onTileAction(op: TileOperation, x: Int, y: Int, z: Int, wrap: Int, overscaledZ: Int, sourceID: String) {
        val tile = "X:${x}, Y:${y}, Z:${z}, Wrap:${wrap}, OverscaledZ:${overscaledZ}, SourceID:${sourceID}"
        when (op) {
            TileOperation.RequestedFromCache -> Logger.i(TAG, "Requesting tile ${tile} from the cache")
            TileOperation.RequestedFromNetwork -> Logger.i(TAG, "Requesting tile ${tile} from the network")
            TileOperation.LoadFromCache -> Logger.i(TAG, "Loading tile ${tile}, requested from the cache")
            TileOperation.LoadFromNetwork -> Logger.i(TAG, "Loading tile ${tile}, requested from the network")
            TileOperation.StartParse -> Logger.i(TAG, "Parsing tile ${tile}")
            TileOperation.EndParse -> Logger.i(TAG, "Completed parsing tile ${tile}")
            TileOperation.Error -> Logger.e(TAG, "An error occured during proccessing for tile ${tile}")
            TileOperation.Cancelled -> Logger.i(TAG, "Pending work on tile ${tile} was cancelled")
            TileOperation.NullOp -> Logger.e(TAG, "An unknown tile operation was emitted for tile ${tile}")
        }
    }

    override fun onDidFinishRenderingFrame(fully: Boolean, stats: RenderingStats) {
        renderStatsTracker.addFrame(stats)
    }
    // # --8<-- [end:mapEvents]

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
        renderStatsTracker.stopReports()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }
}
