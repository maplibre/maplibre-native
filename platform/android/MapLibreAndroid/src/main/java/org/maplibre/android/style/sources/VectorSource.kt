package org.maplibre.android.style.sources

import android.net.Uri
import androidx.annotation.Keep
import androidx.annotation.Size
import androidx.annotation.UiThread
import com.google.gson.JsonObject
import org.maplibre.geojson.Feature
import org.maplibre.android.style.expressions.Expression
import java.net.URL
import java.util.*
import kotlin.collections.ArrayList

/**
 * Vector source, allows the use of vector tiles.
 *
 * @see [the style specification](https://maplibre.org/maplibre-style-spec/.sources-vector)
 */
@UiThread
class VectorSource : Source {
    /**
     * Internal use
     *
     * @param nativePtr - pointer to native peer
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr) {
    }

    /**
     * Create a vector source from a remote url pointing to a TileJSON resource
     *
     * @param id  the source id
     * @param url the TileJSON resource url
     */
    @Deprecated("use {@link #VectorSource(String, Uri)} instead")
    constructor(id: String?, url: URL) : this(id, url.toExternalForm()) {
    }

    /**
     * Create a vector source from an URI.
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
     * @param uri the TileJSON resource uri
     */
    constructor(id: String?, uri: Uri) : this(id, uri.toString()) {}

    /**
     * Create a vector source from an URI.
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
     * @param uri the uri
     */
    constructor(id: String?, uri: String?) : super() {
        initialize(id, uri)
    }

    /**
     * Create a vector source from a tileset
     *
     * @param id      the source id
     * @param tileSet the tileset
     */
    constructor(id: String?, tileSet: TileSet) : super() {
        initialize(id, tileSet.toValueObject())
    }

    /**
     * Sets the state of a feature in this source.
     *
     * Feature state can be read in style expressions through `["feature-state", key]`.
     * The target feature must already have an id in the underlying source data.
     *
     * @param sourceLayerId the source layer id
     * @param featureId     the id of the feature whose state to set
     * @param state         a JSON object with the state key-value pairs to merge
     */
    fun setFeatureState(sourceLayerId: String, featureId: String, state: JsonObject) {
        checkThread()
        nativeSetFeatureState(sourceLayerId, featureId, state)
    }

    /**
     * Gets the current state of a feature in this source.
     *
     * The target feature must already have an id in the underlying source data.
     *
     * @param sourceLayerId the source layer id
     * @param featureId     the id of the feature whose state to get
     * @return the feature state, or null
     */
    fun getFeatureState(sourceLayerId: String, featureId: String): JsonObject? {
        checkThread()
        return nativeGetFeatureState(sourceLayerId, featureId)
    }

    /**
     * Removes state from a feature in this source, or from all features in a source layer
     * when [featureId] is null.
     *
     * @param sourceLayerId the source layer id
     * @param featureId     the id of the feature, or null to target all features
     * @param stateKey      the state key to remove, or null to remove all keys
     */
    fun removeFeatureState(sourceLayerId: String, featureId: String?, stateKey: String?) {
        checkThread()
        nativeRemoveFeatureState(sourceLayerId, featureId, stateKey)
    }

    /**
     * Removes all feature state entries from the given source layer.
     *
     * @param sourceLayerId the source layer id
     */
    fun resetFeatureStates(sourceLayerId: String) {
        checkThread()
        nativeRemoveFeatureState(sourceLayerId, null, null)
    }

    /**
     * Queries the source for features.
     *
     * @param sourceLayerIds the source layer identifiers. At least one must be specified.
     * @param filter         an optional filter expression to filter the returned Features
     * @return the features
     */
    fun querySourceFeatures(@Size(min = 1) sourceLayerIds: Array<String>, filter: Expression?): List<Feature> {
        checkThread()
        val features = querySourceFeatures(sourceLayerIds, filter?.toArray())
        return listOf(*features)
    }

    /**
     * @return The url or null
     */
    @get:Deprecated("use {@link #getUri()} instead")
    val url: String?
        get() {
            checkThread()
            return nativeGetUrl()
        }

    /**
     * Get the source URI.
     *
     * @return The uri or null
     */
    val uri: String?
        get() {
            checkThread()
            return nativeGetUrl()
        }

    @Keep
    protected external fun initialize(layerId: String?, payload: Any?)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    @Keep
    protected external fun nativeGetUrl(): String?

    @Keep
    private external fun querySourceFeatures(sourceLayerId: Array<String>, filter: Array<Any>?): Array<Feature>
}
