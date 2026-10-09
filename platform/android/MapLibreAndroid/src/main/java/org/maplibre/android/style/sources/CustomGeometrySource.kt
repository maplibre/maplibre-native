package org.maplibre.android.style.sources

import androidx.annotation.Keep
import androidx.annotation.UiThread
import androidx.annotation.WorkerThread
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.style.expressions.Expression
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection

/**
 * Custom Vector Source, allows using FeatureCollections.
 *
 *
 * CustomGeometrySource uses a coalescing model for frequent data updates targeting the same tile id,
 * which means that, while the source is attached, the in-progress request as well as the last scheduled
 * request are guaranteed to finish.
 * Any requests scheduled meanwhile can be canceled. Detaching the source cancels outstanding requests.
 *
 * @param id The source id.
 * @param options Custom geometry source options.
 * @param provider The tile provider that returns geometry data for this source.
 */
class CustomGeometrySource
    @UiThread
    constructor(
        id: String?,
        options: CustomGeometrySourceOptions?,
        provider: GeometryTileProvider?,
    ) : Source() {
        private val tileRequests = CustomGeometrySourceTileRequests(provider, ::nativeSetTileData)

        /**
         * Create a CustomGeometrySource
         *
         * @param id       The source id.
         * @param provider The tile provider that returns geometry data for this source.
         */
        @UiThread
        constructor(id: String?, provider: GeometryTileProvider?) : this(id, CustomGeometrySourceOptions(), provider) {
        }

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
        fun invalidateTile(
            zoomLevel: Int,
            x: Int,
            y: Int,
        ) {
            nativeInvalidateTile(zoomLevel, x, y)
        }

        /**
         * Set or update geometry contents of a specific tile. Use this method to update tiles
         * for which `GeometryTileProvider` was previously invoked. This method can be called from
         * background threads. Calls after the source is removed or detached are ignored.
         *
         * @param zoomLevel Tile zoom level.
         * @param x         Tile X coordinate.
         * @param y         Tile Y coordinate.
         * @param data      Feature collection for the tile.
         */
        fun setTileData(
            zoomLevel: Int,
            x: Int,
            y: Int,
            data: FeatureCollection,
        ) {
            tileRequests.setTileData(zoomLevel, x, y, data)
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
            return listOf(*features)
        }

        @Keep
        private external fun initialize(
            sourceId: String?,
            options: Any?,
        )

        @Keep
        private external fun querySourceFeatures(filter: Array<Any>?): Array<Feature>

        @Keep
        private external fun nativeSetTileData(
            z: Int,
            x: Int,
            y: Int,
            data: FeatureCollection,
        )

        @Keep
        private external fun nativeInvalidateTile(
            z: Int,
            x: Int,
            y: Int,
        )

        @Keep
        private external fun nativeInvalidateBounds(bounds: LatLngBounds)

        @Keep
        @Throws(Throwable::class)
        protected external fun finalize()

        // Style.clear() calls this before style replacement or map destruction. Waiting until the
        // native peer's destructor would be too late: the core source's tile loader is destroyed first.
        override fun setDetached() {
            tileRequests.release()
            super.setDetached()
        }

        @WorkerThread
        @Keep
        private fun fetchTile(
            z: Int,
            x: Int,
            y: Int,
        ) {
            tileRequests.fetchTile(z, x, y)
        }

        @WorkerThread
        @Keep
        private fun cancelTile(
            z: Int,
            x: Int,
            y: Int,
        ) {
            tileRequests.cancelTile(z, x, y)
        }

        @Keep
        private fun startThreads() {
            tileRequests.start()
        }

        @Keep
        private fun releaseThreads() {
            tileRequests.release()
        }

        @Keep
        private fun isCancelled(
            z: Int,
            x: Int,
            y: Int,
        ): Boolean = tileRequests.isCancelled(z, x, y)

        companion object {
            const val THREAD_PREFIX = "CustomGeom"
            const val THREAD_POOL_LIMIT = 4
        }
    }
