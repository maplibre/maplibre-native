package org.maplibre.android.location

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.drawable.Drawable
import android.graphics.drawable.GradientDrawable
import android.graphics.drawable.LayerDrawable
import android.location.Location
import android.os.Build
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.location.LocationComponentConstants.INSTANT_LOCATION_TRANSITION_THRESHOLD
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Projection

object Utils {
    private const val TAG = "Mbgl-com.mapbox.mapboxsdk.location.Utils"

    /**
     * Util for finding the shortest path from the current rotated degree to the new degree.
     *
     * @param heading         the new position of the rotation
     * @param previousHeading the current position of the rotation
     * @return the shortest degree of rotation possible
     */
    @JvmStatic
    fun shortestRotation(
        heading: Float,
        previousHeading: Float,
    ): Float {
        var result = heading
        val diff = previousHeading - result
        if (diff > 180.0f) {
            result += 360.0f
        } else if (diff < -180.0f) {
            result -= 360.0f
        }
        return result
    }

    /**
     * Normalizes an angle to be in the [0, 360] range.
     *
     * @param angle the provided angle
     * @return the normalized angle
     */
    @JvmStatic
    fun normalize(angle: Float): Float = (angle % 360 + 360) % 360

    @JvmStatic
    internal fun generateShadow(
        drawable: Drawable,
        elevation: Float,
    ): Bitmap {
        val width = drawable.intrinsicWidth
        val height = drawable.intrinsicHeight
        var bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        drawable.setBounds(0, 0, canvas.width, canvas.height)
        try {
            drawable.draw(canvas)
        } catch (ex: IllegalArgumentException) {
            if (ex.message == "radius must be > 0" && Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                Logger.w(
                    TAG,
                    "Location's shadow gradient drawable has a radius <= 0px, resetting to 1px in order to avoid crashing",
                )
                ensureShadowGradientRadius(drawable)
                return generateShadow(drawable, elevation)
            } else {
                throw ex
            }
        }
        bitmap =
            Bitmap.createScaledBitmap(
                bitmap,
                toEven(width + elevation),
                toEven(height + elevation),
                false,
            )
        return bitmap
    }

    /**
     * We need to ensure that the radius of any [GradientDrawable] is greater than 0 for API levels < 21.
     *
     * @see [mapbox-gl-native-#15026](https://github.com/mapbox/mapbox-gl-native/issues/15026)
     */
    private fun ensureShadowGradientRadius(drawable: Drawable) {
        if (drawable is GradientDrawable) {
            drawable.setGradientRadius(1f)
        } else if (drawable is LayerDrawable) {
            for (i in 0 until drawable.numberOfLayers) {
                val layers = drawable.getDrawable(i)
                if (layers is GradientDrawable) {
                    layers.setGradientRadius(1f)
                }
            }
        }
    }

    @JvmStatic
    internal fun calculateZoomLevelRadius(
        maplibreMap: MapLibreMap,
        location: Location?,
    ): Float {
        if (location == null) {
            return 0f
        }
        val metersPerPixel = maplibreMap.projection.getMetersPerPixelAtLatitude(location.latitude)
        return (location.accuracy * (1 / metersPerPixel)).toFloat()
    }

    @JvmStatic
    internal fun immediateAnimation(
        projection: Projection,
        current: LatLng,
        target: LatLng,
    ): Boolean {
        val metersPerPixel = projection.getMetersPerPixelAtLatitude((current.latitude + target.latitude) / 2)
        val distance = current.distanceTo(target)
        return distance / metersPerPixel > INSTANT_LOCATION_TRANSITION_THRESHOLD
    }

    /**
     * Casts the value to an even integer.
     */
    private fun toEven(value: Float): Int {
        val i = (value + .5f).toInt()
        if (i % 2 == 1) {
            return i - 1
        }
        return i
    }
}
