// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * The background color or pattern of the map.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-background)
 */
@UiThread
class BackgroundLayer : Layer {

    /**
     * Creates a BackgroundLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a BackgroundLayer.
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
    fun withProperties(vararg properties: PropertyValue<*>): BackgroundLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the BackgroundColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val backgroundColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("background-color", nativeGetBackgroundColor())
            return value as PropertyValue<String>
        }

    /**
     * The color with which the background will be drawn.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getBackgroundColorAsInt(): Int {
        checkThread()
        val value = backgroundColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("background-color was set as a Function")
        }
    }

    /**
     * The BackgroundColor property transition options
     */
    var backgroundColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetBackgroundColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetBackgroundColorTransition(options.duration, options.delay)
        }

    /**
     * Get the BackgroundPattern property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val backgroundPattern: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("background-pattern", nativeGetBackgroundPattern())
            return value as PropertyValue<String>
        }

    /**
     * The BackgroundPattern property transition options
     */
    var backgroundPatternTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetBackgroundPatternTransition()
        }
        set(options) {
            checkThread()
            nativeSetBackgroundPatternTransition(options.duration, options.delay)
        }

    /**
     * Get the BackgroundOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val backgroundOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("background-opacity", nativeGetBackgroundOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The BackgroundOpacity property transition options
     */
    var backgroundOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetBackgroundOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetBackgroundOpacityTransition(options.duration, options.delay)
        }

    @Keep
    private external fun nativeGetBackgroundColor(): Any

    @Keep
    private external fun nativeGetBackgroundColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetBackgroundColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetBackgroundPattern(): Any

    @Keep
    private external fun nativeGetBackgroundPatternTransition(): TransitionOptions

    @Keep
    private external fun nativeSetBackgroundPatternTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetBackgroundOpacity(): Any

    @Keep
    private external fun nativeGetBackgroundOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetBackgroundOpacityTransition(duration: Long, delay: Long)

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
