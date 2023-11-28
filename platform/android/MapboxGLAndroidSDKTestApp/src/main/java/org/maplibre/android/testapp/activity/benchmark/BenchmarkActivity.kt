package org.maplibre.android.testapp.activity.benchmark

import android.annotation.SuppressLint
import android.os.Bundle
import android.os.Handler
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import kotlinx.serialization.Serializable
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.*
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.testapp.R
import java.util.*

class FpsStore {
    private val fpsValues = ArrayList<Double>(100000)

    fun add(fps: Double) {
        fpsValues.add(fps)
    }

    fun reset() {
        fpsValues.clear()
    }

    fun low1p(): Double {
        fpsValues.sort()
        return fpsValues.slice(0..(fpsValues.size / 100)).average()
    }

    fun average(): Double {
        return fpsValues.average()
    }
}

data class Result(val average: Double, val low1p: Double)

@Serializable
data class Results(
    var map: MutableMap<String, List<Result>> = mutableMapOf<String, List<Result>>().withDefault { emptyList() })
    {

    fun addResult(styleName: String, fpsStore: FpsStore) {
        val newResults = map.getValue(styleName).plus(Result(fpsStore.average(), fpsStore.low1p()))
        map[styleName] = newResults
    }
}

/**
 * Benchmark using a [android.view.TextureView]
 */
class BenchmarkActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var handler: Handler? = null
    private var delayed: Runnable? = null
    private var fpsStore = FpsStore()
    private var results = Results()
    private var runsLeft = 5

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
    @SuppressLint("DiscouragedApi")
    private var styles: List<Pair<String, String>> = run {
        val default = listOf(Pair("Demotiles", "https://demotiles.maplibre.org/style.json"))
        val styleNames = resources.getStringArray(applicationContext.resources.getIdentifier(
            "benchmark_style_names",
            "array",
            applicationContext.packageName))
        val styleURLs = resources.getStringArray(applicationContext.resources.getIdentifier(
            "benchmark_style_urls",
            "array",
            applicationContext.packageName))

        if (styleNames.isNotEmpty() && styleNames.size == styleURLs.size)
            styleNames.zip(styleURLs)
        else
            default
    }

    @SuppressLint("DiscouragedApi")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_benchmark)
        handler = Handler(mainLooper)
        setupToolbar()
        sendResults()
        setupMapView(savedInstanceState)
    }

    private fun setupToolbar() {
        val actionBar = supportActionBar
        if (actionBar != null) {
            supportActionBar!!.setDisplayHomeAsUpEnabled(true)
            supportActionBar!!.setHomeButtonEnabled(true)
        }
    }

    private fun setupMapView(savedInstanceState: Bundle?) {
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.getMapAsync { maplibreMap: MapLibreMap ->
            this@BenchmarkActivity.maplibreMap = maplibreMap
            maplibreMap.setStyle(styles[0].second)
            setFpsView(maplibreMap)

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
                        results.addResult(styles[style].first, fpsStore)
                        fpsStore.reset()

                        println("FPS results $results")

                        if (style < styles.size - 1) {  // continue with next style
                            maplibreMap.setStyle(styles[style + 1].second)
                            flyTo(maplibreMap, 0, style + 1, zoom)
                        } else if (runsLeft > 0) {  // start over
                            --runsLeft
                            maplibreMap.setStyle(styles[0].second)
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

    private fun setFpsView(maplibreMap: MapLibreMap) {
        maplibreMap.setOnFpsChangedListener { fps: Double ->
            fpsStore.add(fps)
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

    @SuppressLint("DiscouragedApi")
    fun sendResults() {
        val client = OkHttpClient()

        val api = resources.getString(applicationContext.resources.getIdentifier(
            "benchmark_results_api",
            "string",
            applicationContext.packageName))
        if (api.isEmpty()) {
            Logger.i("BenchMarkActivity", "No benchmark_results_api set in developer-config.xml")
            return
        }
        val request = Request.Builder()
            .url(api)
            .post(
                Json.encodeToString(results).toRequestBody("application/json".toMediaType()))
            .build()
        client.newCall(request).execute()
    }

    fun benchmarkDone() {
        sendResults()
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
