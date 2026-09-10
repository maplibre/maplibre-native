package org.maplibre.android.util

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.Keep

/**
 * Default style definition
 *
 * @param url     canonical style url
 * @param name    style name
 * @param version style version
 */
class DefaultStyle
    @Keep
    constructor(
        @field:Keep var url: String?,
        @field:Keep var name: String?,
        @field:Keep var version: Int,
    ) : Parcelable {
        /**
         * Constructs a new default style tuple given a parcel.
         *
         * @param parcel the parcel containing the default style values
         */
        private constructor(parcel: Parcel) : this(
            parcel.readString(),
            parcel.readString(),
            parcel.readInt(),
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
            out.writeString(url)
            out.writeString(name)
            out.writeInt(version)
        }

        companion object {
            /**
             * Inner class responsible for recreating Parcels into objects.
             */
            @JvmField
            val CREATOR: Parcelable.Creator<DefaultStyle> =
                object : Parcelable.Creator<DefaultStyle> {
                    override fun createFromParcel(parcel: Parcel): DefaultStyle = DefaultStyle(parcel)

                    override fun newArray(size: Int): Array<DefaultStyle?> = arrayOfNulls(size)
                }
        }
    }
