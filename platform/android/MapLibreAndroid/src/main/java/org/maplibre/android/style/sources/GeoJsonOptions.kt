package org.maplibre.android.style.sources

import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.expressions.Expression.ExpressionLiteral
/**
 * Builder class for composing GeoJsonSource objects.
 *
 * @see GeoJsonSource
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/.sources-geojson)
 */
class GeoJsonOptions : HashMap<String?, Any?>() {
    /**
     * Minimum zoom level at which to create vector tiles (lower means more field of view detail at low zoom levels).
     *
     * @param minZoom the minimum zoom - Defaults to 0.
     * @return the current instance for chaining
     */
    fun withMinZoom(minZoom: Int): GeoJsonOptions {
        this["minzoom"] = minZoom
        return this
    }

    /**
     * Maximum zoom level at which to create vector tiles (higher means greater detail at high zoom levels).
     *
     * @param maxZoom the maximum zoom - Defaults to 25.5
     * @return the current instance for chaining
     */
    fun withMaxZoom(maxZoom: Int): GeoJsonOptions {
        this["maxzoom"] = maxZoom
        return this
    }

    /**
     * Tile buffer size on each side (measured in 1/512ths of a tile; higher means fewer rendering artifacts near tile
     * edges but slower performance).
     *
     * @param buffer the buffer size - Defaults to 128.
     * @return the current instance for chaining
     */
    fun withBuffer(buffer: Int): GeoJsonOptions {
        this["buffer"] = buffer
        return this
    }

    /**
     * Initialises whether to calculate line distance metrics.
     *
     * @param lineMetrics true to calculate line distance metrics.
     * @return the current instance for chaining
     */
    fun withLineMetrics(lineMetrics: Boolean): GeoJsonOptions {
        this["lineMetrics"] = lineMetrics
        return this
    }

    /**
     * Douglas-Peucker simplification tolerance (higher means simpler geometries and faster performance).
     *
     * @param tolerance the tolerance - Defaults to 0.375
     * @return the current instance for chaining
     */
    fun withTolerance(tolerance: Float): GeoJsonOptions {
        this["tolerance"] = tolerance
        return this
    }

    /**
     * If the data is a collection of point features, setting this to true clusters the points by radius into groups.
     *
     * @param cluster cluster? - Defaults to false
     * @return the current instance for chaining
     */
    fun withCluster(cluster: Boolean): GeoJsonOptions {
        this["cluster"] = cluster
        return this
    }

    /**
     * Max zoom to cluster points on.
     *
     * @param clusterMaxZoom clusterMaxZoom cluster maximum zoom - Defaults to one zoom less than maxzoom (so that last
     * zoom features are not clustered)
     * @return the current instance for chaining
     */
    fun withClusterMaxZoom(clusterMaxZoom: Int): GeoJsonOptions {
        this["clusterMaxZoom"] = clusterMaxZoom
        return this
    }

    /**
     * Radius of each cluster when clustering points, measured in 1/512ths of a tile.
     *
     * @param clusterRadius cluster radius - Defaults to 50
     * @return the current instance for chaining
     */
    fun withClusterRadius(clusterRadius: Int): GeoJsonOptions {
        this["clusterRadius"] = clusterRadius
        return this
    }

    /**
     * An object defining custom properties on the generated clusters if clustering is enabled,
     * aggregating values from clustered points. Has the form {"property_name": [operator, [map_expression]]} or
     * {"property_name": [[operator, accumulated, expression], [map_expression]]}
     *
     * @param propertyName name of the property
     * @param operatorExpr operatorExpr is any expression function that accepts at least 2 operands (e.g. "+" or "max").
     * It accumulates the property value from clusters/points the cluster contains. It can either be
     * a literal with single operator, or be a valid expression
     * @param mapExpr map expression produces the value of a single point, it shall be a valid expression
     * @return the current instance for chaining
     */
    fun withClusterProperty(propertyName: String, operatorExpr: Expression, mapExpr: Expression): GeoJsonOptions {
        val properties = if (containsKey("clusterProperties")) get("clusterProperties") as HashMap<String, Array<Any>>? else HashMap()
        val operator = if (operatorExpr is ExpressionLiteral) operatorExpr.toValue() else operatorExpr.toArray()
        val map: Any = mapExpr.toArray()
        properties!![propertyName] = arrayOf(operator, map)
        this["clusterProperties"] = properties
        return this
    }
}
