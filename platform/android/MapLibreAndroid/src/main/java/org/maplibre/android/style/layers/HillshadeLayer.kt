// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * Client-side hillshading visualization based on DEM data. The implementation supports Mapbox Terrain RGB, Mapzen Terrarium tiles and custom encodings.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-hillshade)
 */
@UiThread
class HillshadeLayer : Layer {

    /**
     * Creates a HillshadeLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a HillshadeLayer.
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
    fun withSourceLayer(sourceLayer: String?): HillshadeLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): HillshadeLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the HillshadeIlluminationDirection property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeIlluminationDirection: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-illumination-direction", nativeGetHillshadeIlluminationDirection())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * Get the HillshadeIlluminationAltitude property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeIlluminationAltitude: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-illumination-altitude", nativeGetHillshadeIlluminationAltitude())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * Get the HillshadeIlluminationAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeIlluminationAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-illumination-anchor", nativeGetHillshadeIlluminationAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the HillshadeExaggeration property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeExaggeration: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-exaggeration", nativeGetHillshadeExaggeration())
            return value as PropertyValue<Float>
        }

    /**
     * The HillshadeExaggeration property transition options
     */
    var hillshadeExaggerationTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHillshadeExaggerationTransition()
        }
        set(options) {
            checkThread()
            nativeSetHillshadeExaggerationTransition(options.duration, options.delay)
        }

    /**
     * Get the HillshadeShadowColor property
     *
     * @return property wrapper value around Array<String>
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeShadowColor: PropertyValue<Array<String>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-shadow-color", nativeGetHillshadeShadowColor())
            return value as PropertyValue<Array<String>>
        }

    /**
     * The HillshadeShadowColor property transition options
     */
    var hillshadeShadowColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHillshadeShadowColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetHillshadeShadowColorTransition(options.duration, options.delay)
        }

    /**
     * Get the HillshadeHighlightColor property
     *
     * @return property wrapper value around Array<String>
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeHighlightColor: PropertyValue<Array<String>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-highlight-color", nativeGetHillshadeHighlightColor())
            return value as PropertyValue<Array<String>>
        }

    /**
     * The HillshadeHighlightColor property transition options
     */
    var hillshadeHighlightColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHillshadeHighlightColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetHillshadeHighlightColorTransition(options.duration, options.delay)
        }

    /**
     * Get the HillshadeAccentColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeAccentColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-accent-color", nativeGetHillshadeAccentColor())
            return value as PropertyValue<String>
        }

    /**
     * The shading color used to accentuate rugged terrain like sharp cliffs and gorges.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getHillshadeAccentColorAsInt(): Int {
        checkThread()
        val value = hillshadeAccentColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("hillshade-accent-color was set as a Function")
        }
    }

    /**
     * The HillshadeAccentColor property transition options
     */
    var hillshadeAccentColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetHillshadeAccentColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetHillshadeAccentColorTransition(options.duration, options.delay)
        }

    /**
     * Get the HillshadeMethod property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val hillshadeMethod: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("hillshade-method", nativeGetHillshadeMethod())
            return value as PropertyValue<String>
        }

    @Keep
    private external fun nativeGetHillshadeIlluminationDirection(): Any

    @Keep
    private external fun nativeGetHillshadeIlluminationAltitude(): Any

    @Keep
    private external fun nativeGetHillshadeIlluminationAnchor(): Any

    @Keep
    private external fun nativeGetHillshadeExaggeration(): Any

    @Keep
    private external fun nativeGetHillshadeExaggerationTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHillshadeExaggerationTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetHillshadeShadowColor(): Any

    @Keep
    private external fun nativeGetHillshadeShadowColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHillshadeShadowColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetHillshadeHighlightColor(): Any

    @Keep
    private external fun nativeGetHillshadeHighlightColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHillshadeHighlightColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetHillshadeAccentColor(): Any

    @Keep
    private external fun nativeGetHillshadeAccentColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetHillshadeAccentColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetHillshadeMethod(): Any

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
