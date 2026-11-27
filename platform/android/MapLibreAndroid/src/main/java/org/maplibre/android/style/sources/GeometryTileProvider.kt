package org.maplibre.android.style.sources

import androidx.annotation.WorkerThread
import org.maplibre.geojson.FeatureCollection
import org.maplibre.android.geometry.LatLngBounds

/**
 * Interface that defines methods for working with [CustomGeometrySource].
 */
interface GeometryTileProvider {
    /***
     * Interface method called by [CustomGeometrySource] to request features for a tile.
     *
     * @param bounds [LatLngBounds] of the tile in the map's projection; in a registered one, the box of its corners.
     * @param zoomLevel Tile zoom level.
     * @return Return a @{link FeatureCollection} to be displayed in the requested tile.
     */
    @WorkerThread
    fun getFeaturesForBounds(bounds: LatLngBounds, zoomLevel: Int): FeatureCollection
}
