package org.maplibre.android.annotations

import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource

interface CoreElementProvider<L : Layer> {
    val layerId: String
    val sourceId: String
    val layer: L
    fun getSource(geoJsonOptions: GeoJsonOptions?): GeoJsonSource
}
