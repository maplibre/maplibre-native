package org.maplibre.android.offline

import android.content.Context
import android.os.Handler
import android.os.Looper
import androidx.annotation.IntDef
import androidx.annotation.Keep
import org.maplibre.android.LibraryLoader
import org.maplibre.android.MapLibre
import org.maplibre.android.net.ConnectivityReceiver
import org.maplibre.android.storage.FileSource

/**
 * An offline region is the basic building block for offline mobile maps.
 * Use [org.maplibre.android.offline.OfflineManager.CreateOfflineRegionCallback]
 * to create a new offline region.
 */
class OfflineRegion @Keep private constructor(offlineRegionPtr: Long, fileSource: FileSource, idParam: Long, definition: OfflineRegionDefinition, metadata: ByteArray) {
    // Members
    // Application context
    private val context: Context

    // Holds the pointer to JNI OfflineRegion
    @Keep
    private val nativePtr: Long = 0

    // Holds a reference to the FileSource to keep it alive
    private val fileSource: FileSource

    /*
   * Getters
   */
    // Region id
    val id: Long

    // delete status
    private var isDeleted = false
    val definition: OfflineRegionDefinition

    /**
     * Arbitrary binary region metadata. The contents are opaque to the SDK implementation;
     * it just stores and retrieves a byte[]. Check the `OfflineActivity` in the TestApp
     * for a sample implementation that uses JSON to store an offline region name.
     */
    var metadata: ByteArray
        private set

    // Makes sure callbacks come back to the main thread
    private val handler = Handler(Looper.getMainLooper())

    /**
     * A region can have a single observer, which gets notified whenever a change
     * to the region's status occurs.
     */
    @Keep
    interface OfflineRegionObserver {
        /**
         * Implement this method to be notified of a change in the status of an
         * offline region. Status changes include any change in state of the members
         * of OfflineRegionStatus.
         *
         *
         * This method will be executed on the main thread.
         *
         *
         * @param status the changed status
         */
        fun onStatusChanged(status: OfflineRegionStatus)

        /**
         * Implement this method to be notified of errors encountered while downloading
         * regional resources. Such errors may be recoverable; for example the implementation
         * will attempt to re-request failed resources based on an exponential backoff
         * algorithm, or when it detects that network access has been restored.
         *
         *
         * This method will be executed on the main thread.
         *
         *
         * @param error the offline region error message
         */
        fun onError(error: OfflineRegionError)

        /*
     * Implement this method to be notified when the limit on the number of MapLibre
     * tiles stored for offline regions has been reached.
     *
     * Once the limit has been reached, the SDK will not download further offline
     * tiles from MapLibre APIs until existing tiles have been removed.
     *
     * This limit does not apply to non-MapLibre tile sources.
     *
     * This method will be executed on the main thread.
     */
        fun mapboxTileCountLimitExceeded(limit: Long)
    }

    /**
     * This callback receives an asynchronous response containing the OfflineRegionStatus
     * of the offline region, or a [String] error message otherwise.
     */
    @Keep
    interface OfflineRegionStatusCallback {
        /**
         * Receives the status
         *
         * @param status the offline region status
         */
        fun onStatus(status: OfflineRegionStatus?)

        /**
         * Receives the error message
         *
         * @param error the error message
         */
        fun onError(error: String?)
    }

    /**
     * This callback receives an asynchronous response containing a notification when
     * an offline region has been deleted, or a [String] error message otherwise.
     */
    @Keep
    interface OfflineRegionDeleteCallback {
        /**
         * Receives the delete notification
         */
        fun onDelete()

        /**
         * Receives the error message
         *
         * @param error the error message
         */
        fun onError(error: String)
    }

    /**
     * This callback receives an asynchronous response containing a notification when
     * an offline region has been invalidated, or a [String] error message otherwise.
     */
    @Keep
    interface OfflineRegionInvalidateCallback {
        /**
         * Receives the invalidate notification
         */
        fun onInvalidate()

        /**
         * Receives the error message
         *
         * @param error the error message
         */
        fun onError(error: String)
    }

    /**
     * This callback receives an asynchronous response containing the newly update
     * OfflineMetadata in the database, or an error message otherwise.
     */
    @Keep
    interface OfflineRegionUpdateMetadataCallback {
        /**
         * Receives the newly update offline region metadata.
         *
         * @param metadata the offline metadata to u[date
         */
        fun onUpdate(metadata: ByteArray)

        /**
         * Receives the error message.
         *
         * @param error the error message to be shown
         */
        fun onError(error: String)
    }

    /**
     * A region is either inactive (not downloading, but previously-downloaded
     * resources are available for use), or active (resources are being downloaded
     * or will be downloaded, if necessary, when network access is available).
     *
     *
     * This state is independent of whether or not the complete set of resources
     * is currently available for offline use. To check if that is the case, use
     * `OfflineRegionStatus.isComplete()`.
     *
     */
    @IntDef(*[STATE_INACTIVE, STATE_ACTIVE])
    @kotlin.annotation.Retention(AnnotationRetention.SOURCE)
    annotation class DownloadState

    // Keep track of the region state
    private var state = STATE_INACTIVE

    /**
     * Gets whether or not the `OfflineRegionObserver` will continue to deliver messages even if
     * the region state has been set as STATE_INACTIVE.
     *
     * @return true if delivering inactive messages
     */
    var isDeliveringInactiveMessages = false
        private set

    /**
     * When set true, the `OfflineRegionObserver` will continue to deliver messages even if
     * the region state has been set as STATE_INACTIVE (operations happen asynchronously). If set
     * false, the client won't be notified of further messages.
     *
     * @param deliverInactiveMessages true if it should deliver inactive messages
     */
    fun setDeliverInactiveMessages(deliverInactiveMessages: Boolean) {
        isDeliveringInactiveMessages = deliverInactiveMessages
    }

    private fun deliverMessages(): Boolean {
        return if (state == STATE_ACTIVE) {
            true
        } else {
            isDeliveringInactiveMessages
        }
    }

    /**
     * Constructor
     *
     *
     * For JNI use only, to create a new offline region, use
     * [com.mapbox.mapboxsdk.offline.OfflineManager.createOfflineRegion] instead.
     */
    init {
        context = MapLibre.getApplicationContext()
        this.fileSource = fileSource
        this.id = idParam
        this.definition = definition
        this.metadata = metadata
        initialize(offlineRegionPtr, fileSource)
    }

    /**
     * Register an observer to be notified when the state of the region changes.
     *
     * @param observer the observer to be notified
     */
    fun setObserver(observer: OfflineRegionObserver?) {
        setOfflineRegionObserver(object : OfflineRegionObserver {
            override fun onStatusChanged(status: OfflineRegionStatus) {
                if (deliverMessages()) {
                    handler.post { observer?.onStatusChanged(status) }
                }
            }

            override fun onError(error: OfflineRegionError) {
                if (deliverMessages()) {
                    handler.post { observer?.onError(error) }
                }
            }

            override fun mapboxTileCountLimitExceeded(limit: Long) {
                if (deliverMessages()) {
                    handler.post { observer?.mapboxTileCountLimitExceeded(limit) }
                }
            }
        })
    }

    /**
     * Pause or resume downloading of regional resources.
     *
     *
     * After a download has been completed, you are required to reset the state of the region to STATE_INACTIVE.
     *
     *
     * @param state the download state
     */
    fun setDownloadState(@DownloadState state: Int) {
        if (this.state == state) {
            return
        }
        if (state == STATE_ACTIVE) {
            ConnectivityReceiver.instance(context).activate()
            fileSource.activate()
        } else {
            fileSource.deactivate()
            ConnectivityReceiver.instance(context).deactivate()
        }
        this.state = state
        setOfflineRegionDownloadState(state)
    }

    /**
     * Retrieve the current status of the region. The query will be executed
     * asynchronously and the results passed to the given callback which will be
     * executed on the main thread.
     *
     * @param callback the callback to invoked.
     */
    fun getStatus(callback: OfflineRegionStatusCallback) {
        fileSource.activate()
        getOfflineRegionStatus(object : OfflineRegionStatusCallback {
            override fun onStatus(status: OfflineRegionStatus?) {
                handler.post {
                    fileSource.deactivate()
                    callback.onStatus(status)
                }
            }

            override fun onError(error: String?) {
                handler.post {
                    fileSource.deactivate()
                    callback.onError(error)
                }
            }
        })
    }

    /**
     * Remove an offline region from the database and perform any resources evictions
     * necessary as a result.
     *
     *
     * Eviction works by removing the least-recently requested resources not also required
     * by other regions, until the database shrinks below a certain size.
     *
     *
     *
     * When the operation is complete or encounters an error, the given callback will be
     * executed on the main thread.
     *
     * Note that this operation can be potentially slow if packing the database
     * occurs automatically ([com.mapbox.mapboxsdk.offline.OfflineManager.runPackDatabaseAutomatically])
     *
     *
     * After you call this method, you may not call any additional methods on this object.
     *
     *
     * @param callback the callback to be invoked
     */
    fun delete(callback: OfflineRegionDeleteCallback) {
        if (!isDeleted) {
            isDeleted = true
            fileSource.activate()
            deleteOfflineRegion(object : OfflineRegionDeleteCallback {
                override fun onDelete() {
                    handler.post {
                        fileSource.deactivate()
                        callback.onDelete()
                        finalize()
                    }
                }

                override fun onError(error: String) {
                    handler.post {
                        isDeleted = false
                        fileSource.deactivate()
                        callback.onError(error)
                    }
                }
            })
        }
    }

    /**
     * Invalidate all the tiles from an offline region forcing MapLibre GL to revalidate
     * the tiles with the server before using. This is more efficient than deleting the
     * offline region and downloading it again because if the data on the cache matches
     * the server, no new data gets transmitted.
     *
     * @param callback the callback to be invoked
     */
    fun invalidate(callback: OfflineRegionInvalidateCallback?) {
        fileSource.activate()
        invalidateOfflineRegion(object : OfflineRegionInvalidateCallback {
            override fun onInvalidate() {
                handler.post {
                    fileSource.deactivate()
                    callback?.onInvalidate()
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
     * Update an offline region metadata from the database.
     *
     *
     * When the operation is complete or encounters an error, the given callback will be
     * executed on the main thread.
     *
     *
     * @param bytes    the metadata in bytes
     * @param callback the callback to be invoked
     */
    fun updateMetadata(
        bytes: ByteArray,
        callback: OfflineRegionUpdateMetadataCallback
    ) {
        updateOfflineRegionMetadata(
            bytes,
            object : OfflineRegionUpdateMetadataCallback {
                override fun onUpdate(metadata: ByteArray) {
                    handler.post {
                        this@OfflineRegion.metadata = metadata
                        callback.onUpdate(metadata)
                    }
                }

                override fun onError(error: String) {
                    handler.post { callback.onError(error) }
                }
            }
        )
    }

    @Keep
    private external fun initialize(offlineRegionPtr: Long, fileSource: FileSource)

    @Keep
    protected external fun finalize()

    @Keep
    private external fun setOfflineRegionObserver(callback: OfflineRegionObserver)

    @Keep
    private external fun setOfflineRegionDownloadState(@DownloadState offlineRegionDownloadState: Int)

    @Keep
    private external fun getOfflineRegionStatus(callback: OfflineRegionStatusCallback)

    @Keep
    private external fun deleteOfflineRegion(callback: OfflineRegionDeleteCallback)

    @Keep
    private external fun updateOfflineRegionMetadata(metadata: ByteArray, callback: OfflineRegionUpdateMetadataCallback)

    @Keep
    private external fun invalidateOfflineRegion(callback: OfflineRegionInvalidateCallback)

    companion object {
        //
        // Static methods
        //
        init {
            LibraryLoader.load()
        }

        const val STATE_INACTIVE = 0
        const val STATE_ACTIVE = 1
    }
}
