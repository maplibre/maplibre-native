package com.mapbox.mapboxsdk.testapp.model.annotations

import android.graphics.Bitmap
import android.os.Parcel
import android.os.Parcelable
import com.mapbox.mapboxsdk.annotations.BaseMarkerOptions
import com.mapbox.mapboxsdk.annotations.IconFactory
import com.mapbox.mapboxsdk.geometry.LatLng

class CountryMarkerOptions : BaseMarkerOptions<CountryMarker?, CountryMarkerOptions?> {
    private var abbrevName: String? = null
    private var flagRes = 0
    fun abbrevName(name: String?): CountryMarkerOptions {
        abbrevName = name
        return getThis()
    }

    fun flagRes(imageRes: Int): CountryMarkerOptions {
        flagRes = imageRes
        return getThis()
    }

    constructor() {}
    private constructor(`in`: Parcel) {
        position(`in`.readParcelable<Parcelable>(LatLng::class.java.classLoader) as LatLng)
        snippet(`in`.readString())
        val iconId = `in`.readString()
        val iconBitmap = `in`.readParcelable<Bitmap>(Bitmap::class.java.classLoader)
        val icon = iconBitmap?.let { IconFactory.recreate(iconId.toString(), it) }
        icon(icon)
        title(`in`.readString())
    }

    override fun getThis(): CountryMarkerOptions {
        return this
    }

    override fun getMarker(): CountryMarker {
        return CountryMarker(this, abbrevName!!, flagRes)
    }

    override fun describeContents(): Int {
        return 0
    }

    override fun writeToParcel(out: Parcel, flags: Int) {
        out.writeParcelable(position, flags)
        out.writeString(snippet)
        out.writeString(icon.id)
        out.writeParcelable(icon.bitmap, flags)
        out.writeString(title)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<CountryMarkerOptions?> =
            object : Parcelable.Creator<CountryMarkerOptions?> {
                override fun createFromParcel(`in`: Parcel): CountryMarkerOptions {
                    return CountryMarkerOptions(`in`)
                }

                override fun newArray(size: Int): Array<CountryMarkerOptions?> {
                    return arrayOfNulls(size)
                }
            }
    }
}
