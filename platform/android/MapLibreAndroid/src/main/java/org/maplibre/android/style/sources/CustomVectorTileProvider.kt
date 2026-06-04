package org.maplibre.android.style.sources

interface CustomVectorTileProvider {
    suspend fun fetchTile(z: Int, x: Int, y: Int): TileData
}
