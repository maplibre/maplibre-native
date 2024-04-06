package org.maplibre.android.style.sources

import androidx.annotation.Keep
import androidx.annotation.UiThread
import androidx.annotation.WorkerThread
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.geometry.LatLngBounds.Companion.from
import org.maplibre.android.style.expressions.Expression
import java.lang.ref.WeakReference
import java.util.Arrays
import java.util.Locale
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.ThreadFactory
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.locks.Lock
import java.util.concurrent.locks.ReentrantLock
import kotlin.collections.ArrayList
import kotlin.collections.HashMap

/**
 * Custom Vector Source, allows using FeatureCollections.
 *
 *
 * CustomGeometrySource uses a coalescing model for frequent data updates targeting the same tile id,
 * which means, that the in-progress request as well as the last scheduled request are guaranteed to finish.
 * Any requests scheduled meanwhile can be canceled.
 */
class CustomGeometrySource @UiThread constructor(id: String?, options: CustomGeometrySourceOptions?, private val provider: GeometryTileProvider?) : Source() {
    private val executorLock: Lock = ReentrantLock()
    private var executor: ThreadPoolExecutor? = null
    private val awaitingTasksMap: MutableMap<TileID, GeometryTileRequest> = HashMap()

    /**
     * A map containing in-progress requests targeting distinct tiles.
     * A request is considered in-progress when it's started by the ThreadPoolExecutor.
     * A request is marked as done when the data is passed from the JNI layer to the core, after the features conversion.
     */
    private val inProgressTasksMap: MutableMap<TileID, AtomicBoolean?> = HashMap()

    /**
     * Create a CustomGeometrySource
     *
     * @param id       The source id.
     * @param provider The tile provider that returns geometry data for this source.
     */
    @UiThread
    constructor(id: String?, provider: GeometryTileProvider?) : this(id, CustomGeometrySourceOptions(), provider) {
    }

    /**
     * Create a CustomGeometrySource with non-default [CustomGeometrySourceOptions].
     *
     * @param id       The source id.
     * @param options  CustomGeometrySourceOptions.
     */
    init {
        initialize(id, options)
    }

    /**
     * Invalidate previously provided features within a given bounds at all zoom levels.
     * Invoking this method will result in new requests to `GeometryTileProvider` for regions
     * that contain, include, or intersect with the provided bounds.
     *
     * @param bounds The region in which features should be invalidated at all zoom levels
     */
    fun invalidateRegion(bounds: LatLngBounds) {
        nativeInvalidateBounds(bounds)
    }

    /**
     * Invalidate the geometry contents of a specific tile. Invoking this method will result
     * in new requests to `GeometryTileProvider` for visible tiles.
     *
     * @param zoomLevel Tile zoom level.
     * @param x         Tile X coordinate.
     * @param y         Tile Y coordinate.
     */
    fun invalidateTile(zoomLevel: Int, x: Int, y: Int) {
        nativeInvalidateTile(zoomLevel, x, y)
    }

    /**
     * Set or update geometry contents of a specific tile. Use this method to update tiles
     * for which `GeometryTileProvider` was previously invoked. This method can be called from
     * background threads.
     *
     * @param zoomLevel Tile zoom level.
     * @param x         Tile X coordinate.
     * @param y         Tile Y coordinate.
     * @param data      Feature collection for the tile.
     */
    fun setTileData(zoomLevel: Int, x: Int, y: Int, data: FeatureCollection) {
        nativeSetTileData(zoomLevel, x, y, data)
    }

    /**
     * Queries the source for features.
     *
     * @param filter an optional filter expression to filter the returned Features
     * @return the features
     */
    fun querySourceFeatures(filter: Expression?): List<Feature> {
        checkThread()
        val features = querySourceFeatures(filter?.toArray())
        return if (features != null) Arrays.asList(*features) else ArrayList()
    }

    @Keep
    private external fun initialize(sourceId: String?, options: Any?)

    @Keep
    private external fun querySourceFeatures(filter: Array<Any>?): Array<Feature>

    @Keep
    private external fun nativeSetTileData(z: Int, x: Int, y: Int, data: FeatureCollection)

    @Keep
    private external fun nativeInvalidateTile(z: Int, x: Int, y: Int)

    @Keep
    private external fun nativeInvalidateBounds(bounds: LatLngBounds)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    private fun setTileData(tileId: TileID, data: FeatureCollection) {
        nativeSetTileData(tileId.z, tileId.x, tileId.y, data)
    }

    /**
     * Tile data request can come from a number of different threads.
     * To remove race condition for requests targeting the same tile id we are first checking if there is a request
     * already enqueued, if yes, we are replacing it.
     * Otherwise, we are checking if there is an in-progress request, if yes,
     * we are creating or replacing an awaiting request.
     * If none of the above, we are enqueueing the request.
     */
    @WorkerThread
    @Keep
    private fun fetchTile(z: Int, x: Int, y: Int) {
        val cancelFlag = AtomicBoolean(false)
        val tileID = TileID(z, x, y)
        val request = GeometryTileRequest(tileID, provider, awaitingTasksMap, inProgressTasksMap, this, cancelFlag)
        synchronized(awaitingTasksMap) {
            synchronized(inProgressTasksMap) {
                if (executor!!.queue.contains(request)) {
                    executor!!.remove(request)
                    executeRequest(request)
                } else if (inProgressTasksMap.containsKey(tileID)) {
                    awaitingTasksMap.put(tileID, request)
                } else {
                    executeRequest(request)
                }
            }
        }
    }

    private fun executeRequest(request: GeometryTileRequest) {
        executorLock.lock()
        try {
            if (executor != null && !executor!!.isShutdown) {
                executor!!.execute(request)
            }
        } finally {
            executorLock.unlock()
        }
    }

    /**
     * We want to cancel only the oldest request, therefore, we are first checking if it's in progress,
     * if not or if the currently in progress request has already been canceled,
     * we are searching for any request in the executor's queue.
     * Otherwise, we are removing an awaiting request targeting this tile id.
     *
     *
     * [GeometryTileRequest.equals] is overridden to cover only the tile id,
     * therefore, we can use an empty request to search the executor's queue.
     */
    @WorkerThread
    @Keep
    private fun cancelTile(z: Int, x: Int, y: Int) {
        val tileID = TileID(z, x, y)
        synchronized(awaitingTasksMap) {
            synchronized(inProgressTasksMap) {
                val cancelFlag = inProgressTasksMap[tileID]
                // check if there is an in progress task
                if (!(cancelFlag != null && cancelFlag.compareAndSet(false, true))) {
                    // if there is no tasks in progress or the in progress task was already cancelled, check the executor's queue
                    val emptyRequest = GeometryTileRequest(tileID, null, null, null, null, null)
                    if (!executor!!.queue.remove(emptyRequest)) {
                        // if there was no tasks in queue, remove from the awaiting map
                        awaitingTasksMap.remove(tileID)
                    }
                }
            }
        }
    }

    @Keep
    private fun startThreads() {
        executorLock.lock()
        executor = try {
            if (executor != null && !executor!!.isShutdown) {
                executor!!.shutdownNow()
            }
            ThreadPoolExecutor(
                THREAD_POOL_LIMIT,
                THREAD_POOL_LIMIT,
                0L,
                TimeUnit.MILLISECONDS,
                LinkedBlockingQueue(),
                object : ThreadFactory {
                    val threadCount = AtomicInteger()
                    val poolId = poolCount.getAndIncrement()
                    override fun newThread(runnable: Runnable): Thread {
                        return Thread(runnable, String.format(Locale.US, "%s-%d-%d", THREAD_PREFIX, poolId, threadCount.getAndIncrement()))
                    }
                }
            )
        } finally {
            executorLock.unlock()
        }
    }

    @Keep
    private fun releaseThreads() {
        executorLock.lock()
        try {
            executor!!.shutdownNow()
        } finally {
            executorLock.unlock()
        }
    }

    @Keep
    private fun isCancelled(z: Int, x: Int, y: Int): Boolean {
        return inProgressTasksMap[TileID(z, x, y)]!!.get()
    }

    internal class TileID(var z: Int, var x: Int, var y: Int) {
        override fun hashCode(): Int {
            return Arrays.hashCode(intArrayOf(z, x, y))
        }

        override fun equals(`object`: Any?): Boolean {
            if (`object` === this) {
                return true
            }
            if (`object` == null || javaClass != `object`.javaClass) {
                return false
            }
            if (`object` is TileID) {
                val other = `object`
                return z == other.z && x == other.x && y == other.y
            }
            return false
        }
    }

    internal class GeometryTileRequest(
        private val id: TileID,
        private val provider: GeometryTileProvider?,
        private val awaiting: MutableMap<TileID, GeometryTileRequest>?,
        private val inProgress: MutableMap<TileID, AtomicBoolean?>?,
        _source: CustomGeometrySource?,
        _cancelled: AtomicBoolean?
    ) : Runnable {
        private val sourceRef: WeakReference<CustomGeometrySource?>
        private val cancelled: AtomicBoolean?

        init {
            sourceRef = WeakReference(_source)
            cancelled = _cancelled
        }

        override fun run() {
            synchronized(awaiting!!) {
                synchronized(inProgress!!) {
                    if (inProgress.containsKey(id)) {
                        // request targeting this tile id is already being processed,
                        // scenario that should occur only if the tile is being requested when
                        // another request is switching threads to execute
                        if (!awaiting.containsKey(id)) {
                            awaiting[id] = this
                        }
                        return
                    } else {
                        inProgress.put(id, cancelled)
                    }
                }
            }
            if (!isCancelled()) {
                val data = provider!!.getFeaturesForBounds(from(id.z, id.x, id.y), id.z)
                val source = sourceRef.get()
                if (!isCancelled() && source != null && data != null) {
                    source.setTileData(id, data)
                }
            }
            synchronized(awaiting) {
                synchronized(inProgress!!) {
                    inProgress.remove(id)

                    // executing the next request targeting the same tile
                    if (awaiting.containsKey(id)) {
                        val queuedRequest = awaiting[id]
                        val source = sourceRef.get()
                        if (source != null && queuedRequest != null) {
                            source.executor!!.execute(queuedRequest)
                        }
                        awaiting.remove(id)
                    }
                }
            }
        }

        private fun isCancelled(): Boolean {
            return cancelled!!.get()
        }

        override fun equals(o: Any?): Boolean {
            if (this === o) {
                return true
            }
            if (o == null || javaClass != o.javaClass) {
                return false
            }
            val request = o as GeometryTileRequest
            return id == request.id
        }
    }

    companion object {
        const val THREAD_PREFIX = "CustomGeom"
        const val THREAD_POOL_LIMIT = 4
        private val poolCount = AtomicInteger()
    }
}
