// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * Client-side elevation coloring based on DEM data. The implementation supports Mapbox Terrain RGB, Mapzen Terrarium tiles and custom encodings.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-color-relief)
 */
@UiThread
class ColorReliefLayer : Layer {

    /**
     * Creates a ColorReliefLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a ColorReliefLayer.
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
    fun withSourceLayer(sourceLayer: String?): ColorReliefLayer {
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
    fun withFilter(filter: Expression): ColorReliefLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): ColorReliefLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the ColorReliefOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val colorReliefOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("color-relief-opacity", nativeGetColorReliefOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The ColorReliefOpacity property transition options
     */
    var colorReliefOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetColorReliefOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetColorReliefOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the ColorReliefColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val colorReliefColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("color-relief-color", nativeGetColorReliefColor())
            return value as PropertyValue<String>
        }

    /**
     * Defines the color of each pixel based on its elevation. Should be an expression that uses `["elevation"]` as input.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getColorReliefColorAsInt(): Int {
        checkThread()
        val value = colorReliefColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("color-relief-color was set as a Function")
        }
    }

    @Keep
    private external fun nativeGetColorReliefOpacity(): Any

    @Keep
    private external fun nativeGetColorReliefOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetColorReliefOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetColorReliefColor(): Any

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
