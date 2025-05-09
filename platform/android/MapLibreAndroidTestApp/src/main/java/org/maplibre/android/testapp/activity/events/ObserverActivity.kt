package org.maplibre.android.testapp.activity.events

import android.app.ActivityManager
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.MapLibre
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.tile.TileOperation
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber
import java.util.*
import kotlin.time.TimeMark
import kotlin.time.TimeSource
import kotlin.time.TimeSource.Monotonic
import org.maplibre.android.log.Logger
import org.maplibre.android.log.Logger.INFO
import org.maplibre.android.maps.MapLibreMap

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
    MapView.OnSpriteRequestedListener {
    private lateinit var mapView: MapView

    companion object {
        const val TAG = "ObserverActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_events)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.addOnPreCompileShaderListener(this)
        mapView.addOnPostCompileShaderListener(this)
        mapView.addOnTileActionListener(this)
        mapView.addOnGlyphsLoadedListener(this)
        mapView.addOnGlyphsRequestedListener(this)
        mapView.addOnSpriteLoadedListener(this)
        mapView.addOnSpriteRequestedListener(this)
        mapView.getMapAsync {
            it.setStyle(
                Style.Builder().fromUri(TestStyles.DEMOTILES)
            )
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
    public fun printActionJournal(map: MapLibreMap) {
        // configure using `MapLibreMapOptions.actionJournal*` methods

        Logger.i(TAG,"ActionJournal files: \n${map.actionJournalLogFiles.joinToString("\n")}")
        Logger.i(TAG,"ActionJournal : \n${map.actionJournalLog.joinToString("\n")}")

        // print only the newest events on each call
        map.clearActionJournalLog()
    }
    // # --8<-- [end:printActionJournal]

    private val shaderTimes: HashMap<String, TimeMark> = HashMap()

    public override fun onPreCompileShader(id: Int, type: Int, defines: String) {
        shaderTimes["${id}-${type}-${defines}"] = TimeSource.Monotonic.markNow()
        Logger.i(TAG, "A new shader is being compiled, shaderID:${id}, backend type:${type}, program configuration:${defines}")
    }

    public override fun onPostCompileShader(id: Int, type: Int, defines: String) {
        val startTime = shaderTimes.get("${id}-${type}-${defines}")
        if (startTime != null) {
            Logger.i(TAG, "A shader has been compiled in ${startTime.elapsedNow()}, shaderID:${id}, backend type:${type}, program configuration:${defines}")
        }
    }

    public override fun onGlyphsRequested(fontStack: Array<String>, rangeStart: Int, rangeEnd: Int) {
        Logger.i(TAG, "Glyphs are being requested for the font stack $fontStack, ranging from $rangeStart to $rangeEnd")
    }

    public override fun onGlyphsLoaded(fontStack: Array<String>, rangeStart: Int, rangeEnd: Int) {
        Logger.i(TAG, "Glyphs have been loaded for the font stack $fontStack, ranging from $rangeStart to $rangeEnd")
    }

    public override fun onSpriteRequested(id: String, url: String) {
        Logger.i(TAG, "The sprite $id has been requested from $url")
    }

    public override fun onSpriteLoaded(id: String, url: String) {
        Logger.i(TAG, "The sprite $id has been loaded from $url")
    }

    public override fun onTileAction(op: TileOperation, x: Int, y: Int, z: Int, wrap: Int, overscaledZ: Int, sourceID: String) {
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
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }
}
