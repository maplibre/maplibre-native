package org.maplibre.android.annotations

import java.util.concurrent.atomic.AtomicLong
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource

/**
 * Concrete instance of a core element provider for Fill.
 */
internal class FillElementProvider : CoreElementProvider<FillLayer> {
    override val layerId: String
    override val sourceId: String

    init {
        val id = ID_GENERATOR.incrementAndGet()
        layerId = String.format(ID_GEOJSON_LAYER, id)
        sourceId = String.format(ID_GEOJSON_SOURCE, id)
    }

    override val layer: FillLayer
        get() = FillLayer(layerId, sourceId)

    override fun getSource(geoJsonOptions: GeoJsonOptions?): GeoJsonSource =
        GeoJsonSource(sourceId, geoJsonOptions)

    companion object {
        private val ID_GENERATOR = AtomicLong(0)
        private const val ID_GEOJSON_LAYER = "mapbox-android-fill-layer-%s"
        private const val ID_GEOJSON_SOURCE = "mapbox-android-fill-source-%s"
    }
}
