package org.maplibre.android.testapp.activity.telemetry

import android.app.ActivityManager
import android.os.Build
import android.os.Bundle
import android.util.DisplayMetrics
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import com.google.gson.Gson
import com.google.gson.JsonObject
import org.maplibre.android.MapLibre
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.module.http.HttpRequestUtil
import org.maplibre.android.testapp.R
import okhttp3.Call
import okhttp3.OkHttpClient
import okhttp3.OkHttpClient.Builder
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber
import java.util.*

/**
 * Test activity showcasing gathering performance measurement data.
 */
class PerformanceMeasurementActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_simple)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        val eventListener = EventListener()
        val okHttpClient: OkHttpClient = Builder()
            .eventListener(eventListener)
            .build()
        HttpRequestUtil.setOkHttpClient(okHttpClient)
        mapView.getMapAsync(
            OnMapReadyCallback { maplibreMap: MapLibreMap ->
                maplibreMap.setStyle(
                    Style.Builder().fromUri(TestStyles.getPredefinedStyleWithFallback("Streets"))
                )
            }
        )
    }

    override fun onStart() {
        super.onStart()
        mapView!!.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView!!.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView!!.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView!!.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }

    override fun onDestroy() {
        HttpRequestUtil.setOkHttpClient(null)
        super.onDestroy()
        mapView!!.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView!!.onSaveInstanceState(outState)
    }

    private class EventListener : okhttp3.EventListener() {
        private val startTimes: MutableMap<String, Long> = HashMap()
        override fun callStart(call: Call) {
            val url = call.request().url.toString()
            startTimes[url] = System.nanoTime()
            super.callStart(call)
            Timber.e("callStart: %s", url)
        }

        override fun callEnd(call: Call) {
            val url = call.request().url.toString()
            Timber.e("callEnd: %s", url)
            val start = startTimes[url]
            if (start != null) {
                val elapsed = System.nanoTime() - start
                triggerPerformanceEvent(url.substring(0, url.indexOf('?')), elapsed)
                startTimes.remove(url)
                Timber.e("callEnd: %s took %d", url, elapsed)
            }
            super.callEnd(call)
        }
    }

    private class Attribute<T> internal constructor(private val name: String, private val value: T)
    companion object {
        private fun triggerPerformanceEvent(style: String, elapsed: Long) {
            val attributes: MutableList<Attribute<String>> = ArrayList()
            attributes.add(
                Attribute("style_id", style)
            )
            attributes.add(
                Attribute("test_perf_event", "true")
            )
            val counters: MutableList<Attribute<Long>> = ArrayList()
            counters.add(Attribute("elapsed", elapsed))
            val metaData = JsonObject()
            metaData.addProperty("os", "android")
            metaData.addProperty("manufacturer", Build.MANUFACTURER)
            metaData.addProperty("brand", Build.BRAND)
            metaData.addProperty("device", Build.MODEL)
            metaData.addProperty("version", Build.VERSION.RELEASE)
            metaData.addProperty("abi", Build.CPU_ABI)
            metaData.addProperty("country", Locale.getDefault().isO3Country)
            metaData.addProperty("ram", ram)
            metaData.addProperty("screenSize", windowSize)
            val gson = Gson()
            val bundle = Bundle()
            bundle.putString("attributes", gson.toJson(attributes))
            bundle.putString("counters", gson.toJson(counters))
            bundle.putString("metadata", metaData.toString())
        }

        private val ram: String
            private get() {
                val actManager = MapLibre.getApplicationContext()
                    .getSystemService(ACTIVITY_SERVICE) as ActivityManager
                val memInfo = ActivityManager.MemoryInfo()
                actManager.getMemoryInfo(memInfo)
                return memInfo.totalMem.toString()
            }
        private val windowSize: String
            private get() {
                val windowManager =
                    MapLibre.getApplicationContext().getSystemService(WINDOW_SERVICE) as WindowManager
                val display = windowManager.defaultDisplay
                val metrics = DisplayMetrics()
                display.getMetrics(metrics)
                val width = metrics.widthPixels
                val height = metrics.heightPixels
                return "{$width,$height}"
            }
    }
}
