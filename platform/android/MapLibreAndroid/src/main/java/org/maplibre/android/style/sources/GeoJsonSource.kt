package org.maplibre.android.style.sources

import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Geometry
import org.maplibre.android.style.expressions.Expression
import java.net.URI
import java.net.URL
import java.util.*

/**
 * GeoJson source, allows using FeatureCollections from Json.
 *
 * @see [the style specification](https://maplibre.org/maplibre-style-spec/.sources-geojson)
 */
@UiThread
class GeoJsonSource : Source {
    /**
     * Internal use
     *
     * @param nativePtr - pointer to native peer
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr) {
    }

    /**
     * Create an empty GeoJsonSource
     *
     * @param id the source id
     */
    constructor(id: String?) : super() {
        initialize(id, null)
        setGeoJson(FeatureCollection.fromFeatures(ArrayList()))
    }

    /**
     * Create an empty GeoJsonSource with non-default GeoJsonOptions.
     *
     * @param id      the source id
     * @param options options
     */
    constructor(id: String?, options: GeoJsonOptions?) : super() {
        initialize(id, options)
        setGeoJson(FeatureCollection.fromFeatures(ArrayList()))
    }

    /**
     * Create a GeoJsonSource from a raw json string
     *
     * @param id      the source id
     * @param geoJson raw Json FeatureCollection
     */
    constructor(id: String?, geoJson: String?) : super() {
        require(!(geoJson == null || geoJson.startsWith("http"))) { "Expected a raw json body" }
        initialize(id, null)
        setGeoJson(geoJson)
    }

    /**
     * Create a GeoJsonSource from a raw json string and non-default GeoJsonOptions
     *
     * @param id      the source id
     * @param geoJson raw Json body
     * @param options options
     */
    constructor(id: String?, geoJson: String?, options: GeoJsonOptions?) : super() {
        require(!(geoJson == null || geoJson.startsWith("http") || geoJson.startsWith("asset") || geoJson.startsWith("file"))) { "Expected a raw json body" }
        initialize(id, options)
        setGeoJson(geoJson)
    }

    /**
     * Create a GeoJsonSource from a remote geo json file
     *
     * @param id  the source id
     * @param url remote json file
     */
    @Deprecated("use {@link #GeoJsonSource(String, URI)} instead")
    constructor(id: String?, url: URL) : super() {
        initialize(id, null)
        nativeSetUrl(url.toExternalForm())
    }

    /**
     * Create a GeoJsonSource from a remote geo json file and non-default GeoJsonOptions
     *
     * @param id      the source id
     * @param url     remote json file
     * @param options options
     */
    @Deprecated("use {@link #GeoJsonSource(String, URI, GeoJsonOptions)} instead")
    constructor(id: String?, url: URL, options: GeoJsonOptions?) : super() {
        initialize(id, options)
        nativeSetUrl(url.toExternalForm())
    }

    /**
     * Create a GeoJsonSource from a geo json URI
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param id  the source id
     * @param uri unique resource identifier
     */
    constructor(id: String?, uri: URI) : super() {
        initialize(id, null)
        nativeSetUrl(uri.toString())
    }

    /**
     * Create a GeoJsonSource from a geo json URI and non-default GeoJsonOptions
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param id      the source id
     * @param uri     remote json file
     * @param options options
     */
    constructor(id: String?, uri: URI, options: GeoJsonOptions?) : super() {
        initialize(id, options)
        nativeSetUrl(uri.toString())
    }

    /**
     * Create a GeoJsonSource from a FeatureCollection.
     *
     * @param id       the source id
     * @param features the features
     */
    constructor(id: String?, features: FeatureCollection?) : super() {
        initialize(id, null)
        setGeoJson(features)
    }

    /**
     * Create a GeoJsonSource from a FeatureCollection and non-default GeoJsonOptions.
     *
     * @param id       the source id
     * @param features the features
     * @param options  options
     */
    constructor(id: String?, features: FeatureCollection?, options: GeoJsonOptions?) : super() {
        initialize(id, options)
        setGeoJson(features)
    }

    /**
     * Create a GeoJsonSource from a [Feature]
     *
     * @param id      the source id
     * @param feature the feature
     */
    constructor(id: String?, feature: Feature?) : super() {
        initialize(id, null)
        setGeoJson(feature)
    }

    /**
     * Create a GeoJsonSource from a [Feature] and non-default [GeoJsonOptions]
     *
     * @param id      the source id
     * @param feature the feature
     * @param options options
     */
    constructor(id: String?, feature: Feature?, options: GeoJsonOptions?) : super() {
        initialize(id, options)
        setGeoJson(feature)
    }

    /**
     * Create a GeoJsonSource from a [Geometry]
     *
     * @param id       the source id
     * @param geometry the geometry
     */
    constructor(id: String?, geometry: Geometry?) : super() {
        initialize(id, null)
        setGeoJson(geometry)
    }

    /**
     * Create a GeoJsonSource from a [Geometry] and non-default [GeoJsonOptions]
     *
     * @param id       the source id
     * @param geometry the geometry
     * @param options  options
     */
    constructor(id: String?, geometry: Geometry?, options: GeoJsonOptions?) : super() {
        initialize(id, options)
        setGeoJson(geometry)
    }

    /**
     * Updates the GeoJson with a single feature. The update is performed asynchronously,
     * so the data won't be immediately visible or available to query when this method returns.
     *
     * @param feature the GeoJSON [Feature] to set
     */
    fun setGeoJson(feature: Feature?) {
        if (detached) {
            return
        }
        checkThread()
        nativeSetFeature(feature)
    }

    /**
     * Updates the GeoJson with a single geometry. The update is performed asynchronously,
     * so the data won't be immediately visible or available to query when this method returns.
     *
     * @param geometry the GeoJSON [Geometry] to set
     */
    fun setGeoJson(geometry: Geometry?) {
        if (detached) {
            return
        }
        checkThread()
        nativeSetGeometry(geometry)
    }

    /**
     * Updates the GeoJson. The update is performed asynchronously,
     * so the data won't be immediately visible or available to query when this method returns.
     *
     * @param featureCollection the GeoJSON FeatureCollection
     */
    fun setGeoJson(featureCollection: FeatureCollection?) {
        if (detached) {
            return
        }
        checkThread()
        if (featureCollection != null && featureCollection.features() != null) {
            val features = featureCollection.features()
            val featuresCopy: List<Feature> = ArrayList(features)
            nativeSetFeatureCollection(FeatureCollection.fromFeatures(featuresCopy))
        } else {
            nativeSetFeatureCollection(featureCollection)
        }
    }

    /**
     * Updates the GeoJson. The update is performed asynchronously,
     * so the data won't be immediately visible or available to query when this method returns.
     *
     * @param json the raw GeoJson FeatureCollection string
     */
    fun setGeoJson(json: String) {
        if (detached) {
            return
        }
        checkThread()
        nativeSetGeoJsonString(json)
    }

    /**
     * Updates the url
     *
     * @param url the GeoJSON FeatureCollection url
     */
    @Deprecated("use {@link #setUri(URI)} instead")
    fun setUrl(url: URL) {
        checkThread()
        this.url = url.toExternalForm()
    }

    /**
     * Updates the URI of the source.
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param uri the GeoJSON FeatureCollection uri
     */
    fun setUri(uri: URI) {
        setUri(uri.toString())
    }

    /**
     * Updates the URI of the source.
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param uri the GeoJSON FeatureCollection uri
     */
    fun setUri(uri: String?) {
        checkThread()
        nativeSetUrl(uri)
    }

    private fun setUrlInternal(url: String?) {
        checkThread()
        nativeSetUrl(url)
    }

    private fun getUrlInternal(): String? {
        checkThread()
        return nativeGetUrl()
    }

    var url: String?

        /**
         * @return The url or null
         */
        @get:Deprecated("use {@link #getUri()} instead")
        get() {
            return getUrlInternal()
        }

        /**
         * Updates the url
         *
         * @param url the GeoJSON FeatureCollection url
         */
        @Deprecated("use {@link #setUri(String)} instead")
        set(value) {
            setUrlInternal(value)
        }

    /**
     * Get the URI of the source.
     *
     * @return The uri or null
     */
    val uri: String?
        get() {
            checkThread()
            return nativeGetUrl()
        }

    /**
     * Queries the source for features.
     *
     * @param filter an optional filter expression to filter the returned Features
     * @return the features
     */
    fun querySourceFeatures(filter: Expression?): List<Feature> {
        checkThread()
        val features = querySourceFeatures(filter?.toArray())
        return if (features != null) Arrays.asList(*features) else ArrayList()
    }

    /**
     * Returns the children of a cluster (on the next zoom level) given its id (cluster_id value from feature properties).
     *
     *
     * Requires configuring this source as a cluster by calling [GeoJsonOptions.withCluster].
     *
     *
     * @param cluster cluster from which to retrieve children from
     * @return a list of features for the underlying children
     */
    fun getClusterChildren(cluster: Feature): FeatureCollection {
        checkThread()
        return FeatureCollection.fromFeatures(nativeGetClusterChildren(cluster)!!)
    }

    /**
     * Returns all the leaves of a cluster (given its cluster_id), with pagination support: limit is the number of leaves
     * to return (set to Infinity for all points), and offset is the amount of points to skip (for pagination).
     *
     *
     * Requires configuring this source as a cluster by calling [GeoJsonOptions.withCluster].
     *
     *
     * @param cluster cluster from which to retrieve leaves from
     * @param limit   limit is the number of points to return
     * @param offset  offset is the amount of points to skip (for pagination)
     * @return a list of features for the underlying leaves
     */
    fun getClusterLeaves(cluster: Feature, limit: Long, offset: Long): FeatureCollection {
        checkThread()
        return FeatureCollection.fromFeatures(nativeGetClusterLeaves(cluster, limit, offset)!!)
    }

    /**
     * Returns the zoom on which the cluster expands into several children (useful for "click to zoom" feature)
     * given the cluster's cluster_id (cluster_id value from feature properties).
     *
     *
     * Requires configuring this source as a cluster by calling [GeoJsonOptions.withCluster].
     *
     *
     * @param cluster cluster from which to retrieve the expansion zoom from
     * @return the zoom on which the cluster expands into several children
     */
    fun getClusterExpansionZoom(cluster: Feature): Int {
        checkThread()
        return nativeGetClusterExpansionZoom(cluster)
    }

    @Keep
    protected external fun initialize(layerId: String?, options: Any?)

    @Keep
    protected external fun nativeSetUrl(url: String?)

    @Keep
    protected external fun nativeGetUrl(): String?

    @Keep
    private external fun nativeSetGeoJsonString(geoJson: String)

    @Keep
    private external fun nativeSetFeatureCollection(geoJson: FeatureCollection?)

    @Keep
    private external fun nativeSetFeature(feature: Feature?)

    @Keep
    private external fun nativeSetGeometry(geometry: Geometry?)

    @Keep
    private external fun querySourceFeatures(filter: Array<Any>?): Array<Feature>

    @Keep
    private external fun nativeGetClusterChildren(feature: Feature): Array<Feature?>?

    @Keep
    private external fun nativeGetClusterLeaves(feature: Feature, limit: Long, offset: Long): Array<Feature?>?

    @Keep
    private external fun nativeGetClusterExpansionZoom(feature: Feature): Int

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()
}
