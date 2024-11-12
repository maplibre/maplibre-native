package org.maplibre.android.snapshotter

import android.graphics.Bitmap
import android.graphics.PointF
import androidx.annotation.Keep
import org.maplibre.android.geometry.LatLng

/**
 * A completed snapshot.
 *
 * @see MapSnapshotter
 */
@Keep
class MapSnapshot private constructor(val nativePtr: Long, bitmap: Bitmap, attributions: Array<String>, showLogo: Boolean) {

    /**
     * @return the large
     */
    val bitmap: Bitmap

    /**
     * @return The attributions for the sources of this snapshot.
     */
    val attributions: Array<String>

    /**
     * @return Flag indicating to show the MapLibre logo.
     */
    val isShowLogo: Boolean

    /**
     * Created from native side
     */
    init {
        this.bitmap = bitmap
        this.attributions = attributions
        isShowLogo = showLogo
    }

    /**
     * Calculate the point in pixels on the Image from geographical coordinates.
     *
     * @param latLng the geographical coordinates
     * @return the point on the image
     */
    @Keep
    external fun pixelForLatLng(latLng: LatLng?): PointF

    /**
     * Calculate geographical coordinates from a point in pixels on the Image
     *
     * @param pointF the point in pixels
     * @return the geographical coordinates
     */
    @Keep
    external fun latLngForPixel(pointF: PointF?): LatLng

    // Unused, needed for peer binding
    @Keep
    private external fun initialize()

    @Keep
    protected external fun finalize()
}
