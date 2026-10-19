package org.maplibre.android.style.sources

sealed class TileData {
    class Mvt(val data: ByteArray) : TileData()

    internal val formatId: Int get() = when (this) {
        is Mvt -> FORMAT_MVT
    }

    internal val bytes: ByteArray get() = when (this) {
        is Mvt -> data
    }

    companion object {
        internal const val FORMAT_MVT = 0
    }
}
