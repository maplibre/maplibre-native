package com.mapbox.mapboxsdk.testapp

import android.os.StrictMode
import android.os.StrictMode.ThreadPolicy
import android.os.StrictMode.VmPolicy
import android.text.TextUtils
import androidx.multidex.MultiDexApplication
import com.mapbox.mapboxsdk.MapStrictMode
import com.mapbox.mapboxsdk.Mapbox
import com.mapbox.mapboxsdk.WellKnownTileServer
import com.mapbox.mapboxsdk.log.Logger
import com.mapbox.mapboxsdk.testapp.utils.ApiKeyUtils
import com.mapbox.mapboxsdk.testapp.utils.TileLoadingMeasurementUtils
import com.mapbox.mapboxsdk.testapp.utils.TimberLogger
import timber.log.Timber
import timber.log.Timber.DebugTree

/**
 * Application class of the test application.
 *
 *
 * Initialises components as LeakCanary, Strictmode, Timber and Mapbox
 *
 */
class MapLibreApplication : MultiDexApplication() {
    override fun onCreate() {
        super.onCreate()
        initializeLogger()
        initializeStrictMode()
        initializeMapbox()
    }

    private fun initializeLogger() {
        Logger.setLoggerDefinition(TimberLogger())
        if (BuildConfig.DEBUG) {
            Timber.plant(DebugTree())
        }
    }

    private fun initializeStrictMode() {
        StrictMode.setThreadPolicy(
            ThreadPolicy.Builder()
                .detectDiskReads()
                .detectDiskWrites()
                .detectNetwork()
                .penaltyLog()
                .build()
        )
        StrictMode.setVmPolicy(
            VmPolicy.Builder()
                .detectLeakedSqlLiteObjects()
                .penaltyLog()
                .penaltyDeath()
                .build()
        )
    }

    private fun initializeMapbox() {
        val apiKey = ApiKeyUtils.getApiKey(applicationContext)
        validateApiKey(apiKey)
        Mapbox.getInstance(applicationContext, apiKey, TILE_SERVER)
        TileLoadingMeasurementUtils.setUpTileLoadingMeasurement()
        MapStrictMode.setStrictModeEnabled(true)
    }

    companion object {
        val TILE_SERVER = WellKnownTileServer.MapTiler
        private const val DEFAULT_API_KEY = "YOUR_API_KEY_GOES_HERE"
        private const val API_KEY_NOT_SET_MESSAGE =
            (
                "In order to run the Test App you need to set a valid " +
                    "API key. During development, you can set the MGL_API_KEY environment variable for the SDK to " +
                    "automatically include it in the Test App. Otherwise, you can manually include it in the " +
                    "res/values/developer-config.xml file in the MapboxGLAndroidSDKTestApp folder."
                )

        private fun validateApiKey(apiKey: String) {
            if (TextUtils.isEmpty(apiKey) || apiKey == DEFAULT_API_KEY) {
                Timber.e(API_KEY_NOT_SET_MESSAGE)
            }
        }
    }
}
