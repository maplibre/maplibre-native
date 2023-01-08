package com.mapbox.mapboxsdk.testapp.activity.telemetry

import okhttp3.OkHttpClient.Builder.eventListener
import okhttp3.OkHttpClient.Builder.build
import okhttp3.Call.request
import okhttp3.Request.url
import okhttp3.HttpUrl.toString
import okhttp3.EventListener.callStart
import okhttp3.EventListener.callEnd
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.maps.MapView
import android.os.Bundle
import com.mapbox.mapboxsdk.testapp.R
import okhttp3.OkHttpClient
import com.mapbox.mapboxsdk.module.http.HttpRequestUtil
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.MapboxMap
import timber.log.Timber
import com.mapbox.mapboxsdk.testapp.activity.telemetry.PerformanceMeasurementActivity
import com.google.gson.Gson
import android.app.ActivityManager
import android.os.Build
import com.mapbox.mapboxsdk.Mapbox
import android.view.WindowManager
import android.util.DisplayMetrics
import com.google.gson.JsonObject
import com.mapbox.mapboxsdk.maps.Style
import okhttp3.Call
import okhttp3.EventListener
import java.util.*

/**
 * Test activity showcasing gathering performance measurement data.
 */
class PerformanceMeasurementActivity : AppCompatActivity() {
    private var mapView: MapView? = null
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
        mapView.getMapAsync(OnMapReadyCallback { mapboxMap: MapboxMap ->
            mapboxMap.setStyle(
                Style.Builder().fromUri(Style.getPredefinedStyle("Streets"))
            )
        })
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
                startTimes.remove(start)
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
                val actManager = Mapbox.getApplicationContext()
                    .getSystemService(ACTIVITY_SERVICE) as ActivityManager
                val memInfo = ActivityManager.MemoryInfo()
                actManager.getMemoryInfo(memInfo)
                return memInfo.totalMem.toString()
            }
        private val windowSize: String
            private get() {
                val windowManager =
                    Mapbox.getApplicationContext().getSystemService(WINDOW_SERVICE) as WindowManager
                val display = windowManager.defaultDisplay
                val metrics = DisplayMetrics()
                display.getMetrics(metrics)
                val width = metrics.widthPixels
                val height = metrics.heightPixels
                return "{$width,$height}"
            }
    }
}