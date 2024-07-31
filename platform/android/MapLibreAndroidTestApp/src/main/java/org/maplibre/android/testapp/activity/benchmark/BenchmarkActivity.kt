package org.maplibre.android.testapp.activity.benchmark

import android.annotation.SuppressLint
import android.app.Activity
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.view.View
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.putJsonObject
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.maplibre.android.BuildConfig.GIT_REVISION
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.log.Logger
import org.maplibre.android.log.Logger.INFO
import org.maplibre.android.maps.*
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.testapp.BuildConfig
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.utils.FpsStore
import org.maplibre.android.testapp.utils.BenchmarkResults
import org.maplibre.android.testapp.utils.FrameTimeStore
import java.io.File

data class BenchmarkInputData(
    val styleNames: List<String>,
    val styleURLs: List<String>,
    val resultsAPI: String = ""
) {
    init {
        if (styleNames.size != styleURLs.size)
            throw Error("Different size: styleNames=$styleNames, styleURLs=$styleURLs")
    }
}

/**
 * Prepares JSON payload that is sent to the API that collects benchmark results.
 * See https://github.com/maplibre/ci-runners
 */
@SuppressLint("NewApi")
fun jsonPayload(styleNames: List<String>, fpsResults: BenchmarkResults, encodingTimeResults: BenchmarkResults, renderingTimeResults: BenchmarkResults): JsonObject {
    return buildJsonObject {
        putJsonObject("resultsPerStyle") {
            for (style in styleNames) {
                putJsonObject(style) {
                    fpsResults.resultsPerStyle[style].let { results ->
                        if (results !== null) {
                            put("avgFps", JsonPrimitive(results.map { it.average }.average()))
                            put("low1pFps", JsonPrimitive(results.map { it.low1p }.average()))
                        }
                    }

                    encodingTimeResults.resultsPerStyle[style].let { results ->
                        if (results !== null) {
                            put("avgEncodingTime", JsonPrimitive(results.map { it.average }.average()))
                            put("low1pEncodingTime", JsonPrimitive(results.map { it.low1p }.average()))
                        }
                    }

                    renderingTimeResults.resultsPerStyle[style].let { results ->
                        if (results !== null) {
                            put("avgRenderingTime", JsonPrimitive(results.map { it.average }.average()))
                            put("low1pRenderingTime", JsonPrimitive(results.map { it.low1p }.average()))
                        }
                    }
                }
            }
        }
        put("deviceManufacturer", JsonPrimitive(Build.MANUFACTURER))
        put("model", JsonPrimitive(Build.MODEL))
        put("renderer", JsonPrimitive(BuildConfig.FLAVOR))
        put("debugBuild", JsonPrimitive(BuildConfig.DEBUG))
        put("gitRevision", JsonPrimitive(GIT_REVISION))
    }
}

/**
 * Benchmark using a [android.view.TextureView]
 */
class BenchmarkActivity : AppCompatActivity() {
    private val TAG = "BenchmarkActivity"

    private lateinit var mapView: MapView
    private var handler: Handler? = null
    private var delayed: Runnable? = null
    private var fpsStore = FpsStore()
    private var encodingTimeStore = FrameTimeStore()
    private var renderingTimeStore = FrameTimeStore()
    private var fpsResults = BenchmarkResults()
    private var encodingTimeResults = BenchmarkResults()
    private var renderingTimeResults = BenchmarkResults()
    private var runsLeft = 5
    private var measureFrameTime = true

    // the styles used for the benchmark
    // can be overridden adding with developer-config.xml
    // ```xml
    //  <array name="benchmark_style_names">
    //    <item>Americana</item>
    //  </array>
    //  <array name="benchmark_style_urls">
    //    <item>https://zelonewolf.github.io/openstreetmap-americana/style.json</item>
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
            val resultsAPI = jsonElement.jsonObject["resultsAPI"]?.jsonPrimitive?.content
            if (styleNames == null || styleURLs == null || resultsAPI == null) {
                throw Error("${jsonFile.name} is missing elements")
            }
            return BenchmarkInputData(
                styleNames = styleNames.toList(),
                styleURLs = styleURLs.toList(),
                resultsAPI = resultsAPI
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
            styleNames = listOf("MapLibre Demotiles"),
            styleURLs = listOf("https://demotiles.maplibre.org/style.json")
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

    private fun setupMapView() {
        mapView = findViewById<View>(R.id.mapView) as MapView
        if (measureFrameTime) {
            mapView.addOnDidFinishRenderingFrameListener { _: Boolean, frameEncodingTime: Double, frameRenderingTime: Double ->
                encodingTimeStore.add(frameEncodingTime * 1e3)
                renderingTimeStore.add(frameRenderingTime * 1e3)
            }
        }
        mapView.getMapAsync { maplibreMap: MapLibreMap ->
            maplibreMap.setStyle(inputData.styleURLs[0])
            maplibreMap.setSwapBehaviorFlush(measureFrameTime)
            if (!measureFrameTime) {
                maplibreMap.setOnFpsChangedListener { fps: Double ->
                    fpsStore.add(fps)
                }
            }

            // Start an animation on the map as well
            flyTo(maplibreMap, 0, 0,14.0)
        }
    }

    private fun flyTo(maplibreMap: MapLibreMap, place: Int, style: Int, zoom: Double) {
        maplibreMap.animateCamera(
            CameraUpdateFactory.newLatLngZoom(PLACES[place], zoom),
            10000,
            object : CancelableCallback {
                override fun onCancel() {
                    delayed = Runnable {
                        delayed = null
                        flyTo(maplibreMap, place, style, zoom)
                    }
                    delayed?.let {
                        handler!!.postDelayed(it, 2000)
                    }
                }

                override fun onFinish() {
                    if (place == PLACES.size - 1) {  // done with tour
                        if (measureFrameTime) {
                            encodingTimeResults.addResult(
                                inputData.styleNames[style],
                                encodingTimeStore
                            )
                            encodingTimeStore.reset()

                            println("Encoding time results $encodingTimeResults")

                            renderingTimeResults.addResult(
                                inputData.styleNames[style],
                                renderingTimeStore
                            )
                            renderingTimeStore.reset()

                            println("Rendering time results $renderingTimeResults")
                        } else {
                            fpsResults.addResult(inputData.styleNames[style], fpsStore)
                            fpsStore.reset()

                            println("FPS results $fpsResults")
                        }
                        println("Benchmark ${jsonPayload(inputData.styleNames, fpsResults, encodingTimeResults, renderingTimeResults)}")

                        if (style < inputData.styleURLs.size - 1) {  // continue with next style
                            maplibreMap.setStyle(inputData.styleURLs[style + 1])
                            flyTo(maplibreMap, 0, style + 1, zoom)
                        } else if (runsLeft > 0) {  // start over
                            --runsLeft
                            maplibreMap.setStyle(inputData.styleURLs[0])
                            flyTo(maplibreMap, 0, 0, zoom)
                        } else {
                            benchmarkDone()
                        }
                        return
                    }

                    // continue with next place
                    flyTo(maplibreMap, place + 1, style, zoom)
                }
            }
        )
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

    private fun sendResults() {
        val api = inputData.resultsAPI
        if (api.isEmpty()) {
            Logger.i(TAG, "Not sending results to API")
            return
        }

        val client = OkHttpClient()

        val payload = jsonPayload(inputData.styleNames, fpsResults, encodingTimeResults, renderingTimeResults)
        Logger.i(TAG, "Sending JSON payload to API: $payload")

        val request = Request.Builder()
            .url(api)
            .post(
                Json.encodeToString(payload).toRequestBody("application/json".toMediaType()))
            .build()
        client.newCall(request).execute()
    }

    private fun benchmarkDone() {
        sendResults()
        setResult(Activity.RESULT_OK)
        finish()
    }

    companion object {
        private val PLACES = arrayOf(
            LatLng(37.7749, -122.4194), // SF
            LatLng(38.9072, -77.0369), // DC
            LatLng(52.3702, 4.8952), // AMS
            LatLng(60.1699, 24.9384), // HEL
            LatLng(-13.1639, -74.2236), // AYA
            LatLng(52.5200, 13.4050), // BER
            LatLng(12.9716, 77.5946), // BAN
            LatLng(31.2304, 121.4737) // SHA
        )
    }
}
