package org.maplibre.android.offline

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.Keep
import org.maplibre.geojson.Feature
import org.maplibre.geojson.Geometry
import org.maplibre.turf.TurfMeasurement
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.geometry.LatLngBounds.Companion.from

/**
 * An offline region defined by a style URL, geometry, zoom range, and
 * device pixel ratio.
 *
 *
 * Both minZoom and maxZoom must be ≥ 0, and maxZoom must be ≥ minZoom.
 *
 *
 * maxZoom may be ∞, in which case for each tile source, the region will include
 * tiles from minZoom up to the maximum zoom level provided by that source.
 *
 *
 * pixelRatio must be ≥ 0 and should typically be 1.0 or 2.0.
 *
 *
 * if includeIdeographs is false, offline region will not include CJK glyphs
 */
class OfflineGeometryRegionDefinition : OfflineRegionDefinition {
    @Keep
    override var styleURL: String?
        private set

    @Keep
    var geometry: Geometry?
        private set

    @Keep
    override var minZoom: Double
        private set

    @Keep
    override var maxZoom: Double
        private set

    @Keep
    override var pixelRatio: Float
        private set

    @Keep
    override var includeIdeographs: Boolean
        private set

    /**
     * Constructor to create an OfflineGeometryRegionDefinition from parameters.
     *
     * @param styleURL   the style
     * @param geometry   the geometry
     * @param minZoom    min zoom
     * @param maxZoom    max zoom
     * @param pixelRatio pixel ratio of the device
     */
    @Keep
    constructor(styleURL: String?, geometry: Geometry?, minZoom: Double, maxZoom: Double, pixelRatio: Float) : this(styleURL, geometry, minZoom, maxZoom, pixelRatio, false)

    /**
     * Constructor to create an OfflineGeometryRegionDefinition from parameters.
     *
     * @param styleURL   the style
     * @param geometry   the geometry
     * @param minZoom    min zoom
     * @param maxZoom    max zoom
     * @param pixelRatio pixel ratio of the device
     * @param includeIdeographs include glyphs for CJK languages
     */
    @Keep
    constructor(styleURL: String?, geometry: Geometry?, minZoom: Double, maxZoom: Double, pixelRatio: Float, includeIdeographs: Boolean) {
        // Note: Also used in JNI
        this.styleURL = styleURL
        this.geometry = geometry
        this.minZoom = minZoom
        this.maxZoom = maxZoom
        this.pixelRatio = pixelRatio
        this.includeIdeographs = includeIdeographs
    }

    /**
     * Constructor to create an OfflineGeometryRegionDefinition from a Parcel.
     *
     * @param parcel the parcel to create the OfflineGeometryRegionDefinition from
     */
    constructor(parcel: Parcel) {
        styleURL = parcel.readString()
        geometry = Feature.fromJson(parcel.readString()!!).geometry()
        minZoom = parcel.readDouble()
        maxZoom = parcel.readDouble()
        pixelRatio = parcel.readFloat()
        includeIdeographs = parcel.readByte().toInt() != 0
    }

    /**
     * Calculates the bounding box for the Geometry it contains
     * to retain backwards compatibility
     *
     * @return the [LatLngBounds] or null
     */
    override val bounds: LatLngBounds?
        get() {
            if (geometry == null) {
                return null
            }
            val bbox = TurfMeasurement.bbox(geometry)
            return from(bbox[3], bbox[2], bbox[1], bbox[0])
        }

    override val type: String
        get() = "shaperegion"

    /*
   * Parceable
   */
    override fun describeContents(): Int {
        return 0
    }

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(styleURL)
        dest.writeString(Feature.fromGeometry(geometry).toJson())
        dest.writeDouble(minZoom)
        dest.writeDouble(maxZoom)
        dest.writeFloat(pixelRatio)
        dest.writeByte((if (includeIdeographs) 1 else 0).toByte())
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<*> = object : Parcelable.Creator<Any?> {
            override fun createFromParcel(`in`: Parcel): OfflineGeometryRegionDefinition? {
                return OfflineGeometryRegionDefinition(`in`)
            }

            override fun newArray(size: Int): Array<OfflineGeometryRegionDefinition?> {
                return arrayOfNulls(size)
            }
        }
    }
}
