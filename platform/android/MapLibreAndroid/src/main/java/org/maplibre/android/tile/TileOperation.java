package org.maplibre.android.tile;

public enum TileOperation {
    Requested,
    LoadFromNetwork,
    LoadFromCache,
    StartParse,
    EndParse,
    Error,
    Cancelled
}
