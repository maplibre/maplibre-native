package org.maplibre.android.camera

import android.graphics.Point
import android.graphics.PointF
import androidx.annotation.IntDef
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapLibreMap
import timber.log.Timber
import java.lang.Double.max
import java.util.Arrays

/**
 * Factory for creating CameraUpdate objects.
 */
object CameraUpdateFactory {
    /**
     * Returns a CameraUpdate that moves the camera to a specified CameraPosition.
     *
     * @param cameraPosition Camera Position to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newCameraPosition(cameraPosition: CameraPosition): CameraUpdate {
        return CameraPositionUpdate(cameraPosition.bearing, cameraPosition.target, cameraPosition.tilt, cameraPosition.zoom, cameraPosition.padding)
    }

    /**
     * Returns a CameraUpdate that moves the center of the screen to a latitude and longitude
     * specified by a LatLng object. This centers the camera on the LatLng object.
     *
     * @param latLng Target location to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLng(latLng: LatLng): CameraUpdate {
        return CameraPositionUpdate(-1.0, latLng, -1.0, -1.0, null)
    }

    /**
     * Returns a CameraUpdate that transforms the camera such that the specified
     * latitude/longitude bounds are centered on screen at the greatest possible zoom level while maintaining
     * current camera position bearing and tilt values.
     *
     *
     * You can specify padding, in order to inset the bounding box from the map view's edges.
     * The padding will not persist and impact following camera transformations.
     *
     *
     * @param bounds  Bounds to match Camera position with
     * @param padding Padding added to the bounds
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLngBounds(bounds: LatLngBounds, padding: Int): CameraUpdate {
        return newLatLngBounds(bounds, padding, padding, padding, padding)
    }

    /**
     * Returns a CameraUpdate that transforms the camera such that the specified
     * latitude/longitude bounds are centered on screen at the greatest possible zoom level while using
     * provided bearing and tilt values.
     *
     *
     * You can specify padding, in order to inset the bounding box from the map view's edges.
     * The padding will not persist and impact following camera transformations.
     *
     *
     * @param bounds  Bounds to match Camera position with
     * @param bearing Bearing to take in account when generating the bounds
     * @param tilt    Tilt to take in account when generating the bounds
     * @param padding Padding added to the bounds
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLngBounds(bounds: LatLngBounds, bearing: Double, tilt: Double, padding: Int): CameraUpdate {
        return newLatLngBounds(bounds, bearing, tilt, padding, padding, padding, padding)
    }

    /**
     * Returns a CameraUpdate that transforms the camera such that the specified
     * latitude/longitude bounds are centered on screen at the greatest possible zoom level while maintaining
     * current camera position bearing and tilt values.
     *
     *
     * You can specify padding, in order to inset the bounding box from the map view's edges.
     * The padding will not persist and impact following camera transformations.
     *
     *
     * @param bounds        Bounds to base the Camera position out of
     * @param paddingLeft   Padding left of the bounds
     * @param paddingTop    Padding top of the bounds
     * @param paddingRight  Padding right of the bounds
     * @param paddingBottom Padding bottom of the bounds
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLngBounds(bounds: LatLngBounds, paddingLeft: Int, paddingTop: Int, paddingRight: Int, paddingBottom: Int): CameraUpdate {
        return CameraBoundsUpdate(bounds, null, null, paddingLeft, paddingTop, paddingRight, paddingBottom)
    }

    /**
     * Returns a CameraUpdate that transforms the camera such that the specified
     * latitude/longitude bounds are centered on screen at the greatest possible zoom level while using
     * provided bearing and tilt values.
     *
     *
     * You can specify padding, in order to inset the bounding box from the map view's edges.
     * The padding will not persist and impact following camera transformations.
     *
     *
     * @param bounds        Bounds to base the Camera position out of
     * @param bearing       Bearing to take in account when generating the bounds
     * @param tilt          Tilt to take in account when generating the bounds
     * @param paddingLeft   Padding left of the bounds
     * @param paddingTop    Padding top of the bounds
     * @param paddingRight  Padding right of the bounds
     * @param paddingBottom Padding bottom of the bounds
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLngBounds(bounds: LatLngBounds, bearing: Double, tilt: Double, paddingLeft: Int, paddingTop: Int, paddingRight: Int, paddingBottom: Int): CameraUpdate {
        return CameraBoundsUpdate(bounds, bearing, tilt, paddingLeft, paddingTop, paddingRight, paddingBottom)
    }

    /**
     * Returns a CameraUpdate that moves the center of the screen to a latitude and longitude
     * specified by a LatLng object taking the specified padding into account.
     *
     * @param latLng Target location to change to
     * @param zoom   Zoom level to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLngZoom(latLng: LatLng, zoom: Double): CameraUpdate {
        return CameraPositionUpdate(-1.0, latLng, -1.0, zoom, null)
    }

    /**
     * Returns a CameraUpdate that moves the center of the screen to a latitude and longitude
     * specified by a LatLng object, and moves to the given zoom level.
     * Applied padding is going to persist and impact following camera transformations.
     *
     * @param latLng Target location to change to
     * @param left   Left padding
     * @param top    Top padding
     * @param right  Right padding
     * @param bottom Bottom padding
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun newLatLngPadding(latLng: LatLng, left: Double, top: Double, right: Double, bottom: Double): CameraUpdate {
        return CameraPositionUpdate(-1.0, latLng, -1.0, -1.0, doubleArrayOf(left, top, right, bottom))
    }

    /**
     * Returns a CameraUpdate that shifts the zoom level of the current camera viewpoint.
     *
     * @param amount Amount of zoom level to change with
     * @param focus  Focus point of zoom
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun zoomBy(amount: Double, focus: Point): CameraUpdate {
        return ZoomUpdate(amount, focus.x.toFloat(), focus.y.toFloat())
    }

    /**
     * Returns a CameraUpdate that shifts the zoom level of the current camera viewpoint.
     *
     * @param amount Amount of zoom level to change with
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun zoomBy(amount: Double): CameraUpdate {
        return ZoomUpdate(ZoomUpdate.ZOOM_BY, amount)
    }

    /**
     * Returns a CameraUpdate that zooms in on the map by moving the viewpoint's height closer to
     * the Earth's surface. The zoom increment is 1.0.
     *
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun zoomIn(): CameraUpdate {
        return ZoomUpdate(ZoomUpdate.ZOOM_IN)
    }

    /**
     * Returns a CameraUpdate that zooms out on the map by moving the viewpoint's height farther
     * away from the Earth's surface. The zoom increment is -1.0.
     *
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun zoomOut(): CameraUpdate {
        return ZoomUpdate(ZoomUpdate.ZOOM_OUT)
    }

    /**
     * Returns a CameraUpdate that moves the camera viewpoint to a particular zoom level.
     *
     * @param zoom Zoom level to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun zoomTo(zoom: Double): CameraUpdate {
        return ZoomUpdate(ZoomUpdate.ZOOM_TO, zoom)
    }

    /**
     * Returns a CameraUpdate that moves the camera viewpoint to a particular bearing.
     *
     * @param bearing Bearing to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun bearingTo(bearing: Double): CameraUpdate {
        return CameraPositionUpdate(bearing, null, -1.0, -1.0, null)
    }

    /**
     * Returns a CameraUpdate that moves the camera viewpoint to a particular tilt.
     *
     * @param tilt Tilt to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun tiltTo(tilt: Double): CameraUpdate {
        return CameraPositionUpdate(-1.0, null, tilt, -1.0, null)
    }

    /**
     * Returns a CameraUpdate that when animated changes the camera padding.
     * Applied padding is going to persist and impact following camera transformations.
     *
     *
     * Specified in left, top, right, bottom order.
     *
     *
     * @param padding Padding to change to
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun paddingTo(padding: DoubleArray?): CameraUpdate {
        return CameraPositionUpdate(-1.0, null, -1.0, -1.0, padding)
    }

    /**
     * Returns a CameraUpdate that when animated changes the camera padding.
     * Applied padding is going to persist and impact following camera transformations.
     *
     *
     * Specified in left, top, right, bottom order.
     *
     *
     * @return CameraUpdate Final Camera Position
     */
    @JvmStatic
    fun paddingTo(left: Double, top: Double, right: Double, bottom: Double): CameraUpdate {
        return paddingTo(doubleArrayOf(left, top, right, bottom))
    }

    //
    // CameraUpdate types
    //
    class CameraPositionUpdate(val bearing: Double, val target: LatLng?, val tilt: Double, val zoom: Double, val padding: DoubleArray?) : CameraUpdate {

        override fun getCameraPosition(maplibreMap: MapLibreMap): CameraPosition {
            if (target == null) {
                val previousPosition = maplibreMap.cameraPosition
                return CameraPosition.Builder(this).target(previousPosition.target).build()
            }
            return CameraPosition.Builder(this).build()
        }

        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }
            val that = other as CameraPositionUpdate
            if (java.lang.Double.compare(that.bearing, bearing) != 0) {
                return false
            }
            if (java.lang.Double.compare(that.tilt, tilt) != 0) {
                return false
            }
            if (java.lang.Double.compare(that.zoom, zoom) != 0) {
                return false
            }
            return if (if (target != null) target != that.target else that.target != null) {
                false
            } else {
                Arrays.equals(padding, that.padding)
            }
        }

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

        override fun toString(): String {
            return ("CameraPositionUpdate{" + "bearing=" + bearing + ", target=" + target + ", tilt=" + tilt + ", zoom=" + zoom + ", padding=" + Arrays.toString(padding) + '}')
        }
    }

    internal class CameraBoundsUpdate(val bounds: LatLngBounds, private val bearing: Double?, private val tilt: Double?, val padding: IntArray) : CameraUpdate {
        constructor(bounds: LatLngBounds, bearing: Double?, tilt: Double?, paddingLeft: Int, paddingTop: Int, paddingRight: Int, paddingBottom: Int) : this(
            bounds,
            bearing,
            tilt,
            intArrayOf(paddingLeft, paddingTop, paddingRight, paddingBottom)
        ) {
        }

        override fun getCameraPosition(maplibreMap: MapLibreMap): CameraPosition? {
            return if (bearing == null && tilt == null) {
                // use current camera position tilt and bearing
                maplibreMap.getCameraForLatLngBounds(bounds, padding)
            } else {
                // use provided tilt and bearing
                assert(bearing != null)
                assert(tilt != null)
                maplibreMap.getCameraForLatLngBounds(bounds, padding, bearing!!, tilt!!)
            }
        }

        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }
            val that = other as CameraBoundsUpdate
            return if (bounds != that.bounds) {
                false
            } else {
                padding.contentEquals(that.padding)
            }
        }

        override fun hashCode(): Int {
            var result = bounds.hashCode()
            result = 31 * result + Arrays.hashCode(padding)
            return result
        }

        override fun toString(): String {
            return ("CameraBoundsUpdate{" + "bounds=" + bounds + ", padding=" + padding.contentToString() + '}')
        }
    }

    class ZoomUpdate : CameraUpdate {
        @IntDef(ZOOM_IN, ZOOM_OUT, ZOOM_BY, ZOOM_TO, ZOOM_TO_POINT)
        @Retention(AnnotationRetention.SOURCE)
        internal annotation class Type

        @get:Type
        @Type
        val type: Int
        val zoom: Double
        var x = 0f
            private set
        var y = 0f
            private set

        constructor(@Type type: Int) {
            this.type = type
            zoom = 0.0
        }

        constructor(@Type type: Int, zoom: Double) {
            this.type = type
            this.zoom = zoom
        }

        constructor(zoom: Double, x: Float, y: Float) {
            type = ZOOM_TO_POINT
            this.zoom = zoom
            this.x = x
            this.y = y
        }

        private fun transformZoom(currentZoomArg: Double): Double {
            return when (type) {
                ZOOM_IN -> currentZoomArg + 1
                ZOOM_OUT -> {
                    max(currentZoomArg - 1, 0.0)
                }

                ZOOM_TO -> zoom
                ZOOM_BY, ZOOM_TO_POINT -> currentZoomArg + zoom
                else -> {
                    Timber.e("Unprocessed when branch")
                    4.0
                }
            }
        }

        override fun getCameraPosition(maplibreMap: MapLibreMap): CameraPosition {
            val cameraPosition = maplibreMap.cameraPosition
            return if (type != ZOOM_TO_POINT) {
                CameraPosition.Builder(cameraPosition).zoom(transformZoom(cameraPosition.zoom)).build()
            } else {
                CameraPosition.Builder(cameraPosition).zoom(transformZoom(cameraPosition.zoom)).target(maplibreMap.projection.fromScreenLocation(PointF(x, y))).build()
            }
        }

        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }
            val that = other as ZoomUpdate
            if (type != that.type) {
                return false
            }
            if (java.lang.Double.compare(that.zoom, zoom) != 0) {
                return false
            }
            return if (java.lang.Float.compare(that.x, x) != 0) {
                false
            } else {
                that.y.compareTo(y) == 0
            }
        }

        override fun hashCode(): Int {
            var result: Int
            val temp: Long
            result = type
            temp = java.lang.Double.doubleToLongBits(zoom)
            result = 31 * result + (temp xor (temp ushr 32)).toInt()
            result = 31 * result + if (x != +0.0f) java.lang.Float.floatToIntBits(x) else 0
            result = 31 * result + if (y != +0.0f) java.lang.Float.floatToIntBits(y) else 0
            return result
        }

        override fun toString(): String {
            return ("ZoomUpdate{" + "type=" + type + ", zoom=" + zoom + ", x=" + x + ", y=" + y + '}')
        }

        companion object {
            const val ZOOM_IN = 0
            const val ZOOM_OUT = 1
            const val ZOOM_BY = 2
            const val ZOOM_TO = 3
            const val ZOOM_TO_POINT = 4
        }
    }
}
