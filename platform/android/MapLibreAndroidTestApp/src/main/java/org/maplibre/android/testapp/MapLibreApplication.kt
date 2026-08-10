package org.maplibre.android.testapp

import android.os.StrictMode
import android.os.StrictMode.ThreadPolicy
import android.os.StrictMode.VmPolicy
import android.text.TextUtils
import android.widget.Toast
import androidx.multidex.MultiDexApplication
import org.maplibre.android.MapStrictMode
import org.maplibre.android.MapLibre
import org.maplibre.android.WellKnownTileServer
import org.maplibre.android.log.Logger
import org.maplibre.android.testapp.utils.ApiKeyUtils
import org.maplibre.android.testapp.utils.RenderingEnginePreference
import org.maplibre.android.testapp.utils.TileLoadingMeasurementUtils
import org.maplibre.android.testapp.utils.TimberLogger
import timber.log.Timber
import timber.log.Timber.DebugTree

/**
 * Application class of the test application.
 *
 *
 * Initialises components as LeakCanary, Strictmode, Timber and MapLibre
 *
 */
open class MapLibreApplication : MultiDexApplication() {
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
                .detectLeakedClosableObjects()
                .penaltyLog()
                .penaltyDeath()
                .build()
        )
    }

    private fun initializeMapbox() {
        val apiKey = ApiKeyUtils.getApiKey(applicationContext)
        if (apiKey != null) {
            validateApiKey(apiKey)
        }
        initializeMapLibre(apiKey)
        TileLoadingMeasurementUtils.setUpTileLoadingMeasurement()
        MapStrictMode.setStrictModeEnabled(true)
    }

    // The rendering engine is bound to the MapLibre instance at construction time, so a
    // persisted choice has to be passed into getInstance() itself rather than set
    // separately beforehand. A build installed over a different flavor (same
    // applicationId across flavors) may have a persisted choice the compiled-in backend
    // doesn't support, so an unsupported choice can't be allowed to crash startup — fall
    // back to the default engine instead.
    //
    // If nothing is persisted yet (first launch, or preference cleared), MapLibre.getInstance()
    // itself selects Vulkan automatically when this device's hardware supports it.
    private fun initializeMapLibre(apiKey: String?) {
        val persistedType = RenderingEnginePreference.load(this)
        try {
            MapLibre.getInstance(applicationContext, apiKey, TILE_SERVER, persistedType)
        } catch (error: UnsupportedOperationException) {
            RenderingEnginePreference.clear(this)

            Timber.w(error, "Rendering engine %s is not supported by this build", persistedType)
            Toast.makeText(
                this,
                getString(R.string.rendering_engine_unsupported, persistedType?.name),
                Toast.LENGTH_LONG
            ).show()
            MapLibre.getInstance(applicationContext, apiKey, TILE_SERVER)
        }
    }

    companion object {
        val TILE_SERVER = WellKnownTileServer.MapLibre
        private const val DEFAULT_API_KEY = "YOUR_API_KEY_GOES_HERE"
        private const val API_KEY_NOT_SET_MESSAGE =
            (
                "In order to run the Test App you need to set a valid " +
                    "API key. During development, you can set the MLN_API_KEY environment variable for the SDK to " +
                    "automatically include it in the Test App. Otherwise, you can manually include it in the " +
                    "res/values/developer-config.xml file in the MapLibreAndroidTestApp folder."
                )

        private fun validateApiKey(apiKey: String) {
            if (TextUtils.isEmpty(apiKey) || apiKey == DEFAULT_API_KEY) {
                Timber.e(API_KEY_NOT_SET_MESSAGE)
            }
        }
    }
}
