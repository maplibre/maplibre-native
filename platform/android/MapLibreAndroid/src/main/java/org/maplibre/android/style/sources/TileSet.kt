package org.maplibre.android.style.sources

import androidx.annotation.Size
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds

/**
 * Tile set, allows using TileJson specification as source.
 * Note that `encoding` is only relevant to `raster-dem` sources, and is not supported in the TileJson spec.
 *
 * @see [The tileset specification](https://github.com/mapbox/tilejson-spec/tree/master/2.1.0)
 */
class TileSet(val tilejson: String, vararg tiles: String) {

    /**
     * A name describing the tileset. The name can
     * contain any legal character. Implementations SHOULD NOT interpret the
     * name as HTML.
     * "name": "compositing",
     *
     * @param name the name to be set
     */
    var name: String? = null

    /**
     * A text description of the tileset. The
     * description can contain any legal character.
     * Implementations SHOULD NOT
     * interpret the description as HTML.
     * "description": "A simple, light grey world."
     *
     * @param description the description to set
     */
    var description: String? = null
    var version: String? = null

    /**
     * Default: null. Contains an attribution to be displayed
     * when the map is shown to a user. Implementations MAY decide to treat this
     * as HTML or literal text. For security reasons, make absolutely sure that
     * this field can't be abused as a vector for XSS or beacon tracking.
     * "attribution": "[OSM contributors](http:openstreetmap.org)",
     *
     * @param attribution the attribution to set
     */
    var attribution: String? = null

    /**
     * Contains a mustache template to be used to
     * format data from grids for interaction.
     * See https:github.com/mapbox/utfgrid-spec/tree/master/1.2
     * for the interactivity specification.
     * "template": "{{#__teaser__}}{{NAME}}{{/__teaser__}}"
     *
     * @param template the template to set
     */
    var template: String? = null

    /**
     * Contains a legend to be displayed with the map.
     * Implementations MAY decide to treat this as HTML or literal text.
     * For security reasons, make absolutely sure that this field can't be
     * abused as a vector for XSS or beacon tracking.
     * "legend": "Dangerous zones are red, safe zones are green"
     *
     * @param legend the legend to set
     */
    var legend: String? = null

    /**
     * Default: "xyz". Either "xyz" or "tms". Influences the y
     * direction of the tile coordinates.
     * The global-mercator (aka Spherical Mercator) profile is assumed.
     * "scheme": "xyz"
     *
     * @param scheme the scheme to set
     */
    var scheme: String? = null
    val tiles: Array<String>
    var grids: Array<String>? = null
        private set
    var data: Array<String>? = null
        private set

    @JvmField
    var minZoom: Float? = null

    @JvmField
    var maxZoom: Float? = null

    var bounds: Array<Float>? = null
        private set
    var center: Array<Float>? = null
        private set

    /**
     * Default: "mapbox". The encoding formula for a raster-dem tileset.
     * Supported values are "mapbox" and "terrarium".
     *
     * @param encoding the String encoding formula to set
     */
    var encoding: String? = null

    /**
     * @param tilejson A semver.org style version number. Describes the version of the TileJSON spec that is implemented
     * by this JSON object.
     * @param tiles    An array of tile endpoints. {z}, {x} and {y}, if present, are replaced with the corresponding
     * integers.
     * If multiple endpoints are specified, clients may use any combination of endpoints. All endpoints
     * MUST return the same
     * content for the same URL. The array MUST contain at least one endpoint.
     * Example: "http:localhost:8888/admin/1.0.0/world-light,broadband/{z}/{x}/{y}.png"
     */
    init {
        this.tiles = arrayOf(*tiles)
    }

    /**
     * An array of interactivity endpoints. {z}, {x}
     * and {y}, if present, are replaced with the corresponding integers. If multiple
     * endpoints are specified, clients may use any combination of endpoints.
     * All endpoints MUST return the same content for the same URL.
     * If the array doesn't contain any entries, interactivity is not supported
     * for this tileset.     See https:github.com/mapbox/utfgrid-spec/tree/master/1.2
     * for the interactivity specification.
     *
     *
     * Example: "http:localhost:8888/admin/1.0.0/broadband/{z}/{x}/{y}.grid.json"
     *
     *
     * @param grids the grids to set
     */
    @Suppress("unused")
    fun setGrids(vararg grids: String) {
        this.grids = arrayOf(*grids)
    }

    /**
     * An array of data files in GeoJSON format.
     * {z}, {x} and {y}, if present,
     * are replaced with the corresponding integers. If multiple
     * endpoints are specified, clients may use any combination of endpoints.
     * All endpoints MUST return the same content for the same URL.
     * If the array doesn't contain any entries, then no data is present in
     * the map.
     *
     *
     * "http:localhost:8888/admin/data.geojson"
     *
     *
     * @param data the data array to set
     */
    fun setData(vararg data: String) {
        this.data = arrayOf(*data)
    }

    fun getMinZoom(): Float {
        return minZoom!!
    }

    /**
     * 0. &gt;= 0, &lt; 22. An integer specifying the minimum zoom level.
     *
     * @param minZoom the minZoom level to set
     */
    fun setMinZoom(minZoom: Float) {
        this.minZoom = minZoom
    }

    fun getMaxZoom(): Float {
        return maxZoom!!
    }

    /**
     * 0. &gt;= 0, &lt;= 22. An integer specifying the maximum zoom level.
     *
     * @param maxZoom the maxZoom level to set
     */
    fun setMaxZoom(maxZoom: Float) {
        this.maxZoom = maxZoom
    }

    /**
     * Default: [-180, -90, 180, 90]. The maximum extent of available map tiles. Bounds MUST define an area
     * covered by all zoom levels. The bounds are represented in WGS:84
     * latitude and longitude values, in the order left, bottom, right, top.
     * Values may be integers or floating point numbers.
     *
     * @param bounds the Float array to set
     */
    fun setBounds(@Size(value = 4) vararg bounds: Float) {
        this.bounds = bounds.toTypedArray()
    }

    /**
     * Default: [-180, -90, 180, 90]. The maximum extent of available map tiles. Bounds MUST define an area
     * covered by all zoom levels. The bounds are represented in WGS:84
     * latitude and longitude values, in the order left, bottom, right, top.
     * Values are floating point numbers.
     *
     * @param left the Float left bound
     * @param bottom the Float bottom bound
     * @param right the Float right bound
     * @param top the Float top bound
     */
    fun setBounds(left: Float, bottom: Float, right: Float, top: Float) {
        setBounds(left, bottom, right, top)
    }

    /**
     * Default: [-180, -90, 180, 90]. The maximum extent of available map tiles. Bounds MUST define an area
     * covered by all zoom levels. The bounds are represented in WGS:84
     * latitude and longitude values, as a Float array of exactly four elements in the order
     * left, bottom, right, top. They are all floating point numbers.
     *
     * @param bounds The Array of floats containing bounds in the order left, bottom, right, top
     */
    @Deprecated("Not strongly typed", ReplaceWith("setBounds(bounds: LatLngBounds"))
    fun setBounds(@Size(value = 4) bounds: Array<Float>) {
        this.bounds = bounds
    }

    /**
     * Default: [-180, -90, 180, 90]. The maximum extent of available map tiles. Bounds MUST define an area
     * covered by all zoom levels. The bounds are represented in WGS:84
     *
     * @param bounds The LatLngBounds instance containing bounds
     */
    fun setBounds(bounds: LatLngBounds) {
        setBounds(bounds.longitudeWest.toFloat(), bounds.latitudeSouth.toFloat(), bounds.longitudeEast.toFloat(), bounds.latitudeNorth.toFloat())
    }

    /**
     * The first value is the longitude, the second is latitude (both in
     * WGS:84 values),
     * Longitude and latitude MUST be within the specified bounds.
     * Implementations can use this value to set the default location. If the
     * value is null, implementations may use their own algorithm for
     * determining a default location.
     *
     * @param center the Float array to set as lattitude, longitude
     */
    @Deprecated("This function is not type safe", ReplaceWith("setCenter(center:LatLng)"))
    fun setCenter(@Size(value = 2) vararg center: Float) {
        val latLng = LatLng(center[1].toDouble(), center[0].toDouble())
        setCenter(latLng)
    }

    /**
     * Set the center.
     * Longitude and latitude MUST be within the specified bounds.
     * Implementations can use this value to set the default location. If the
     * value is null, implementations may use their own algorithm for
     * determining a default location.
     *
     * @param center the LatLng value to use a the center
     */
    fun setCenter(center: LatLng) {
        this.center = arrayOf(center.longitude.toFloat(), center.latitude.toFloat())
    }

    fun toValueObject(): Map<String, Any> {
        val result: MutableMap<String, Any> = HashMap()
        result["tilejson"] = tilejson
        result["tiles"] = tiles
        if (name != null) {
            result["name"] = name!!
        }
        if (description != null) {
            result["description"] = description!!
        }
        if (version != null) {
            result["version"] = version!!
        }
        if (attribution != null) {
            result["attribution"] = attribution!!
        }
        if (template != null) {
            result["template"] = template!!
        }
        if (legend != null) {
            result["legend"] = legend!!
        }
        if (scheme != null) {
            result["scheme"] = scheme!!
        }
        if (grids != null) {
            result["grids"] = grids!!
        }
        if (data != null) {
            result["data"] = data!!
        }
        if (minZoom != null) {
            result["minzoom"] = minZoom!!
        }
        if (maxZoom != null) {
            result["maxzoom"] = maxZoom!!
        }
        if (bounds != null) {
            result["bounds"] = bounds!!
        }
        if (center != null) {
            result["center"] = center!!
        }
        if (encoding != null) {
            result["encoding"] = encoding!!
        }
        return result
    }
}
