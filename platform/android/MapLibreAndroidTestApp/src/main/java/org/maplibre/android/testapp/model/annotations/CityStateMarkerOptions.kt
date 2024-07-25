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
class CityStateMarkerOptions : BaseMarkerOptions<CityStateMarker?, CityStateMarkerOptions?> {
    private var infoWindowBackgroundColor: String? = null
    fun infoWindowBackground(color: String?): CityStateMarkerOptions {
        infoWindowBackgroundColor = color
        return getThis()
    }

    constructor()

    private constructor(`in`: Parcel) {
        position(`in`.readParcelable(LatLng::class.java.classLoader, LatLng::class.java))
        snippet(`in`.readString())
        val iconId = `in`.readString()
        val iconBitmap = `in`.readParcelable(Bitmap::class.java.classLoader, Bitmap::class.java)
        val icon = iconBitmap?.let { IconFactory.recreate(iconId.toString(), it) }
        icon(icon)
        title(`in`.readString())
    }

    override fun getThis(): CityStateMarkerOptions {
        return this
    }

    override fun getMarker(): CityStateMarker? {
        return infoWindowBackgroundColor?.let { CityStateMarker(this, it) }
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
        val CREATOR: Parcelable.Creator<CityStateMarkerOptions?> =
            object : Parcelable.Creator<CityStateMarkerOptions?> {
                override fun createFromParcel(`in`: Parcel): CityStateMarkerOptions {
                    return CityStateMarkerOptions(`in`)
                }

                override fun newArray(size: Int): Array<CityStateMarkerOptions?> {
                    return arrayOfNulls(size)
                }
            }
    }
}
