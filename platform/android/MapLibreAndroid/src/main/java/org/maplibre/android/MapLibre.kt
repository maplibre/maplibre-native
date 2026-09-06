package org.maplibre.android

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.AssetManager
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.exceptions.MapLibreConfigurationException
import org.maplibre.android.net.ConnectivityReceiver
import org.maplibre.android.storage.FileSource
import org.maplibre.android.util.DefaultStyle
import org.maplibre.android.util.TileServerOptions
import org.maplibre.android.utils.ThreadUtils
import timber.log.Timber

/**
 * The entry point to initialize the MapLibre Android SDK.
 *
 * Obtain a reference by calling [getInstance].
 * Usually this class is configured in Application#onCreate() and is responsible for the
 * active API key, application context, and connectivity state.
 */
@UiThread
@SuppressLint("StaticFieldLeak")
@Keep
class MapLibre internal constructor(
    private val context: Context,
    private var apiKey: String?,
) {
    private var tileServerOptions: TileServerOptions? = null

    internal constructor(
        context: Context,
        apiKey: String?,
        options: TileServerOptions,
    ) : this(context, apiKey) {
        tileServerOptions = options
    }

    companion object {
        private const val TAG = "Mbgl-MapLibre"
        private var moduleProvider: ModuleProvider? = null

        @Suppress("ktlint:standard:property-naming")
        private var INSTANCE: MapLibre? = null

        /**
         * Get an instance of MapLibre.
         *
         * This class manages the API key, application context, and connectivity state.
         *
         * @param context Android context which holds or is an application context
         * @return the single instance of MapLibre
         */
        @UiThread
        @JvmStatic
        @Synchronized
        fun getInstance(context: Context): MapLibre = getInstance(context, null, WellKnownTileServer.MapLibre)

        /**
         * Get an instance of MapLibre.
         *
         * This class manages the API key, application context, and connectivity state.
         *
         * @param context Android context which holds or is an application context
         * @param apiKey api key
         * @param tileServer the tile server whose predefined configuration will be used to
         *                   bootstrap the SDK. The predefined configuration includes
         *                   rules for converting resource URLs between normal and canonical forms
         *                   and set of predefined styles available on the server.
         * @return the single instance of MapLibre
         */
        @UiThread
        @JvmStatic
        @Synchronized
        fun getInstance(
            context: Context,
            apiKey: String?,
            tileServer: WellKnownTileServer,
        ): MapLibre = getInstance(context, apiKey, tileServer, null)

        /**
         * Get an instance of MapLibre, explicitly picking the rendering engine to use.
         *
         * This class manages the API key, application context, and connectivity state.
         *
         * @param context Android context which holds or is an application context
         * @param apiKey api key
         * @param tileServer the tile server whose predefined configuration will be used to
         *                   bootstrap the SDK. The predefined configuration includes
         *                   rules for converting resource URLs between normal and canonical forms
         *                   and set of predefined styles available on the server.
         * @param type the rendering engine to use; ignored if MapLibre has already been
         *             initialized in this process. If null, the default rendering engine will be used.
         * @return the single instance of MapLibre
         * @throws UnsupportedOperationException if [type] isn't supported by this
         *     build of the SDK (single-backend flavors only support their compiled-in backend).
         */
        @UiThread
        @JvmStatic
        @Synchronized
        fun getInstance(
            context: Context,
            apiKey: String?,
            tileServer: WellKnownTileServer,
            type: RenderingEngine.Type?,
        ): MapLibre {
            ThreadUtils.init(context)
            ThreadUtils.checkThread(TAG)
            val instance = INSTANCE
            if (instance == null) {
                Timber.plant()
                val appContext = context.applicationContext
                RenderingEngine.setCurrentType(type ?: RenderingEngine.getDefaultRenderingEngine(appContext))
                FileSource.initializeFileDirsPaths(appContext)
                INSTANCE = MapLibre(appContext, apiKey)
                ConnectivityReceiver.instance(appContext)
            } else {
                instance.apiKey = apiKey
            }

            val tileServerOptions = TileServerOptions.get(tileServer)
            INSTANCE!!.tileServerOptions = tileServerOptions
            val fileSource = FileSource.getInstance(context)
            fileSource.setTileServerOptions(tileServerOptions)
            fileSource.setApiKey(apiKey)
            return INSTANCE!!
        }

        /**
         * Get the current active API key for this application.
         *
         * @return API key
         */
        @JvmStatic
        fun getApiKey(): String? {
            validateMapLibre()
            return INSTANCE!!.apiKey
        }

        /**
         * Set the current active apiKey.
         */
        @JvmStatic
        fun setApiKey(apiKey: String?) {
            validateMapLibre()
            throwIfApiKeyInvalid(apiKey)
            INSTANCE!!.apiKey = apiKey
            FileSource.getInstance(getApplicationContext()).setApiKey(apiKey)
        }

        /**
         * Get tile server configuration.
         */
        @JvmStatic
        fun getTileServerOptions(): TileServerOptions? {
            validateMapLibre()
            return INSTANCE!!.tileServerOptions
        }

        /**
         * Get all pre-defined styles
         *
         * @return Array of predefined styles
         */
        @JvmStatic
        fun getPredefinedStyles(): Array<DefaultStyle>? {
            validateMapLibre()
            return INSTANCE!!.tileServerOptions?.defaultStyles
        }

        /**
         * Get predefined style by name
         *
         * @return Predefined style if found
         */
        @JvmStatic
        fun getPredefinedStyle(name: String?): DefaultStyle? {
            validateMapLibre()
            val styles = INSTANCE!!.tileServerOptions?.defaultStyles ?: return null
            return styles.firstOrNull { it.name.equals(name, ignoreCase = true) }
        }

        /**
         * Application context
         *
         * @return the application context
         */
        @JvmStatic
        fun getApplicationContext(): Context {
            validateMapLibre()
            return INSTANCE!!.context
        }

        /**
         * Manually sets the connectivity state of the app. This is useful for apps which control their
         * own connectivity state and want to bypass any checks to the ConnectivityManager.
         *
         * @param connected flag to determine the connectivity state, true for connected, false for
         *                  disconnected, and null for ConnectivityManager to determine.
         */
        @JvmStatic
        @Synchronized
        fun setConnected(connected: Boolean?) {
            validateMapLibre()
            ConnectivityReceiver.instance(INSTANCE!!.context).setConnected(connected)
        }

        /**
         * Determines whether we have an internet connection available. Please do not rely on this
         * method in your apps. This method is used internally by the SDK.
         *
         * @return true if there is an internet connection, false otherwise
         */
        @JvmStatic
        @Synchronized
        fun isConnected(): Boolean {
            validateMapLibre()
            return ConnectivityReceiver.instance(INSTANCE!!.context).isConnected
        }

        /**
         * Get the module provider
         *
         * @return moduleProvider
         */
        @JvmStatic
        fun getModuleProvider(): ModuleProvider = moduleProvider ?: ModuleProviderImpl().also { moduleProvider = it }

        /**
         * Set the module provider. Call this as soon as possible.
         * @param provider The ModuleProvider instance to set
         */
        @JvmStatic
        fun setModuleProvider(provider: ModuleProvider?) {
            moduleProvider = provider
        }

        /**
         * Runtime validation of MapLibre creation.
         */
        private fun validateMapLibre() {
            if (INSTANCE == null) {
                throw MapLibreConfigurationException()
            }
        }

        /**
         * Runtime validation of MapLibre access token
         *
         * @param apiKey the access token to validate
         * @return true is valid, false otherwise
         */
        @JvmStatic
        internal fun isApiKeyValid(apiKey: String?): Boolean {
            if (apiKey == null) {
                return false
            }
            return apiKey.trim().lowercase(MapLibreConstants.MAPLIBRE_LOCALE).isNotEmpty()
        }

        /**
         * Throws exception when access token is invalid
         */
        @JvmStatic
        fun throwIfApiKeyInvalid(apiKey: String?) {
            if (!isApiKeyValid(apiKey)) {
                throw MapLibreConfigurationException(
                    "A valid API key is required, currently provided key is: $apiKey",
                )
            }
        }

        /**
         * Internal use. Check if the [MapLibre.INSTANCE] is present.
         */
        @JvmStatic
        fun hasInstance(): Boolean = INSTANCE != null

        /**
         * Internal use. Returns AssetManager. Called from JNI.
         *
         * @return the asset manager
         */
        @JvmStatic
        private fun getAssetManager(): AssetManager = getApplicationContext().resources.assets
    }
}
