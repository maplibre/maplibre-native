// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * A filled circle.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-circle)
 */
@UiThread
class CircleLayer : Layer {

    /**
     * Creates a CircleLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a CircleLayer.
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
     * The source layer.
     */
    var sourceLayer: String?
        get() {
            checkThread()
            return nativeGetSourceLayer()
        }
        set(sourceLayer) {
            checkThread()
            nativeSetSourceLayer(sourceLayer)
        }

    /**
     * Set the source Layer.
     *
     * @param sourceLayer the source layer to set
     * @return This
     */
    fun withSourceLayer(sourceLayer: String?): CircleLayer {
        this.sourceLayer = sourceLayer
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
     * Set a single expression filter.
     *
     * @param filter the expression filter to set
     */
    fun setFilter(filter: Expression) {
        checkThread()
        nativeSetFilter(filter.toArray())
    }

    /**
     * Set a single expression filter.
     *
     * @param filter the expression filter to set
     * @return This
     */
    fun withFilter(filter: Expression): CircleLayer {
        setFilter(filter)
        return this
    }

    /**
     * A single expression filter.
     *
     * Use [setFilter] to set the filter.
     */
    val filter: Expression?
        get() {
            checkThread()
            val jsonElement = nativeGetFilter()
            return if (jsonElement != null) {
                Expression.Converter.convert(jsonElement)
            } else {
                null
            }
        }

    /**
     * Set a property or properties.
     *
     * @param properties the var-args properties
     * @return This
     */
    fun withProperties(vararg properties: PropertyValue<*>): CircleLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the CircleSortKey property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val circleSortKey: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-sort-key", nativeGetCircleSortKey())
            return value as PropertyValue<Float>
        }

    /**
     * Get the CircleRadius property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val circleRadius: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-radius", nativeGetCircleRadius())
            return value as PropertyValue<Float>
        }

    /**
     * The CircleRadius property transition options
     */
    var circleRadiusTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleRadiusTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleRadiusTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val circleColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-color", nativeGetCircleColor())
            return value as PropertyValue<String>
        }

    /**
     * The fill color of the circle.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getCircleColorAsInt(): Int {
        checkThread()
        val value = circleColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("circle-color was set as a Function")
        }
    }

    /**
     * The CircleColor property transition options
     */
    var circleColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleColorTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleBlur property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val circleBlur: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-blur", nativeGetCircleBlur())
            return value as PropertyValue<Float>
        }

    /**
     * The CircleBlur property transition options
     */
    var circleBlurTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleBlurTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleBlurTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val circleOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-opacity", nativeGetCircleOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The CircleOpacity property transition options
     */
    var circleOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleTranslate property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val circleTranslate: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-translate", nativeGetCircleTranslate())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The CircleTranslate property transition options
     */
    var circleTranslateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleTranslateTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleTranslateTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleTranslateAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val circleTranslateAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-translate-anchor", nativeGetCircleTranslateAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the CirclePitchScale property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val circlePitchScale: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-pitch-scale", nativeGetCirclePitchScale())
            return value as PropertyValue<String>
        }

    /**
     * Get the CirclePitchAlignment property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val circlePitchAlignment: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-pitch-alignment", nativeGetCirclePitchAlignment())
            return value as PropertyValue<String>
        }

    /**
     * Get the CircleStrokeWidth property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val circleStrokeWidth: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-stroke-width", nativeGetCircleStrokeWidth())
            return value as PropertyValue<Float>
        }

    /**
     * The CircleStrokeWidth property transition options
     */
    var circleStrokeWidthTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleStrokeWidthTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleStrokeWidthTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleStrokeColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val circleStrokeColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-stroke-color", nativeGetCircleStrokeColor())
            return value as PropertyValue<String>
        }

    /**
     * The stroke color of the circle.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getCircleStrokeColorAsInt(): Int {
        checkThread()
        val value = circleStrokeColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("circle-stroke-color was set as a Function")
        }
    }

    /**
     * The CircleStrokeColor property transition options
     */
    var circleStrokeColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleStrokeColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleStrokeColorTransition(options.duration, options.delay)
        }

    /**
     * Get the CircleStrokeOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val circleStrokeOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("circle-stroke-opacity", nativeGetCircleStrokeOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The CircleStrokeOpacity property transition options
     */
    var circleStrokeOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetCircleStrokeOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetCircleStrokeOpacityTransition(options.duration, options.delay)
        }

    @Keep
    private external fun nativeGetCircleSortKey(): Any

    @Keep
    private external fun nativeGetCircleRadius(): Any

    @Keep
    private external fun nativeGetCircleRadiusTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleRadiusTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleColor(): Any

    @Keep
    private external fun nativeGetCircleColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleBlur(): Any

    @Keep
    private external fun nativeGetCircleBlurTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleBlurTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleOpacity(): Any

    @Keep
    private external fun nativeGetCircleOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleTranslate(): Any

    @Keep
    private external fun nativeGetCircleTranslateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleTranslateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleTranslateAnchor(): Any

    @Keep
    private external fun nativeGetCirclePitchScale(): Any

    @Keep
    private external fun nativeGetCirclePitchAlignment(): Any

    @Keep
    private external fun nativeGetCircleStrokeWidth(): Any

    @Keep
    private external fun nativeGetCircleStrokeWidthTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleStrokeWidthTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleStrokeColor(): Any

    @Keep
    private external fun nativeGetCircleStrokeColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleStrokeColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetCircleStrokeOpacity(): Any

    @Keep
    private external fun nativeGetCircleStrokeOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetCircleStrokeOpacityTransition(duration: Long, delay: Long)

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
