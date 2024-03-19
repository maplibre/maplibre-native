package org.maplibre.android.offline

import android.annotation.SuppressLint
import android.content.Context
import android.os.Handler
import android.os.Looper
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.LibraryLoader
import org.maplibre.android.R
import org.maplibre.android.geometry.LatLngBounds.Companion.world
import org.maplibre.android.net.ConnectivityReceiver
import org.maplibre.android.storage.FileSource
import org.maplibre.android.utils.FileUtils
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.IOException
import java.nio.channels.FileChannel

/**
 * The offline manager is the main entry point for offline-related functionality.
 *
 * It'll help you list and create offline regions.
 */
@UiThread
class OfflineManager private constructor(context: Context) {
    // Native peer pointer
    @Keep
    private val nativePtr: Long = 0

    // Reference to the file source to keep it alive for the
    // lifetime of this object
    private val fileSource: FileSource

    // Makes sure callbacks come back to the main thread
    private val handler = Handler(Looper.getMainLooper())

    // The application context
    private val context: Context

    /**
     * This callback receives an asynchronous response containing a list of all
     * OfflineRegion in the database or an error message otherwise.
     */
    @Keep
    interface ListOfflineRegionsCallback {
        /**
         * Receives the list of offline regions.
         *
         * @param offlineRegions the offline region array
         */
        fun onList(offlineRegions: Array<OfflineRegion>?)

        /**
         * Receives the error message.
         *
         * @param error the error message
         */
        fun onError(error: String)
    }

    /**
     * This callback receives an asynchronous response containing the newly created
     * OfflineRegion in the database or an error message otherwise.
     */
    @Keep
    interface CreateOfflineRegionCallback {
        /**
         * Receives the newly created offline region.
         *
         * @param offlineRegion the offline region to create
         */
        fun onCreate(offlineRegion: OfflineRegion)

        /**
         * Receives the error message.
         *
         * @param error the error message to be shown
         */
        fun onError(error: String)
    }

    /**
     * This callback receives an asynchronous response containing a list of all
     * OfflineRegion added to the database during the merge.
     */
    @Keep
    interface MergeOfflineRegionsCallback {
        /**
         * Receives the list of merged offline regions.
         *
         * @param offlineRegions the offline region array
         */
        fun onMerge(offlineRegions: Array<OfflineRegion>?)

        /**
         * Receives the error message.
         *
         * @param error the error message
         */
        fun onError(error: String)
    }

    init {
        this.context = context.applicationContext
        fileSource = FileSource.getInstance(this.context)
        initialize(fileSource)

        // Delete any existing previous ambient cache database
        deleteAmbientDatabase(this.context)
    }

    private fun deleteAmbientDatabase(context: Context) {
        val path = FileSource.getInternalCachePath(context) + File.separator + "mbgl-cache.db"
        FileUtils.deleteFile(path)
    }

    /**
     * Retrieve all regions in the offline database.
     *
     * The query will be executed asynchronously and the results passed to the given
     * callback on the main thread.
     *
     * @param callback the callback to be invoked
     */
    fun listOfflineRegions(callback: ListOfflineRegionsCallback) {
        fileSource.activate()
        listOfflineRegions(
            fileSource,
            object : ListOfflineRegionsCallback {
                override fun onList(offlineRegions: Array<OfflineRegion>?) {
                    handler.post {
                        fileSource.deactivate()
                        callback.onList(offlineRegions)
                    }
                }

                override fun onError(error: String) {
                    handler.post {
                        fileSource.deactivate()
                        callback.onError(error)
                    }
                }
            }
        )
    }

    /**
     * Merge offline regions from a secondary database into the main offline database.
     *
     * When the merge is completed, or fails, the [MergeOfflineRegionsCallback] will be invoked on the main thread.
     * The callback reference is **strongly kept** throughout the process,
     * so it needs to be wrapped in a weak reference or released on the client side if necessary.
     *
     * The secondary database may need to be upgraded to the latest schema.
     * This is done in-place and requires write-access to the provided path.
     * If the app's process doesn't have write-access to the provided path,
     * the file will be copied to the temporary, internal directory for the duration of the merge.
     *
     * Only resources and tiles that belong to a region will be copied over. Identical
     * regions will be flattened into a single new region in the main database.
     *
     * The operation will be aborted and [MergeOfflineRegionsCallback.onError] with an appropriate message
     * will be invoked if the merge would result in the offline tile count limit being exceeded.
     *
     * Merged regions may not be in a completed status if the secondary database
     * does not contain all the tiles or resources required by the region definition.
     *
     * @param path     secondary database writable path
     * @param callback completion/error callback
     */
    fun mergeOfflineRegions(path: String, callback: MergeOfflineRegionsCallback) {
        val src = File(path)
        Thread {
            var errorMessage: String? = null
            if (src.canWrite()) {
                handler.post { // path writable, merge and update schema in place if necessary
                    mergeOfflineDatabaseFiles(src, callback, false)
                }
            } else if (src.canRead()) {
                // path not writable, copy the the file to temp directory
                val dst = File(FileSource.getInternalCachePath(context), src.name)
                try {
                    copyTempDatabaseFile(src, dst)
                    handler.post { // merge and update schema using the copy
                        mergeOfflineDatabaseFiles(dst, callback, true)
                    }
                } catch (ex: IOException) {
                    ex.printStackTrace()
                    errorMessage = ex.message
                }
            } else {
                // path not readable, abort
                errorMessage = "Secondary database needs to be located in a readable path."
            }
            if (errorMessage != null) {
                val finalErrorMessage: String = errorMessage
                handler.post { callback.onError(finalErrorMessage) }
            }
        }.start()
    }

    /**
     * Delete existing database and re-initialize.
     *
     * When the operation is complete or encounters an error, the given callback will be
     * executed on the database thread; it is the responsibility of the platform bindings
     * to re-execute a user-provided callback on the main thread.
     *
     * @param callback the callback to be invoked when the database was reset or when the operation erred.
     */
    fun resetDatabase(callback: FileSourceCallback?) {
        fileSource.activate()
        nativeResetDatabase(object : FileSourceCallback {
            override fun onSuccess() {
                handler.post {
                    fileSource.deactivate()
                    callback?.onSuccess()
                }
            }

            override fun onError(message: String) {
                handler.post {
                    fileSource.deactivate()
                    callback?.onError(message)
                }
            }
        })
    }

    /**
     * Packs the existing database file into a minimal amount of disk space.
     *
     * This operation has a performance impact as it will vacuum the database,
     * forcing it to move pages on the filesystem.
     *
     * When the operation is complete or encounters an error, the given callback will be
     * executed on the database thread; it is the responsibility of the SDK bindings
     * to re-execute a user-provided callback on the main thread.
     *
     * @param callback the callback to be invoked when the database was reset or when the operation erred.
     */
    fun packDatabase(callback: FileSourceCallback?) {
        fileSource.activate()
        nativePackDatabase(object : FileSourceCallback {
            override fun onSuccess() {
                handler.post {
                    fileSource.deactivate()
                    callback?.onSuccess()
                }
            }

            override fun onError(message: String) {
                handler.post {
                    fileSource.deactivate()
                    callback?.onError(message)
                }
            }
        })
    }

    /**
     * Forces re-validation of the ambient cache.
     *
     * Forces MapLibre Native to revalidate resources stored in the ambient
     * cache with the tile server before using them, making sure they
     * are the latest version. This is more efficient than cleaning the
     * cache because if the resource is considered valid after the server
     * lookup, it will not get downloaded again.
     *
     * Resources overlapping with offline regions will not be affected
     * by this call.
     *
     * @param callback the callback to be invoked when the ambient cache was invalidated or when the operation erred.
     */
    fun invalidateAmbientCache(callback: FileSourceCallback?) {
        fileSource.activate()
        nativeInvalidateAmbientCache(object : FileSourceCallback {
            override fun onSuccess() {
                handler.post {
                    fileSource.deactivate()
                    callback?.onSuccess()
                }
            }

            override fun onError(message: String) {
                handler.post {
                    fileSource.deactivate()
                    callback?.onError(message)
                }
            }
        })
    }

    /**
     * Erase resources from the ambient cache, freeing storage space.
     *
     * Erases the ambient cache, freeing resources.
     * Note that this operation can be potentially slow if packing the database
     * occurs automatically ([OfflineManager.runPackDatabaseAutomatically]).
     *
     * Resources overlapping with offline regions will not be affected
     * by this call.
     *
     *
     * @param callback the callback to be invoked when the ambient cache was cleared or when the operation erred.
     */
    fun clearAmbientCache(callback: FileSourceCallback?) {
        fileSource.activate()
        nativeClearAmbientCache(object : FileSourceCallback {
            override fun onSuccess() {
                handler.post {
                    fileSource.deactivate()
                    callback?.onSuccess()
                }
            }

            override fun onError(message: String) {
                handler.post {
                    fileSource.deactivate()
                    callback?.onError(message)
                }
            }
        })
    }

    /**
     * Sets the maximum size in bytes for the ambient cache.
     *
     * This call is potentially expensive because it will try
     * to trim the data in case the database is larger than the
     * size defined. The size of offline regions are not affected
     * by this settings, but the ambient cache will always try
     * to not exceed the maximum size defined, taking into account
     * the current size for the offline regions.
     *
     * Note that if you use the SDK's offline functionality, your ability to set the ambient cache size will be limited.
     * Space that offline regions take up detract from the space available for ambient caching, and the ambient cache
     * size does not block offline downloads. For example: if the maximum cache size is set to 50 MB and 40 MB are
     * already used by offline regions, the ambient cache size will effectively be 10 MB.
     *
     * Setting the size to 0 will disable the cache if there is no
     * offline region on the database.
     *
     * This method should always be called at the start of an app, before setting the style and loading a map.
     * Otherwise, the map will instantiate with the default cache size of 50 MB.
     *
     * @param size     the maximum size of the ambient cache
     * @param callback the callback to be invoked when the the maximum size has been set or when the operation erred.
     */
    fun setMaximumAmbientCacheSize(size: Long, callback: FileSourceCallback?) {
        fileSource.activate()
        nativeSetMaximumAmbientCacheSize(
            size,
            object : FileSourceCallback {
                override fun onSuccess() {
                    handler.post {
                        fileSource.deactivate()
                        callback?.onSuccess()
                    }
                }

                override fun onError(message: String) {
                    fileSource.activate()
                    handler.post {
                        fileSource.deactivate()
                        callback?.onError(message)
                    }
                }
            }
        )
    }

    /**
     * This callback receives an asynchronous response indicating if an operation has succeeded or failed.
     */
    @Keep
    interface FileSourceCallback {
        /**
         * Receives the success of an operation
         */
        fun onSuccess()

        /**
         * Receives an error message if an operation was not successful
         *
         * @param message the error message
         */
        fun onError(message: String)
    }

    private fun mergeOfflineDatabaseFiles(file: File, callback: MergeOfflineRegionsCallback, isTemporaryFile: Boolean) {
        fileSource.activate()
        mergeOfflineRegions(
            fileSource,
            file.absolutePath,
            object : MergeOfflineRegionsCallback {
                override fun onMerge(offlineRegions: Array<OfflineRegion>?) {
                    if (isTemporaryFile) {
                        file.delete()
                    }
                    handler.post {
                        fileSource.deactivate()
                        callback.onMerge(offlineRegions)
                    }
                }

                override fun onError(error: String) {
                    if (isTemporaryFile) {
                        file.delete()
                    }
                    handler.post {
                        fileSource.deactivate()
                        callback.onError(error)
                    }
                }
            }
        )
    }

    /**
     * Creates an offline region in the database by downloading the resources needed to use
     * the given region offline.
     *
     * When the initial database queries have completed, the provided callback will be
     * executed on the main thread.
     *
     * Note that the resulting region will be in an inactive download state; to begin
     * downloading resources, call `OfflineRegion.setDownloadState(DownloadState.STATE_ACTIVE)`,
     * optionally registering an `OfflineRegionObserver` beforehand.
     *
     * @param definition the offline region definition
     * @param metadata   the metadata in bytes
     * @param callback   the callback to be invoked
     */
    fun createOfflineRegion(definition: OfflineRegionDefinition, metadata: ByteArray, callback: CreateOfflineRegionCallback) {
        if (!isValidOfflineRegionDefinition(definition)) {
            callback.onError(String.format(context.getString(R.string.maplibre_offline_error_region_definition_invalid), definition.bounds))
            return
        }
        ConnectivityReceiver.instance(context).activate()
        FileSource.getInstance(context).activate()
        createOfflineRegion(
            fileSource,
            definition,
            metadata,
            object : CreateOfflineRegionCallback {
                override fun onCreate(offlineRegion: OfflineRegion) {
                    handler.post {
                        ConnectivityReceiver.instance(context).deactivate()
                        FileSource.getInstance(context).deactivate()
                        callback.onCreate(offlineRegion)
                    }
                }

                override fun onError(error: String) {
                    handler.post {
                        ConnectivityReceiver.instance(context).deactivate()
                        FileSource.getInstance(context).deactivate()
                        callback.onError(error)
                    }
                }
            }
        )
    }

    /**
     * Validates if the offline region definition bounds is valid for an offline region download.
     *
     * @param definition the offline region definition
     * @return true if the region fits the world bounds.
     */
    private fun isValidOfflineRegionDefinition(definition: OfflineRegionDefinition): Boolean {
        return world().contains(definition.bounds!!)
    }

    /**
     * Sets the maximum number of tiles that may be downloaded and stored on the current device.
     * By default, the limit is set to 6,000.
     *
     * Once this limit is reached, [OfflineRegion.OfflineRegionObserver.mapboxTileCountLimitExceeded]
     * fires every additional attempt to download additional tiles until already downloaded tiles are removed
     * by calling [OfflineRegion.delete].
     *
     * @param limit the maximum number of tiles allowed to be downloaded
     */
    @Keep
    external fun setOfflineMapboxTileCountLimit(limit: Long)

    /**
     * Sets whether database file packing occurs automatically.
     * By default, the automatic database file packing is enabled.
     *
     * If packing is enabled, database file packing occurs automatically
     * after an offline region is deleted by calling
     * [OfflineRegion.delete]
     * or the ambient cache is cleared by calling [OfflineManager.clearAmbientCache].
     *
     * If packing is disabled, disk space will not be freed after
     * resources are removed unless [OfflineManager.packDatabase] is explicitly called.
     *
     * @param autopack flag setting the automatic database file packing.
     */

    @Keep
    external fun runPackDatabaseAutomatically(autopack: Boolean)

    @Keep
    private external fun initialize(fileSource: FileSource)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    @Keep
    private external fun listOfflineRegions(fileSource: FileSource, callback: ListOfflineRegionsCallback)

    @Keep
    private external fun createOfflineRegion(fileSource: FileSource, definition: OfflineRegionDefinition, metadata: ByteArray, callback: CreateOfflineRegionCallback)

    @Keep
    private external fun mergeOfflineRegions(fileSource: FileSource, path: String, callback: MergeOfflineRegionsCallback)

    @Keep
    private external fun nativeResetDatabase(callback: FileSourceCallback?)

    @Keep
    private external fun nativePackDatabase(callback: FileSourceCallback?)

    @Keep
    private external fun nativeInvalidateAmbientCache(callback: FileSourceCallback?)

    @Keep
    private external fun nativeClearAmbientCache(callback: FileSourceCallback?)

    @Keep
    private external fun nativeSetMaximumAmbientCacheSize(size: Long, callback: FileSourceCallback?)

    /**
     * Insert the provided resource into the ambient cache
     * This method mimics the caching that would take place if the equivalent
     * resource were requested in the process of map rendering.
     * Use this method to pre-warm the cache with resources you know
     * will be requested.
     *
     * This call is asynchronous: the data may not be immediately available
     * for in-progress requests, although subsequent requests should have
     * access to the cached data.
     *
     * @param url            The URL of the resource to insert
     * @param data           Response data to store for this resource. Data is expected to be uncompressed;
     * internally, the cache will compress data as necessary.
     * @param modified       Optional "modified" response header, in seconds since 1970, or 0 if not set
     * @param expires        Optional "expires" response header, in seconds since 1970, or 0 if not set
     * @param etag           Optional "entity tag" response header
     * @param mustRevalidate Indicates whether response can be used after it's stale
     */
    @Keep
    external fun putResourceWithUrl(url: String?, data: ByteArray?, modified: Long, expires: Long, etag: String?, mustRevalidate: Boolean)

    companion object {
        private const val TAG = "Mbgl - OfflineManager"

        //
        // Static methods
        //
        init {
            LibraryLoader.load()
        }

        // This object is implemented as a singleton
        @SuppressLint("StaticFieldLeak")
        private var instance: OfflineManager? = null

        /**
         * Get the single instance of offline manager.
         *
         * @param context the context used to host the offline manager
         * @return the single instance of offline manager
         */
        @Synchronized
        @JvmStatic
        fun getInstance(context: Context): OfflineManager {
            if (instance == null) {
                instance = OfflineManager(context)
            }
            return instance!!
        }

        @Throws(IOException::class)
        private fun copyTempDatabaseFile(sourceFile: File, destFile: File) {
            if (!destFile.exists() && !destFile.createNewFile()) {
                throw IOException("Unable to copy database file for merge.")
            }
            var source: FileChannel? = null
            var destination: FileChannel? = null
            try {
                source = FileInputStream(sourceFile).channel
                destination = FileOutputStream(destFile).channel
                destination.transferFrom(source, 0, source.size())
            } catch (ex: IOException) {
                throw IOException(String.format("Unable to copy database file for merge. %s", ex.message))
            } finally {
                source?.close()
                destination?.close()
            }
        }
    }
}
