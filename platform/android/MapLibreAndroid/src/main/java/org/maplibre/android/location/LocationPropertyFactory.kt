// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.location

import androidx.annotation.ColorInt
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.types.Formatted
import org.maplibre.android.style.layers.LayoutPropertyValue
import org.maplibre.android.style.layers.PaintPropertyValue
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.utils.ColorUtils.colorToRgbaString

/**
 * Constructs paint/layout properties for Layers
 *
 * @see [Layer style documentation](https://maplibre.org/maplibre-style-spec/#layers)
 */
internal object LocationPropertyFactory {

    /**
     * Set the property visibility.
     *
     * @param value the visibility value
     * @return property wrapper around visibility
     */
    @JvmStatic
    fun visibility(@Property.VISIBILITY value: String?): PropertyValue<String> = LayoutPropertyValue("visibility", value)

    /**
     * The amount of the perspective compensation, between 0 and 1. A value of 1 produces a location indicator of constant width across the screen. A value of 0 makes it scale naturally according to the viewing projection.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun perspectiveCompensation(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("perspective-compensation", value)

    /**
     * The amount of the perspective compensation, between 0 and 1. A value of 1 produces a location indicator of constant width across the screen. A value of 0 makes it scale naturally according to the viewing projection.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun perspectiveCompensation(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("perspective-compensation", expression)

    /**
     * The displacement off the center of the top image and the shadow image when the pitch of the map is greater than 0. This helps producing a three-dimensional appearence.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun imageTiltDisplacement(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("image-tilt-displacement", value)

    /**
     * The displacement off the center of the top image and the shadow image when the pitch of the map is greater than 0. This helps producing a three-dimensional appearence.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun imageTiltDisplacement(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("image-tilt-displacement", expression)

    /**
     * The bearing of the location indicator.
     *
     * @param value a Double value
     * @return property wrapper around Double
     */
    @JvmStatic
    fun bearing(value: Double?): PropertyValue<Double> =
        PaintPropertyValue("bearing", value)

    /**
     * The bearing of the location indicator.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun bearing(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("bearing", expression)

    /**
     * An array of [latitude, longitude, altitude] position of the location indicator.
     *
     * @param value a Array<Double> value
     * @return property wrapper around Array<Double>
     */
    @JvmStatic
    fun location(value: Array<Double>?): PropertyValue<Array<Double>> =
        PaintPropertyValue("location", value)

    /**
     * An array of [latitude, longitude, altitude] position of the location indicator.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun location(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("location", expression)

    /**
     * The accuracy, in meters, of the position source used to retrieve the position of the location indicator.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun accuracyRadius(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("accuracy-radius", value)

    /**
     * The accuracy, in meters, of the position source used to retrieve the position of the location indicator.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun accuracyRadius(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("accuracy-radius", expression)

    /**
     * The size of the top image, as a scale factor applied to the size of the specified image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun topImageSize(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("top-image-size", value)

    /**
     * The size of the top image, as a scale factor applied to the size of the specified image.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun topImageSize(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("top-image-size", expression)

    /**
     * The size of the bearing image, as a scale factor applied to the size of the specified image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun bearingImageSize(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("bearing-image-size", value)

    /**
     * The size of the bearing image, as a scale factor applied to the size of the specified image.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun bearingImageSize(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("bearing-image-size", expression)

    /**
     * The size of the shadow image, as a scale factor applied to the size of the specified image.
     *
     * @param value a Float value
     * @return property wrapper around Float
     */
    @JvmStatic
    fun shadowImageSize(value: Float?): PropertyValue<Float> =
        PaintPropertyValue("shadow-image-size", value)

    /**
     * The size of the shadow image, as a scale factor applied to the size of the specified image.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun shadowImageSize(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("shadow-image-size", expression)

    /**
     * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun accuracyRadiusColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("accuracy-radius-color", colorToRgbaString(value))

    /**
     * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun accuracyRadiusColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("accuracy-radius-color", value)

    /**
     * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun accuracyRadiusColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("accuracy-radius-color", expression)

    /**
     * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @param value a int color value
     * @return property wrapper around String color
     */
    @JvmStatic
    fun accuracyRadiusBorderColor(@ColorInt value: Int): PropertyValue<String> =
        PaintPropertyValue("accuracy-radius-border-color", colorToRgbaString(value))

    /**
     * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun accuracyRadiusBorderColor(value: String?): PropertyValue<String> =
        PaintPropertyValue("accuracy-radius-border-color", value)

    /**
     * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
     *
     * @param expression an expression statement
     * @return property wrapper around an expression statement
     */
    @JvmStatic
    fun accuracyRadiusBorderColor(expression: Expression?): PropertyValue<Expression> =
        PaintPropertyValue("accuracy-radius-border-color", expression)

    /**
     * Name of image in sprite to use as the top of the location indicator.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun topImage(value: String?): PropertyValue<String> =
        LayoutPropertyValue("top-image", value)

    /**
     * Name of image in sprite to use as the top of the location indicator.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun topImage(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("top-image", value)

    /**
     * Name of image in sprite to use as the middle of the location indicator.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun bearingImage(value: String?): PropertyValue<String> =
        LayoutPropertyValue("bearing-image", value)

    /**
     * Name of image in sprite to use as the middle of the location indicator.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun bearingImage(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("bearing-image", value)

    /**
     * Name of image in sprite to use as the background of the location indicator.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun shadowImage(value: String?): PropertyValue<String> =
        LayoutPropertyValue("shadow-image", value)

    /**
     * Name of image in sprite to use as the background of the location indicator.
     *
     * @param value a String value
     * @return property wrapper around String
     */
    @JvmStatic
    fun shadowImage(value: Expression?): PropertyValue<Expression> =
        LayoutPropertyValue("shadow-image", value)

}
