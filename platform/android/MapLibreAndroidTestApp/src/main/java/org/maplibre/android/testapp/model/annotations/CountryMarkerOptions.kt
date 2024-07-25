package org.maplibre.android.testapp.model.annotations

import android.graphics.Bitmap
import android.os.Build
import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.RequiresApi
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.IconFactory
import org.maplibre.android.geometry.LatLng

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
class CountryMarkerOptions private constructor(`in`: Parcel) :
    BaseMarkerOptions<CountryMarker?, CountryMarkerOptions?>() {
    private var abbrevName: String? = null
    private var flagRes = 0

    init {
        position(`in`.readParcelable(LatLng::class.java.classLoader, LatLng::class.java))
        snippet(`in`.readString())
        val iconId = `in`.readString()
        val iconBitmap = `in`.readParcelable(Bitmap::class.java.classLoader, Bitmap::class.java)
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
