package org.maplibre.android.style.sources

import androidx.annotation.Keep
import androidx.annotation.UiThread
import java.net.URI
import java.net.URL

/**
 * A raster DEM source.
 *
 * @see [The style specification](https://maplibre.org/maplibre-style-spec/.sources-raster-dem)
 */

@UiThread
class RasterDemSource : Source {
    /**
     * Internal use
     *
     * @param nativePtr - pointer to native peer
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr) {
    }

    /**
     * Create the raster dem source from an URL
     *
     * @param id  the source id
     * @param url the source url
     */
    @Deprecated("use {@link #RasterDemSource(String, URI)} instead")
    constructor(id: String?, url: URL) : this(id, url.toExternalForm()) {
    }

    /**
     * Create the raster dem source from an URI.
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
     * @param uri the source uri
     */
    constructor(id: String?, uri: URI) : this(id, uri.toString()) {}

    /**
     * Create the raster dem source from an URI
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
     * @param uri the source uri
     */
    constructor(id: String?, uri: String?) : super() {
        initialize(id, uri, DEFAULT_TILE_SIZE)
    }

    /**
     * Create the raster source from an URL with a specific tile size
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
     * @param id       the source id
     * @param uri     the source url
     * @param tileSize the tile size
     */
    constructor(id: String?, uri: String?, tileSize: Int) : super() {
        initialize(id, uri, tileSize)
    }

    /**
     * Create the raster dem source from a [TileSet]
     *
     * @param id      the source id
     * @param tileSet the [TileSet]
     */
    constructor(id: String?, tileSet: TileSet) : super() {
        initialize(id, tileSet.toValueObject(), DEFAULT_TILE_SIZE)
    }

    /**
     * Create the raster source from a [TileSet] with a specific tile size
     *
     * @param id       the source id
     * @param tileSet  the [TileSet]
     * @param tileSize tje tile size
     */
    constructor(id: String?, tileSet: TileSet, tileSize: Int) : super() {
        initialize(id, tileSet.toValueObject(), tileSize)
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
    protected external fun initialize(layerId: String?, payload: Any?, tileSize: Int)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    @Keep
    private external fun nativeGetUrl(): String

    companion object {
        const val DEFAULT_TILE_SIZE = 512
    }
}
