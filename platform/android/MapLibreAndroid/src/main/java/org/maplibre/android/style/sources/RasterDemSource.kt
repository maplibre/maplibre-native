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
        initialize(id, uri, DEFAULT_TILE_SIZE, null)
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
        initialize(id, uri, tileSize, null)
    }

    /**
     * Create the raster dem source from an URL with a specific tile size and elevation encoding.
     *
     *
     * Use this for a source whose tiles are not Mapbox Terrain-RGB, for example a Terrarium
     * tileset behind a `pmtiles://` URL, which has no [TileSet] to carry the encoding.
     *
     * @param id       the source id
     * @param uri      the source url
     * @param tileSize the tile size
     * @param encoding the elevation encoding formula of the tiles, [ENCODING_MAPBOX] or
     * [ENCODING_TERRARIUM]; null keeps the encoding the TileJSON gives, which is "mapbox" by default
     * @throws IllegalArgumentException if the encoding is not one of the supported values
     */
    constructor(id: String?, uri: String?, tileSize: Int, encoding: String?) : super() {
        require(encoding == null || encoding == ENCODING_MAPBOX || encoding == ENCODING_TERRARIUM) {
            "Unsupported encoding \"$encoding\", use \"$ENCODING_MAPBOX\" or \"$ENCODING_TERRARIUM\""
        }
        initialize(id, uri, tileSize, encoding)
    }

    /**
     * Create the raster dem source from a [TileSet]
     *
     * @param id      the source id
     * @param tileSet the [TileSet]
     */
    constructor(id: String?, tileSet: TileSet) : super() {
        initialize(id, tileSet.toValueObject(), DEFAULT_TILE_SIZE, null)
    }

    /**
     * Create the raster source from a [TileSet] with a specific tile size
     *
     * @param id       the source id
     * @param tileSet  the [TileSet]
     * @param tileSize tje tile size
     */
    constructor(id: String?, tileSet: TileSet, tileSize: Int) : super() {
        initialize(id, tileSet.toValueObject(), tileSize, null)
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

    /**
     * Get the elevation encoding formula the source decodes its tiles with.
     *
     * @return "mapbox" or "terrarium"
     */
    val encoding: String
        get() {
            checkThread()
            return nativeGetEncoding()
        }

    @Keep
    protected external fun initialize(layerId: String?, payload: Any?, tileSize: Int, encoding: String?)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()

    @Keep
    private external fun nativeGetUrl(): String

    @Keep
    private external fun nativeGetEncoding(): String

    companion object {
        const val DEFAULT_TILE_SIZE = 512

        /**
         * The elevation encoding of [Mapbox Terrain-RGB](https://docs.mapbox.com/help/troubleshooting/access-elevation-data/#mapbox-terrain-rgb)
         * tiles, the default.
         */
        const val ENCODING_MAPBOX = "mapbox"

        /**
         * The elevation encoding of [Mapzen Terrarium](https://aws.amazon.com/public-datasets/terrain/) tiles.
         */
        const val ENCODING_TERRARIUM = "terrarium"
    }
}
