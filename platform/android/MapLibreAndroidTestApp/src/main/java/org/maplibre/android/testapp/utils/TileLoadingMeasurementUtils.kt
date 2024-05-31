package org.maplibre.android.testapp.utils

import android.app.ActivityManager
import android.content.Context
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.Build
import android.os.Bundle
import android.util.DisplayMetrics
import android.view.WindowManager
import androidx.annotation.StringDef
import com.google.gson.Gson
import com.google.gson.JsonObject
import org.maplibre.android.MapStrictMode
import org.maplibre.android.MapLibre
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.module.http.HttpRequestUtil
import okhttp3.Interceptor
import okhttp3.Interceptor.Chain
import okhttp3.OkHttpClient
import okhttp3.OkHttpClient.Builder
import okhttp3.Request
import okhttp3.Response
import timber.log.Timber
import java.io.IOException
import java.util.*

class TileLoadingMeasurementUtils {

    companion object {

        private const val ATTRIBUTE_REQUEST_URL = "requestUrl"
        fun setUpTileLoadingMeasurement() {
            if (isTileLoadingMeasurementOn) {
                val okHttpClient: OkHttpClient = Builder()
                    .addNetworkInterceptor(TileLoadingInterceptor())
                    .build()
                HttpRequestUtil.setOkHttpClient(okHttpClient)
            }
        }

        private val isTileLoadingMeasurementOn: Boolean
            private get() = isBooleanMetaDataValueOn(
                MapLibreConstants.KEY_META_DATA_MEASURE_TILE_DOWNLOAD_ON,
                MapLibreConstants.DEFAULT_MEASURE_TILE_DOWNLOAD_ON
            )

        private fun isBooleanMetaDataValueOn(propKey: String, defaultValue: Boolean): Boolean {
            try {
                // Try getting a custom value from the app Manifest
                val context = MapLibre.getApplicationContext()
                val appInfo = context.packageManager.getApplicationInfo(
                    context.packageName,
                    PackageManager.GET_META_DATA
                )
                if (appInfo.metaData != null) {
                    return appInfo.metaData.getBoolean(propKey, defaultValue)
                }
            } catch (exception: PackageManager.NameNotFoundException) {
                Timber.e("Failed to read the package metadata: $exception")
                MapStrictMode.strictModeViolation(exception)
            } catch (exception: Exception) {
                Timber.e("Failed to read key: $propKey $exception")
                MapStrictMode.strictModeViolation(exception)
            }
            return defaultValue
        }

        /**
         * This Interceptor allows to measure time spent getting a response object over network.
         * The following data will be collected:
         * responseCode, elapsedMS
         * requestUrl (request string till the question mark),
         * and device metadata.
         */
        internal class TileLoadingInterceptor : Interceptor {
            @StringDef(*[CONNECTION_NONE, CONNECTION_CELLULAR, CONNECTION_WIFI])
            @kotlin.annotation.Retention(AnnotationRetention.SOURCE)
            internal annotation class ConnectionState

            @Throws(IOException::class)
            override fun intercept(chain: Chain): Response {
                val request: Request = chain.request()
                var elapsed = System.nanoTime()
                val response: Response = chain.proceed(request)
                elapsed = System.nanoTime() - elapsed
                triggerPerformanceEvent(response, elapsed / 1000000)
                return response
            }

            private fun triggerPerformanceEvent(response: Response, elapsedMs: Long) {
                val attributes: MutableList<Attribute<String>> = ArrayList()
                val request = getUrl(response.request)
                attributes.add(Attribute("requestUrl", request))
                attributes.add(Attribute("responseCode", response.code.toString()))
                attributes.add(
                    Attribute("connectionState", connectionState)
                )
                val counters: MutableList<Attribute<Long>?> = ArrayList()
                counters.add(Attribute("elapsedMS", elapsedMs))
                val bundle = Bundle()
                val gson = Gson()
                bundle.putString("attributes", gson.toJson(attributes))
                bundle.putString("counters", gson.toJson(counters))
                bundle.putString("metadata", metadata)
            }

            companion object {
                private var metadata: String? = null
                    private get() {
                        if (field == null) {
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
                            field = metaData.toString()
                        }
                        return field
                    }
                private const val CONNECTION_NONE = "none"
                private const val CONNECTION_CELLULAR = "cellular"
                private const val CONNECTION_WIFI = "wifi"
                private fun getUrl(request: Request): String {
                    val url = request.url.toString()
                    return url.substring(0, url.indexOf('?'))
                }

                private val ram: String
                    private get() {
                        val actManager = MapLibre.getApplicationContext()
                            .getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
                        val memInfo = ActivityManager.MemoryInfo()
                        actManager.getMemoryInfo(memInfo)
                        return memInfo.totalMem.toString()
                    }
                private val windowSize: String
                    private get() {
                        val windowManager = MapLibre.getApplicationContext()
                            .getSystemService(Context.WINDOW_SERVICE) as WindowManager
                        val display = windowManager.defaultDisplay
                        val metrics = DisplayMetrics()
                        display.getMetrics(metrics)
                        val width = metrics.widthPixels
                        val height = metrics.heightPixels
                        return "{$width,$height}"
                    }

                @get:ConnectionState
                private val connectionState: String
                    private get() {
                        val appContext = MapLibre.getApplicationContext()
                        val connectivityManager =
                            appContext.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                            if (connectivityManager != null) {
                                val capabilities =
                                    connectivityManager.getNetworkCapabilities(connectivityManager.activeNetwork)
                                if (capabilities != null) {
                                    if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                                        return CONNECTION_WIFI
                                    } else if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR)) {
                                        return CONNECTION_CELLULAR
                                    }
                                }
                            }
                        } else {
                            if (connectivityManager != null) {
                                val activeNetwork = connectivityManager.activeNetworkInfo
                                if (activeNetwork != null) {
                                    if (activeNetwork.type == ConnectivityManager.TYPE_WIFI) {
                                        return CONNECTION_WIFI
                                    } else if (activeNetwork.type == ConnectivityManager.TYPE_MOBILE) {
                                        return CONNECTION_CELLULAR
                                    }
                                }
                            }
                        }
                        return CONNECTION_NONE
                    }
            }
        }

        private class Attribute<T> internal constructor(private val name: String, private val value: T)
    }
}
