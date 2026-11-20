// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.light

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils
import org.maplibre.android.utils.ThreadUtils

/**
 * The global light source.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#light)
 */
@UiThread
class Light
    /**
     * Creates a Light.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) {

    @Keep
    @Suppress("unused")
    private var nativePtr: Long

    init {
        checkThread()
        this.nativePtr = nativePtr
    }

    /**
     * The Anchor property. Whether extruded geometries are lit relative to the map or viewport.
     */
    @get:Property.ANCHOR
    @setparam:Property.ANCHOR
    var anchor: String
        get() {
            checkThread()
            return nativeGetAnchor()
        }
        set(value) {
            checkThread()
            nativeSetAnchor(value)
        }

    /**
     * The Position property. Position of the light source relative to lit (extruded) geometries, in [r radial coordinate, a azimuthal angle, p polar angle] where r indicates the distance from the center of the base of an object to its light, a indicates the position of the light relative to 0&#xB0; (0&#xB0; when `light.anchor` is set to `viewport` corresponds to the top of the viewport, or 0&#xB0; when `light.anchor` is set to `map` corresponds to due north, and degrees proceed clockwise), and p indicates the height of the light (from 0&#xB0;, directly above, to 180&#xB0;, directly below).
     */
    var position: Position
        get() {
            checkThread()
            return nativeGetPosition()
        }
        set(value) {
            checkThread()
            nativeSetPosition(value)
        }

    /**
     * The Position property transition options.
     */
    var positionTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetPositionTransition()
        }
        set(options) {
            checkThread()
            nativeSetPositionTransition(options.duration, options.delay)
        }

    /**
     * The Color property. Color tint for lighting extruded geometries.
     */
    var color: String
        get() {
            checkThread()
            return nativeGetColor()
        }
        set(value) {
            checkThread()
            nativeSetColor(value)
        }

    /**
     * Set the Color property. Color tint for lighting extruded geometries.
     *
     * @param color as int
     */
    fun setColor(@ColorInt color: Int) {
        checkThread()
        nativeSetColor(ColorUtils.colorToRgbaString(color))
    }

    /**
     * The Color property transition options.
     */
    var colorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetColorTransition(options.duration, options.delay)
        }

    /**
     * The Intensity property. Intensity of lighting (on a scale from 0 to 1). Higher numbers will present as more extreme contrast.
     */
    var intensity: Float
        get() {
            checkThread()
            return nativeGetIntensity()
        }
        set(value) {
            checkThread()
            nativeSetIntensity(value)
        }

    /**
     * The Intensity property transition options.
     */
    var intensityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIntensityTransition()
        }
        set(options) {
            checkThread()
            nativeSetIntensityTransition(options.duration, options.delay)
        }

    private fun checkThread() {
        ThreadUtils.checkThread(TAG)
    }

    @Keep
    private external fun nativeSetAnchor(anchor: String)

    @Keep
    private external fun nativeGetAnchor(): String

    @Keep
    private external fun nativeSetPosition(position: Position)

    @Keep
    private external fun nativeGetPosition(): Position

    @Keep
    private external fun nativeGetPositionTransition(): TransitionOptions

    @Keep
    private external fun nativeSetPositionTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeSetColor(color: String)

    @Keep
    private external fun nativeGetColor(): String

    @Keep
    private external fun nativeGetColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeSetIntensity(intensity: Float)

    @Keep
    private external fun nativeGetIntensity(): Float

    @Keep
    private external fun nativeGetIntensityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIntensityTransition(duration: Long, delay: Long)

    private companion object {
        const val TAG = "Mbgl-Light"
    }
}
