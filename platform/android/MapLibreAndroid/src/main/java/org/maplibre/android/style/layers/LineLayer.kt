// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * A stroked line.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-line)
 */
@UiThread
class LineLayer : Layer {

    /**
     * Creates a LineLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a LineLayer.
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
    fun withSourceLayer(sourceLayer: String?): LineLayer {
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
    fun withFilter(filter: Expression): LineLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): LineLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the LineCap property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val lineCap: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-cap", nativeGetLineCap())
            return value as PropertyValue<String>
        }

    /**
     * Get the LineJoin property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val lineJoin: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-join", nativeGetLineJoin())
            return value as PropertyValue<String>
        }

    /**
     * Get the LineMiterLimit property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineMiterLimit: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-miter-limit", nativeGetLineMiterLimit())
            return value as PropertyValue<Float>
        }

    /**
     * Get the LineRoundLimit property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineRoundLimit: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-round-limit", nativeGetLineRoundLimit())
            return value as PropertyValue<Float>
        }

    /**
     * Get the LineSortKey property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineSortKey: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-sort-key", nativeGetLineSortKey())
            return value as PropertyValue<Float>
        }

    /**
     * Get the LineOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-opacity", nativeGetLineOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The LineOpacity property transition options
     */
    var lineOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the LineColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val lineColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-color", nativeGetLineColor())
            return value as PropertyValue<String>
        }

    /**
     * The color with which the line will be drawn.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getLineColorAsInt(): Int {
        checkThread()
        val value = lineColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("line-color was set as a Function")
        }
    }

    /**
     * The LineColor property transition options
     */
    var lineColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineColorTransition(options.duration, options.delay)
        }

    /**
     * Get the LineTranslate property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val lineTranslate: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-translate", nativeGetLineTranslate())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The LineTranslate property transition options
     */
    var lineTranslateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineTranslateTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineTranslateTransition(options.duration, options.delay)
        }

    /**
     * Get the LineTranslateAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val lineTranslateAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-translate-anchor", nativeGetLineTranslateAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the LineWidth property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineWidth: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-width", nativeGetLineWidth())
            return value as PropertyValue<Float>
        }

    /**
     * The LineWidth property transition options
     */
    var lineWidthTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineWidthTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineWidthTransition(options.duration, options.delay)
        }

    /**
     * Get the LineGapWidth property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineGapWidth: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-gap-width", nativeGetLineGapWidth())
            return value as PropertyValue<Float>
        }

    /**
     * The LineGapWidth property transition options
     */
    var lineGapWidthTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineGapWidthTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineGapWidthTransition(options.duration, options.delay)
        }

    /**
     * Get the LineOffset property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineOffset: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-offset", nativeGetLineOffset())
            return value as PropertyValue<Float>
        }

    /**
     * The LineOffset property transition options
     */
    var lineOffsetTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineOffsetTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineOffsetTransition(options.duration, options.delay)
        }

    /**
     * Get the LineBlur property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val lineBlur: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-blur", nativeGetLineBlur())
            return value as PropertyValue<Float>
        }

    /**
     * The LineBlur property transition options
     */
    var lineBlurTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineBlurTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineBlurTransition(options.duration, options.delay)
        }

    /**
     * Get the LineDasharray property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val lineDasharray: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-dasharray", nativeGetLineDasharray())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The LineDasharray property transition options
     */
    var lineDasharrayTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLineDasharrayTransition()
        }
        set(options) {
            checkThread()
            nativeSetLineDasharrayTransition(options.duration, options.delay)
        }

    /**
     * Get the LinePattern property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val linePattern: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-pattern", nativeGetLinePattern())
            return value as PropertyValue<String>
        }

    /**
     * The LinePattern property transition options
     */
    var linePatternTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetLinePatternTransition()
        }
        set(options) {
            checkThread()
            nativeSetLinePatternTransition(options.duration, options.delay)
        }

    /**
     * Get the LineGradient property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val lineGradient: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("line-gradient", nativeGetLineGradient())
            return value as PropertyValue<String>
        }

    /**
     * Defines a gradient with which to color a line feature. Can only be used with GeoJSON sources that specify `"lineMetrics": true`.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getLineGradientAsInt(): Int {
        checkThread()
        val value = lineGradient
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("line-gradient was set as a Function")
        }
    }

    @Keep
    private external fun nativeGetLineCap(): Any

    @Keep
    private external fun nativeGetLineJoin(): Any

    @Keep
    private external fun nativeGetLineMiterLimit(): Any

    @Keep
    private external fun nativeGetLineRoundLimit(): Any

    @Keep
    private external fun nativeGetLineSortKey(): Any

    @Keep
    private external fun nativeGetLineOpacity(): Any

    @Keep
    private external fun nativeGetLineOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineColor(): Any

    @Keep
    private external fun nativeGetLineColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineTranslate(): Any

    @Keep
    private external fun nativeGetLineTranslateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineTranslateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineTranslateAnchor(): Any

    @Keep
    private external fun nativeGetLineWidth(): Any

    @Keep
    private external fun nativeGetLineWidthTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineWidthTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineGapWidth(): Any

    @Keep
    private external fun nativeGetLineGapWidthTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineGapWidthTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineOffset(): Any

    @Keep
    private external fun nativeGetLineOffsetTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineOffsetTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineBlur(): Any

    @Keep
    private external fun nativeGetLineBlurTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineBlurTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineDasharray(): Any

    @Keep
    private external fun nativeGetLineDasharrayTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLineDasharrayTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLinePattern(): Any

    @Keep
    private external fun nativeGetLinePatternTransition(): TransitionOptions

    @Keep
    private external fun nativeSetLinePatternTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetLineGradient(): Any

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
