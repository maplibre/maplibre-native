// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * A heatmap.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-heatmap)
 */
@UiThread
class HeatmapLayer : Layer {

    /**
     * Creates a HeatmapLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a HeatmapLayer.
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
    fun withSourceLayer(sourceLayer: String?): HeatmapLayer {
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
    fun withFilter(filter: Expression): HeatmapLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): HeatmapLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the HeatmapRadius property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val heatmapRadius: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("heatmap-radius", nativeGetHeatmapRadius())
            return value as PropertyValue<Float>
        }

    /**
     * The HeatmapRadius property transition options
     */
    var heatmapRadiusTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHeatmapRadiusTransition()
        }
        set(options) {
            checkThread()
            nativeSetHeatmapRadiusTransition(options.duration, options.delay)
        }

    /**
     * Get the HeatmapWeight property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val heatmapWeight: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("heatmap-weight", nativeGetHeatmapWeight())
            return value as PropertyValue<Float>
        }

    /**
     * Get the HeatmapIntensity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val heatmapIntensity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("heatmap-intensity", nativeGetHeatmapIntensity())
            return value as PropertyValue<Float>
        }

    /**
     * The HeatmapIntensity property transition options
     */
    var heatmapIntensityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHeatmapIntensityTransition()
        }
        set(options) {
            checkThread()
            nativeSetHeatmapIntensityTransition(options.duration, options.delay)
        }

    /**
     * Get the HeatmapColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val heatmapColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("heatmap-color", nativeGetHeatmapColor())
            return value as PropertyValue<String>
        }

    /**
     * Defines the color of each pixel based on its density value in a heatmap.  Should be an expression that uses `["heatmap-density"]` as input.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getHeatmapColorAsInt(): Int {
        checkThread()
        val value = heatmapColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("heatmap-color was set as a Function")
        }
    }

    /**
     * Get the HeatmapOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val heatmapOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("heatmap-opacity", nativeGetHeatmapOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The HeatmapOpacity property transition options
     */
    var heatmapOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHeatmapOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetHeatmapOpacityTransition(options.duration, options.delay)
        }

    @Keep
    private external fun nativeGetHeatmapRadius(): Any

    @Keep
    private external fun nativeGetHeatmapRadiusTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHeatmapRadiusTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetHeatmapWeight(): Any

    @Keep
    private external fun nativeGetHeatmapIntensity(): Any

    @Keep
    private external fun nativeGetHeatmapIntensityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHeatmapIntensityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetHeatmapColor(): Any

    @Keep
    private external fun nativeGetHeatmapOpacity(): Any

    @Keep
    private external fun nativeGetHeatmapOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHeatmapOpacityTransition(duration: Long, delay: Long)

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
