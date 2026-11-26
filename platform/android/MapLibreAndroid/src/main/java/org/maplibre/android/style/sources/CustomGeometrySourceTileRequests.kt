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
import java.util.concurrent.locks.ReentrantReadWriteLock
import kotlin.concurrent.read
import kotlin.concurrent.withLock
import kotlin.concurrent.write

/**
 * Keeps native submissions from overlapping source detachment, without holding a lock while invoking
 * the application's provider. Each attachment has separate request state so a worker from an old
 * executor cannot submit data or change the bookkeeping of a newly attached source.
 *
 * Submissions hold the read side of [attachment], so workers convert and submit tiles in parallel
 * while start() and release() wait for the submissions in flight. Request bookkeeping uses [lock],
 * which is only held briefly, so tile requests from the core tile loader don't wait for a submission.
 * A thread that needs both acquires [attachment] first.
 */
internal class CustomGeometrySourceTileRequests(
    private val provider: GeometryTileProvider?,
    private val submit: (Int, Int, Int, FeatureCollection) -> Unit,
    private val createExecutor: () -> ThreadPoolExecutor = ::newExecutor,
) {
    private val attachment = ReentrantReadWriteLock()
    private val lock = ReentrantLock()
    private var state: State? = null

    fun start() {
        attachment.write {
            release()
            lock.withLock { state = State(createExecutor()) }
        }
    }

    fun release() {
        attachment.write {
            lock.withLock {
                val previous = state ?: return
                state = null
                previous.inProgress.values.forEach { it.set(true) }
                previous.inProgress.clear()
                previous.awaiting.clear()
                previous.executor.shutdownNow()
            }
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
        attachment.read {
            if (state != null) submit(z, x, y, data)
        }
    }

    // Native submission calls back here while holding the read side of [attachment].
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
                attachment.read {
                    if (state === request.state && !request.cancelled.get()) {
                        submit(id.z, id.x, id.y, data)
                    }
                }
            }
        } catch (e: Exception) {
            // release() interrupts workers, which providers may surface as an exception. A failure only
            // matters for a request whose result is still wanted.
            if (lock.withLock { state === request.state && !request.cancelled.get() }) throw e
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
