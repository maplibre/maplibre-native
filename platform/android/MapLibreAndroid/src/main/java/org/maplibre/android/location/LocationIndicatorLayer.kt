// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.location

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * 
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-location-indicator)
 */
@UiThread
internal class LocationIndicatorLayer : Layer {

    /**
     * Creates a LocationIndicatorLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a LocationIndicatorLayer.
     *
     * @param layerId the id of the layer
     */
    constructor(layerId: String?) : super() {
        initialize(layerId)
    }

    @Keep
    protected external fun initialize(layerId: String?)

    /**
     * Set a property or properties.
     *
     * @param properties the var-args properties
     * @return This
     */
    fun withProperties(vararg properties: PropertyValue<*>): LocationIndicatorLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the TopImage property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val topImage: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("top-image", nativeGetTopImage())
            return value as PropertyValue<String>
        }

    /**
     * Get the BearingImage property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val bearingImage: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("bearing-image", nativeGetBearingImage())
            return value as PropertyValue<String>
        }

    /**
     * Get the ShadowImage property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val shadowImage: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("shadow-image", nativeGetShadowImage())
            return value as PropertyValue<String>
        }

    /**
     * Get the PerspectiveCompensation property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val perspectiveCompensation: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("perspective-compensation", nativeGetPerspectiveCompensation())
            return value as PropertyValue<Float>
        }

    /**
     * Get the ImageTiltDisplacement property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val imageTiltDisplacement: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("image-tilt-displacement", nativeGetImageTiltDisplacement())
            return value as PropertyValue<Float>
        }

    /**
     * Get the Bearing property
     *
     * @return property wrapper value around Double
     */
    @Suppress("UNCHECKED_CAST")
    val bearing: PropertyValue<Double>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("bearing", nativeGetBearing())
            return value as PropertyValue<Double>
        }

    /**
     * The Bearing property transition options
     */
    var bearingTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetBearingTransition()
        }
        set(options) {
            checkThread()
            nativeSetBearingTransition(options.duration, options.delay)
        }

    /**
     * Get the Location property
     *
     * @return property wrapper value around Array<Double>
     */
    @Suppress("UNCHECKED_CAST")
    val location: PropertyValue<Array<Double>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("location", nativeGetLocation())
            return value as PropertyValue<Array<Double>>
        }

    /**
     * The Location property transition options
     */
    var locationTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLocationTransition()
        }
        set(options) {
            checkThread()
            nativeSetLocationTransition(options.duration, options.delay)
        }

    /**
     * Get the AccuracyRadius property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val accuracyRadius: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("accuracy-radius", nativeGetAccuracyRadius())
            return value as PropertyValue<Float>
        }

    /**
     * The AccuracyRadius property transition options
     */
    var accuracyRadiusTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetAccuracyRadiusTransition()
        }
        set(options) {
            checkThread()
            nativeSetAccuracyRadiusTransition(options.duration, options.delay)
        }

    /**
     * Get the TopImageSize property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val topImageSize: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("top-image-size", nativeGetTopImageSize())
            return value as PropertyValue<Float>
        }

    /**
     * The TopImageSize property transition options
     */
    var topImageSizeTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTopImageSizeTransition()
        }
        set(options) {
            checkThread()
            nativeSetTopImageSizeTransition(options.duration, options.delay)
        }

    /**
     * Get the BearingImageSize property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val bearingImageSize: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("bearing-image-size", nativeGetBearingImageSize())
            return value as PropertyValue<Float>
        }

    /**
     * The BearingImageSize property transition options
     */
    var bearingImageSizeTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetBearingImageSizeTransition()
        }
        set(options) {
            checkThread()
            nativeSetBearingImageSizeTransition(options.duration, options.delay)
        }

    /**
     * Get the ShadowImageSize property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val shadowImageSize: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("shadow-image-size", nativeGetShadowImageSize())
            return value as PropertyValue<Float>
        }

    /**
     * The ShadowImageSize property transition options
     */
    var shadowImageSizeTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetShadowImageSizeTransition()
        }
        set(options) {
            checkThread()
            nativeSetShadowImageSizeTransition(options.duration, options.delay)
        }

    /**
     * Get the AccuracyRadiusColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val accuracyRadiusColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("accuracy-radius-color", nativeGetAccuracyRadiusColor())
            return value as PropertyValue<String>
        }

    /**
     * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getAccuracyRadiusColorAsInt(): Int {
        checkThread()
        val value = accuracyRadiusColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("accuracy-radius-color was set as a Function")
        }
    }

    /**
     * The AccuracyRadiusColor property transition options
     */
    var accuracyRadiusColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetAccuracyRadiusColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetAccuracyRadiusColorTransition(options.duration, options.delay)
        }

    /**
     * Get the AccuracyRadiusBorderColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val accuracyRadiusBorderColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("accuracy-radius-border-color", nativeGetAccuracyRadiusBorderColor())
            return value as PropertyValue<String>
        }

    /**
     * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getAccuracyRadiusBorderColorAsInt(): Int {
        checkThread()
        val value = accuracyRadiusBorderColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("accuracy-radius-border-color was set as a Function")
        }
    }

    /**
     * The AccuracyRadiusBorderColor property transition options
     */
    var accuracyRadiusBorderColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetAccuracyRadiusBorderColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetAccuracyRadiusBorderColorTransition(options.duration, options.delay)
        }

    @Keep
    private external fun nativeGetTopImage(): Any

    @Keep
    private external fun nativeGetBearingImage(): Any

    @Keep
    private external fun nativeGetShadowImage(): Any

    @Keep
    private external fun nativeGetPerspectiveCompensation(): Any

    @Keep
    private external fun nativeGetImageTiltDisplacement(): Any

    @Keep
    private external fun nativeGetBearing(): Any

    @Keep
    private external fun nativeGetBearingTransition(): TransitionOptions

    @Keep
    private external fun nativeSetBearingTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLocation(): Any

    @Keep
    private external fun nativeGetLocationTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLocationTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetAccuracyRadius(): Any

    @Keep
    private external fun nativeGetAccuracyRadiusTransition(): TransitionOptions

    @Keep
    private external fun nativeSetAccuracyRadiusTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTopImageSize(): Any

    @Keep
    private external fun nativeGetTopImageSizeTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTopImageSizeTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetBearingImageSize(): Any

    @Keep
    private external fun nativeGetBearingImageSizeTransition(): TransitionOptions

    @Keep
    private external fun nativeSetBearingImageSizeTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetShadowImageSize(): Any

    @Keep
    private external fun nativeGetShadowImageSizeTransition(): TransitionOptions

    @Keep
    private external fun nativeSetShadowImageSizeTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetAccuracyRadiusColor(): Any

    @Keep
    private external fun nativeGetAccuracyRadiusColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetAccuracyRadiusColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetAccuracyRadiusBorderColor(): Any

    @Keep
    private external fun nativeGetAccuracyRadiusBorderColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetAccuracyRadiusBorderColorTransition(duration: Long, delay: Long)

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
