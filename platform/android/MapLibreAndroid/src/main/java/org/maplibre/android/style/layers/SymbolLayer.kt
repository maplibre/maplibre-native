// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.types.Formatted
import org.maplibre.android.utils.ColorUtils.rgbaToColor

/**
 * An icon or a text label.
 *
 * @see [The online documentation](https://maplibre.org/maplibre-style-spec/#layers-symbol)
 */
@UiThread
class SymbolLayer : Layer {

    /**
     * Creates a SymbolLayer.
     *
     * @param nativePtr pointer used by core
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr)

    /**
     * Creates a SymbolLayer.
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
    fun withSourceLayer(sourceLayer: String?): SymbolLayer {
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
    fun withFilter(filter: Expression): SymbolLayer {
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
    fun withProperties(vararg properties: PropertyValue<*>): SymbolLayer {
        setProperties(*properties)
        return this
    }

    // Property getters

    /**
     * Get the SymbolPlacement property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val symbolPlacement: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("symbol-placement", nativeGetSymbolPlacement())
            return value as PropertyValue<String>
        }

    /**
     * Get the SymbolSpacing property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val symbolSpacing: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("symbol-spacing", nativeGetSymbolSpacing())
            return value as PropertyValue<Float>
        }

    /**
     * Get the SymbolAvoidEdges property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val symbolAvoidEdges: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("symbol-avoid-edges", nativeGetSymbolAvoidEdges())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the SymbolSortKey property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val symbolSortKey: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("symbol-sort-key", nativeGetSymbolSortKey())
            return value as PropertyValue<Float>
        }

    /**
     * Get the SymbolZOrder property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val symbolZOrder: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("symbol-z-order", nativeGetSymbolZOrder())
            return value as PropertyValue<String>
        }

    /**
     * Get the IconAllowOverlap property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val iconAllowOverlap: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-allow-overlap", nativeGetIconAllowOverlap())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the IconIgnorePlacement property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val iconIgnorePlacement: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-ignore-placement", nativeGetIconIgnorePlacement())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the IconOptional property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val iconOptional: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-optional", nativeGetIconOptional())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the IconRotationAlignment property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconRotationAlignment: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-rotation-alignment", nativeGetIconRotationAlignment())
            return value as PropertyValue<String>
        }

    /**
     * Get the IconSize property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val iconSize: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-size", nativeGetIconSize())
            return value as PropertyValue<Float>
        }

    /**
     * Get the IconTextFit property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconTextFit: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-text-fit", nativeGetIconTextFit())
            return value as PropertyValue<String>
        }

    /**
     * Get the IconTextFitPadding property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val iconTextFitPadding: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-text-fit-padding", nativeGetIconTextFitPadding())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * Get the IconImage property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconImage: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-image", nativeGetIconImage())
            return value as PropertyValue<String>
        }

    /**
     * Get the IconRotate property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val iconRotate: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-rotate", nativeGetIconRotate())
            return value as PropertyValue<Float>
        }

    /**
     * Get the IconPadding property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val iconPadding: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-padding", nativeGetIconPadding())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * Get the IconKeepUpright property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val iconKeepUpright: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-keep-upright", nativeGetIconKeepUpright())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the IconOffset property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val iconOffset: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-offset", nativeGetIconOffset())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * Get the IconAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-anchor", nativeGetIconAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the IconPitchAlignment property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconPitchAlignment: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-pitch-alignment", nativeGetIconPitchAlignment())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextPitchAlignment property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textPitchAlignment: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-pitch-alignment", nativeGetTextPitchAlignment())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextRotationAlignment property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textRotationAlignment: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-rotation-alignment", nativeGetTextRotationAlignment())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextField property
     *
     * @return property wrapper value around Formatted
     */
    @Suppress("UNCHECKED_CAST")
    val textField: PropertyValue<Formatted>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-field", nativeGetTextField())
            return value as PropertyValue<Formatted>
        }

    /**
     * Get the TextFont property
     *
     * @return property wrapper value around Array<String>
     */
    @Suppress("UNCHECKED_CAST")
    val textFont: PropertyValue<Array<String>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-font", nativeGetTextFont())
            return value as PropertyValue<Array<String>>
        }

    /**
     * Get the TextSize property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textSize: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-size", nativeGetTextSize())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextMaxWidth property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textMaxWidth: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-max-width", nativeGetTextMaxWidth())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextLineHeight property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textLineHeight: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-line-height", nativeGetTextLineHeight())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextLetterSpacing property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textLetterSpacing: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-letter-spacing", nativeGetTextLetterSpacing())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextJustify property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textJustify: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-justify", nativeGetTextJustify())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextRadialOffset property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textRadialOffset: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-radial-offset", nativeGetTextRadialOffset())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextVariableAnchor property
     *
     * @return property wrapper value around Array<String>
     */
    @Suppress("UNCHECKED_CAST")
    val textVariableAnchor: PropertyValue<Array<String>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-variable-anchor", nativeGetTextVariableAnchor())
            return value as PropertyValue<Array<String>>
        }

    /**
     * Get the TextVariableAnchorOffset property
     *
     * @return property wrapper value around Array<Any>
     */
    @Suppress("UNCHECKED_CAST")
    val textVariableAnchorOffset: PropertyValue<Array<Any>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-variable-anchor-offset", nativeGetTextVariableAnchorOffset())
            return value as PropertyValue<Array<Any>>
        }

    /**
     * Get the TextAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-anchor", nativeGetTextAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextMaxAngle property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textMaxAngle: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-max-angle", nativeGetTextMaxAngle())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextWritingMode property
     *
     * @return property wrapper value around Array<String>
     */
    @Suppress("UNCHECKED_CAST")
    val textWritingMode: PropertyValue<Array<String>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-writing-mode", nativeGetTextWritingMode())
            return value as PropertyValue<Array<String>>
        }

    /**
     * Get the TextRotate property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textRotate: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-rotate", nativeGetTextRotate())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextPadding property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textPadding: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-padding", nativeGetTextPadding())
            return value as PropertyValue<Float>
        }

    /**
     * Get the TextKeepUpright property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val textKeepUpright: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-keep-upright", nativeGetTextKeepUpright())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the TextTransform property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textTransform: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-transform", nativeGetTextTransform())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextOffset property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val textOffset: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-offset", nativeGetTextOffset())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * Get the TextAllowOverlap property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val textAllowOverlap: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-allow-overlap", nativeGetTextAllowOverlap())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the TextIgnorePlacement property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val textIgnorePlacement: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-ignore-placement", nativeGetTextIgnorePlacement())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the TextOptional property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val textOptional: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-optional", nativeGetTextOptional())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the SymbolScreenSpace property
     *
     * @return property wrapper value around Boolean
     */
    @Suppress("UNCHECKED_CAST")
    val symbolScreenSpace: PropertyValue<Boolean>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("symbol-screen-space", nativeGetSymbolScreenSpace())
            return value as PropertyValue<Boolean>
        }

    /**
     * Get the IconOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val iconOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-opacity", nativeGetIconOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The IconOpacity property transition options
     */
    var iconOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIconOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetIconOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the IconColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-color", nativeGetIconColor())
            return value as PropertyValue<String>
        }

    /**
     * The color of the icon. This can only be used with SDF icons.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getIconColorAsInt(): Int {
        checkThread()
        val value = iconColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("icon-color was set as a Function")
        }
    }

    /**
     * The IconColor property transition options
     */
    var iconColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIconColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetIconColorTransition(options.duration, options.delay)
        }

    /**
     * Get the IconHaloColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconHaloColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-halo-color", nativeGetIconHaloColor())
            return value as PropertyValue<String>
        }

    /**
     * The color of the icon's halo. Icon halos can only be used with SDF icons.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getIconHaloColorAsInt(): Int {
        checkThread()
        val value = iconHaloColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("icon-halo-color was set as a Function")
        }
    }

    /**
     * The IconHaloColor property transition options
     */
    var iconHaloColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIconHaloColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetIconHaloColorTransition(options.duration, options.delay)
        }

    /**
     * Get the IconHaloWidth property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val iconHaloWidth: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-halo-width", nativeGetIconHaloWidth())
            return value as PropertyValue<Float>
        }

    /**
     * The IconHaloWidth property transition options
     */
    var iconHaloWidthTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIconHaloWidthTransition()
        }
        set(options) {
            checkThread()
            nativeSetIconHaloWidthTransition(options.duration, options.delay)
        }

    /**
     * Get the IconHaloBlur property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val iconHaloBlur: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-halo-blur", nativeGetIconHaloBlur())
            return value as PropertyValue<Float>
        }

    /**
     * The IconHaloBlur property transition options
     */
    var iconHaloBlurTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIconHaloBlurTransition()
        }
        set(options) {
            checkThread()
            nativeSetIconHaloBlurTransition(options.duration, options.delay)
        }

    /**
     * Get the IconTranslate property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val iconTranslate: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-translate", nativeGetIconTranslate())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The IconTranslate property transition options
     */
    var iconTranslateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetIconTranslateTransition()
        }
        set(options) {
            checkThread()
            nativeSetIconTranslateTransition(options.duration, options.delay)
        }

    /**
     * Get the IconTranslateAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val iconTranslateAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("icon-translate-anchor", nativeGetIconTranslateAnchor())
            return value as PropertyValue<String>
        }

    /**
     * Get the TextOpacity property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textOpacity: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-opacity", nativeGetTextOpacity())
            return value as PropertyValue<Float>
        }

    /**
     * The TextOpacity property transition options
     */
    var textOpacityTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTextOpacityTransition()
        }
        set(options) {
            checkThread()
            nativeSetTextOpacityTransition(options.duration, options.delay)
        }

    /**
     * Get the TextColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-color", nativeGetTextColor())
            return value as PropertyValue<String>
        }

    /**
     * The color with which the text will be drawn.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getTextColorAsInt(): Int {
        checkThread()
        val value = textColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("text-color was set as a Function")
        }
    }

    /**
     * The TextColor property transition options
     */
    var textColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTextColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetTextColorTransition(options.duration, options.delay)
        }

    /**
     * Get the TextHaloColor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textHaloColor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-halo-color", nativeGetTextHaloColor())
            return value as PropertyValue<String>
        }

    /**
     * The color of the text's halo, which helps it stand out from backgrounds.
     *
     * @return int representation of a rgba string color
     * @throws RuntimeException thrown if property isn't a value
     */
    @ColorInt
    fun getTextHaloColorAsInt(): Int {
        checkThread()
        val value = textHaloColor
        if (value.isValue()) {
            return rgbaToColor(value.getValue()!!)
        } else {
            throw RuntimeException("text-halo-color was set as a Function")
        }
    }

    /**
     * The TextHaloColor property transition options
     */
    var textHaloColorTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTextHaloColorTransition()
        }
        set(options) {
            checkThread()
            nativeSetTextHaloColorTransition(options.duration, options.delay)
        }

    /**
     * Get the TextHaloWidth property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textHaloWidth: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-halo-width", nativeGetTextHaloWidth())
            return value as PropertyValue<Float>
        }

    /**
     * The TextHaloWidth property transition options
     */
    var textHaloWidthTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTextHaloWidthTransition()
        }
        set(options) {
            checkThread()
            nativeSetTextHaloWidthTransition(options.duration, options.delay)
        }

    /**
     * Get the TextHaloBlur property
     *
     * @return property wrapper value around Float
     */
    @Suppress("UNCHECKED_CAST")
    val textHaloBlur: PropertyValue<Float>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-halo-blur", nativeGetTextHaloBlur())
            return value as PropertyValue<Float>
        }

    /**
     * The TextHaloBlur property transition options
     */
    var textHaloBlurTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTextHaloBlurTransition()
        }
        set(options) {
            checkThread()
            nativeSetTextHaloBlurTransition(options.duration, options.delay)
        }

    /**
     * Get the TextTranslate property
     *
     * @return property wrapper value around Array<Float>
     */
    @Suppress("UNCHECKED_CAST")
    val textTranslate: PropertyValue<Array<Float>>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-translate", nativeGetTextTranslate())
            return value as PropertyValue<Array<Float>>
        }

    /**
     * The TextTranslate property transition options
     */
    var textTranslateTransition: TransitionOptions
        get() {
            checkThread()
            return nativeGetTextTranslateTransition()
        }
        set(options) {
            checkThread()
            nativeSetTextTranslateTransition(options.duration, options.delay)
        }

    /**
     * Get the TextTranslateAnchor property
     *
     * @return property wrapper value around String
     */
    @Suppress("UNCHECKED_CAST")
    val textTranslateAnchor: PropertyValue<String>
        get() {
            checkThread()
            val value = PropertyValue<Any?>("text-translate-anchor", nativeGetTextTranslateAnchor())
            return value as PropertyValue<String>
        }

    @Keep
    private external fun nativeGetSymbolPlacement(): Any

    @Keep
    private external fun nativeGetSymbolSpacing(): Any

    @Keep
    private external fun nativeGetSymbolAvoidEdges(): Any

    @Keep
    private external fun nativeGetSymbolSortKey(): Any

    @Keep
    private external fun nativeGetSymbolZOrder(): Any

    @Keep
    private external fun nativeGetIconAllowOverlap(): Any

    @Keep
    private external fun nativeGetIconIgnorePlacement(): Any

    @Keep
    private external fun nativeGetIconOptional(): Any

    @Keep
    private external fun nativeGetIconRotationAlignment(): Any

    @Keep
    private external fun nativeGetIconSize(): Any

    @Keep
    private external fun nativeGetIconTextFit(): Any

    @Keep
    private external fun nativeGetIconTextFitPadding(): Any

    @Keep
    private external fun nativeGetIconImage(): Any

    @Keep
    private external fun nativeGetIconRotate(): Any

    @Keep
    private external fun nativeGetIconPadding(): Any

    @Keep
    private external fun nativeGetIconKeepUpright(): Any

    @Keep
    private external fun nativeGetIconOffset(): Any

    @Keep
    private external fun nativeGetIconAnchor(): Any

    @Keep
    private external fun nativeGetIconPitchAlignment(): Any

    @Keep
    private external fun nativeGetTextPitchAlignment(): Any

    @Keep
    private external fun nativeGetTextRotationAlignment(): Any

    @Keep
    private external fun nativeGetTextField(): Any

    @Keep
    private external fun nativeGetTextFont(): Any

    @Keep
    private external fun nativeGetTextSize(): Any

    @Keep
    private external fun nativeGetTextMaxWidth(): Any

    @Keep
    private external fun nativeGetTextLineHeight(): Any

    @Keep
    private external fun nativeGetTextLetterSpacing(): Any

    @Keep
    private external fun nativeGetTextJustify(): Any

    @Keep
    private external fun nativeGetTextRadialOffset(): Any

    @Keep
    private external fun nativeGetTextVariableAnchor(): Any

    @Keep
    private external fun nativeGetTextVariableAnchorOffset(): Any

    @Keep
    private external fun nativeGetTextAnchor(): Any

    @Keep
    private external fun nativeGetTextMaxAngle(): Any

    @Keep
    private external fun nativeGetTextWritingMode(): Any

    @Keep
    private external fun nativeGetTextRotate(): Any

    @Keep
    private external fun nativeGetTextPadding(): Any

    @Keep
    private external fun nativeGetTextKeepUpright(): Any

    @Keep
    private external fun nativeGetTextTransform(): Any

    @Keep
    private external fun nativeGetTextOffset(): Any

    @Keep
    private external fun nativeGetTextAllowOverlap(): Any

    @Keep
    private external fun nativeGetTextIgnorePlacement(): Any

    @Keep
    private external fun nativeGetTextOptional(): Any

    @Keep
    private external fun nativeGetSymbolScreenSpace(): Any

    @Keep
    private external fun nativeGetIconOpacity(): Any

    @Keep
    private external fun nativeGetIconOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIconOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetIconColor(): Any

    @Keep
    private external fun nativeGetIconColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIconColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetIconHaloColor(): Any

    @Keep
    private external fun nativeGetIconHaloColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIconHaloColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetIconHaloWidth(): Any

    @Keep
    private external fun nativeGetIconHaloWidthTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIconHaloWidthTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetIconHaloBlur(): Any

    @Keep
    private external fun nativeGetIconHaloBlurTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIconHaloBlurTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetIconTranslate(): Any

    @Keep
    private external fun nativeGetIconTranslateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetIconTranslateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetIconTranslateAnchor(): Any

    @Keep
    private external fun nativeGetTextOpacity(): Any

    @Keep
    private external fun nativeGetTextOpacityTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTextOpacityTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTextColor(): Any

    @Keep
    private external fun nativeGetTextColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTextColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTextHaloColor(): Any

    @Keep
    private external fun nativeGetTextHaloColorTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTextHaloColorTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTextHaloWidth(): Any

    @Keep
    private external fun nativeGetTextHaloWidthTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTextHaloWidthTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTextHaloBlur(): Any

    @Keep
    private external fun nativeGetTextHaloBlurTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTextHaloBlurTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTextTranslate(): Any

    @Keep
    private external fun nativeGetTextTranslateTransition(): TransitionOptions

    @Keep
    private external fun nativeSetTextTranslateTransition(duration: Long, delay: Long)

    @Keep
    private external fun nativeGetTextTranslateAnchor(): Any

    @Keep
    @Throws(Throwable::class)
    protected override external fun finalize()
}
