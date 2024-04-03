package org.maplibre.android.camera

import android.content.res.TypedArray
import android.os.Build
import android.os.Parcel
import android.os.Parcelable
import androidx.annotation.FloatRange
import androidx.annotation.Keep
import androidx.annotation.Size
import org.maplibre.android.R
import org.maplibre.android.camera.CameraPosition.Builder
import org.maplibre.android.camera.CameraUpdateFactory.CameraPositionUpdate
import org.maplibre.android.camera.CameraUpdateFactory.ZoomUpdate
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.utils.MathUtils
import java.util.Arrays

/**
 * Resembles the position, angle, zoom and tilt of the user's viewpoint.
 */
class CameraPosition
/**
 * Constructs a CameraPosition.
 *
 * @param target  The target location to align with the center of the screen.
 * @param zoom    Zoom level at target. See zoom(float) for details of restrictions.
 * @param tilt    The camera angle, in degrees, from the nadir (directly down). See tilt(float)
 * for details of restrictions.
 * @param bearing Direction that the camera is pointing in, in degrees clockwise from north.
 * This value will be normalized to be within 0 degrees inclusive and 360 degrees
 * exclusive.
 * @param padding Padding in pixels. Specified in left, top, right, bottom order.
 * @throws NullPointerException     if target is null
 * @throws IllegalArgumentException if tilt is outside the range of 0 to 90 degrees inclusive.
 */ @Keep internal constructor(

    /**
     * The location that the camera is pointing at.
     */
    @field:Keep
    @JvmField
    val target: LatLng?,

    /**
     * Zoom level near the center of the screen. See [Builder.zoom] for the definition of the camera's
     * zoom level.
     */
    @field:Keep
    @JvmField
    val zoom: Double,

    /**
     * The angle, in degrees, of the camera angle from the nadir (directly facing the Earth).
     * See [Builder.tilt] for details of restrictions on the range of values.
     */
    @field:Keep
    @JvmField
    val tilt: Double,

    /**
     * Direction that the camera is pointing in, in degrees clockwise from north.
     */
    @field:Keep
    @JvmField
    val bearing: Double,

    /**
     * Padding in pixels. Specified in left, top, right, bottom order.
     * See [Builder.padding] for the definition of the camera's padding.
     */
    @field:Keep
    @JvmField
    val padding: DoubleArray?

) : Parcelable {

    /**
     * Constructs a CameraPosition.
     *
     * @param target  The target location to align with the center of the screen.
     * @param zoom    Zoom level at target. See zoom(float) for details of restrictions.
     * @param tilt    The camera angle, in degrees, from the nadir (directly down). See tilt(float)
     * for details of restrictions.
     * @param bearing Direction that the camera is pointing in, in degrees clockwise from north.
     * This value will be normalized to be within 0 degrees inclusive and 360 degrees
     * exclusive.
     * @throws NullPointerException     if target is null
     * @throws IllegalArgumentException if tilt is outside the range of 0 to 90 degrees inclusive.
     */
    @Deprecated("use {@link CameraPosition#CameraPosition(LatLng, double, double, double, double[])} instead.")
    internal constructor(target: LatLng?, zoom: Double, tilt: Double, bearing: Double) : this(target, zoom, tilt, bearing, null) {
    }

    /**
     * Describe the kinds of special objects contained in this Parcelable's
     * marshalled representation.
     *
     * @return integer 0.
     */
    override fun describeContents(): Int {
        return 0
    }

    /**
     * Flatten this object in to a Parcel.
     *
     * @param out   The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written. May be 0 or
     * [.PARCELABLE_WRITE_RETURN_VALUE].
     */
    override fun writeToParcel(out: Parcel, flags: Int) {
        out.writeDouble(bearing)
        out.writeParcelable(target, flags)
        out.writeDouble(tilt)
        out.writeDouble(zoom)
        if (padding != null) {
            val length = padding.size
            out.writeInt(length)
            for (v in padding) {
                out.writeDouble(v)
            }
        } else {
            out.writeInt(-1)
        }
    }

    /**
     * Returns a String with the camera target, zoom, bearing and tilt.
     *
     * @return A String with CameraPosition information.
     */
    override fun toString(): String {
        return ("Target: " + target + ", Zoom:" + zoom + ", Bearing:" + bearing + ", Tilt:" + tilt + ", Padding:" + Arrays.toString(padding))
    }

    /**
     * Compares this [CameraPosition] object with another [CameraPosition] and
     * determines if their target, zoom, tilt, and bearing match.
     *
     * @param other Another [CameraPosition] to compare with this object.
     * @return True if target, zoom, tilt, and bearing match this [CameraPosition] object.
     * Else, false.
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }
        val cameraPosition = other as CameraPosition
        if (target != null && target != cameraPosition.target) {
            return false
        } else if (zoom != cameraPosition.zoom) {
            return false
        } else if (tilt != cameraPosition.tilt) {
            return false
        } else if (bearing != cameraPosition.bearing) {
            return false
        } else if (!Arrays.equals(padding, cameraPosition.padding)) {
            return false
        }
        return true
    }

    /**
     * Gives an integer which can be used as the bucket number for storing elements of the set/map.
     * This bucket number is the address of the element inside the set/map. There's no guarantee
     * that this hash value will be consistent between different Java implementations, or even
     * between different execution runs of the same program.
     *
     * @return integer value you can use for storing element.
     */
    override fun hashCode(): Int {
        var result: Int
        var temp: Long
        temp = java.lang.Double.doubleToLongBits(bearing)
        result = (temp xor (temp ushr 32)).toInt()
        result = 31 * result + (target?.hashCode() ?: 0)
        temp = java.lang.Double.doubleToLongBits(tilt)
        result = 31 * result + (temp xor (temp ushr 32)).toInt()
        temp = java.lang.Double.doubleToLongBits(zoom)
        result = 31 * result + (temp xor (temp ushr 32)).toInt()
        result = 31 * result + Arrays.hashCode(padding)
        return result
    }

    /**
     * Builder for composing CameraPosition objects.
     */
    class Builder {
        private var bearing = -1.0
        private var target: LatLng? = null
        private var tilt = -1.0
        private var zoom = -1.0
        private var padding: DoubleArray? = null

        /**
         * Create an empty builder.
         */
        constructor() : super() {}

        /**
         * Create a builder with an existing CameraPosition data.
         *
         * @param previous Existing CameraPosition values to use
         */
        constructor(previous: CameraPosition?) : super() {
            if (previous != null) {
                bearing = previous.bearing
                target = previous.target
                tilt = previous.tilt
                zoom = previous.zoom
                padding = previous.padding
            }
        }

        /**
         * Create a builder with an existing CameraPosition data.
         *
         * @param typedArray TypedArray containing attribute values
         */
        constructor(typedArray: TypedArray?) : super() {
            if (typedArray != null) {
                bearing = typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraBearing, 0.0f).toDouble()
                val lat = typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraTargetLat, 0.0f).toDouble()
                val lng = typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraTargetLng, 0.0f).toDouble()
                target = LatLng(lat, lng)
                tilt = typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraTilt, 0.0f).toDouble()
                zoom = typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_cameraZoom, 0.0f).toDouble()
            }
        }

        /**
         * Create a builder from an existing CameraPositionUpdate update.
         *
         * @param update Update containing camera options
         */
        constructor(update: CameraPositionUpdate?) : super() {
            if (update != null) {
                bearing = update.bearing
                target = update.target
                tilt = update.tilt
                zoom = update.zoom
                padding = update.padding
            }
        }

        /**
         * Create builder from an existing CameraPositionUpdate update.
         *
         * @param update Update containing camera options
         */
        constructor(update: ZoomUpdate?) : super() {
            if (update != null) {
                zoom = update.zoom
            }
        }

        /**
         * Sets the direction that the camera is pointing in, in degrees clockwise from north.
         *
         * @param bearing Bearing
         * @return this
         */
        fun bearing(bearing: Double): Builder {
            var direction = bearing
            while (direction >= 360) {
                direction -= 360.0
            }
            while (direction < 0) {
                direction += 360.0
            }
            this.bearing = direction
            return this
        }

        /**
         * Sets the location where the camera is pointing at.
         *
         * @param location target of the camera
         * @return this
         */
        fun target(location: LatLng?): Builder {
            target = location
            return this
        }

        /**
         * Set the tilt of the camera in degrees
         *
         *
         * value is clamped to [MapLibreConstants.MINIMUM_TILT] and [MapLibreConstants.MAXIMUM_TILT].
         *
         *
         * @param tilt Tilt value of the camera
         * @return this
         */
        fun tilt(@FloatRange(from = MapLibreConstants.MINIMUM_TILT, to = MapLibreConstants.MAXIMUM_TILT) tilt: Double): Builder {
            this.tilt = MathUtils.clamp(tilt, MapLibreConstants.MINIMUM_TILT, MapLibreConstants.MAXIMUM_TILT)
            return this
        }

        /**
         * Set the zoom of the camera
         *
         *
         * Zoom ranges from [MapLibreConstants.MINIMUM_ZOOM] to [MapLibreConstants.MAXIMUM_ZOOM]
         *
         *
         * @param zoom Zoom value of the camera
         * @return this
         */
        fun zoom(@FloatRange(from = MapLibreConstants.MINIMUM_ZOOM.toDouble(), to = MapLibreConstants.MAXIMUM_ZOOM.toDouble()) zoom: Double): Builder {
            this.zoom = zoom
            return this
        }

        /**
         * Padding in pixels that shifts the viewport by the specified amount.
         * Applied padding is going to persist and impact following camera transformations.
         *
         *
         * Specified in left, top, right, bottom order.
         *
         *
         * @param padding Camera padding
         * @return this
         */
        fun padding(@Size(4) padding: DoubleArray?): Builder {
            this.padding = padding
            return this
        }

        /**
         * Padding in pixels that shifts the viewport by the specified amount.
         * Applied padding is going to persist and impact following camera transformations.
         *
         *
         * Specified in left, top, right, bottom order.
         *
         *
         * @return this
         */
        fun padding(left: Double, top: Double, right: Double, bottom: Double): Builder {
            padding = doubleArrayOf(left, top, right, bottom)
            return this
        }

        /**
         * Builds the CameraPosition.
         *
         * @return CameraPosition
         */
        fun build(): CameraPosition {
            return CameraPosition(target, zoom, tilt, bearing, padding)
        }
    }

    companion object {
        @JvmField
        val DEFAULT = CameraPosition(LatLng(), 0.0, 0.0, 0.0, doubleArrayOf(0.0, 0.0, 0.0, 0.0))

        @JvmField
        val CREATOR: Parcelable.Creator<CameraPosition> = object : Parcelable.Creator<CameraPosition> {
            override fun createFromParcel(parcel: Parcel): CameraPosition {
                val bearing = parcel.readDouble()

                @Suppress("DEPRECATION")
                val target = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    parcel.readParcelable(LatLng::class.java.classLoader, LatLng::class.java)
                } else {
                    parcel.readParcelable(LatLng::class.java.classLoader)
                }
                val tilt = parcel.readDouble()
                val zoom = parcel.readDouble()
                var padding: DoubleArray? = null
                val paddingSize = parcel.readInt()
                if (paddingSize > 0) {
                    padding = DoubleArray(paddingSize)
                    for (i in 0 until paddingSize) {
                        padding[i] = parcel.readDouble()
                    }
                }
                return CameraPosition(target, zoom, tilt, bearing, padding)
            }

            override fun newArray(size: Int): Array<CameraPosition?> {
                return arrayOfNulls(size)
            }
        }
    }
}
