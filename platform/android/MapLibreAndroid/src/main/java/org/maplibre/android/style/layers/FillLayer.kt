// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * A filled polygon with an optional stroked border.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-fill)
 */
@UiThread
class FillLayer : Layer {

    /**
     * Creates a FillLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a FillLayer.
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
    fun withSourceLayer(sourceLayer: String?): FillLayer {
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
    fun withFilter(filter: Expression): FillLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): FillLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the FillSortKey property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val fillSortKey: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-sort-key", nativeGetFillSortKey())
            return value as PropertyValue<Float>
        }

    /**
     * Get the FillAntialias property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val fillAntialias: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-antialias", nativeGetFillAntialias())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the FillOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val fillOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-opacity", nativeGetFillOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The FillOpacity property transition options
     */
    var fillOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the FillColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-color", nativeGetFillColor())
            return value as PropertyValue<String>
        }

    /**
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getFillColorAsInt(): Int {
        checkThread()
        val value = fillColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("fill-color was set as a Function")
        }
    }

    /**
     * The FillColor property transition options
     */
    var fillColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillColorTransition(options.duration, options.delay)
        }

    /**
     * Get the FillOutlineColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillOutlineColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-outline-color", nativeGetFillOutlineColor())
            return value as PropertyValue<String>
        }

    /**
     * The outline color of the fill. Matches the value of `fill-color` if unspecified.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getFillOutlineColorAsInt(): Int {
        checkThread()
        val value = fillOutlineColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("fill-outline-color was set as a Function")
        }
    }

    /**
     * The FillOutlineColor property transition options
     */
    var fillOutlineColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillOutlineColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillOutlineColorTransition(options.duration, options.delay)
        }

    /**
     * Get the FillTranslate property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val fillTranslate: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-translate", nativeGetFillTranslate())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The FillTranslate property transition options
     */
    var fillTranslateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillTranslateTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillTranslateTransition(options.duration, options.delay)
        }

    /**
     * Get the FillTranslateAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillTranslateAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-translate-anchor", nativeGetFillTranslateAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the FillPattern property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillPattern: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-pattern", nativeGetFillPattern())
            return value as PropertyValue<String>
        }

    /**
     * The FillPattern property transition options
     */
    var fillPatternTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillPatternTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillPatternTransition(options.duration, options.delay)
        }

    @Keep
    private external fun nativeGetFillSortKey(): Any

    @Keep
    private external fun nativeGetFillAntialias(): Any

    @Keep
    private external fun nativeGetFillOpacity(): Any

    @Keep
    private external fun nativeGetFillOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillColor(): Any

    @Keep
    private external fun nativeGetFillColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillOutlineColor(): Any

    @Keep
    private external fun nativeGetFillOutlineColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillOutlineColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillTranslate(): Any

    @Keep
    private external fun nativeGetFillTranslateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillTranslateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillTranslateAnchor(): Any

    @Keep
    private external fun nativeGetFillPattern(): Any

    @Keep
    private external fun nativeGetFillPatternTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillPatternTransition(duration: Long, delay: Long)

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
