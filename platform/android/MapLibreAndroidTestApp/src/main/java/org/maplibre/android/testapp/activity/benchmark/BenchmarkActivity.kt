package org.maplibre.android.testapp.activity.benchmark

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.PowerManager
import android.view.View
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import org.maplibre.android.camera.CameraUpdate
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.log.Logger
import org.maplibre.android.log.Logger.INFO
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.RenderingStats
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.BenchmarkInputData
import org.maplibre.android.testapp.utils.BenchmarkResult
import org.maplibre.android.testapp.utils.BenchmarkRun
import org.maplibre.android.testapp.utils.BenchmarkRunResult
import org.maplibre.android.testapp.utils.FrameTimeStore
import org.maplibre.android.testapp.utils.animateCameraSuspend
import org.maplibre.android.testapp.utils.jsonPayload
import org.maplibre.android.testapp.utils.setStyleSuspend
import java.io.File
import java.util.ArrayList
import kotlin.collections.flatMap
import kotlin.collections.toTypedArray
import kotlin.coroutines.resume

/**
 * Benchmark using a [android.view.TextureView]
 */
class BenchmarkActivity : AppCompatActivity() {
    private val TAG = "BenchmarkActivity"

    private lateinit var mapView: MapView
    private var handler: Handler? = null
    private var delayed: Runnable? = null

    // the styles used for the benchmark
    // can be overridden adding with developer-config.xml
    // ```xml
    //  <array name="benchmark_style_names">
    //    <item>Americana</item>
    //  </array>
    //  <array name="benchmark_style_urls">
    //    <item>https://americanamap.org/style.json</item>
    // </array>
    // ```
    private lateinit var inputData: BenchmarkInputData

    @SuppressLint("DiscouragedApi")
    private fun getArrayFromResources(name: String): Array<String> {
        return try {
            resources.getStringArray(applicationContext.resources.getIdentifier(
                name,
                "array",
                applicationContext.packageName))
        } catch (e: Throwable) {
            emptyArray()
        }
    }

    private fun getBenchmarkInputData(): BenchmarkInputData {
        // read input for benchmark from JSON file (on CI)
        val jsonFile = File("${Environment.getExternalStorageDirectory()}/instrumentation-test-input.json")
        Logger.i(TAG, "Environment.getExternalStorageDirectory() = ${Environment.getExternalStorageDirectory()}")
        Logger.i(TAG, "jsonFile.isFile = ${jsonFile.isFile}")

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R && Environment.isExternalStorageManager() && jsonFile.isFile) {
            val jsonFileContents = jsonFile.readText()
            val jsonElement = Json.parseToJsonElement(jsonFileContents)
            val styleNames = jsonElement.jsonObject["styleNames"]?.jsonArray?.map { it.jsonPrimitive.content }
            val styleURLs = jsonElement.jsonObject["styleURLs"]?.jsonArray?.map { it.jsonPrimitive.content }
            if (styleNames == null || styleURLs == null) {
                throw Error("${jsonFile.name} is missing elements")
            }
            return BenchmarkInputData(
                styleNames = styleNames.toList(),
                styleURLs = styleURLs.toList()
            )
        } else {
            Logger.i(TAG, "${jsonFile.name} not found, reading from developer-config.xml")
        }

        // try to read from developer-config.xml instead (for development)
        val styleNames = getArrayFromResources("benchmark_style_names")
        val styleURLs = getArrayFromResources("benchmark_style_urls")
        if (styleNames.isNotEmpty() && styleURLs.isNotEmpty()) {
            return BenchmarkInputData(
                styleNames = styleNames.toList(),
                styleURLs = styleURLs.toList()
            )
        }

        // return default
        return BenchmarkInputData(
            styleNames = listOf(
                "AWS Open Data Standard Light",
//                "Facebook Light",
                "Americana",
//                "Protomaps Light",
//                "Versatiles Colorful",
               "OpenFreeMap Bright"
            ),
            styleURLs = listOf(
                "https://maps.geo.us-east-2.amazonaws.com/maps/v0/maps/OpenDataStyle/style-descriptor?key=v1.public.eyJqdGkiOiI1NjY5ZTU4My0yNWQwLTQ5MjctODhkMS03OGUxOTY4Y2RhMzgifR_7GLT66TNRXhZJ4KyJ-GK1TPYD9DaWuc5o6YyVmlikVwMaLvEs_iqkCIydspe_vjmgUVsIQstkGoInXV_nd5CcmqRMMa-_wb66SxDdbeRDvmmkpy2Ow_LX9GJDgL2bbiCws0wupJPFDwWCWFLwpK9ICmzGvNcrPbX5uczOQL0N8V9iUvziA52a1WWkZucIf6MUViFRf3XoFkyAT15Ll0NDypAzY63Bnj8_zS8bOaCvJaQqcXM9lrbTusy8Ftq8cEbbK5aMFapXRjug7qcrzUiQ5sr0g23qdMvnKJQFfo7JuQn8vwAksxrQm6A0ByceEXSfyaBoVpFcTzEclxUomhY.NjAyMWJkZWUtMGMyOS00NmRkLThjZTMtODEyOTkzZTUyMTBi",
//                "https://external.xx.fbcdn.net/maps/vt/style/canterbury_1_0/?locale=en_US",
                "https://americanamap.org/style.json",
//                "https://api.protomaps.com/styles/v2/light.json?key=e761cc7daedf832a",
//                "https://tiles.versatiles.org/assets/styles/colorful.json",
               "https://tiles.openfreemap.org/styles/bright"
            )
        )
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Logger.setVerbosity(INFO)

        setContentView(R.layout.activity_benchmark)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        handler = Handler(mainLooper)
        setupToolbar()
        inputData = getBenchmarkInputData()
        setupMapView()
    }

    private fun setupToolbar() {
        val actionBar = supportActionBar
        if (actionBar != null) {
            supportActionBar!!.setDisplayHomeAsUpEnabled(true)
            supportActionBar!!.setHomeButtonEnabled(true)
        }
    }

    private fun getThermalStatus(): Int {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val powerManager = getSystemService(Context.POWER_SERVICE) as PowerManager
            return powerManager.currentThermalStatus
        }

        return -1;
    }

    private fun setupMapView() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val powerManager = getSystemService(Context.POWER_SERVICE) as PowerManager
            powerManager.addThermalStatusListener {
                    status -> println("Thermal status changed $status")
            }
        }

        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.getMapAsync { maplibreMap: MapLibreMap ->
            val benchmarkResult = BenchmarkResult(arrayListOf())

            val benchmarkSlowDuration = 70000
            val benchmarkFastDuration = 15000

            lifecycleScope.launch {
                val benchmarkRuns = inputData.styleNames.zip(inputData.styleURLs).flatMap { (styleName, styleUrl) ->
                    listOf(
                        BenchmarkRun(styleName, styleUrl, true, benchmarkSlowDuration),
                        BenchmarkRun(styleName, styleUrl, false, benchmarkSlowDuration)
                    )
                }.toTypedArray()
                val benchmarkIterations = 4
                for (i in 0 until benchmarkIterations) {
                    for (benchmarkRun in benchmarkRuns) {
                        val benchmarkRunResult = doBenchmarkRun(
                            maplibreMap,
                            // do one fast run to cache needed tiles
                            if (i == 0)  benchmarkRun.copy(duration = benchmarkFastDuration) else benchmarkRun)
                        val benchmarkPair = Pair(benchmarkRun, benchmarkRunResult)
                        // don't store results for fast run
                        if (i != 0) benchmarkResult.runs.add(benchmarkPair)
                        println(jsonPayload(BenchmarkResult(arrayListOf(benchmarkPair))))
                    }
                }

                println(jsonPayload(benchmarkResult))
                storeResults(benchmarkResult)
                benchmarkDone()
            }
        }
    }

    private suspend fun doBenchmarkRun(maplibreMap: MapLibreMap, benchmarkRun: BenchmarkRun): BenchmarkRunResult {
        var numFrames = 0

        val encodingTimeStore = FrameTimeStore()
        val renderingTimeStore = FrameTimeStore()

        maplibreMap.setSwapBehaviorFlush(benchmarkRun.syncRendering)

        val listener = MapView.OnDidFinishRenderingFrameWithStatsListener { _: Boolean, stats: RenderingStats ->
            encodingTimeStore.add(stats.encodingTime * 1e3)
            renderingTimeStore.add(stats.renderingTime * 1e3)
            numFrames++;
        }
        mapView.addOnDidFinishRenderingFrameListener(listener)
        mapView.setStyleSuspend(benchmarkRun.styleURL)
        numFrames = 0

        val startTime = System.nanoTime()

        for (place in PLACES) {
            maplibreMap.animateCameraSuspend(
                CameraUpdateFactory.newLatLngZoom(place, 14.0),
                benchmarkRun.duration
            )
        }
        val endTime = System.nanoTime()
        val fps = (numFrames * 1E9) / (endTime - startTime)

        mapView.removeOnDidFinishRenderingFrameListener(listener)

        return BenchmarkRunResult(fps, encodingTimeStore, renderingTimeStore, getThermalStatus())
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
        if (handler != null && delayed != null) {
            handler!!.removeCallbacks(delayed!!)
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    private fun storeResults(benchmarkResult: BenchmarkResult) {
        val payload = jsonPayload(benchmarkResult)
        val dataDir = this.filesDir
        val benchmarkResultsFile = File(dataDir, "benchmark_results.json")
        benchmarkResultsFile.writeText(Json.encodeToString(payload))
    }

    private fun benchmarkDone() {
        setResult(Activity.RESULT_OK)
        finish()
    }

    companion object {
        private val PLACES = arrayOf(
            LatLng(37.7749, -122.4194), // SF
            LatLng(38.9072, -77.0369), // DC
            LatLng(52.3702, 4.8952), // AMS
            LatLng(60.1699, 24.9384), // HEL
//            LatLng(-13.1639, -74.2236), // AYA
//            LatLng(52.5200, 13.4050), // BER
//            LatLng(12.9716, 77.5946), // BAN
//            LatLng(31.2304, 121.4737) // SHA
        )
    }
}
