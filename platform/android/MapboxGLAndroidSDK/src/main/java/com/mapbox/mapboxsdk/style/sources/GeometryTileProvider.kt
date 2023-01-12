package com.mapbox.mapboxsdk.style.sources

import androidx.annotation.WorkerThread
import com.mapbox.geojson.FeatureCollection
import com.mapbox.mapboxsdk.geometry.LatLngBounds

/**
 * Interface that defines methods for working with [CustomGeometrySource].
 */
interface GeometryTileProvider {
    /***
     * Interface method called by [CustomGeometrySource] to request features for a tile.
     *
     * @param bounds [LatLngBounds] of the tile.
     * @param zoomLevel Tile zoom level.
     * @return Return a @{link FeatureCollection} to be displayed in the requested tile.
     */
    @WorkerThread
    fun getFeaturesForBounds(bounds: LatLngBounds, zoomLevel: Int): FeatureCollection
}
