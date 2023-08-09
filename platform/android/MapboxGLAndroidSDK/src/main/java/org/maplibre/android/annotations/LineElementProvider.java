package org.maplibre.android.annotations;

import androidx.annotation.Nullable;

import org.maplibre.android.style.layers.LineLayer;
import org.maplibre.android.style.sources.GeoJsonOptions;
import org.maplibre.android.style.sources.GeoJsonSource;

import java.util.concurrent.atomic.AtomicLong;

/**
 * Concrete instance of a core element provider for Line.
 */
class LineElementProvider implements CoreElementProvider<LineLayer> {

    private static final AtomicLong ID_GENERATOR = new AtomicLong(0);
    private static final String ID_GEOJSON_LAYER = "mapbox-android-line-layer-%s";
    private static final String ID_GEOJSON_SOURCE = "mapbox-android-line-source-%s";

    private final String layerId;
    private final String sourceId;

    LineElementProvider() {
        long id = ID_GENERATOR.incrementAndGet();
        this.layerId = String.format(ID_GEOJSON_LAYER, id);
        this.sourceId = String.format(ID_GEOJSON_SOURCE, id);
    }

    @Override
    public String getLayerId() {
        return layerId;
    }

    @Override
    public String getSourceId() {
        return sourceId;
    }

    @Override
    public LineLayer getLayer() {
        return new LineLayer(layerId, sourceId);
    }

    @Override
    public GeoJsonSource getSource(@Nullable GeoJsonOptions geoJsonOptions) {
        return new GeoJsonSource(sourceId, geoJsonOptions);
    }
}