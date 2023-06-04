package org.maplibre.android.offline

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.Keep
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds

/**
 * An offline region defined by a style URL, geographic bounding box, zoom range, and
 * device pixel ratio.
 *
 *
 * Both minZoom and maxZoom must be &#x2265; 0, and maxZoom must be &#x2265; minZoom.
 *
 *
 * maxZoom may be &#x221E;, in which case for each tile source, the region will include
 * tiles from minZoom up to the maximum zoom level provided by that source.
 *
 *
 * pixelRatio must be &#x2265; 0 and should typically be 1.0 or 2.0.
 *
 *
 * if includeIdeographs is false, offline region will not include CJK glyphs
 */
class OfflineTilePyramidRegionDefinition : OfflineRegionDefinition {
    @Keep
    override var styleURL: String?
        private set

    @Keep
    override val bounds: LatLngBounds?

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
     * Constructor to create an OfflineTilePyramidDefinition from parameters.
     *
     * @param styleURL   the style
     * @param bounds     the bounds
     * @param minZoom    min zoom
     * @param maxZoom    max zoom
     * @param pixelRatio pixel ratio of the device
     */
    @Keep
    constructor(styleURL: String?, bounds: LatLngBounds, minZoom: Double, maxZoom: Double, pixelRatio: Float) : this(styleURL, bounds, minZoom, maxZoom, pixelRatio, false) {
    }

    /**
     * Constructor to create an OfflineTilePyramidDefinition from parameters.
     *
     * @param styleURL   the style
     * @param bounds     the bounds
     * @param minZoom    min zoom
     * @param maxZoom    max zoom
     * @param pixelRatio pixel ratio of the device
     * @param includeIdeographs include glyphs for CJK languages
     */
    @Keep
    constructor(styleURL: String?, bounds: LatLngBounds, minZoom: Double, maxZoom: Double, pixelRatio: Float, includeIdeographs: Boolean) {
        // Note: Also used in JNI
        this.styleURL = styleURL
        this.bounds = bounds
        this.minZoom = minZoom
        this.maxZoom = maxZoom
        this.pixelRatio = pixelRatio
        this.includeIdeographs = includeIdeographs
    }

    /**
     * Constructor to create an OfflineTilePyramidDefinition from a Parcel.
     *
     * @param parcel the parcel to create the OfflineTilePyramidDefinition from
     */
    constructor(parcel: Parcel) {
        styleURL = parcel.readString()
        bounds = LatLngBounds.Builder().include(LatLng(parcel.readDouble(), parcel.readDouble())).include(LatLng(parcel.readDouble(), parcel.readDouble())).build()
        minZoom = parcel.readDouble()
        maxZoom = parcel.readDouble()
        pixelRatio = parcel.readFloat()
        includeIdeographs = parcel.readByte().toInt() != 0
    }

    override val type: String
        get() = "tileregion"

    /*
   * Parceable
   */
    override fun describeContents(): Int {
        return 0
    }

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(styleURL)
        if (bounds != null) {
            dest.writeDouble(bounds.latitudeNorth)
            dest.writeDouble(bounds.longitudeEast)
            dest.writeDouble(bounds.latitudeSouth)
            dest.writeDouble(bounds.longitudeWest)
        }
        dest.writeDouble(minZoom)
        dest.writeDouble(maxZoom)
        dest.writeFloat(pixelRatio)
        dest.writeByte((if (includeIdeographs) 1 else 0).toByte())
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<*> = object : Parcelable.Creator<Any?> {
            override fun createFromParcel(`in`: Parcel): OfflineTilePyramidRegionDefinition? {
                return OfflineTilePyramidRegionDefinition(`in`)
            }

            override fun newArray(size: Int): Array<OfflineTilePyramidRegionDefinition?> {
                return arrayOfNulls(size)
            }
        }
    }
}
