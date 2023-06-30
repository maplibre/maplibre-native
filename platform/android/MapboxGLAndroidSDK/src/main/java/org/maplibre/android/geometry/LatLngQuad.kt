package org.maplibre.android.geometry

import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.Keep

/**
 * A geographical area representing a non-aligned quadrilateral
 *
 *
 * This class does not wrap values to the world bounds
 *
 */
class LatLngQuad
/**
 * Construct a new LatLngQuad based on its corners,
 * in order top left, top right, bottom left, bottom right
 */
@Keep constructor(@field:Keep val topLeft: LatLng, @field:Keep val topRight: LatLng, @field:Keep val bottomRight: LatLng, @field:Keep val bottomLeft: LatLng) : Parcelable {

    override fun describeContents(): Int {
        return 0
    }

    override fun writeToParcel(out: Parcel, arg1: Int) {
        topLeft.writeToParcel(out, arg1)
        topRight.writeToParcel(out, arg1)
        bottomRight.writeToParcel(out, arg1)
        bottomLeft.writeToParcel(out, arg1)
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as LatLngQuad

        if (topLeft != other.topLeft) return false
        if (topRight != other.topRight) return false
        if (bottomRight != other.bottomRight) return false
        if (bottomLeft != other.bottomLeft) return false

        return true
    }

    override fun hashCode(): Int {
        var result = topLeft.hashCode()
        result = 31 * result + topRight.hashCode()
        result = 31 * result + bottomRight.hashCode()
        result = 31 * result + bottomLeft.hashCode()
        return result
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<LatLngQuad> = object : Parcelable.Creator<LatLngQuad> {
            override fun createFromParcel(parcel: Parcel): LatLngQuad {
                return readFromParcel(parcel)
            }

            override fun newArray(size: Int): Array<LatLngQuad?> {
                return arrayOfNulls(size)
            }
        }

        private fun readFromParcel(parcel: Parcel): LatLngQuad {
            val topLeft = LatLng(parcel)
            val topRight = LatLng(parcel)
            val bottomRight = LatLng(parcel)
            val bottomLeft = LatLng(parcel)
            return LatLngQuad(topLeft, topRight, bottomRight, bottomLeft)
        }
    }
}
