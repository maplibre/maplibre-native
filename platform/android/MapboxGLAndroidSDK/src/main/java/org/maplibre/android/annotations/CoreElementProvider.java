package org.maplibre.android.annotations;

import androidx.annotation.Nullable;

import org.maplibre.android.style.layers.Layer;
import org.maplibre.android.style.sources.GeoJsonOptions;
import org.maplibre.android.style.sources.GeoJsonSource;

interface CoreElementProvider<L extends Layer> {

    String getLayerId();

    String getSourceId();

    L getLayer();

    GeoJsonSource getSource(@Nullable GeoJsonOptions geoJsonOptions);
}
