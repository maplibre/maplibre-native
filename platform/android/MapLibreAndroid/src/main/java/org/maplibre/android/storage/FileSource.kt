package org.maplibre.android.storage

import android.content.Context
import android.content.pm.PackageManager
import android.os.AsyncTask
import android.os.Environment
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.MapLibre
import org.maplibre.android.MapStrictMode
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.log.Logger
import org.maplibre.android.util.TileServerOptions
import org.maplibre.android.utils.FileUtils
import org.maplibre.android.utils.ThreadUtils
import java.io.File
import java.util.concurrent.locks.Lock
import java.util.concurrent.locks.ReentrantLock

/**
 * Holds a central reference to the core's DefaultFileSource for as long as
 * there are active mapviews / offline managers
 */
class FileSource private constructor(
    cachePath: String,
) {
    @Keep
    private val nativePtr: Long = 0

    init {
        val options = MapLibre.getTileServerOptions()
        initialize(MapLibre.getApiKey(), cachePath, options)
    }

    /**
     * This callback allows implementors to transform URLs before they are requested
     * from the internet. This can be used add or remove custom parameters, or reroute
     * certain requests to other servers or endpoints.
     */
    @Keep
    interface ResourceTransformCallback {
        /**
         * Called whenever a URL needs to be transformed.
         *
         * @param kind the kind of URL to be transformed.
         * @param url  the  URL to be transformed
         * @return a URL that will now be downloaded.
         */
        fun onURL(
            @Resource.Kind kind: Int,
            url: String,
        ): String
    }

    /**
     * This callback receives an asynchronous response containing the new path of the
     * resources cache database.
     */
    @Keep
    interface ResourcesCachePathChangeCallback {
        /**
         * Receives the new database path
         *
         * @param path the path of the current resources cache database
         */
        fun onSuccess(path: String)

        /**
         * Receives an error message if setting the path was not successful
         *
         * @param message the error message
         */
        fun onError(message: String)
    }

    @Keep
    external fun setTileServerOptions(tileServerOptions: TileServerOptions?)

    @Keep
    external fun isActivated(): Boolean

    @Keep
    external fun activate()

    @Keep
    external fun deactivate()

    @Keep
    external fun setApiKey(apiKey: String?)

    @Keep
    external fun getApiKey(): String

    @Keep
    external fun setApiBaseUrl(baseUrl: String?)

    @Keep
    external fun getApiBaseUrl(): String

    /**
     * Sets a callback for transforming URLs requested from the internet
     *
     * The callback will be executed on the main thread once for every requested URL.
     *
     * @param callback the callback to be invoked or null to reset
     */
    @Keep
    external fun setResourceTransform(callback: ResourceTransformCallback?)

    @Keep
    private external fun setResourceCachePath(
        path: String,
        callback: ResourcesCachePathChangeCallback,
    )

    @Keep
    private external fun initialize(
        apiKey: String?,
        cachePath: String,
        options: TileServerOptions?,
    )

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    companion object {
        private const val TAG = "Mbgl-FileSource"
        private const val MAPBOX_SHARED_PREFERENCE_RESOURCES_CACHE_PATH = "fileSourceResourcesCachePath"
        private val resourcesCachePathLoaderLock: Lock = ReentrantLock()
        private val internalCachePathLoaderLock: Lock = ReentrantLock()
        private var resourcesCachePath: String? = null
        private var internalCachePath: String? = null

        // File source instance is kept alive after initialization
        @Suppress("ktlint:standard:property-naming")
        private var INSTANCE: FileSource? = null

        /**
         * Get the single instance of FileSource.
         *
         * @param context the context to derive the cache path from
         * @return the single instance of FileSource
         */
        @JvmStatic
        @UiThread
        @Synchronized
        fun getInstance(context: Context): FileSource {
            var instance = INSTANCE
            if (instance == null) {
                instance = FileSource(getResourcesCachePath(context))
                INSTANCE = instance
            }

            return instance
        }

        /**
         * Get files directory path for a context.
         *
         * @param context the context to derive the files directory path from
         * @return the files directory path
         */
        private fun getCachePath(context: Context): String {
            val preferences =
                context.getSharedPreferences(
                    MapLibreConstants.MAPLIBRE_SHARED_PREFERENCES,
                    Context.MODE_PRIVATE,
                )
            var cachePath = preferences.getString(MAPBOX_SHARED_PREFERENCE_RESOURCES_CACHE_PATH, null)

            if (!isPathWritable(cachePath)) {
                // Use default path
                cachePath = getDefaultCachePath(context)

                // Reset stored cache path
                val editor =
                    context
                        .getSharedPreferences(
                            MapLibreConstants.MAPLIBRE_SHARED_PREFERENCES,
                            Context.MODE_PRIVATE,
                        ).edit()
                editor.remove(MAPBOX_SHARED_PREFERENCE_RESOURCES_CACHE_PATH).apply()
            }

            return cachePath!!
        }

        /**
         * Get the default resources cache path depending on the external storage configuration
         *
         * @param context the context to derive the files directory path from
         * @return the default directory path
         */
        private fun getDefaultCachePath(context: Context): String {
            if (isExternalStorageConfiguration(context) && isExternalStorageReadable()) {
                val externalFilesDir = context.getExternalFilesDir(null)
                if (externalFilesDir != null) {
                    return externalFilesDir.absolutePath
                }
            }
            return context.filesDir.absolutePath
        }

        private fun isExternalStorageConfiguration(context: Context): Boolean {
            // Default value
            var isExternalStorageConfiguration = MapLibreConstants.DEFAULT_SET_STORAGE_EXTERNAL

            try {
                // Try getting a custom value from the app Manifest
                val appInfo =
                    context.packageManager.getApplicationInfo(
                        context.packageName,
                        PackageManager.GET_META_DATA,
                    )
                if (appInfo.metaData != null) {
                    isExternalStorageConfiguration =
                        appInfo.metaData.getBoolean(
                            MapLibreConstants.KEY_META_DATA_SET_STORAGE_EXTERNAL,
                            MapLibreConstants.DEFAULT_SET_STORAGE_EXTERNAL,
                        )
                }
            } catch (exception: PackageManager.NameNotFoundException) {
                Logger.e(TAG, "Failed to read the package metadata: ", exception)
                MapStrictMode.strictModeViolation(exception)
            } catch (exception: Exception) {
                Logger.e(TAG, "Failed to read the storage key: ", exception)
                MapStrictMode.strictModeViolation(exception)
            }
            return isExternalStorageConfiguration
        }

        /**
         * Checks if external storage is available to at least read. In order for this to work, make
         * sure you include &lt;uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" /&gt;
         * (or WRITE_EXTERNAL_STORAGE) for API level &lt; 18 in your app Manifest.
         *
         * Code from https://developer.android.com/guide/topics/data/data-storage.html#filesExternal
         *
         * @return true if external storage is readable
         */
        @JvmStatic
        fun isExternalStorageReadable(): Boolean {
            val state = Environment.getExternalStorageState()
            if (Environment.MEDIA_MOUNTED == state || Environment.MEDIA_MOUNTED_READ_ONLY == state) {
                return true
            }

            Logger.w(
                TAG,
                "External storage was requested but it isn't readable. For API level < 18" +
                    " make sure you've requested READ_EXTERNAL_STORAGE or WRITE_EXTERNAL_STORAGE" +
                    " permissions in your app Manifest (defaulting to internal storage).",
            )

            return false
        }

        /**
         * Initializes file directories paths.
         *
         * @param context the context to derive paths from
         */
        @JvmStatic
        @UiThread
        fun initializeFileDirsPaths(context: Context) {
            ThreadUtils.checkThread(TAG)
            FileDirsPathsTask().execute(context)
        }

        @Suppress("DEPRECATION")
        private class FileDirsPathsTask : AsyncTask<Context, Void, Void?>() {
            override fun doInBackground(vararg contexts: Context): Void? {
                getResourcesCachePath(contexts[0])
                getInternalCachePath(contexts[0])
                return null
            }
        }

        /**
         * Get files directory path for a context.
         *
         * @param context the context to derive the files directory path from
         * @return the files directory path
         */
        @JvmStatic
        fun getResourcesCachePath(context: Context): String {
            resourcesCachePathLoaderLock.lock()
            try {
                var path = resourcesCachePath
                if (path == null) {
                    path = getCachePath(context)
                    resourcesCachePath = path
                }
                return path
            } finally {
                resourcesCachePathLoaderLock.unlock()
            }
        }

        /**
         * Get internal cache path for a context.
         *
         * @param context the context to derive the internal cache path from
         * @return the internal cache path
         */
        @JvmStatic
        fun getInternalCachePath(context: Context): String {
            internalCachePathLoaderLock.lock()
            try {
                var path = internalCachePath
                if (path == null) {
                    path = context.cacheDir.absolutePath
                    internalCachePath = path
                }
                return path
            } finally {
                internalCachePathLoaderLock.unlock()
            }
        }

        /**
         * Changes the path of the resources cache database.
         *
         * The callback reference is **strongly kept** throughout the process,
         * so it needs to be wrapped in a weak reference or released on the client side if necessary.
         *
         * @param context  the context of the path
         * @param path     the new database path
         * @param callback the callback to obtain the result
         */
        @Deprecated(
            "Use setResourcesCachePath(String, ResourcesCachePathChangeCallback) instead",
            ReplaceWith("setResourcesCachePath(path, callback)"),
        )
        @JvmStatic
        fun setResourcesCachePath(
            context: Context,
            path: String,
            callback: ResourcesCachePathChangeCallback,
        ) {
            setResourcesCachePath(path, callback)
        }

        /**
         * Changes the path of the resources cache database.
         *
         * The callback reference is **strongly kept** throughout the process,
         * so it needs to be wrapped in a weak reference or released on the client side if necessary.
         *
         * @param path     the new database path
         * @param callback the callback to obtain the result
         */
        @JvmStatic
        fun setResourcesCachePath(
            path: String,
            callback: ResourcesCachePathChangeCallback,
        ) {
            val applicationContext = MapLibre.getApplicationContext()
            // make sure the file source is initialized before the path is changed
            getInstance(applicationContext)

            if (path == getResourcesCachePath(applicationContext)) {
                // no need to change the path
                callback.onSuccess(path)
            } else {
                FileUtils
                    .CheckFileWritePermissionTask(
                        object : FileUtils.OnCheckFileWritePermissionListener {
                            override fun onWritePermissionGranted() {
                                val editor =
                                    applicationContext
                                        .getSharedPreferences(
                                            MapLibreConstants.MAPLIBRE_SHARED_PREFERENCES,
                                            Context.MODE_PRIVATE,
                                        ).edit()
                                editor.putString(MAPBOX_SHARED_PREFERENCE_RESOURCES_CACHE_PATH, path)
                                editor.apply()
                                internalSetResourcesCachePath(applicationContext, path, callback)
                            }

                            override fun onError() {
                                val message = "Path is not writable: $path"
                                Logger.e(TAG, message)
                                callback.onError(message)
                            }
                        },
                    ).execute(File(path))
            }
        }

        private fun internalSetResourcesCachePath(
            context: Context,
            path: String,
            callback: ResourcesCachePathChangeCallback,
        ) {
            val fileSource = getInstance(context)
            val active = fileSource.isActivated()
            if (!active) {
                fileSource.activate()
            }

            fileSource.setResourceCachePath(
                path,
                object : ResourcesCachePathChangeCallback {
                    override fun onSuccess(path: String) {
                        if (!active) {
                            fileSource.deactivate()
                        }
                        resourcesCachePathLoaderLock.lock()
                        resourcesCachePath = path
                        resourcesCachePathLoaderLock.unlock()
                        callback.onSuccess(path)
                    }

                    override fun onError(message: String) {
                        if (!active) {
                            fileSource.deactivate()
                        }
                        callback.onError(message)
                    }
                },
            )
        }

        private fun isPathWritable(path: String?): Boolean {
            if (path.isNullOrEmpty()) {
                return false
            }
            return File(path).canWrite()
        }
    }
}
