package org.maplibre.android.style.sources

import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.geojson.FeatureCollection
import java.lang.ref.WeakReference
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.withLock

/**
 * Serializes native submissions with source detachment, without holding the lock while invoking
 * the application's provider. Each attachment has separate request state so a worker from an old
 * executor cannot submit data or change the bookkeeping of a newly attached source.
 */
internal class CustomGeometrySourceTileRequests(
    private val provider: GeometryTileProvider?,
    private val submit: (Int, Int, Int, FeatureCollection) -> Unit,
    private val createExecutor: () -> ThreadPoolExecutor = ::newExecutor,
) {
    private val lock = ReentrantLock()
    private var state: State? = null

    fun start() {
        lock.withLock {
            release()
            state = State(createExecutor())
        }
    }

    fun release() {
        lock.withLock {
            val previous = state ?: return
            state = null
            previous.inProgress.values.forEach { it.set(true) }
            previous.inProgress.clear()
            previous.awaiting.clear()
            previous.executor.shutdownNow()
        }
    }

    fun fetchTile(
        z: Int,
        x: Int,
        y: Int,
    ) {
        lock.withLock {
            val current = state ?: return
            val id = TileID(z, x, y)
            val request = Request(id, current, this)
            if (current.executor.queue.contains(request)) {
                current.executor.remove(request)
                current.executor.execute(request)
            } else if (current.inProgress.containsKey(id)) {
                current.awaiting[id] = request
            } else {
                current.executor.execute(request)
            }
        }
    }

    fun cancelTile(
        z: Int,
        x: Int,
        y: Int,
    ) {
        lock.withLock {
            val current = state ?: return
            val id = TileID(z, x, y)
            val cancelled = current.inProgress[id]
            if (cancelled == null || !cancelled.compareAndSet(false, true)) {
                if (!current.executor.queue.remove(Request(id, current, this))) {
                    current.awaiting.remove(id)
                }
            }
        }
    }

    fun setTileData(
        z: Int,
        x: Int,
        y: Int,
        data: FeatureCollection,
    ) {
        lock.withLock {
            if (state != null) submit(z, x, y, data)
        }
    }

    // Native submission calls back here while holding lock; the lock must be reentrant.
    fun isCancelled(
        z: Int,
        x: Int,
        y: Int,
    ): Boolean =
        lock.withLock {
            state?.inProgress?.get(TileID(z, x, y))?.get() ?: true
        }

    private fun run(request: Request) {
        lock.withLock {
            val current = state
            if (current !== request.state) return
            if (current.inProgress.containsKey(request.id)) {
                // A previously queued request may start while another worker handles the same tile.
                // Preserve the latest request already waiting for that tile.
                if (!current.awaiting.containsKey(request.id)) current.awaiting[request.id] = request
                return
            }
            current.inProgress[request.id] = request.cancelled
        }

        try {
            if (!request.cancelled.get()) {
                val id = request.id
                val data = provider!!.getFeaturesForBounds(LatLngBounds.from(id.z, id.x, id.y), id.z)
                lock.withLock {
                    if (state === request.state && !request.cancelled.get()) {
                        submit(id.z, id.x, id.y, data)
                    }
                }
            }
        } finally {
            lock.withLock {
                val current = state
                if (current === request.state) {
                    current.inProgress.remove(request.id)
                    current.awaiting.remove(request.id)?.let { current.executor.execute(it) }
                }
            }
        }
    }

    private class State(
        val executor: ThreadPoolExecutor,
    ) {
        val awaiting = HashMap<TileID, Request>()
        val inProgress = HashMap<TileID, AtomicBoolean>()
    }

    private data class TileID(
        val z: Int,
        val x: Int,
        val y: Int,
    )

    private class Request(
        val id: TileID,
        val state: State,
        owner: CustomGeometrySourceTileRequests,
    ) : Runnable {
        val cancelled = AtomicBoolean(false)
        private val owner = WeakReference(owner)

        override fun run() {
            owner.get()?.run(this)
        }

        override fun equals(other: Any?): Boolean = other is Request && id == other.id

        override fun hashCode(): Int = id.hashCode()
    }

    companion object {
        private val poolCount = AtomicInteger()

        private fun newExecutor(): ThreadPoolExecutor {
            val poolId = poolCount.getAndIncrement()
            val threadCount = AtomicInteger()
            return ThreadPoolExecutor(
                CustomGeometrySource.THREAD_POOL_LIMIT,
                CustomGeometrySource.THREAD_POOL_LIMIT,
                0L,
                TimeUnit.MILLISECONDS,
                LinkedBlockingQueue(),
            ) { runnable ->
                Thread(runnable, "${CustomGeometrySource.THREAD_PREFIX}-$poolId-${threadCount.getAndIncrement()}")
            }
        }
    }
}
