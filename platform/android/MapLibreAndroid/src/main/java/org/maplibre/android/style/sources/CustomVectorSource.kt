package org.maplibre.android.style.sources

import androidx.annotation.Keep
import androidx.annotation.UiThread
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.launch
import java.util.concurrent.ConcurrentHashMap

@UiThread
class CustomVectorSource(
    id: String,
    private val provider: CustomVectorTileProvider,
    private val scope: CoroutineScope,
    minZoom: Int = 0,
    maxZoom: Int = 18
) : Source() {

    private val activeJobs = ConcurrentHashMap<TileID, Job>()

    init {
        initialize(id, minZoom, maxZoom)
    }

    fun setTileData(z: Int, x: Int, y: Int, data: TileData) {
        nativeSetTileData(z, x, y, data.bytes, data.formatId)
    }

    fun invalidateTile(z: Int, x: Int, y: Int) {
        nativeInvalidateTile(z, x, y)
    }

    @Keep
    private fun fetchTile(z: Int, x: Int, y: Int) {
        val tileId = TileID(z, x, y)
        activeJobs.remove(tileId)?.cancel()

        val job = scope.launch {
            try {
                val tileData = provider.fetchTile(z, x, y)
                nativeSetTileData(z, x, y, tileData.bytes, tileData.formatId)
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                nativeSetTileError(z, x, y, e.message ?: "Tile fetch failed")
            } finally {
                activeJobs.remove(tileId)
            }
        }
        activeJobs[tileId] = job
    }

    @Keep
    private fun cancelTile(z: Int, x: Int, y: Int) {
        activeJobs.remove(TileID(z, x, y))?.cancel()
    }

    @Keep
    private fun onAddedToMap() { /* no-op: scope is user-provided */ }

    @Keep
    private fun onRemovedFromMap() {
        activeJobs.values.forEach { it.cancel() }
        activeJobs.clear()
    }

    @Keep
    private external fun initialize(sourceId: String, minZoom: Int, maxZoom: Int)

    @Keep
    private external fun nativeSetTileData(z: Int, x: Int, y: Int, data: ByteArray, format: Int)

    @Keep
    private external fun nativeSetTileError(z: Int, x: Int, y: Int, message: String)

    @Keep
    private external fun nativeInvalidateTile(z: Int, x: Int, y: Int)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    internal data class TileID(val z: Int, val x: Int, val y: Int)

}
