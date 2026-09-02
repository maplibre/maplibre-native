// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.types.Formatted
import org.maplibre.android.utils.ColorUtils.colorToRgbaString

/**
 * Constructs paint/layout properties for Layers
 *
 * @see [Layer style documentation](https://maplibre.org/maplibre-style-spec/#layers)
 */
object PropertyFactory {

    /**
     * Set the property visibility.
     *
     * @param value the visibility value
     * @return property wrapper around visibility
     */
    @JvmStatic
    fun visibility(@Property.VISIBILITY value: String?): PropertyValue<String> = LayoutPropertyValue("visibility", value)

    /**
     * Whether or not the fill should be antialiased.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun fillAntialias(value: Boolean?): PropertyValue<Boolean> =
        PaintPropertyValue("fill-antialias", value)

    /**
     * Whether or not the fill should be antialiased.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillAntialias(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-antialias", expression)

    /**
     * The opacity of the entire fill layer. In contrast to the [PropertyFactory.fillColor], this value will also affect the 1px stroke around the fill, if the stroke is used.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("fill-opacity", value)

    /**
     * The opacity of the entire fill layer. In contrast to the [PropertyFactory.fillColor], this value will also affect the 1px stroke around the fill, if the stroke is used.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-opacity", expression)

    /**
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun fillColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("fill-color", colorToRgbaString(value))

    /**
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-color", value)

    /**
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-color", expression)

    /**
     * The outline color of the fill. Matches the value of [PropertyFactory.fillColor] if unspecified.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun fillOutlineColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("fill-outline-color", colorToRgbaString(value))

    /**
     * The outline color of the fill. Matches the value of [PropertyFactory.fillColor] if unspecified.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillOutlineColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-outline-color", value)

    /**
     * The outline color of the fill. Matches the value of [PropertyFactory.fillColor] if unspecified.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillOutlineColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-outline-color", expression)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun fillTranslate(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("fill-translate", value)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillTranslate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-translate", expression)

    /**
     * Controls the frame of reference for [PropertyFactory.fillTranslate].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillTranslateAnchor(@Property.FILL_TRANSLATE_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-translate-anchor", value)

    /**
     * Controls the frame of reference for [PropertyFactory.fillTranslate].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillTranslateAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-translate-anchor", expression)

    /**
     * Name of image in sprite to use for drawing image fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillPattern(value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-pattern", value)

    /**
     * Name of image in sprite to use for drawing image fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillPattern(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-pattern", expression)

    /**
     * The opacity at which the line will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("line-opacity", value)

    /**
     * The opacity at which the line will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-opacity", expression)

    /**
     * The color with which the line will be drawn.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun lineColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("line-color", colorToRgbaString(value))

    /**
     * The color with which the line will be drawn.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("line-color", value)

    /**
     * The color with which the line will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-color", expression)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun lineTranslate(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("line-translate", value)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineTranslate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-translate", expression)

    /**
     * Controls the frame of reference for [PropertyFactory.lineTranslate].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineTranslateAnchor(@Property.LINE_TRANSLATE_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("line-translate-anchor", value)

    /**
     * Controls the frame of reference for [PropertyFactory.lineTranslate].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineTranslateAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-translate-anchor", expression)

    /**
     * Stroke thickness.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineWidth(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("line-width", value)

    /**
     * Stroke thickness.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineWidth(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-width", expression)

    /**
     * Draws a line casing outside of a line's actual path. Value indicates the width of the inner gap.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineGapWidth(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("line-gap-width", value)

    /**
     * Draws a line casing outside of a line's actual path. Value indicates the width of the inner gap.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineGapWidth(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-gap-width", expression)

    /**
     * The line's offset. For linear features, a positive value offsets the line to the right, relative to the direction of the line, and a negative value to the left. For polygon features, a positive value results in an inset, and a negative value results in an outset.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineOffset(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("line-offset", value)

    /**
     * The line's offset. For linear features, a positive value offsets the line to the right, relative to the direction of the line, and a negative value to the left. For polygon features, a positive value results in an inset, and a negative value results in an outset.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineOffset(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-offset", expression)

    /**
     * Blur applied to the line, in density-independent pixels.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineBlur(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("line-blur", value)

    /**
     * Blur applied to the line, in density-independent pixels.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineBlur(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-blur", expression)

    /**
     * Specifies the lengths of the alternating dashes and gaps that form the dash pattern. The lengths are later scaled by the line width. To convert a dash length to density-independent pixels, multiply the length by the current line width. Note that GeoJSON sources with `lineMetrics: true` specified won't render dashed lines to the expected scale. Also note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun lineDasharray(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("line-dasharray", value)

    /**
     * Specifies the lengths of the alternating dashes and gaps that form the dash pattern. The lengths are later scaled by the line width. To convert a dash length to density-independent pixels, multiply the length by the current line width. Note that GeoJSON sources with `lineMetrics: true` specified won't render dashed lines to the expected scale. Also note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineDasharray(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-dasharray", expression)

    /**
     * Name of image in sprite to use for drawing image lines. For seamless patterns, image width must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun linePattern(value: String?): PropertyValue<String> =
        PaintPropertyValue("line-pattern", value)

    /**
     * Name of image in sprite to use for drawing image lines. For seamless patterns, image width must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun linePattern(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-pattern", expression)

    /**
     * Defines a gradient with which to color a line feature. Can only be used with GeoJSON sources that specify `"lineMetrics": true`.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun lineGradient(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("line-gradient", colorToRgbaString(value))

    /**
     * Defines a gradient with which to color a line feature. Can only be used with GeoJSON sources that specify `"lineMetrics": true`.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineGradient(value: String?): PropertyValue<String> =
        PaintPropertyValue("line-gradient", value)

    /**
     * Defines a gradient with which to color a line feature. Can only be used with GeoJSON sources that specify `"lineMetrics": true`.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun lineGradient(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("line-gradient", expression)

    /**
     * The opacity at which the icon will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("icon-opacity", value)

    /**
     * The opacity at which the icon will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-opacity", expression)

    /**
     * The color of the icon. This can only be used with SDF icons.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun iconColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("icon-color", colorToRgbaString(value))

    /**
     * The color of the icon. This can only be used with SDF icons.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("icon-color", value)

    /**
     * The color of the icon. This can only be used with SDF icons.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-color", expression)

    /**
     * The color of the icon's halo. Icon halos can only be used with SDF icons.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun iconHaloColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("icon-halo-color", colorToRgbaString(value))

    /**
     * The color of the icon's halo. Icon halos can only be used with SDF icons.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconHaloColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("icon-halo-color", value)

    /**
     * The color of the icon's halo. Icon halos can only be used with SDF icons.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconHaloColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-halo-color", expression)

    /**
     * Distance of halo to the icon outline. 

The unit is in density-independent pixels only for SDF sprites that were created with a blur radius of 8, multiplied by the display density. I.e., the radius needs to be 16 for `@2x` sprites, etc.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconHaloWidth(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("icon-halo-width", value)

    /**
     * Distance of halo to the icon outline. 

The unit is in density-independent pixels only for SDF sprites that were created with a blur radius of 8, multiplied by the display density. I.e., the radius needs to be 16 for `@2x` sprites, etc.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconHaloWidth(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-halo-width", expression)

    /**
     * Fade out the halo towards the outside.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconHaloBlur(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("icon-halo-blur", value)

    /**
     * Fade out the halo towards the outside.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconHaloBlur(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-halo-blur", expression)

    /**
     * Distance that the icon's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconTranslate(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("icon-translate", value)

    /**
     * Distance that the icon's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconTranslate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-translate", expression)

    /**
     * Controls the frame of reference for [PropertyFactory.iconTranslate].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconTranslateAnchor(@Property.ICON_TRANSLATE_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("icon-translate-anchor", value)

    /**
     * Controls the frame of reference for [PropertyFactory.iconTranslate].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun iconTranslateAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("icon-translate-anchor", expression)

    /**
     * The opacity at which the text will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("text-opacity", value)

    /**
     * The opacity at which the text will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-opacity", expression)

    /**
     * The color with which the text will be drawn.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun textColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("text-color", colorToRgbaString(value))

    /**
     * The color with which the text will be drawn.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("text-color", value)

    /**
     * The color with which the text will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-color", expression)

    /**
     * The color of the text's halo, which helps it stand out from backgrounds.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun textHaloColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("text-halo-color", colorToRgbaString(value))

    /**
     * The color of the text's halo, which helps it stand out from backgrounds.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textHaloColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("text-halo-color", value)

    /**
     * The color of the text's halo, which helps it stand out from backgrounds.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textHaloColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-halo-color", expression)

    /**
     * Distance of halo to the font outline. Max text halo width is 1/4 of the font-size.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textHaloWidth(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("text-halo-width", value)

    /**
     * Distance of halo to the font outline. Max text halo width is 1/4 of the font-size.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textHaloWidth(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-halo-width", expression)

    /**
     * The halo's fadeout distance towards the outside.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textHaloBlur(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("text-halo-blur", value)

    /**
     * The halo's fadeout distance towards the outside.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textHaloBlur(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-halo-blur", expression)

    /**
     * Distance that the text's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun textTranslate(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("text-translate", value)

    /**
     * Distance that the text's anchor is moved from its original placement. Positive values indicate right and down, while negative values indicate left and up.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textTranslate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-translate", expression)

    /**
     * Controls the frame of reference for [PropertyFactory.textTranslate].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textTranslateAnchor(@Property.TEXT_TRANSLATE_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("text-translate-anchor", value)

    /**
     * Controls the frame of reference for [PropertyFactory.textTranslate].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun textTranslateAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("text-translate-anchor", expression)

    /**
     * Circle radius.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleRadius(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("circle-radius", value)

    /**
     * Circle radius.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleRadius(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-radius", expression)

    /**
     * The fill color of the circle.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun circleColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("circle-color", colorToRgbaString(value))

    /**
     * The fill color of the circle.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun circleColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("circle-color", value)

    /**
     * The fill color of the circle.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-color", expression)

    /**
     * Amount to blur the circle. 1 blurs the circle such that only the centerpoint is full opacity.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleBlur(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("circle-blur", value)

    /**
     * Amount to blur the circle. 1 blurs the circle such that only the centerpoint is full opacity.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleBlur(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-blur", expression)

    /**
     * The opacity at which the circle will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("circle-opacity", value)

    /**
     * The opacity at which the circle will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-opacity", expression)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun circleTranslate(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("circle-translate", value)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up, respectively.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleTranslate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-translate", expression)

    /**
     * Controls the frame of reference for [PropertyFactory.circleTranslate].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun circleTranslateAnchor(@Property.CIRCLE_TRANSLATE_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("circle-translate-anchor", value)

    /**
     * Controls the frame of reference for [PropertyFactory.circleTranslate].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleTranslateAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-translate-anchor", expression)

    /**
     * Controls the scaling behavior of the circle when the map is pitched.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun circlePitchScale(@Property.CIRCLE_PITCH_SCALE value: String?): PropertyValue<String> =
        PaintPropertyValue("circle-pitch-scale", value)

    /**
     * Controls the scaling behavior of the circle when the map is pitched.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circlePitchScale(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-pitch-scale", expression)

    /**
     * Orientation of circle when map is pitched.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun circlePitchAlignment(@Property.CIRCLE_PITCH_ALIGNMENT value: String?): PropertyValue<String> =
        PaintPropertyValue("circle-pitch-alignment", value)

    /**
     * Orientation of circle when map is pitched.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circlePitchAlignment(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-pitch-alignment", expression)

    /**
     * The width of the circle's stroke. Strokes are placed outside of the [PropertyFactory.circleRadius].
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleStrokeWidth(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("circle-stroke-width", value)

    /**
     * The width of the circle's stroke. Strokes are placed outside of the [PropertyFactory.circleRadius].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleStrokeWidth(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-stroke-width", expression)

    /**
     * The stroke color of the circle.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun circleStrokeColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("circle-stroke-color", colorToRgbaString(value))

    /**
     * The stroke color of the circle.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun circleStrokeColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("circle-stroke-color", value)

    /**
     * The stroke color of the circle.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleStrokeColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-stroke-color", expression)

    /**
     * The opacity of the circle's stroke.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleStrokeOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("circle-stroke-opacity", value)

    /**
     * The opacity of the circle's stroke.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun circleStrokeOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("circle-stroke-opacity", expression)

    /**
     * Radius of influence of one heatmap point in density-independent pixels. Increasing the value makes the heatmap smoother, but less detailed.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun heatmapRadius(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("heatmap-radius", value)

    /**
     * Radius of influence of one heatmap point in density-independent pixels. Increasing the value makes the heatmap smoother, but less detailed.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun heatmapRadius(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("heatmap-radius", expression)

    /**
     * A measure of how much an individual point contributes to the heatmap. A value of 10 would be equivalent to having 10 points of weight 1 in the same spot. Especially useful when combined with clustering.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun heatmapWeight(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("heatmap-weight", value)

    /**
     * A measure of how much an individual point contributes to the heatmap. A value of 10 would be equivalent to having 10 points of weight 1 in the same spot. Especially useful when combined with clustering.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun heatmapWeight(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("heatmap-weight", expression)

    /**
     * Similar to [PropertyFactory.heatmapWeight] but controls the intensity of the heatmap globally. Primarily used for adjusting the heatmap based on zoom level.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun heatmapIntensity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("heatmap-intensity", value)

    /**
     * Similar to [PropertyFactory.heatmapWeight] but controls the intensity of the heatmap globally. Primarily used for adjusting the heatmap based on zoom level.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun heatmapIntensity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("heatmap-intensity", expression)

    /**
     * Defines the color of each pixel based on its density value in a heatmap.  Should be an expression that uses `["heatmap-density"]` as input.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun heatmapColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("heatmap-color", colorToRgbaString(value))

    /**
     * Defines the color of each pixel based on its density value in a heatmap.  Should be an expression that uses `["heatmap-density"]` as input.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun heatmapColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("heatmap-color", value)

    /**
     * Defines the color of each pixel based on its density value in a heatmap.  Should be an expression that uses `["heatmap-density"]` as input.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun heatmapColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("heatmap-color", expression)

    /**
     * The global opacity at which the heatmap layer will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun heatmapOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("heatmap-opacity", value)

    /**
     * The global opacity at which the heatmap layer will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun heatmapOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("heatmap-opacity", expression)

    /**
     * The opacity of the entire fill extrusion layer. This is rendered on a per-layer, not per-feature, basis, and data-driven styling is not available.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillExtrusionOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("fill-extrusion-opacity", value)

    /**
     * The opacity of the entire fill extrusion layer. This is rendered on a per-layer, not per-feature, basis, and data-driven styling is not available.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-opacity", expression)

    /**
     * The base color of the extruded fill. The extrusion's surfaces will be shaded differently based on this color in combination with the root `light` settings. If this color is specified as `rgba` with an alpha component, the alpha component will be ignored; use [PropertyFactory.fillExtrusionOpacity] to set layer opacity.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun fillExtrusionColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("fill-extrusion-color", colorToRgbaString(value))

    /**
     * The base color of the extruded fill. The extrusion's surfaces will be shaded differently based on this color in combination with the root `light` settings. If this color is specified as `rgba` with an alpha component, the alpha component will be ignored; use [PropertyFactory.fillExtrusionOpacity] to set layer opacity.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillExtrusionColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-extrusion-color", value)

    /**
     * The base color of the extruded fill. The extrusion's surfaces will be shaded differently based on this color in combination with the root `light` settings. If this color is specified as `rgba` with an alpha component, the alpha component will be ignored; use [PropertyFactory.fillExtrusionOpacity] to set layer opacity.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-color", expression)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up (on the flat plane), respectively.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun fillExtrusionTranslate(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("fill-extrusion-translate", value)

    /**
     * The geometry's offset. Values are [x, y] where negatives indicate left and up (on the flat plane), respectively.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionTranslate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-translate", expression)

    /**
     * Controls the frame of reference for [PropertyFactory.fillExtrusionTranslate].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillExtrusionTranslateAnchor(@Property.FILL_EXTRUSION_TRANSLATE_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-extrusion-translate-anchor", value)

    /**
     * Controls the frame of reference for [PropertyFactory.fillExtrusionTranslate].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionTranslateAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-translate-anchor", expression)

    /**
     * Name of image in sprite to use for drawing images on extruded fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun fillExtrusionPattern(value: String?): PropertyValue<String> =
        PaintPropertyValue("fill-extrusion-pattern", value)

    /**
     * Name of image in sprite to use for drawing images on extruded fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionPattern(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-pattern", expression)

    /**
     * The height with which to extrude this layer.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillExtrusionHeight(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("fill-extrusion-height", value)

    /**
     * The height with which to extrude this layer.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionHeight(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-height", expression)

    /**
     * The height with which to extrude the base of this layer. Must be less than or equal to [PropertyFactory.fillExtrusionHeight].
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillExtrusionBase(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("fill-extrusion-base", value)

    /**
     * The height with which to extrude the base of this layer. Must be less than or equal to [PropertyFactory.fillExtrusionHeight].
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionBase(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-base", expression)

    /**
     * Whether to apply a vertical gradient to the sides of a fill-extrusion layer. If true, sides will be shaded slightly darker farther down.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun fillExtrusionVerticalGradient(value: Boolean?): PropertyValue<Boolean> =
        PaintPropertyValue("fill-extrusion-vertical-gradient", value)

    /**
     * Whether to apply a vertical gradient to the sides of a fill-extrusion layer. If true, sides will be shaded slightly darker farther down.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun fillExtrusionVerticalGradient(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("fill-extrusion-vertical-gradient", expression)

    /**
     * The opacity at which the image will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-opacity", value)

    /**
     * The opacity at which the image will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-opacity", expression)

    /**
     * Rotates hues around the color wheel.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterHueRotate(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-hue-rotate", value)

    /**
     * Rotates hues around the color wheel.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterHueRotate(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-hue-rotate", expression)

    /**
     * Increase or reduce the brightness of the image. The value is the minimum brightness.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterBrightnessMin(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-brightness-min", value)

    /**
     * Increase or reduce the brightness of the image. The value is the minimum brightness.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterBrightnessMin(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-brightness-min", expression)

    /**
     * Increase or reduce the brightness of the image. The value is the maximum brightness.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterBrightnessMax(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-brightness-max", value)

    /**
     * Increase or reduce the brightness of the image. The value is the maximum brightness.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterBrightnessMax(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-brightness-max", expression)

    /**
     * Increase or reduce the saturation of the image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterSaturation(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-saturation", value)

    /**
     * Increase or reduce the saturation of the image.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterSaturation(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-saturation", expression)

    /**
     * Increase or reduce the contrast of the image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterContrast(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-contrast", value)

    /**
     * Increase or reduce the contrast of the image.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterContrast(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-contrast", expression)

    /**
     * The resampling/interpolation method to use for overscaling, also known as texture magnification filter
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun rasterResampling(@Property.RASTER_RESAMPLING value: String?): PropertyValue<String> =
        PaintPropertyValue("raster-resampling", value)

    /**
     * The resampling/interpolation method to use for overscaling, also known as texture magnification filter
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterResampling(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-resampling", expression)

    /**
     * Fade duration when a new tile is added, or when a video is started or its coordinates are updated.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun rasterFadeDuration(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("raster-fade-duration", value)

    /**
     * Fade duration when a new tile is added, or when a video is started or its coordinates are updated.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun rasterFadeDuration(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("raster-fade-duration", expression)

    /**
     * The direction of the light source(s) used to generate the hillshading with 0 as the top of the viewport if [Property.HILLSHADE_ILLUMINATION_ANCHOR] is set to `viewport` and due north if [Property.HILLSHADE_ILLUMINATION_ANCHOR] is set to `map`. Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun hillshadeIlluminationDirection(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("hillshade-illumination-direction", value)

    /**
     * The direction of the light source(s) used to generate the hillshading with 0 as the top of the viewport if [Property.HILLSHADE_ILLUMINATION_ANCHOR] is set to `viewport` and due north if [Property.HILLSHADE_ILLUMINATION_ANCHOR] is set to `map`. Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeIlluminationDirection(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-illumination-direction", expression)

    /**
     * The altitude of the light source(s) used to generate the hillshading with 0 as sunset and 90 as noon. Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun hillshadeIlluminationAltitude(value: Array<Float>?): PropertyValue<Array<Float>> =
        PaintPropertyValue("hillshade-illumination-altitude", value)

    /**
     * The altitude of the light source(s) used to generate the hillshading with 0 as sunset and 90 as noon. Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeIlluminationAltitude(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-illumination-altitude", expression)

    /**
     * Direction of light source when map is rotated.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun hillshadeIlluminationAnchor(@Property.HILLSHADE_ILLUMINATION_ANCHOR value: String?): PropertyValue<String> =
        PaintPropertyValue("hillshade-illumination-anchor", value)

    /**
     * Direction of light source when map is rotated.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeIlluminationAnchor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-illumination-anchor", expression)

    /**
     * Intensity of the hillshade
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun hillshadeExaggeration(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("hillshade-exaggeration", value)

    /**
     * Intensity of the hillshade
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeExaggeration(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-exaggeration", expression)

    /**
     * The shading color of areas that face away from the light source(s). Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun hillshadeShadowColor(value: Array<String>?): PropertyValue<Array<String>> =
        PaintPropertyValue("hillshade-shadow-color", value)

    /**
     * The shading color of areas that face away from the light source(s). Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeShadowColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-shadow-color", expression)

    /**
     * The shading color of areas that faces towards the light source(s). Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun hillshadeHighlightColor(value: Array<String>?): PropertyValue<Array<String>> =
        PaintPropertyValue("hillshade-highlight-color", value)

    /**
     * The shading color of areas that faces towards the light source(s). Only when [Property.HILLSHADE_METHOD] is set to `multidirectional` can you specify multiple light sources.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeHighlightColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-highlight-color", expression)

    /**
     * The shading color used to accentuate rugged terrain like sharp cliffs and gorges.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun hillshadeAccentColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("hillshade-accent-color", colorToRgbaString(value))

    /**
     * The shading color used to accentuate rugged terrain like sharp cliffs and gorges.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun hillshadeAccentColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("hillshade-accent-color", value)

    /**
     * The shading color used to accentuate rugged terrain like sharp cliffs and gorges.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeAccentColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-accent-color", expression)

    /**
     * The hillshade algorithm to use, one of `standard`, `basic`, `combined`, `igor`, or `multidirectional`. ![image](assets/hillshade_methods.png)
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun hillshadeMethod(@Property.HILLSHADE_METHOD value: String?): PropertyValue<String> =
        PaintPropertyValue("hillshade-method", value)

    /**
     * The hillshade algorithm to use, one of `standard`, `basic`, `combined`, `igor`, or `multidirectional`. ![image](assets/hillshade_methods.png)
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun hillshadeMethod(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("hillshade-method", expression)

    /**
     * The opacity at which the color-relief will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun colorReliefOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("color-relief-opacity", value)

    /**
     * The opacity at which the color-relief will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun colorReliefOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("color-relief-opacity", expression)

    /**
     * Defines the color of each pixel based on its elevation. Should be an expression that uses `["elevation"]` as input.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun colorReliefColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("color-relief-color", colorToRgbaString(value))

    /**
     * Defines the color of each pixel based on its elevation. Should be an expression that uses `["elevation"]` as input.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun colorReliefColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("color-relief-color", value)

    /**
     * Defines the color of each pixel based on its elevation. Should be an expression that uses `["elevation"]` as input.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun colorReliefColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("color-relief-color", expression)

    /**
     * The color with which the background will be drawn.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun backgroundColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("background-color", colorToRgbaString(value))

    /**
     * The color with which the background will be drawn.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun backgroundColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("background-color", value)

    /**
     * The color with which the background will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun backgroundColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("background-color", expression)

    /**
     * Name of image in sprite to use for drawing an image background. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun backgroundPattern(value: String?): PropertyValue<String> =
        PaintPropertyValue("background-pattern", value)

    /**
     * Name of image in sprite to use for drawing an image background. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun backgroundPattern(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("background-pattern", expression)

    /**
     * The opacity at which the background will be drawn.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun backgroundOpacity(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("background-opacity", value)

    /**
     * The opacity at which the background will be drawn.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun backgroundOpacity(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("background-opacity", expression)

    /**
     * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillSortKey(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("fill-sort-key", value)

    /**
     * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillSortKey(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("fill-sort-key", value)

    /**
     * The display of line endings.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineCap(@Property.LINE_CAP value: String?): PropertyValue<String> =
        LayoutPropertyValue("line-cap", value)

    /**
     * The display of line endings.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineCap(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("line-cap", value)

    /**
     * The display of lines when joining.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineJoin(@Property.LINE_JOIN value: String?): PropertyValue<String> =
        LayoutPropertyValue("line-join", value)

    /**
     * The display of lines when joining.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun lineJoin(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("line-join", value)

    /**
     * Used to automatically convert miter joins to bevel joins for sharp angles.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineMiterLimit(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("line-miter-limit", value)

    /**
     * Used to automatically convert miter joins to bevel joins for sharp angles.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineMiterLimit(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("line-miter-limit", value)

    /**
     * Used to automatically convert round joins to miter joins for shallow angles.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineRoundLimit(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("line-round-limit", value)

    /**
     * Used to automatically convert round joins to miter joins for shallow angles.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineRoundLimit(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("line-round-limit", value)

    /**
     * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineSortKey(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("line-sort-key", value)

    /**
     * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun lineSortKey(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("line-sort-key", value)

    /**
     * Label placement relative to its geometry.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun symbolPlacement(@Property.SYMBOL_PLACEMENT value: String?): PropertyValue<String> =
        LayoutPropertyValue("symbol-placement", value)

    /**
     * Label placement relative to its geometry.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun symbolPlacement(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("symbol-placement", value)

    /**
     * Distance between two symbol anchors.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun symbolSpacing(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("symbol-spacing", value)

    /**
     * Distance between two symbol anchors.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun symbolSpacing(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("symbol-spacing", value)

    /**
     * If true, the symbols will not cross tile edges to avoid mutual collisions. Recommended in layers that don't have enough padding in the vector tile to prevent collisions, or if it is a point symbol layer placed after a line symbol layer. When using a client that supports global collision detection, like MapLibre GL JS version 0.42.0 or greater, enabling this property is not needed to prevent clipped labels at tile boundaries.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun symbolAvoidEdges(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("symbol-avoid-edges", value)

    /**
     * If true, the symbols will not cross tile edges to avoid mutual collisions. Recommended in layers that don't have enough padding in the vector tile to prevent collisions, or if it is a point symbol layer placed after a line symbol layer. When using a client that supports global collision detection, like MapLibre GL JS version 0.42.0 or greater, enabling this property is not needed to prevent clipped labels at tile boundaries.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun symbolAvoidEdges(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("symbol-avoid-edges", value)

    /**
     * Sorts features in ascending order based on this value. Features with lower sort keys are drawn and placed first.  When [PropertyFactory.iconAllowOverlap] or [PropertyFactory.textAllowOverlap] is `false`, features with a lower sort key will have priority during placement. When [PropertyFactory.iconAllowOverlap] or [PropertyFactory.textAllowOverlap] is set to `true`, features with a higher sort key will overlap over features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun symbolSortKey(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("symbol-sort-key", value)

    /**
     * Sorts features in ascending order based on this value. Features with lower sort keys are drawn and placed first.  When [PropertyFactory.iconAllowOverlap] or [PropertyFactory.textAllowOverlap] is `false`, features with a lower sort key will have priority during placement. When [PropertyFactory.iconAllowOverlap] or [PropertyFactory.textAllowOverlap] is set to `true`, features with a higher sort key will overlap over features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun symbolSortKey(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("symbol-sort-key", value)

    /**
     * Determines whether overlapping symbols in the same layer are rendered in the order that they appear in the data source or by their y-position relative to the viewport. To control the order and prioritization of symbols otherwise, use [PropertyFactory.symbolSortKey].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun symbolZOrder(@Property.SYMBOL_Z_ORDER value: String?): PropertyValue<String> =
        LayoutPropertyValue("symbol-z-order", value)

    /**
     * Determines whether overlapping symbols in the same layer are rendered in the order that they appear in the data source or by their y-position relative to the viewport. To control the order and prioritization of symbols otherwise, use [PropertyFactory.symbolSortKey].
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun symbolZOrder(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("symbol-z-order", value)

    /**
     * If true, the icon will be visible even if it collides with other previously drawn symbols.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconAllowOverlap(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("icon-allow-overlap", value)

    /**
     * If true, the icon will be visible even if it collides with other previously drawn symbols.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconAllowOverlap(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-allow-overlap", value)

    /**
     * If true, other symbols can be visible even if they collide with the icon.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconIgnorePlacement(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("icon-ignore-placement", value)

    /**
     * If true, other symbols can be visible even if they collide with the icon.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconIgnorePlacement(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-ignore-placement", value)

    /**
     * If true, text will display without their corresponding icons when the icon collides with other symbols and the text does not.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconOptional(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("icon-optional", value)

    /**
     * If true, text will display without their corresponding icons when the icon collides with other symbols and the text does not.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconOptional(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-optional", value)

    /**
     * In combination with [Property.SYMBOL_PLACEMENT], determines the rotation behavior of icons.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconRotationAlignment(@Property.ICON_ROTATION_ALIGNMENT value: String?): PropertyValue<String> =
        LayoutPropertyValue("icon-rotation-alignment", value)

    /**
     * In combination with [Property.SYMBOL_PLACEMENT], determines the rotation behavior of icons.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconRotationAlignment(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-rotation-alignment", value)

    /**
     * Scales the original size of the icon by the provided factor. The new pixel size of the image will be the original pixel size multiplied by [PropertyFactory.iconSize]. 1 is the original size; 3 triples the size of the image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconSize(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("icon-size", value)

    /**
     * Scales the original size of the icon by the provided factor. The new pixel size of the image will be the original pixel size multiplied by [PropertyFactory.iconSize]. 1 is the original size; 3 triples the size of the image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconSize(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-size", value)

    /**
     * Scales the icon to fit around the associated text.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconTextFit(@Property.ICON_TEXT_FIT value: String?): PropertyValue<String> =
        LayoutPropertyValue("icon-text-fit", value)

    /**
     * Scales the icon to fit around the associated text.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconTextFit(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-text-fit", value)

    /**
     * Size of the additional area added to dimensions determined by [Property.ICON_TEXT_FIT], in clockwise order: top, right, bottom, left.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconTextFitPadding(value: Array<Float>?): PropertyValue<Array<Float>> =
        LayoutPropertyValue("icon-text-fit-padding", value)

    /**
     * Size of the additional area added to dimensions determined by [Property.ICON_TEXT_FIT], in clockwise order: top, right, bottom, left.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconTextFitPadding(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-text-fit-padding", value)

    /**
     * Name of image in sprite to use for drawing an image background.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconImage(value: String?): PropertyValue<String> =
        LayoutPropertyValue("icon-image", value)

    /**
     * Name of image in sprite to use for drawing an image background.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconImage(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-image", value)

    /**
     * Rotates the icon clockwise.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconRotate(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("icon-rotate", value)

    /**
     * Rotates the icon clockwise.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconRotate(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-rotate", value)

    /**
     * Size of additional area round the icon bounding box used for detecting symbol collisions.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun iconPadding(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("icon-padding", value)

    /**
     * Size of additional area round the icon bounding box used for detecting symbol collisions.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconPadding(value: Array<Float>?): PropertyValue<Array<Float>> =
        LayoutPropertyValue("icon-padding", value)

    /**
     * Size of additional area round the icon bounding box used for detecting symbol collisions.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconPadding(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-padding", value)

    /**
     * If true, the icon may be flipped to prevent it from being rendered upside-down.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconKeepUpright(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("icon-keep-upright", value)

    /**
     * If true, the icon may be flipped to prevent it from being rendered upside-down.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun iconKeepUpright(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-keep-upright", value)

    /**
     * Offset distance of icon from its anchor. Positive values indicate right and down, while negative values indicate left and up. Each component is multiplied by the value of [PropertyFactory.iconSize] to obtain the final offset in density-independent pixels. When combined with [PropertyFactory.iconRotate] the offset will be as if the rotated direction was up.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconOffset(value: Array<Float>?): PropertyValue<Array<Float>> =
        LayoutPropertyValue("icon-offset", value)

    /**
     * Offset distance of icon from its anchor. Positive values indicate right and down, while negative values indicate left and up. Each component is multiplied by the value of [PropertyFactory.iconSize] to obtain the final offset in density-independent pixels. When combined with [PropertyFactory.iconRotate] the offset will be as if the rotated direction was up.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun iconOffset(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-offset", value)

    /**
     * Part of the icon placed closest to the anchor.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconAnchor(@Property.ICON_ANCHOR value: String?): PropertyValue<String> =
        LayoutPropertyValue("icon-anchor", value)

    /**
     * Part of the icon placed closest to the anchor.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconAnchor(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-anchor", value)

    /**
     * Orientation of icon when map is pitched.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconPitchAlignment(@Property.ICON_PITCH_ALIGNMENT value: String?): PropertyValue<String> =
        LayoutPropertyValue("icon-pitch-alignment", value)

    /**
     * Orientation of icon when map is pitched.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun iconPitchAlignment(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("icon-pitch-alignment", value)

    /**
     * Orientation of text when map is pitched.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textPitchAlignment(@Property.TEXT_PITCH_ALIGNMENT value: String?): PropertyValue<String> =
        LayoutPropertyValue("text-pitch-alignment", value)

    /**
     * Orientation of text when map is pitched.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textPitchAlignment(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-pitch-alignment", value)

    /**
     * In combination with [Property.SYMBOL_PLACEMENT], determines the rotation behavior of the individual glyphs forming the text.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textRotationAlignment(@Property.TEXT_ROTATION_ALIGNMENT value: String?): PropertyValue<String> =
        LayoutPropertyValue("text-rotation-alignment", value)

    /**
     * In combination with [Property.SYMBOL_PLACEMENT], determines the rotation behavior of the individual glyphs forming the text.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textRotationAlignment(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-rotation-alignment", value)

    /**
     * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with default/inherited formatting options.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textField(value: String?): PropertyValue<String> =
        LayoutPropertyValue("text-field", value)

    /**
     * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with default/inherited formatting options.
     *
     * @param value a Formatted value
     * @return property wrapper around Formatted
     */
    @JvmStatic
    fun textField(value: Formatted?): PropertyValue<Formatted> =
        LayoutPropertyValue("text-field", value)

    /**
     * Value to use for a text label. If a plain `string` is provided, it will be treated as a `formatted` with default/inherited formatting options.
     *
     * @param value a Formatted value
     * @return property wrapper around Formatted
     */
    @JvmStatic
    fun textField(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-field", value)

    /**
     * Fonts to use for displaying text. If the `glyphs` root property is specified, this array is joined together and interpreted as a font stack name. Otherwise, it is interpreted as a cascading fallback list of local font names.
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun textFont(value: Array<String>?): PropertyValue<Array<String>> =
        LayoutPropertyValue("text-font", value)

    /**
     * Fonts to use for displaying text. If the `glyphs` root property is specified, this array is joined together and interpreted as a font stack name. Otherwise, it is interpreted as a cascading fallback list of local font names.
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun textFont(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-font", value)

    /**
     * Font size.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textSize(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-size", value)

    /**
     * Font size.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textSize(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-size", value)

    /**
     * The maximum line width for text wrapping.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textMaxWidth(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-max-width", value)

    /**
     * The maximum line width for text wrapping.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textMaxWidth(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-max-width", value)

    /**
     * Text leading value for multi-line text.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textLineHeight(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-line-height", value)

    /**
     * Text leading value for multi-line text.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textLineHeight(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-line-height", value)

    /**
     * Text tracking amount.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textLetterSpacing(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-letter-spacing", value)

    /**
     * Text tracking amount.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textLetterSpacing(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-letter-spacing", value)

    /**
     * Text justification options.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textJustify(@Property.TEXT_JUSTIFY value: String?): PropertyValue<String> =
        LayoutPropertyValue("text-justify", value)

    /**
     * Text justification options.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textJustify(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-justify", value)

    /**
     * Radial offset of text, in the direction of the symbol's anchor. Useful in combination with [PropertyFactory.textVariableAnchor], which defaults to using the two-dimensional [PropertyFactory.textOffset] if present.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textRadialOffset(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-radial-offset", value)

    /**
     * Radial offset of text, in the direction of the symbol's anchor. Useful in combination with [PropertyFactory.textVariableAnchor], which defaults to using the two-dimensional [PropertyFactory.textOffset] if present.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textRadialOffset(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-radial-offset", value)

    /**
     * To increase the chance of placing high-priority labels on the map, you can provide an array of [Property.TEXT_ANCHOR] locations: the renderer will attempt to place the label at each location, in order, before moving onto the next label. Use `text-justify: auto` to choose justification based on anchor position. To apply an offset, use the [PropertyFactory.textRadialOffset] or the two-dimensional [PropertyFactory.textOffset].
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun textVariableAnchor(value: Array<String>?): PropertyValue<Array<String>> =
        LayoutPropertyValue("text-variable-anchor", value)

    /**
     * To increase the chance of placing high-priority labels on the map, you can provide an array of [Property.TEXT_ANCHOR] locations: the renderer will attempt to place the label at each location, in order, before moving onto the next label. Use `text-justify: auto` to choose justification based on anchor position. To apply an offset, use the [PropertyFactory.textRadialOffset] or the two-dimensional [PropertyFactory.textOffset].
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun textVariableAnchor(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-variable-anchor", value)

    /**
     * To increase the chance of placing high-priority labels on the map, you can provide an array of [Property.TEXT_ANCHOR] locations, each paired with an offset value. The renderer will attempt to place the label at each location, in order, before moving on to the next location+offset. Use `text-justify: auto` to choose justification based on anchor position. 

 The length of the array must be even, and must alternate between enum and point entries. i.e., each anchor location must be accompanied by a point, and that point defines the offset when the corresponding anchor location is used. Positive offset values indicate right and down, while negative values indicate left and up. Anchor locations may repeat, allowing the renderer to try multiple offsets to try and place a label using the same anchor. 

 When present, this property takes precedence over [Property.TEXT_ANCHOR], [PropertyFactory.textVariableAnchor], [PropertyFactory.textOffset], and [PropertyFactory.textRadialOffset]. 

 ```json 

 { "text-variable-anchor-offset": ["top", [0, 4], "left", [3,0], "bottom", [1, 1]] } 

 ``` 

 When the renderer chooses the `top` anchor, `[0, 4]` will be used for [PropertyFactory.textOffset]; the text will be shifted down by 4 ems. 

 When the renderer chooses the `left` anchor, `[3, 0]` will be used for [PropertyFactory.textOffset]; the text will be shifted right by 3 ems.
     *
     * @param value a Array<Any> value
     * @return property wrapper around Array<Any>
     */
    @JvmStatic
    fun textVariableAnchorOffset(value: Array<Any>?): PropertyValue<Array<Any>> =
        LayoutPropertyValue("text-variable-anchor-offset", value)

    /**
     * To increase the chance of placing high-priority labels on the map, you can provide an array of [Property.TEXT_ANCHOR] locations, each paired with an offset value. The renderer will attempt to place the label at each location, in order, before moving on to the next location+offset. Use `text-justify: auto` to choose justification based on anchor position. 

 The length of the array must be even, and must alternate between enum and point entries. i.e., each anchor location must be accompanied by a point, and that point defines the offset when the corresponding anchor location is used. Positive offset values indicate right and down, while negative values indicate left and up. Anchor locations may repeat, allowing the renderer to try multiple offsets to try and place a label using the same anchor. 

 When present, this property takes precedence over [Property.TEXT_ANCHOR], [PropertyFactory.textVariableAnchor], [PropertyFactory.textOffset], and [PropertyFactory.textRadialOffset]. 

 ```json 

 { "text-variable-anchor-offset": ["top", [0, 4], "left", [3,0], "bottom", [1, 1]] } 

 ``` 

 When the renderer chooses the `top` anchor, `[0, 4]` will be used for [PropertyFactory.textOffset]; the text will be shifted down by 4 ems. 

 When the renderer chooses the `left` anchor, `[3, 0]` will be used for [PropertyFactory.textOffset]; the text will be shifted right by 3 ems.
     *
     * @param value a Array<Any> value
     * @return property wrapper around Array<Any>
     */
    @JvmStatic
    fun textVariableAnchorOffset(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-variable-anchor-offset", value)

    /**
     * Part of the text placed closest to the anchor.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textAnchor(@Property.TEXT_ANCHOR value: String?): PropertyValue<String> =
        LayoutPropertyValue("text-anchor", value)

    /**
     * Part of the text placed closest to the anchor.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textAnchor(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-anchor", value)

    /**
     * Maximum angle change between adjacent characters.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textMaxAngle(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-max-angle", value)

    /**
     * Maximum angle change between adjacent characters.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textMaxAngle(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-max-angle", value)

    /**
     * The property allows control over a symbol's orientation. Note that the property values act as a hint, so that a symbol whose language doesn’t support the provided orientation will be laid out in its natural orientation. Example: English point symbol will be rendered horizontally even if array value contains single 'vertical' enum value. The order of elements in an array define priority order for the placement of an orientation variant.
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun textWritingMode(value: Array<String>?): PropertyValue<Array<String>> =
        LayoutPropertyValue("text-writing-mode", value)

    /**
     * The property allows control over a symbol's orientation. Note that the property values act as a hint, so that a symbol whose language doesn’t support the provided orientation will be laid out in its natural orientation. Example: English point symbol will be rendered horizontally even if array value contains single 'vertical' enum value. The order of elements in an array define priority order for the placement of an orientation variant.
     *
     * @param value a Array<String> value
     * @return property wrapper around Array<String>
     */
    @JvmStatic
    fun textWritingMode(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-writing-mode", value)

    /**
     * Rotates the text clockwise.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textRotate(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-rotate", value)

    /**
     * Rotates the text clockwise.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textRotate(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-rotate", value)

    /**
     * Size of the additional area around the text bounding box used for detecting symbol collisions.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textPadding(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("text-padding", value)

    /**
     * Size of the additional area around the text bounding box used for detecting symbol collisions.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun textPadding(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-padding", value)

    /**
     * If true, the text may be flipped vertically to prevent it from being rendered upside-down.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textKeepUpright(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("text-keep-upright", value)

    /**
     * If true, the text may be flipped vertically to prevent it from being rendered upside-down.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textKeepUpright(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-keep-upright", value)

    /**
     * Specifies how to capitalize text, similar to the CSS [PropertyFactory.textTransform] property.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textTransform(@Property.TEXT_TRANSFORM value: String?): PropertyValue<String> =
        LayoutPropertyValue("text-transform", value)

    /**
     * Specifies how to capitalize text, similar to the CSS [PropertyFactory.textTransform] property.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun textTransform(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-transform", value)

    /**
     * Offset distance of text from its anchor. Positive values indicate right and down, while negative values indicate left and up. If used with text-variable-anchor, input values will be taken as absolute values. Offsets along the x- and y-axis will be applied automatically based on the anchor position.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun textOffset(value: Array<Float>?): PropertyValue<Array<Float>> =
        LayoutPropertyValue("text-offset", value)

    /**
     * Offset distance of text from its anchor. Positive values indicate right and down, while negative values indicate left and up. If used with text-variable-anchor, input values will be taken as absolute values. Offsets along the x- and y-axis will be applied automatically based on the anchor position.
     *
     * @param value a Array<Float> value
     * @return property wrapper around Array<Float>
     */
    @JvmStatic
    fun textOffset(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-offset", value)

    /**
     * If true, the text will be visible even if it collides with other previously drawn symbols.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textAllowOverlap(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("text-allow-overlap", value)

    /**
     * If true, the text will be visible even if it collides with other previously drawn symbols.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textAllowOverlap(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-allow-overlap", value)

    /**
     * If true, other symbols can be visible even if they collide with the text.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textIgnorePlacement(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("text-ignore-placement", value)

    /**
     * If true, other symbols can be visible even if they collide with the text.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textIgnorePlacement(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-ignore-placement", value)

    /**
     * If true, icons will display without their corresponding text when the text collides with other symbols and the icon does not.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textOptional(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("text-optional", value)

    /**
     * If true, icons will display without their corresponding text when the text collides with other symbols and the icon does not.
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun textOptional(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("text-optional", value)

    /**
     * Internal use only
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun symbolScreenSpace(value: Boolean?): PropertyValue<Boolean> =
        LayoutPropertyValue("symbol-screen-space", value)

    /**
     * Internal use only
     *
     * @param value a Boolean value
     * @return property wrapper around Boolean
     */
    @JvmStatic
    fun symbolScreenSpace(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("symbol-screen-space", value)

    /**
     * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleSortKey(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("circle-sort-key", value)

    /**
     * Sorts features in ascending order based on this value. Features with a higher sort key will appear above features with a lower sort key.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun circleSortKey(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("circle-sort-key", value)

    /**
     * The distance from the corner that will be cut and replaced with rounded corner.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillExtrusionRoundedCornerDistance(value: Float?): PropertyValue<Float> =
        LayoutPropertyValue("fill-extrusion-rounded-corner-distance", value)

    /**
     * The distance from the corner that will be cut and replaced with rounded corner.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun fillExtrusionRoundedCornerDistance(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("fill-extrusion-rounded-corner-distance", value)

}
