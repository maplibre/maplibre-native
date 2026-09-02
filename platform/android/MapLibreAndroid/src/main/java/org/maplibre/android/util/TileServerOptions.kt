package org.maplibre.android.util

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.Keep
import org.maplibre.android.WellKnownTileServer

/**
 * Tile server options - baseUrl and similar properties
 *
 * @param baseURL              tile server base url
 * @param uriSchemeAlias       scheme alias such as maplibre://
 * @param sourceTemplate       tile source url template
 * @param sourceDomainName     tile source domain name
 * @param sourceVersionPrefix  tile source version prefix
 * @param styleTemplate        style url template
 * @param styleDomainName      the name of style domain in canonical url
 * @param styleVersionPrefix   version prefix
 * @param spritesTemplate      sprites url template
 * @param spritesDomainName    the name of the sprites domain in canonical url
 * @param spritesVersionPrefix the sprite version prefix
 * @param glyphsTemplate       glyphs url template
 * @param glyphsDomainName     the name of the glyphs domain in canonical url
 * @param glyphsVersionPrefix  the glyphs version prefix
 * @param tileTemplate         tile url template
 * @param tileDomainName       the name of the tile domain in canonical url
 * @param tileVersionPrefix    the tile version prefix
 * @param apiKeyParameterName  the name of api key parameter
 * @param apiKeyRequired       indicates if API key is required
 * @param defaultStyle         the name of the default style
 * @param defaultStyles        the list of default styles
 */
class TileServerOptions
    @Keep
    constructor(
        @field:Keep var baseURL: String?,
        @field:Keep var uriSchemeAlias: String?,
        @field:Keep var sourceTemplate: String?,
        @field:Keep var sourceDomainName: String?,
        @field:Keep var sourceVersionPrefix: String?,
        @field:Keep var styleTemplate: String?,
        @field:Keep var styleDomainName: String?,
        @field:Keep var styleVersionPrefix: String?,
        @field:Keep var spritesTemplate: String?,
        @field:Keep var spritesDomainName: String?,
        @field:Keep var spritesVersionPrefix: String?,
        @field:Keep var glyphsTemplate: String?,
        @field:Keep var glyphsDomainName: String?,
        @field:Keep var glyphsVersionPrefix: String?,
        @field:Keep var tileTemplate: String?,
        @field:Keep var tileDomainName: String?,
        @field:Keep var tileVersionPrefix: String?,
        @field:Keep var apiKeyParameterName: String?,
        @field:Keep var apiKeyRequired: Boolean,
        @field:Keep var defaultStyle: String?,
        @field:Keep var defaultStyles: Array<DefaultStyle>,
    ) : Parcelable {
        /**
         * Constructs a new tile server options tuple given a parcel.
         *
         * @param parcel the parcel containing the tile server options values
         */
        private constructor(parcel: Parcel) : this(
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readByte() != 0.toByte(),
            parcel.readString(),
            parcel.createTypedArray(DefaultStyle.CREATOR) ?: emptyArray(),
        )

        /**
         * Describe the kinds of special objects contained in this Parcelable instance's marshaled representation.
         *
         * @return a bitmask indicating the set of special object types marshaled by this Parcelable object instance.
         */
        override fun describeContents(): Int = 0

        /**
         * Flatten this object in to a Parcel.
         *
         * @param out   The Parcel in which the object should be written.
         * @param flags Additional flags about how the object should be written
         */
        override fun writeToParcel(
            out: Parcel,
            flags: Int,
        ) {
            out.writeString(baseURL)
            out.writeString(uriSchemeAlias)
            out.writeString(sourceTemplate)
            out.writeString(sourceDomainName)
            out.writeString(sourceVersionPrefix)
            out.writeString(styleTemplate)
            out.writeString(styleDomainName)
            out.writeString(styleVersionPrefix)
            out.writeString(spritesTemplate)
            out.writeString(spritesDomainName)
            out.writeString(spritesVersionPrefix)
            out.writeString(glyphsTemplate)
            out.writeString(glyphsDomainName)
            out.writeString(glyphsVersionPrefix)
            out.writeString(tileTemplate)
            out.writeString(tileDomainName)
            out.writeString(tileVersionPrefix)
            out.writeString(apiKeyParameterName)
            out.writeByte((if (apiKeyRequired) 1 else 0).toByte())
            out.writeString(defaultStyle)
            out.writeTypedArray(defaultStyles, 0)
        }

        companion object {
            /**
             * Inner class responsible for recreating Parcels into objects.
             */
            @JvmField
            val CREATOR: Parcelable.Creator<TileServerOptions> =
                object : Parcelable.Creator<TileServerOptions> {
                    override fun createFromParcel(parcel: Parcel): TileServerOptions = TileServerOptions(parcel)

                    override fun newArray(size: Int): Array<TileServerOptions?> = arrayOfNulls(size)
                }

            @JvmStatic
            fun get(tileServer: WellKnownTileServer): TileServerOptions =
                when (tileServer) {
                    WellKnownTileServer.Mapbox -> mapboxConfiguration()
                    WellKnownTileServer.MapTiler -> mapTilerConfiguration()
                    WellKnownTileServer.MapLibre -> mapLibreConfiguration()
                }

            @Keep
            @JvmStatic
            @Suppress("unused")
            private external fun defaultConfiguration(): TileServerOptions

            @Keep
            @JvmStatic
            private external fun mapboxConfiguration(): TileServerOptions

            @Keep
            @JvmStatic
            private external fun mapTilerConfiguration(): TileServerOptions

            @Keep
            @JvmStatic
            private external fun mapLibreConfiguration(): TileServerOptions
        }
    }
