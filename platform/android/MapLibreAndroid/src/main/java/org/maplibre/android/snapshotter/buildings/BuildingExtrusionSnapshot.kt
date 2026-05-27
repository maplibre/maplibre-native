package org.maplibre.android.snapshotter.buildings

import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.FillExtrusionLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.geojson.FeatureCollection

/**
 * Helpers for adding **fill-extrusion** building layers to styles used with
 * [org.maplibre.android.snapshotter.MapSnapshotter].
 *
 * [MapSnapshotter] must be created and used on the main thread (see [org.maplibre.android.snapshotter.MapSnapshotter]).
 * Pass the returned [Style.Builder] from [applyToStyleBuilder] to
 * [org.maplibre.android.snapshotter.MapSnapshotter.Options.withStyleBuilder].
 *
 * For large datasets, filter features with [GeoJsonViewportFilter.filterFeaturesInBounds] before building the source.
 */
object BuildingExtrusionSnapshot {

    /** Default id for the [GeoJsonSource] added by [applyToStyleBuilder]. */
    const val DEFAULT_SOURCE_ID: String = "maplibre-snapshot-buildings"

    /** Default id for the [FillExtrusionLayer] added by [applyToStyleBuilder]. */
    const val DEFAULT_LAYER_ID: String = "maplibre-snapshot-buildings-extrusion"

    /**
     * Creates a [GeoJsonSource] containing the given GeoJSON features.
     */
    @JvmStatic
    @JvmOverloads
    fun createGeoJsonSource(
        sourceId: String = DEFAULT_SOURCE_ID,
        features: FeatureCollection
    ): GeoJsonSource {
        return GeoJsonSource(sourceId, features)
    }

    /**
     * Creates a [FillExtrusionLayer] with height read from a numeric feature property (default `"height"`).
     * [minZoom] / [maxZoom] control when the layer participates in rendering (style-spec layer zoom range).
     */
    @JvmStatic
    @JvmOverloads
    fun createFillExtrusionLayer(
        layerId: String = DEFAULT_LAYER_ID,
        sourceId: String = DEFAULT_SOURCE_ID,
        minZoom: Float = 14f,
        maxZoom: Float = 24f,
        heightProperty: String = "height",
        fillExtrusionColor: String = "#9e9e9e",
        fillExtrusionOpacity: Float = 0.9f
    ): FillExtrusionLayer {
        val layer = FillExtrusionLayer(layerId, sourceId)
        layer.setMinZoom(minZoom)
        layer.setMaxZoom(maxZoom)
        layer.setProperties(
            PropertyFactory.fillExtrusionHeight(Expression.get(heightProperty)),
            PropertyFactory.fillExtrusionColor(fillExtrusionColor),
            PropertyFactory.fillExtrusionOpacity(fillExtrusionOpacity)
        )
        return layer
    }

    /**
     * Adds a [GeoJsonSource] and a [FillExtrusionLayer] above [aboveLayerId] in the style loaded by the snapshotter.
     * Choose [aboveLayerId] from a layer id that exists in your base style (for example a label or symbol layer).
     */
    @JvmStatic
    @JvmOverloads
    fun applyToStyleBuilder(
        builder: Style.Builder,
        featureCollection: FeatureCollection,
        aboveLayerId: String,
        sourceId: String = DEFAULT_SOURCE_ID,
        layerId: String = DEFAULT_LAYER_ID,
        minZoom: Float = 14f,
        maxZoom: Float = 24f,
        heightProperty: String = "height",
        fillExtrusionColor: String = "#9e9e9e",
        fillExtrusionOpacity: Float = 0.9f
    ): Style.Builder {
        val source = createGeoJsonSource(sourceId, featureCollection)
        val layer = createFillExtrusionLayer(
            layerId,
            sourceId,
            minZoom,
            maxZoom,
            heightProperty,
            fillExtrusionColor,
            fillExtrusionOpacity
        )
        return builder.withSource(source).withLayerAbove(layer, aboveLayerId)
    }
}
