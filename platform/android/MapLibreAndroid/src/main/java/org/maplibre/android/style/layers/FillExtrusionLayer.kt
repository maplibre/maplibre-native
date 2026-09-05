// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * An extruded (3D) polygon.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-fill-extrusion)
 */
@UiThread
class FillExtrusionLayer : Layer {

    /**
     * Creates a FillExtrusionLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a FillExtrusionLayer.
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
    fun withSourceLayer(sourceLayer: String?): FillExtrusionLayer {
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
    fun withFilter(filter: Expression): FillExtrusionLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): FillExtrusionLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the FillExtrusionRoundedCornerDistance property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionRoundedCornerDistance: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-rounded-corner-distance", nativeGetFillExtrusionRoundedCornerDistance())
            return value as PropertyValue<Float>
        }

    /**
     * Get the FillExtrusionOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-opacity", nativeGetFillExtrusionOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The FillExtrusionOpacity property transition options
     */
    var fillExtrusionOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillExtrusionOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillExtrusionOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the FillExtrusionColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-color", nativeGetFillExtrusionColor())
            return value as PropertyValue<String>
        }

    /**
     * The base color of the extruded fill. The extrusion's surfaces will be shaded differently based on this color in combination with the root `light` settings. If this color is specified as `rgba` with an alpha component, the alpha component will be ignored; use `fill-extrusion-opacity` to set layer opacity.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getFillExtrusionColorAsInt(): Int {
        checkThread()
        val value = fillExtrusionColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("fill-extrusion-color was set as a Function")
        }
    }

    /**
     * The FillExtrusionColor property transition options
     */
    var fillExtrusionColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillExtrusionColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillExtrusionColorTransition(options.duration, options.delay)
        }

    /**
     * Get the FillExtrusionTranslate property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionTranslate: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-translate", nativeGetFillExtrusionTranslate())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The FillExtrusionTranslate property transition options
     */
    var fillExtrusionTranslateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillExtrusionTranslateTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillExtrusionTranslateTransition(options.duration, options.delay)
        }

    /**
     * Get the FillExtrusionTranslateAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionTranslateAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-translate-anchor", nativeGetFillExtrusionTranslateAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the FillExtrusionPattern property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionPattern: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-pattern", nativeGetFillExtrusionPattern())
            return value as PropertyValue<String>
        }

    /**
     * The FillExtrusionPattern property transition options
     */
    var fillExtrusionPatternTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillExtrusionPatternTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillExtrusionPatternTransition(options.duration, options.delay)
        }

    /**
     * Get the FillExtrusionHeight property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionHeight: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-height", nativeGetFillExtrusionHeight())
            return value as PropertyValue<Float>
        }

    /**
     * The FillExtrusionHeight property transition options
     */
    var fillExtrusionHeightTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillExtrusionHeightTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillExtrusionHeightTransition(options.duration, options.delay)
        }

    /**
     * Get the FillExtrusionBase property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionBase: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-base", nativeGetFillExtrusionBase())
            return value as PropertyValue<Float>
        }

    /**
     * The FillExtrusionBase property transition options
     */
    var fillExtrusionBaseTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetFillExtrusionBaseTransition()
        }
        set(options) {
            checkThread()
            nativeSetFillExtrusionBaseTransition(options.duration, options.delay)
        }

    /**
     * Get the FillExtrusionVerticalGradient property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val fillExtrusionVerticalGradient: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("fill-extrusion-vertical-gradient", nativeGetFillExtrusionVerticalGradient())
            return value as PropertyValue<Boolean>
        }

    @Keep
    private external fun nativeGetFillExtrusionRoundedCornerDistance(): Any

    @Keep
    private external fun nativeGetFillExtrusionOpacity(): Any

    @Keep
    private external fun nativeGetFillExtrusionOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillExtrusionOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillExtrusionColor(): Any

    @Keep
    private external fun nativeGetFillExtrusionColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillExtrusionColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillExtrusionTranslate(): Any

    @Keep
    private external fun nativeGetFillExtrusionTranslateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillExtrusionTranslateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillExtrusionTranslateAnchor(): Any

    @Keep
    private external fun nativeGetFillExtrusionPattern(): Any

    @Keep
    private external fun nativeGetFillExtrusionPatternTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillExtrusionPatternTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillExtrusionHeight(): Any

    @Keep
    private external fun nativeGetFillExtrusionHeightTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillExtrusionHeightTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillExtrusionBase(): Any

    @Keep
    private external fun nativeGetFillExtrusionBaseTransition(): TransitionOptions

    @Keep
    private external fun nativeSetFillExtrusionBaseTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetFillExtrusionVerticalGradient(): Any

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
