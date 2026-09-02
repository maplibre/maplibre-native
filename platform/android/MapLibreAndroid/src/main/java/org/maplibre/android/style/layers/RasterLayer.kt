// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * Raster map textures such as satellite imagery.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-raster)
 */
@UiThread
class RasterLayer : Layer {

    /**
     * Creates a RasterLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a RasterLayer.
     *
     * @param layerId  the id of the layer
     * @param sourceId the id of the source
     */
    constructor(layerId: String?, sourceId: String?) : super() {
        initialize(layerId, sourceId)
    }

    @Keep
    protected external fun initialize(layerId: String?, sourceId: String?)

    /**
     * Set the source layer.
     *
     * @param sourceLayer the source layer to set
     */
    fun setSourceLayer(sourceLayer: String?) {
        checkThread()
        nativeSetSourceLayer(sourceLayer)
    }

    /**
     * Set the source Layer.
     *
     * @param sourceLayer the source layer to set
     * @return This
     */
    fun withSourceLayer(sourceLayer: String?): RasterLayer {
        setSourceLayer(sourceLayer)
        return this
    }

    /**
     * The id of the source.
     */
    val sourceId: String
        get() {
            checkThread()
            return nativeGetSourceId()
        }

    /**
     * Set a property or properties.
     *
     * @param properties the var-args properties
     * @return This
     */
    fun withProperties(vararg properties: PropertyValue<*>): RasterLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the RasterOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-opacity", nativeGetRasterOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The RasterOpacity property transition options
     */
    var rasterOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetRasterOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetRasterOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the RasterHueRotate property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterHueRotate: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-hue-rotate", nativeGetRasterHueRotate())
            return value as PropertyValue<Float>
        }

    /**
     * The RasterHueRotate property transition options
     */
    var rasterHueRotateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetRasterHueRotateTransition()
        }
        set(options) {
            checkThread()
            nativeSetRasterHueRotateTransition(options.duration, options.delay)
        }

    /**
     * Get the RasterBrightnessMin property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterBrightnessMin: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-brightness-min", nativeGetRasterBrightnessMin())
            return value as PropertyValue<Float>
        }

    /**
     * The RasterBrightnessMin property transition options
     */
    var rasterBrightnessMinTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetRasterBrightnessMinTransition()
        }
        set(options) {
            checkThread()
            nativeSetRasterBrightnessMinTransition(options.duration, options.delay)
        }

    /**
     * Get the RasterBrightnessMax property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterBrightnessMax: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-brightness-max", nativeGetRasterBrightnessMax())
            return value as PropertyValue<Float>
        }

    /**
     * The RasterBrightnessMax property transition options
     */
    var rasterBrightnessMaxTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetRasterBrightnessMaxTransition()
        }
        set(options) {
            checkThread()
            nativeSetRasterBrightnessMaxTransition(options.duration, options.delay)
        }

    /**
     * Get the RasterSaturation property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterSaturation: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-saturation", nativeGetRasterSaturation())
            return value as PropertyValue<Float>
        }

    /**
     * The RasterSaturation property transition options
     */
    var rasterSaturationTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetRasterSaturationTransition()
        }
        set(options) {
            checkThread()
            nativeSetRasterSaturationTransition(options.duration, options.delay)
        }

    /**
     * Get the RasterContrast property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterContrast: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-contrast", nativeGetRasterContrast())
            return value as PropertyValue<Float>
        }

    /**
     * The RasterContrast property transition options
     */
    var rasterContrastTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetRasterContrastTransition()
        }
        set(options) {
            checkThread()
            nativeSetRasterContrastTransition(options.duration, options.delay)
        }

    /**
     * Get the RasterResampling property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val rasterResampling: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-resampling", nativeGetRasterResampling())
            return value as PropertyValue<String>
        }

    /**
     * Get the RasterFadeDuration property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val rasterFadeDuration: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("raster-fade-duration", nativeGetRasterFadeDuration())
            return value as PropertyValue<Float>
        }

    @Keep
    private external fun nativeGetRasterOpacity(): Any

    @Keep
    private external fun nativeGetRasterOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetRasterOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetRasterHueRotate(): Any

    @Keep
    private external fun nativeGetRasterHueRotateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetRasterHueRotateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetRasterBrightnessMin(): Any

    @Keep
    private external fun nativeGetRasterBrightnessMinTransition(): TransitionOptions

    @Keep
    private external fun nativeSetRasterBrightnessMinTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetRasterBrightnessMax(): Any

    @Keep
    private external fun nativeGetRasterBrightnessMaxTransition(): TransitionOptions

    @Keep
    private external fun nativeSetRasterBrightnessMaxTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetRasterSaturation(): Any

    @Keep
    private external fun nativeGetRasterSaturationTransition(): TransitionOptions

    @Keep
    private external fun nativeSetRasterSaturationTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetRasterContrast(): Any

    @Keep
    private external fun nativeGetRasterContrastTransition(): TransitionOptions

    @Keep
    private external fun nativeSetRasterContrastTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetRasterResampling(): Any

    @Keep
    private external fun nativeGetRasterFadeDuration(): Any

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
