package org.maplibre.android.style.expressions

import android.annotation.SuppressLint
import androidx.annotation.ColorInt
import androidx.annotation.Size
import com.google.gson.Gson
import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonNull
import com.google.gson.JsonObject
import com.google.gson.JsonPrimitive
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.utils.ColorUtils.colorToRgbaArray
import org.maplibre.geojson.GeoJson
import org.maplibre.geojson.Polygon
import org.maplibre.geojson.gson.GeometryGeoJson
import java.util.Arrays
import java.util.Locale

/**
 * The value for any layout property, paint property, or filter may be specified as an expression.
 * An expression defines a formula for computing the value of the property using the operators described below.
 * The set of expression operators provided by MapLibre GL includes:
 *
 * - Element
 * - Mathematical operators for performing arithmetic and other operations on numeric values
 * - Logical operators for manipulating boolean values and making conditional decisions
 * - String operators for manipulating strings
 * - Data operators, providing access to the properties of source features
 * - Camera operators, providing access to the parameters defining the current map view
 *
 * Expressions are represented as JSON arrays.
 * The first element of an expression array is a string naming the expression operator,
 * e.g. "*"or "case". Subsequent elements (if any) are the arguments to the expression.
 * Each argument is either a literal value (a string, number, boolean, or null), or another expression array.
 *
 * Data expression: a data expression is any expression that access feature data -- that is,
 * any expression that uses one of the data operators:get,has,id,geometry-type, or properties.
 * Data expressions allow a feature's properties to determine its appearance.
 * They can be used to differentiate features within the same layer and to create data visualizations.
 *
 * Camera expression: a camera expression is any expression that uses the zoom operator.
 * Such expressions allow the the appearance of a layer to change with the map's zoom level.
 * Camera expressions can be used to create the appearance of depth and to control data density.
 *
 * Composition: a single expression may use a mix of data operators, camera operators, and other operators.
 * Such composite expressions allows a layer's appearance to be determined by
 * a combination of the zoom level and individual feature properties.
 *
 * Example expression:
 *
 * ```kotlin
 * val fillLayer = FillLayer("layer-id", "source-id")
 * fillLayer.setProperties(
 *   fillColor(
 *     interpolate( linear(), zoom(),
 *       stop(12, step(get("stroke-width"),
 *         color(Color.BLACK),
 *         stop(1f, color(Color.RED)),
 *         stop(2f, color(Color.WHITE)),
 *         stop(3f, color(Color.BLUE))
 *       )),
 *       stop(15, step(get("stroke-width"),
 *         color(Color.BLACK),
 *         stop(1f, color(Color.YELLOW)),
 *         stop(2f, color(Color.LTGRAY)),
 *         stop(3f, color(Color.CYAN))
 *       )),
 *       stop(18, step(get("stroke-width"),
 *         color(Color.BLACK),
 *         stop(1f, color(Color.WHITE)),
 *         stop(2f, color(Color.GRAY)),
 *         stop(3f, color(Color.GREEN))
 *       ))
 *     )
 *   )
 * )
 * ```
 */
open class Expression {
    private val operator: String?

    private val arguments: kotlin.Array<out Expression>?

    /**
     * Creates an empty expression for expression literals
     */
    internal constructor() {
        operator = null
        arguments = null
    }

    /**
     * Creates an expression from its operator and varargs expressions.
     *
     * @param operator  the expression operator
     * @param arguments expressions input
     */
    constructor(operator: String, vararg arguments: Expression) {
        this.operator = operator
        this.arguments = arguments
    }

    /**
     * Converts the expression to Object array representation.
     *
     * The output will later be converted to a JSON Object array.
     *
     * @return the converted object array expression
     */
    open fun toArray(): kotlin.Array<Any?> {
        val array: MutableList<Any?> = ArrayList()
        array.add(operator)
        if (arguments != null) {
            for (argument in arguments) {
                if (argument is ValueExpression) {
                    array.add(argument.toValue())
                } else {
                    array.add(argument.toArray())
                }
            }
        }
        return array.toTypedArray()
    }

    /**
     * Returns a string representation of the object that matches the definition set in the style specification.
     *
     * If this expression contains a coma (, ) delimited literal, like 'rgba(r, g, b, a)`,
     * it will be enclosed with double quotes (").
     *
     * @return a string representation of the object.
     */
    override fun toString(): String {
        val builder = StringBuilder()
        builder.append("[\"").append(operator).append("\"")
        if (arguments != null) {
            for (argument in arguments) {
                builder.append(", ")
                builder.append(argument.toString())
            }
        }
        builder.append("]")
        return builder.toString()
    }

    /**
     * Indicates whether some other object is "equal to" this one.
     *
     * @param other the other object
     * @return true if equal, false if not
     */
    override fun equals(other: Any?): Boolean {
        super.equals(other)
        if (this === other) {
            return true
        }

        if (other == null || other !is Expression) {
            return false
        }

        if (operator != other.operator) {
            return false
        }
        return Arrays.deepEquals(arguments, other.arguments)
    }

    /**
     * Returns a hash code value for the expression.
     *
     * @return a hash code value for this expression
     */
    override fun hashCode(): Int {
        var result = operator?.hashCode() ?: 0
        result = 31 * result + Arrays.hashCode(arguments)
        return result
    }

    /**
     * ExpressionLiteral wraps an object to be used as a literal in an expression.
     *
     * ExpressionLiteral is created with the [literal] overloads.
     *
     * @constructor Create an expression literal.
     * @param object the object to be treated as literal
     */
    @Suppress("EXPOSED_SUPER_INTERFACE")
    open class ExpressionLiteral(
        `object`: Any,
    ) : Expression(),
        ValueExpression {
        @JvmField
        protected var literal: Any

        init {
            var value: Any = `object`
            if (value is String) {
                value = unwrapStringLiteral(value)
            } else if (value is Number) {
                value = value.toFloat()
            }
            literal = value
        }

        /**
         * Get the literal object.
         *
         * @return the literal object
         */
        override fun toValue(): Any {
            if (literal is PropertyValue<*>) {
                throw IllegalArgumentException(
                    "PropertyValue are not allowed as an expression literal, use value instead.",
                )
            } else if (literal is ExpressionLiteral) {
                return (literal as ExpressionLiteral).toValue()
            }
            return literal
        }

        override fun toArray(): kotlin.Array<Any?> = arrayOf("literal", literal)

        /**
         * Returns a string representation of the expression literal.
         *
         * @return a string representation of the object.
         */
        override fun toString(): String =
            if (literal is String) {
                "\"" + literal + "\""
            } else {
                literal.toString()
            }

        /**
         * Indicates whether some other object is "equal to" this one.
         *
         * @param other the other object
         * @return true if equal, false if not
         */
        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }
            if (!super.equals(other)) {
                return false
            }

            val that = other as ExpressionLiteral

            return literal == that.literal
        }

        /**
         * Returns a hash code value for the expression literal.
         *
         * @return a hash code value for this expression literal
         */
        override fun hashCode(): Int {
            var result = super.hashCode()
            result = 31 * result + literal.hashCode()
            return result
        }

        private companion object {
            fun unwrapStringLiteral(value: String): String =
                if (value.length > 1 && value[0] == '"' && value[value.length - 1] == '"') {
                    value.substring(1, value.length - 1)
                } else {
                    value
                }
        }
    }

    /**
     * Expression interpolator type.
     *
     * Is used for first parameter of [interpolate].
     */
    open class Interpolator internal constructor(
        operator: String,
        vararg arguments: Expression,
    ) : Expression(operator, *arguments)

    /**
     * Expression array type.
     */
    open class Array

    /**
     * Expression stop type.
     *
     * Can be used for [stop] as part of varargs parameter in
     * [step] or [interpolate].
     */
    open class Stop internal constructor(
        private val value: Any,
        private val output: Any,
    ) {
        internal companion object {
            /**
             * Converts a varargs of Stops to a Expression array.
             *
             * @param stops the stops to convert
             * @return the converted stops as an expression array
             */
            fun toExpressionArray(vararg stops: Stop): kotlin.Array<Expression> {
                val expressions = arrayOfNulls<Expression>(stops.size * 2)
                for (i in stops.indices) {
                    val stop = stops[i]
                    var inputValue: Any = stop.value
                    var outputValue: Any = stop.output

                    if (inputValue !is Expression) {
                        inputValue = literal(inputValue)
                    }

                    if (outputValue !is Expression) {
                        outputValue = literal(outputValue)
                    }

                    expressions[i * 2] = inputValue as Expression
                    expressions[i * 2 + 1] = outputValue as Expression
                }
                @Suppress("UNCHECKED_CAST")
                return expressions as kotlin.Array<Expression>
            }
        }
    }

    /**
     * Holds format entries used in a [format] expression.
     */
    open class FormatEntry internal constructor(
        internal val text: Expression,
        internal val options: kotlin.Array<out FormatOption>?,
    )

    /**
     * Base class for an option entry that is encapsulated as a json object member for an expression.
     *
     * @constructor Create an option entry that is encapsulated as a json object member for an expression.
     * @param type  json object member name
     * @param value json object member value
     */
    private open class Option internal constructor(
        internal val type: String,
        internal val value: Expression,
    )

    /**
     * Holds format options used in a [numberFormat] expression.
     */
    @Suppress("EXPOSED_SUPER_CLASS")
    open class NumberFormatOption internal constructor(
        type: String,
        value: Expression,
    ) : Option(type, value) {
        companion object {
            /**
             * Number formatting option for specifying the locale to use, as a BCP 47 language tag.
             *
             * @param string the locale to use while performing number formatting
             * @return number format option
             */
            @JvmStatic
            fun locale(string: Expression): NumberFormatOption = NumberFormatOption("locale", string)

            /**
             * Number formatting option for specifying the locale to use, as a BCP 47 language tag.
             *
             * @param string the locale to use while performing number formatting
             * @return number format option
             */
            @JvmStatic
            fun locale(string: String): NumberFormatOption = NumberFormatOption("locale", literal(string))

            /**
             * Number formatting option for specifying the currency to use, an ISO 4217 code.
             *
             * @param string the currency to use while performing number formatting
             * @return number format option
             */
            @JvmStatic
            fun currency(string: Expression): NumberFormatOption = NumberFormatOption("currency", string)

            /**
             * Number formatting options for specifying the currency to use, an ISO 4217 code.
             *
             * @param string the currency to use while performing number formatting
             * @return number format option
             */
            @JvmStatic
            fun currency(string: String): NumberFormatOption = NumberFormatOption("currency", literal(string))

            /**
             * Number formatting options for specifying the minimum fraction digits to include.
             *
             * @param number the amount of minimum fraction digits to include
             * @return number format option
             */
            @JvmStatic
            fun minFractionDigits(number: Expression): NumberFormatOption = NumberFormatOption("min-fraction-digits", number)

            /**
             * Number formatting options for specifying the minimum fraction digits to include.
             *
             * @param number the amount of minimum fraction digits to include
             * @return number format option
             */
            @JvmStatic
            fun minFractionDigits(number: Int): NumberFormatOption = NumberFormatOption("min-fraction-digits", literal(number))

            /**
             * Number formatting options for specifying the maximum fraction digits to include.
             *
             * @param number the amount of minimum fraction digits to include
             * @return number format option
             */
            @JvmStatic
            fun maxFractionDigits(number: Expression): NumberFormatOption = NumberFormatOption("max-fraction-digits", number)

            /**
             * Number formatting options for specifying the maximum fraction digits to include.
             *
             * @param number the amount of minimum fraction digits to include
             * @return number format option
             */
            @JvmStatic
            fun maxFractionDigits(number: Int): NumberFormatOption = NumberFormatOption("max-fraction-digits", literal(number))
        }
    }

    /**
     * Holds format options used in a [formatEntry] that builds
     * a [format] expression.
     *
     * If an option is not set, it defaults to the base value defined for the symbol.
     */
    @Suppress("EXPOSED_SUPER_CLASS")
    open class FormatOption internal constructor(
        type: String,
        value: Expression,
    ) : Option(type, value) {
        companion object {
            /**
             * If set, the font-scale argument specifies a scaling factor relative to the text-size
             * specified in the root layout properties.
             *
             * "font-scale" is required to be of a resulting type number.
             *
             * @param expression expression
             * @return format option
             */
            @JvmStatic
            fun formatFontScale(expression: Expression): FormatOption = FormatOption("font-scale", expression)

            /**
             * If set, the font-scale argument specifies a scaling factor relative to the text-size
             * specified in the root layout properties.
             *
             * "font-scale" is required to be of a resulting type number.
             *
             * @param scale value
             * @return format option
             */
            @JvmStatic
            fun formatFontScale(scale: Double): FormatOption = FormatOption("font-scale", literal(scale))

            /**
             * If set, the text-font argument overrides the font specified by the root layout properties.
             *
             * "text-font" is required to be a literal array.
             *
             * The requested font stack has to be a part of the used style.
             * For more information see [the documentation](https://www.mapbox.com/help/define-font-stack/).
             *
             * @param expression expression
             * @return format option
             */
            @JvmStatic
            fun formatTextFont(expression: Expression): FormatOption = FormatOption("text-font", expression)

            /**
             * If set, the text-font argument overrides the font specified by the root layout properties.
             *
             * "text-font" is required to be a literal array.
             *
             * The requested font stack has to be a part of the used style.
             * For more information see [the documentation](https://www.mapbox.com/help/define-font-stack/).
             *
             * @param fontStack value
             * @return format option
             */
            @JvmStatic
            fun formatTextFont(fontStack: kotlin.Array<String>): FormatOption = FormatOption("text-font", literal(fontStack))

            /**
             * If set, the text-color argument overrides the color specified by the root paint properties.
             *
             * @param expression expression
             * @return format option
             */
            @JvmStatic
            fun formatTextColor(expression: Expression): FormatOption = FormatOption("text-color", expression)

            /**
             * If set, the text-color argument overrides the color specified by the root paint properties.
             *
             * @param color value
             * @return format option
             */
            @JvmStatic
            fun formatTextColor(
                @ColorInt color: Int,
            ): FormatOption = FormatOption("text-color", color(color))
        }
    }

    /**
     * Converts a JsonArray or a raw expression to a Java expression.
     */
    object Converter {
        private val gson = Gson()

        /**
         * Converts a JsonArray to an expression
         *
         * @param jsonArray the json array to convert
         * @return the expression
         */
        @JvmStatic
        fun convert(jsonArray: JsonArray): Expression {
            if (jsonArray.size() == 0) {
                throw IllegalArgumentException("Can't convert empty jsonArray expressions")
            }

            val operator = jsonArray.get(0).asString
            val arguments: MutableList<Expression> = ArrayList()
            if (operator == "within") {
                return within(Polygon.fromJson(jsonArray.get(1).toString()))
            } else if (operator == "distance") {
                return distance(GeometryGeoJson.fromJson(jsonArray.get(1).toString()))
            }
            for (i in 1 until jsonArray.size()) {
                val jsonElement = jsonArray.get(i)
                if (operator == "literal" && jsonElement is JsonArray) {
                    val array = arrayOfNulls<Any>(jsonElement.size())
                    for (j in 0 until jsonElement.size()) {
                        val element = jsonElement.get(j)
                        if (element is JsonPrimitive) {
                            array[j] = convertToValue(element)
                        } else {
                            throw IllegalArgumentException("Nested literal arrays are not supported.")
                        }
                    }

                    arguments.add(ExpressionLiteralArray(array))
                } else {
                    arguments.add(convert(jsonElement))
                }
            }
            return Expression(operator, *arguments.toTypedArray())
        }

        /**
         * Converts a JsonElement to an expression
         *
         * @param jsonElement the json element to convert
         * @return the expression
         */
        @JvmStatic
        fun convert(jsonElement: JsonElement): Expression =
            when (jsonElement) {
                is JsonArray -> {
                    convert(jsonElement)
                }

                is JsonPrimitive -> {
                    convert(jsonElement)
                }

                is JsonNull -> {
                    ExpressionLiteral("")
                }

                is JsonObject -> {
                    val map: MutableMap<String, Expression> = HashMap()
                    for (key in jsonElement.keySet()) {
                        map[key] = convert(jsonElement.get(key))
                    }
                    ExpressionMap(map)
                }

                else -> {
                    throw RuntimeException("Unsupported expression conversion for " + jsonElement.javaClass)
                }
            }

        /**
         * Converts a JsonPrimitive to an expression literal
         *
         * @param jsonPrimitive the json primitive to convert
         * @return the expression literal
         */
        private fun convert(jsonPrimitive: JsonPrimitive): Expression = ExpressionLiteral(convertToValue(jsonPrimitive))

        /**
         * Converts a JsonPrimitive to value
         *
         * @param jsonPrimitive the json primitive to convert
         * @return the value
         */
        private fun convertToValue(jsonPrimitive: JsonPrimitive): Any =
            when {
                jsonPrimitive.isBoolean -> jsonPrimitive.asBoolean

                jsonPrimitive.isNumber -> jsonPrimitive.asFloat

                jsonPrimitive.isString -> jsonPrimitive.asString

                else -> throw RuntimeException(
                    "Unsupported literal expression conversion for " + jsonPrimitive.javaClass,
                )
            }

        /**
         * Converts a raw expression to a DSL equivalent.
         *
         * @param rawExpression the raw expression to convert
         * @return the resulting expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/)
         */
        @JvmStatic
        fun convert(rawExpression: String): Expression = convert(gson.fromJson(rawExpression, JsonArray::class.java))
    }

    /**
     * Expression to wrap Object[] as a literal
     *
     * @constructor Create an expression literal.
     * @param object the object to be treated as literal
     */
    private class ExpressionLiteralArray(
        `object`: kotlin.Array<out Any?>,
    ) : ExpressionLiteral(`object`) {
        /**
         * Convert the expression array to a string representation.
         *
         * @return the string representation of the expression array
         */
        override fun toString(): String {
            val array = literal as kotlin.Array<*>
            val builder = StringBuilder("[")
            for (i in array.indices) {
                val argument = array[i]
                if (argument is String) {
                    builder.append("\"").append(argument).append("\"")
                } else {
                    builder.append(argument)
                }

                if (i != array.size - 1) {
                    builder.append(", ")
                }
            }
            builder.append("]")
            return builder.toString()
        }

        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }

            val that = other as ExpressionLiteralArray

            return Arrays.equals(this.literal as kotlin.Array<*>, that.literal as kotlin.Array<*>)
        }

        override fun hashCode(): Int = super.hashCode()
    }

    /**
     * Wraps an expression value stored in a Map.
     */
    @Suppress("EXPOSED_SUPER_INTERFACE")
    private class ExpressionMap(
        private val map: MutableMap<String, Expression>,
    ) : Expression(),
        ValueExpression {
        override fun toValue(): Any {
            val unwrappedMap: MutableMap<String, Any?> = HashMap()
            for (key in map.keys) {
                val expression = map[key]
                if (expression is ValueExpression) {
                    unwrappedMap[key] = expression.toValue()
                } else {
                    unwrappedMap[key] = expression!!.toArray()
                }
            }

            return unwrappedMap
        }

        override fun toString(): String {
            val builder = StringBuilder()
            builder.append("{")
            for (key in map.keys) {
                builder.append("\"").append(key).append("\": ")
                builder.append(map[key])
                builder.append(", ")
            }

            if (map.size > 0) {
                builder.delete(builder.length - 2, builder.length)
            }

            builder.append("}")
            return builder.toString()
        }

        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }
            if (!super.equals(other)) {
                return false
            }
            val that = other as ExpressionMap
            return map == that.map
        }

        override fun hashCode(): Int {
            var result = super.hashCode()
            result = 31 * result + map.hashCode()
            return result
        }
    }

    /**
     * Interface used to describe expressions that hold a Java value.
     */
    private interface ValueExpression {
        fun toValue(): Any
    }

    companion object {
        /**
         * Create a literal number expression.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(literal(10.0f))
         * )
         * ```
         *
         * @param number the number
         * @return the expression
         */
        @JvmStatic
        fun literal(number: Number): Expression = ExpressionLiteral(number)

        /**
         * Create a literal string expression.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(literal("Text"))
         * )
         * ```
         *
         * @param string the string
         * @return the expression
         */
        @JvmStatic
        fun literal(string: String): Expression = ExpressionLiteral(string)

        /**
         * Create a literal boolean expression.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillAntialias(literal(true))
         * )
         * ```
         *
         * @param bool the boolean
         * @return the expression
         */
        @JvmStatic
        fun literal(bool: Boolean): Expression = ExpressionLiteral(bool)

        /**
         * Create a literal object expression.
         *
         * @param object the object
         * @return the expression
         */
        @JvmStatic
        fun literal(`object`: Any): Expression {
            if (`object`.javaClass.isArray) {
                return literal(toObjectArray(`object`))
            } else if (`object` is Expression) {
                throw RuntimeException("Can't convert an expression to a literal")
            }
            return ExpressionLiteral(`object`)
        }

        /**
         * Create a literal array expression
         *
         * @param array the array
         * @return the expression
         */
        @JvmStatic
        fun literal(array: kotlin.Array<out Any?>): Expression = Expression("literal", ExpressionLiteralArray(array))

        /**
         * Expression literal utility method to convert a color int to an color expression
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(color(Color.GREEN))
         * )
         * ```
         *
         * @param color the int color
         * @return the color expression
         */
        @JvmStatic
        fun color(
            @ColorInt color: Int,
        ): Expression {
            val rgbaArray = colorToRgbaArray(color)
            return rgba(rgbaArray[0], rgbaArray[1], rgbaArray[2], rgbaArray[3])
        }

        /**
         * Creates a color value from red, green, and blue components, which must range between 0 and 255,
         * and an alpha component of 1.
         *
         * If any component is out of range, the expression is an error.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *         rgb(
         *             literal(255.0f),
         *             literal(255.0f),
         *             literal(255.0f)
         *         )
         *     )
         * )
         * ```
         *
         * @param red   red color expression
         * @param green green color expression
         * @param blue  blue color expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#rgb)
         */
        @JvmStatic
        fun rgb(
            red: Expression,
            green: Expression,
            blue: Expression,
        ): Expression = Expression("rgb", red, green, blue)

        /**
         * Creates a color value from red, green, and blue components, which must range between 0 and 255,
         * and an alpha component of 1.
         *
         * If any component is out of range, the expression is an error.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *         rgb(255.0f, 255.0f, 255.0f)
         *     )
         * )
         * ```
         *
         * @param red   red color value
         * @param green green color value
         * @param blue  blue color value
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#rgb)
         */
        @JvmStatic
        fun rgb(
            red: Number,
            green: Number,
            blue: Number,
        ): Expression = rgb(literal(red), literal(green), literal(blue))

        /**
         * Creates a color value from red, green, blue components, which must range between 0 and 255,
         * and an alpha component which must range between 0 and 1.
         *
         * If any component is out of range, the expression is an error.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *         rgba(
         *             literal(255.0f),
         *             literal(255.0f),
         *             literal(255.0f),
         *             literal(1.0f)
         *         )
         *     )
         * )
         * ```
         *
         * @param red   red color value
         * @param green green color value
         * @param blue  blue color value
         * @param alpha alpha color value
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#rgba)
         */
        @JvmStatic
        fun rgba(
            red: Expression,
            green: Expression,
            blue: Expression,
            alpha: Expression,
        ): Expression = Expression("rgba", red, green, blue, alpha)

        /**
         * Creates a color value from red, green, blue components, which must range between 0 and 255,
         * and an alpha component which must range between 0 and 1.
         *
         * If any component is out of range, the expression is an error.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *         rgba(255.0f, 255.0f, 255.0f, 1.0f)
         *     )
         * )
         * ```
         *
         * @param red   red color value
         * @param green green color value
         * @param blue  blue color value
         * @param alpha alpha color value
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#rgba)
         */
        @JvmStatic
        fun rgba(
            red: Number,
            green: Number,
            blue: Number,
            alpha: Number,
        ): Expression = rgba(literal(red), literal(green), literal(blue), literal(alpha))

        /**
         * Returns a four-element array containing the input color's red, green, blue, and alpha components, in that order.
         *
         * @param expression an expression to convert to a color
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#to-rgba)
         */
        @JvmStatic
        fun toRgba(expression: Expression): Expression = Expression("to-rgba", expression)

        /**
         * Returns true if the input values are equal, false otherwise.
         * The inputs must be numbers, strings, or booleans, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     eq(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#==)
         */
        @JvmStatic
        fun eq(
            compareOne: Expression,
            compareTwo: Expression,
        ): Expression = Expression("==", compareOne, compareTwo)

        /**
         * Returns true if the input values are equal, false otherwise.
         * The inputs must be numbers, strings, or booleans, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     eq(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#==)
         */
        @JvmStatic
        fun eq(
            compareOne: Expression,
            compareTwo: Expression,
            collator: Expression,
        ): Expression = Expression("==", compareOne, compareTwo, collator)

        /**
         * Returns true if the input values are equal, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     eq(get("keyToValue"), true)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second boolean
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#==)
         */
        @JvmStatic
        fun eq(
            compareOne: Expression,
            compareTwo: Boolean,
        ): Expression = eq(compareOne, literal(compareTwo))

        /**
         * Returns true if the input values are equal, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     eq(get("keyToValue"), "value")
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#==)
         */
        @JvmStatic
        fun eq(
            compareOne: Expression,
            compareTwo: String,
        ): Expression = eq(compareOne, literal(compareTwo))

        /**
         * Returns true if the input values are equal, false otherwise.
         * The inputs must be numbers, strings, or booleans, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     eq(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second String
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#==)
         */
        @JvmStatic
        fun eq(
            compareOne: Expression,
            compareTwo: String,
            collator: Expression,
        ): Expression = eq(compareOne, literal(compareTwo), collator)

        /**
         * Returns true if the input values are equal, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     eq(get("keyToValue"), 2.0f)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#==)
         */
        @JvmStatic
        fun eq(
            compareOne: Expression,
            compareTwo: Number,
        ): Expression = eq(compareOne, literal(compareTwo))

        /**
         * Returns true if the input values are not equal, false otherwise.
         * The inputs must be numbers, strings, or booleans, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!=)
         */
        @JvmStatic
        fun neq(
            compareOne: Expression,
            compareTwo: Expression,
        ): Expression = Expression("!=", compareOne, compareTwo)

        /**
         * Returns true if the input values are not equal, false otherwise.
         * The inputs must be numbers, strings, or booleans, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!=)
         */
        @JvmStatic
        fun neq(
            compareOne: Expression,
            compareTwo: Expression,
            collator: Expression,
        ): Expression = Expression("!=", compareOne, compareTwo, collator)

        /**
         * Returns true if the input values are equal, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), true)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second boolean
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!=)
         */
        @JvmStatic
        fun neq(
            compareOne: Expression,
            compareTwo: Boolean,
        ): Expression = Expression("!=", compareOne, literal(compareTwo))

        /**
         * Returns `true` if the input values are not equal, `false` otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), "value")
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!=)
         */
        @JvmStatic
        fun neq(
            compareOne: Expression,
            compareTwo: String,
        ): Expression = Expression("!=", compareOne, literal(compareTwo))

        /**
         * Returns true if the input values are not equal, false otherwise.
         * The inputs must be numbers, strings, or booleans, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second String
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!=)
         */
        @JvmStatic
        fun neq(
            compareOne: Expression,
            compareTwo: String,
            collator: Expression,
        ): Expression = Expression("!=", compareOne, literal(compareTwo), collator)

        /**
         * Returns `true` if the input values are not equal, `false` otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), 2.0f)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!=)
         */
        @JvmStatic
        fun neq(
            compareOne: Expression,
            compareTwo: Number,
        ): Expression = Expression("!=", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is strictly greater than the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gt(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E)
         */
        @JvmStatic
        fun gt(
            compareOne: Expression,
            compareTwo: Expression,
        ): Expression = Expression(">", compareOne, compareTwo)

        /**
         * Returns true if the first input is strictly greater than the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gt(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E)
         */
        @JvmStatic
        fun gt(
            compareOne: Expression,
            compareTwo: Expression,
            collator: Expression,
        ): Expression = Expression(">", compareOne, compareTwo, collator)

        /**
         * Returns true if the first input is strictly greater than the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gt(get("keyToValue"), 2.0f)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E)
         */
        @JvmStatic
        fun gt(
            compareOne: Expression,
            compareTwo: Number,
        ): Expression = Expression(">", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is strictly greater than the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gt(get("keyToValue"), "value")
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E)
         */
        @JvmStatic
        fun gt(
            compareOne: Expression,
            compareTwo: String,
        ): Expression = Expression(">", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is strictly greater than the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gt(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second String
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E)
         */
        @JvmStatic
        fun gt(
            compareOne: Expression,
            compareTwo: String,
            collator: Expression,
        ): Expression = Expression(">", compareOne, literal(compareTwo), collator)

        /**
         * Returns true if the first input is strictly less than the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lt(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C)
         */
        @JvmStatic
        fun lt(
            compareOne: Expression,
            compareTwo: Expression,
        ): Expression = Expression("<", compareOne, compareTwo)

        /**
         * Returns true if the first input is strictly less than the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lt(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C)
         */
        @JvmStatic
        fun lt(
            compareOne: Expression,
            compareTwo: Expression,
            collator: Expression,
        ): Expression = Expression("<", compareOne, compareTwo, collator)

        /**
         * Returns true if the first input is strictly less than the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lt(get("keyToValue"), 2.0f)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C)
         */
        @JvmStatic
        fun lt(
            compareOne: Expression,
            compareTwo: Number,
        ): Expression = Expression("<", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is strictly less than the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lt(get("keyToValue"), "value")
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C)
         */
        @JvmStatic
        fun lt(
            compareOne: Expression,
            compareTwo: String,
        ): Expression = Expression("<", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is strictly less than the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lt(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second String
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C)
         */
        @JvmStatic
        fun lt(
            compareOne: Expression,
            compareTwo: String,
            collator: Expression,
        ): Expression = Expression("<", compareOne, literal(compareTwo), collator)

        /**
         * Returns true if the first input is greater than or equal to the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gte(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E%3D)
         */
        @JvmStatic
        fun gte(
            compareOne: Expression,
            compareTwo: Expression,
        ): Expression = Expression(">=", compareOne, compareTwo)

        /**
         * Returns true if the first input is greater than or equal to the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gte(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E%3D)
         */
        @JvmStatic
        fun gte(
            compareOne: Expression,
            compareTwo: Expression,
            collator: Expression,
        ): Expression = Expression(">=", compareOne, compareTwo, collator)

        /**
         * Returns true if the first input is greater than or equal to the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gte(get("keyToValue"), 2.0f)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E%3D)
         */
        @JvmStatic
        fun gte(
            compareOne: Expression,
            compareTwo: Number,
        ): Expression = Expression(">=", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is greater than or equal to the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     neq(get("keyToValue"), "value")
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E%3D)
         */
        @JvmStatic
        fun gte(
            compareOne: Expression,
            compareTwo: String,
        ): Expression = Expression(">=", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is greater than or equal to the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     gte(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second String
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3E%3D)
         */
        @JvmStatic
        fun gte(
            compareOne: Expression,
            compareTwo: String,
            collator: Expression,
        ): Expression = Expression(">=", compareOne, literal(compareTwo), collator)

        /**
         * Returns true if the first input is less than or equal to the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lte(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C%3D)
         */
        @JvmStatic
        fun lte(
            compareOne: Expression,
            compareTwo: Expression,
        ): Expression = Expression("<=", compareOne, compareTwo)

        /**
         * Returns true if the first input is less than or equal to the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lte(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second expression
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C%3D)
         */
        @JvmStatic
        fun lte(
            compareOne: Expression,
            compareTwo: Expression,
            collator: Expression,
        ): Expression = Expression("<=", compareOne, compareTwo, collator)

        /**
         * Returns true if the first input is less than or equal to the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lte(get("keyToValue"), 2.0f)
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C%3D)
         */
        @JvmStatic
        fun lte(
            compareOne: Expression,
            compareTwo: Number,
        ): Expression = Expression("<=", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is less than or equal to the second, false otherwise.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lte(get("keyToValue"), "value")
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C%3D)
         */
        @JvmStatic
        fun lte(
            compareOne: Expression,
            compareTwo: String,
        ): Expression = Expression("<=", compareOne, literal(compareTwo))

        /**
         * Returns true if the first input is less than or equal to the second, false otherwise.
         * The inputs must be numbers or strings, and both of the same type.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     lte(get("keyToValue"), get("keyToOtherValue"), collator(true, false))
         * )
         * ```
         *
         * @param compareOne the first expression
         * @param compareTwo the second String
         * @param collator   the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%3C%3D)
         */
        @JvmStatic
        fun lte(
            compareOne: Expression,
            compareTwo: String,
            collator: Expression,
        ): Expression = Expression("<=", compareOne, literal(compareTwo), collator)

        /**
         * Returns `true` if all the inputs are `true`, `false` otherwise.
         *
         * The inputs are evaluated in order, and evaluation is short-circuiting:
         * once an input expression evaluates to `false`,
         * the result is `false` and no further input expressions are evaluated.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     all(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#all)
         */
        @JvmStatic
        fun all(vararg input: Expression): Expression = Expression("all", *input)

        /**
         * Returns `true` if any of the inputs are `true`, `false` otherwise.
         *
         * The inputs are evaluated in order, and evaluation is short-circuiting:
         * once an input expression evaluates to `true`,
         * the result is `true` and no further input expressions are evaluated.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     any(get("keyToValue"), get("keyToOtherValue"))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#any)
         */
        @JvmStatic
        fun any(vararg input: Expression): Expression = Expression("any", *input)

        /**
         * Logical negation. Returns `true` if the input is `false`, and `false` if the input is `true`.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     not(get("keyToValue"))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!)
         */
        @JvmStatic
        fun not(input: Expression): Expression = Expression("!", input)

        /**
         * Logical negation. Returns `true` if the input is `false`, and `false` if the input is `true`.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     not(false)
         * )
         * ```
         *
         * @param input boolean input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#!)
         */
        @JvmStatic
        fun not(input: Boolean): Expression = not(literal(input))

        /**
         * Selects the first output whose corresponding test condition evaluates to true.
         *
         * For each case a condition and an output should be provided.
         * The last parameter should provide the default output.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     iconSize(
         *         switchCase(
         *             get(KEY_TO_BOOLEAN), literal(3.0f),
         *             get(KEY_TO_OTHER_BOOLEAN), literal(5.0f),
         *             literal(1.0f) // default value
         *         )
         *     )
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#case)
         */
        @JvmStatic
        fun switchCase(
            @Size(min = 1) vararg input: Expression,
        ): Expression = Expression("case", *input)

        /**
         * Selects the output whose label value matches the input value, or the fallback value if no match is found.
         * The `input` can be any string or number expression.
         * Each label can either be a single literal value or an array of values.
         * If types of the input and keys don't match, or the input value doesn't exist,
         * the expresion will fail without falling back to the default value.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textColor(
         *         match(get("keyToValue"),
         *             literal(1), rgba(255, 0, 0, 1.0f),
         *             literal(2), rgba(0, 0, 255.0f, 1.0f),
         *             rgba(0.0f, 255.0f, 0.0f, 1.0f)
         *         )
         *     )
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#match)
         */
        @JvmStatic
        fun match(
            @Size(min = 2) vararg input: Expression,
        ): Expression = Expression("match", *input)

        /**
         * Selects the output whose label value matches the input value, or the fallback value if no match is found.
         * The `input` can be any string or number expression.
         * Each label can either be a single literal value or an array of values.
         * If types of the input and keys don't match, or the input value doesn't exist,
         * the expresion will fail without falling back to the default value.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *   textColor(
         *     match(get("keyToValue"), rgba(0.0f, 255.0f, 0.0f, 1.0f),
         *       stop(1f, rgba(255, 0, 0, 1.0f)),
         *       stop(2f, rgba(0, 0, 255.0f, 1.0f))
         *     )
         *   )
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#match)
         */
        @JvmStatic
        fun match(
            input: Expression,
            defaultOutput: Expression,
            vararg stops: Stop,
        ): Expression = match(*join(join(arrayOf(input), Stop.toExpressionArray(*stops)), arrayOf(defaultOutput)))

        /**
         * Evaluates each expression in turn until the first non-null value is obtained, and returns that value.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textColor(
         *         coalesce(
         *             get("keyToNullValue"),
         *             get("keyToNonNullValue")
         *         )
         *     )
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#coalesce)
         */
        @JvmStatic
        fun coalesce(vararg input: Expression): Expression = Expression("coalesce", *input)

        /**
         * Gets the feature properties object.
         *
         * Note that in some cases, it may be more efficient to use [get]} instead.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(get("key-to-value", properties()))
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#properties)
         */
        @JvmStatic
        fun properties(): Expression = Expression("properties")

        /**
         * Gets the feature's geometry type: Point, MultiPoint, LineString, MultiLineString, Polygon, MultiPolygon.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(concat(get("key-to-value"), literal(" "), geometryType()))
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#geometry-types)
         */
        @JvmStatic
        fun geometryType(): Expression = Expression("geometry-type")

        /**
         * Gets the feature's id, if it has one.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(id())
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#id)
         */
        @JvmStatic
        fun id(): Expression = Expression("id")

        /**
         * Gets a value from feature state.
         *
         * @param property the state property to read
         * @return the expression
         */
        @JvmStatic
        fun featureState(property: String): Expression = Expression("feature-state", literal(property))

        /**
         * Gets the value of a cluster property accumulated so far. Can only be used in the clusterProperties
         * option of a clustered GeoJSON source.
         *
         * Example usage:
         *
         * ```kotlin
         *  val options = GeoJsonOptions()
         *                              .withCluster(true)
         *                              .withClusterProperty("max", max(accumulated(), get("max")).toArray(), get("mag").toArray())
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#accumulated)
         */
        @JvmStatic
        fun accumulated(): Expression = Expression("accumulated")

        /**
         * Gets the kernel density estimation of a pixel in a heatmap layer,
         * which is a relative measure of how many data points are crowded around a particular pixel.
         * Can only be used in the `heatmap-color` property.
         *
         * Example usage:
         *
         * ```kotlin
         * val layer = HeatmapLayer("layer-id", "source-id")
         * layer.setProperties(
         *     heatmapColor(interpolate(linear(), heatmapDensity(),
         *         literal(0), rgba(33, 102, 172, 0),
         *         literal(0.2), rgb(103, 169, 207),
         *         literal(0.4), rgb(209, 229, 240),
         *         literal(0.6), rgb(253, 219, 199),
         *         literal(0.8), rgb(239, 138, 98),
         *         literal(1), rgb(178, 24, 43))
         *     )
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#heatmap-density)
         */
        @JvmStatic
        fun heatmapDensity(): Expression = Expression("heatmap-density")

        /**
         * Gets the progress along a gradient line. Can only be used in the line-gradient property.
         *
         * Example usage:
         *
         * ```kotlin
         * val layer = LineLayer("layer-id", "source-id")
         * layer.setProperties(
         *     lineGradient(interpolate(
         *         linear(), lineProgress(),
         *         stop(0f, rgb(0, 0, 255)),
         *         stop(0.5f, rgb(0, 255, 0)),
         *         stop(1f, rgb(255, 0, 0)))
         *     )
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#line-progress)
         */
        @JvmStatic
        fun lineProgress(): Expression = Expression("line-progress")

        /**
         * Retrieves an item from an array.
         *
         * @param number     the index expression
         * @param expression the array expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#at)
         */
        @JvmStatic
        fun at(
            number: Expression,
            expression: Expression,
        ): Expression = Expression("at", number, expression)

        /**
         * Retrieves an item from an array.
         *
         * @param number     the index expression
         * @param expression the array expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#at)
         */
        @JvmStatic
        fun at(
            number: Number,
            expression: Expression,
        ): Expression = at(literal(number), expression)

        /**
         * Retrieves whether an item exists in an array or a substring exists in a string.
         *
         * @param needle   the item expression
         * @param haystack the array or string expression
         * @return true if exists.
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#in)
         */
        @JvmStatic
        fun `in`(
            needle: Expression,
            haystack: Expression,
        ): Expression = Expression("in", needle, haystack)

        /**
         * Returns the first position at which a `needle` can be found in a `haystack`.
         *
         * @param needle   the item expression
         * @param haystack the array or string expression
         * @return position in the array or string or -1 if not found.
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#index-of)
         */
        @JvmStatic
        fun indexOf(
            keyword: Expression,
            input: Expression,
        ): Expression = Expression("index-of", keyword, input)

        /**
         * Returns the first position at which a `needle` can be found in a `haystack`.
         *
         * @param needle   the item expression
         * @param haystack the array or string expression
         * @param fromIndex the index to start searching from
         * @return position in the array or string or -1 if not found.
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#index-of)
         */
        @JvmStatic
        fun indexOf(
            keyword: Expression,
            input: Expression,
            fromIndex: Expression,
        ): Expression = Expression("index-of", keyword, input, fromIndex)

        /**
         * Returns items from an array or a substring from a string from a specified start index.
         * The return value is inclusive of the start index.
         *
         * @param input the array or string expression
         * @param fromIndex the index to start slice from
         * @return array or string
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#slice)
         */
        @JvmStatic
        fun slice(
            input: Expression,
            fromIndex: Expression,
        ): Expression = Expression("slice", input, fromIndex)

        /**
         * Returns items from an array or a substring from a string between a start index and an end index if set.
         * The return value is inclusive of the start index, but not of the end index.
         *
         * @param input the array or string expression
         * @param fromIndex the index to start slice from
         * @param toIndex the index to end slice at
         * @return array or string
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#slice)
         */
        @JvmStatic
        fun slice(
            input: Expression,
            fromIndex: Expression,
            toIndex: Expression,
        ): Expression = Expression("slice", input, fromIndex, toIndex)

        /**
         * Retrieves whether an item exists in an array or a substring exists in a string.
         *
         * @param needle   the item expression
         * @param haystack the array or string expression
         * @return true if exists.
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#in)
         */
        @JvmStatic
        fun `in`(
            needle: Number,
            haystack: Expression,
        ): Expression = Expression("in", literal(needle), haystack)

        /**
         * Retrieves whether an item exists in an array or a substring exists in a string.
         *
         * @param needle   the item expression
         * @param haystack the array or string expression
         * @return true if exists.
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#in)
         */
        @JvmStatic
        fun `in`(
            needle: String,
            haystack: Expression,
        ): Expression = Expression("in", literal(needle), haystack)

        /**
         * Retrieves the shortest distance between two geometries.
         * The returned value can be consumed as an input into another expression for changing a paint or layout property
         * or filtering features by distance.
         *
         * Currently supports `Point`, `MultiPoint`, `LineString`, `MultiLineString` geometry types.
         *
         * @param geoJson the target feature geoJson.
         *                Currently supports `Point`, `MultiPoint`, `LineString`, `MultiLineString`, `Polygon`, `MultiPolygon`
         *                geometry types
         * @return the distance in the unit "meters".
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#distance)
         */
        @JvmStatic
        fun distance(geoJson: GeoJson): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            map["json"] = literal(geoJson.toJson())
            return Expression("distance", ExpressionMap(map))
        }

        @JvmStatic
        fun within(polygon: Polygon): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            map["type"] = literal(polygon.type())
            map["json"] = literal(polygon.toJson())
            return Expression("within", ExpressionMap(map))
        }

        /**
         * Retrieves a property value from the current feature's properties,
         * or from another object if a second argument is provided.
         * Returns null if the requested property is missing.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(get("key-to-feature"))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#get)
         */
        @JvmStatic
        fun get(input: Expression): Expression = Expression("get", input)

        /**
         * Retrieves a property value from the current feature's properties,
         * or from another object if a second argument is provided.
         * Returns null if the requested property is missing.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(get("key-to-feature"))
         * )
         * ```
         *
         * @param input string input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#get)
         */
        @JvmStatic
        fun get(input: String): Expression = get(literal(input))

        /**
         * Retrieves a property value from another object.
         * Returns null if the requested property is missing.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(get("key-to-property", get("key-to-object")))
         * )
         * ```
         *
         * @param key    a property value key
         * @param object an expression object
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#get)
         */
        @JvmStatic
        fun get(
            key: Expression,
            `object`: Expression,
        ): Expression = Expression("get", key, `object`)

        /**
         * Retrieves a property value from another object.
         * Returns null if the requested property is missing.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(get("key-to-property", get("key-to-object")))
         * )
         * ```
         *
         * @param key    a property value key
         * @param object an expression object
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#get)
         */
        @JvmStatic
        fun get(
            key: String,
            `object`: Expression,
        ): Expression = get(literal(key), `object`)

        /**
         * Tests for the presence of an property value in the current feature's properties.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     has(get("keyToValue"))
         * )
         * ```
         *
         * @param key the expression property value key
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#has)
         */
        @JvmStatic
        fun has(key: Expression): Expression = Expression("has", key)

        /**
         * Tests for the presence of an property value in the current feature's properties.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     has("keyToValue")
         * )
         * ```
         *
         * @param key the property value key
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#has)
         */
        @JvmStatic
        fun has(key: String): Expression = has(literal(key))

        /**
         * Tests for the presence of an property value from another object.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     has(get("keyToValue"), get("keyToObject"))
         * )
         * ```
         *
         * @param key    the expression property value key
         * @param object an expression object
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#has)
         */
        @JvmStatic
        fun has(
            key: Expression,
            `object`: Expression,
        ): Expression = Expression("has", key, `object`)

        /**
         * Tests for the presence of an property value from another object.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setFilter(
         *     has("keyToValue", get("keyToObject"))
         * )
         * ```
         *
         * @param key    the property value key
         * @param object an expression object
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#has)
         */
        @JvmStatic
        fun has(
            key: String,
            `object`: Expression,
        ): Expression = has(literal(key), `object`)

        /**
         * Gets the length of an array or string.
         *
         * @param expression an expression object or expression string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#lenght)
         */
        @JvmStatic
        fun length(expression: Expression): Expression = Expression("length", expression)

        /**
         * Gets the length of an array or string.
         *
         * @param input a string
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#lenght)
         */
        @JvmStatic
        fun length(input: String): Expression = length(literal(input))

        /**
         * Returns mathematical constant ln(2).
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(product(literal(10.0f), ln2()))
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#ln2)
         */
        @JvmStatic
        fun ln2(): Expression = Expression("ln2")

        /**
         * Returns the mathematical constant pi.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(product(literal(10.0f), pi()))
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#pi)
         */
        @JvmStatic
        fun pi(): Expression = Expression("pi")

        /**
         * Returns the mathematical constant e.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(product(literal(10.0f), e()))
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#e)
         */
        @JvmStatic
        fun e(): Expression = Expression("e")

        /**
         * Returns the sum of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(sum(literal(10.0f), ln2(), pi()))
         * )
         * ```
         *
         * @param numbers the numbers to calculate the sum for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#+)
         */
        @JvmStatic
        fun sum(
            @Size(min = 2) vararg numbers: Expression,
        ): Expression = Expression("+", *numbers)

        /**
         * Returns the sum of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(sum(10.0f, 5.0f, 3.0f))
         * )
         * ```
         *
         * @param numbers the numbers to calculate the sum for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#+)
         */
        @SuppressLint("Range")
        @JvmStatic
        fun sum(
            @Size(min = 2) vararg numbers: Number,
        ): Expression = sum(*numbers.map { literal(it) }.toTypedArray())

        /**
         * Returns the product of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(product(literal(10.0f), ln2()))
         * )
         * ```
         *
         * @param numbers the numbers to calculate the product for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#*)
         */
        @JvmStatic
        fun product(
            @Size(min = 2) vararg numbers: Expression,
        ): Expression = Expression("*", *numbers)

        /**
         * Returns the product of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(product(10.0f, 2.0f))
         * )
         * ```
         *
         * @param numbers the numbers to calculate the product for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#*)
         */
        @SuppressLint("Range")
        @JvmStatic
        fun product(
            @Size(min = 2) vararg numbers: Number,
        ): Expression = product(*numbers.map { literal(it) }.toTypedArray())

        /**
         * Returns the result of subtracting a number from 0.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(subtract(pi()))
         * )
         * ```
         *
         * @param number the number subtract from 0
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#-)
         */
        @JvmStatic
        fun subtract(number: Expression): Expression = Expression("-", number)

        /**
         * Returns the result of subtracting a number from 0.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(subtract(10.0f))
         * )
         * ```
         *
         * @param number the number subtract from 0
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#-)
         */
        @JvmStatic
        fun subtract(number: Number): Expression = subtract(literal(number))

        /**
         * Returns the result of subtracting the second input from the first.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(subtract(literal(10.0f), pi()))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#-)
         */
        @JvmStatic
        fun subtract(
            first: Expression,
            second: Expression,
        ): Expression = Expression("-", first, second)

        /**
         * Returns the result of subtracting the second input from the first.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(subtract(10.0f, 20.0f))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#-)
         */
        @JvmStatic
        fun subtract(
            first: Number,
            second: Number,
        ): Expression = subtract(literal(first), literal(second))

        /**
         * Returns the result of floating point division of the first input by the second.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(division(literal(10.0f), pi()))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#/)
         */
        @JvmStatic
        fun division(
            first: Expression,
            second: Expression,
        ): Expression = Expression("/", first, second)

        /**
         * Returns the result of floating point division of the first input by the second.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(division(10.0f, 20.0f))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#/)
         */
        @JvmStatic
        fun division(
            first: Number,
            second: Number,
        ): Expression = division(literal(first), literal(second))

        /**
         * Returns the remainder after integer division of the first input by the second.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(mod(literal(10.0f), pi()))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%25)
         */
        @JvmStatic
        fun mod(
            first: Expression,
            second: Expression,
        ): Expression = Expression("%", first, second)

        /**
         * Returns the remainder after integer division of the first input by the second.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(mod(10.0f, 10.0f))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%25)
         */
        @JvmStatic
        fun mod(
            first: Number,
            second: Number,
        ): Expression = mod(literal(first), literal(second))

        /**
         * Returns the result of raising the first input to the power specified by the second.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(pow(pi(), literal(2.0f)))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%5E)
         */
        @JvmStatic
        fun pow(
            first: Expression,
            second: Expression,
        ): Expression = Expression("^", first, second)

        /**
         * Returns the result of raising the first input to the power specified by the second.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(pow(5.0f, 2.0f))
         * )
         * ```
         *
         * @param first  the first number
         * @param second the second number
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#%5E)
         */
        @JvmStatic
        fun pow(
            first: Number,
            second: Number,
        ): Expression = pow(literal(first), literal(second))

        /**
         * Returns the square root of the input
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(sqrt(pi()))
         * )
         * ```
         *
         * @param number the number to take the square root from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#sqrt)
         */
        @JvmStatic
        fun sqrt(number: Expression): Expression = Expression("sqrt", number)

        /**
         * Returns the square root of the input
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(sqrt(25.0f))
         * )
         * ```
         *
         * @param number the number to take the square root from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#sqrt)
         */
        @JvmStatic
        fun sqrt(number: Number): Expression = sqrt(literal(number))

        /**
         * Returns the base-ten logarithm of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(log10(pi()))
         * )
         * ```
         *
         * @param number the number to take base-ten logarithm from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#log10)
         */
        @JvmStatic
        fun log10(number: Expression): Expression = Expression("log10", number)

        /**
         * Returns the base-ten logarithm of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(log10(10))
         * )
         * ```
         *
         * @param number the number to take base-ten logarithm from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#log10)
         */
        @JvmStatic
        fun log10(number: Number): Expression = log10(literal(number))

        /**
         * Returns the natural logarithm of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(ln(pi()))
         * )
         * ```
         *
         * @param number the number to take natural logarithm from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#ln)
         */
        @JvmStatic
        fun ln(number: Expression): Expression = Expression("ln", number)

        /**
         * Returns the natural logarithm of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(ln(10))
         * )
         * ```
         *
         * @param number the number to take natural logarithm from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#ln)
         */
        @JvmStatic
        fun ln(number: Number): Expression = ln(literal(number))

        /**
         * Returns the base-two logarithm of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(log2(pi()))
         * )
         * ```
         *
         * @param number the number to take base-two logarithm from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#log2)
         */
        @JvmStatic
        fun log2(number: Expression): Expression = Expression("log2", number)

        /**
         * Returns the base-two logarithm of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(log2(2))
         * )
         * ```
         *
         * @param number the number to take base-two logarithm from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#log2)
         */
        @JvmStatic
        fun log2(number: Number): Expression = log2(literal(number))

        /**
         * Returns the sine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(sin(pi()))
         * )
         * ```
         *
         * @param number the number to calculate the sine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#sin)
         */
        @JvmStatic
        fun sin(number: Expression): Expression = Expression("sin", number)

        /**
         * Returns the sine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(sin(90.0f))
         * )
         * ```
         *
         * @param number the number to calculate the sine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#sin)
         */
        @JvmStatic
        fun sin(number: Number): Expression = sin(literal(number))

        /**
         * Returns the cosine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(cos(pi()))
         * )
         * ```
         *
         * @param number the number to calculate the cosine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#cos)
         */
        @JvmStatic
        fun cos(number: Expression): Expression = Expression("cos", number)

        /**
         * Returns the cosine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(cos(0))
         * )
         * ```
         *
         * @param number the number to calculate the cosine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#cos)
         */
        @JvmStatic
        fun cos(number: Number): Expression = Expression("cos", literal(number))

        /**
         * Returns the tangent of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(tan(pi()))
         * )
         * ```
         *
         * @param number the number to calculate the tangent for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#tan)
         */
        @JvmStatic
        fun tan(number: Expression): Expression = Expression("tan", number)

        /**
         * Returns the tangent of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(tan(45.0f))
         * )
         * ```
         *
         * @param number the number to calculate the tangent for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#tan)
         */
        @JvmStatic
        fun tan(number: Number): Expression = Expression("tan", literal(number))

        /**
         * Returns the arcsine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(asin(pi()))
         * )
         * ```
         *
         * @param number the number to calculate the arcsine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#asin)
         */
        @JvmStatic
        fun asin(number: Expression): Expression = Expression("asin", number)

        /**
         * Returns the arcsine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(asin(90))
         * )
         * ```
         *
         * @param number the number to calculate the arcsine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#asin)
         */
        @JvmStatic
        fun asin(number: Number): Expression = asin(literal(number))

        /**
         * Returns the arccosine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(acos(pi()))
         * )
         * ```
         *
         * @param number the number to calculate the arccosine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#acos)
         */
        @JvmStatic
        fun acos(number: Expression): Expression = Expression("acos", number)

        /**
         * Returns the arccosine of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(acos(0))
         * )
         * ```
         *
         * @param number the number to calculate the arccosine for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#acos)
         */
        @JvmStatic
        fun acos(number: Number): Expression = acos(literal(number))

        /**
         * Returns the arctangent of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(asin(pi()))
         * )
         * ```
         *
         * @param number the number to calculate the arctangent for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#atan)
         */
        @JvmStatic
        fun atan(number: Expression): Expression = Expression("atan", number)

        /**
         * Returns the arctangent of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(atan(90))
         * )
         * ```
         *
         * @param number the number to calculate the arctangent for
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#atan)
         */
        @JvmStatic
        fun atan(number: Number): Expression = atan(literal(number))

        /**
         * Returns the minimum value of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(min(pi(), literal(3.14f), literal(3.15f)))
         * )
         * ```
         *
         * @param numbers varargs of numbers to get the minimum from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#min)
         */
        @JvmStatic
        fun min(
            @Size(min = 1) vararg numbers: Expression,
        ): Expression = Expression("min", *numbers)

        /**
         * Returns the minimum value of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(min(3.141, 3.14f, 3.15f))
         * )
         * ```
         *
         * @param numbers varargs of numbers to get the minimum from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#min)
         */
        @SuppressLint("Range")
        @JvmStatic
        fun min(
            @Size(min = 1) vararg numbers: Number,
        ): Expression = min(*numbers.map { literal(it) }.toTypedArray())

        /**
         * Returns the maximum value of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(max(pi(), product(pi(), pi())))
         * )
         * ```
         *
         * @param numbers varargs of numbers to get the maximum from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#max)
         */
        @JvmStatic
        fun max(
            @Size(min = 1) vararg numbers: Expression,
        ): Expression = Expression("max", *numbers)

        /**
         * Returns the maximum value of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(max(3.141, 3.14f, 3.15f))
         * )
         * ```
         *
         * @param numbers varargs of numbers to get the maximum from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#max)
         */
        @SuppressLint("Range")
        @JvmStatic
        fun max(
            @Size(min = 1) vararg numbers: Number,
        ): Expression = max(*numbers.map { literal(it) }.toTypedArray())

        /**
         * Rounds the input to the nearest integer.
         * Halfway values are rounded away from zero.
         * For example `[\"round\", -1.5]` evaluates to -2.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(round(pi()))
         * )
         * ```
         *
         * @param expression number expression to round
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#round)
         */
        @JvmStatic
        fun round(expression: Expression): Expression = Expression("round", expression)

        /**
         * Rounds the input to the nearest integer.
         * Halfway values are rounded away from zero.
         * For example `[\"round\", -1.5]` evaluates to -2.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(round(3.14159265359f))
         * )
         * ```
         *
         * @param number number to round
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#round)
         */
        @JvmStatic
        fun round(number: Number): Expression = round(literal(number))

        /**
         * Returns the absolute value of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(abs(subtract(pi())))
         * )
         * ```
         *
         * @param expression number expression to get absolute value from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#abs)
         */
        @JvmStatic
        fun abs(expression: Expression): Expression = Expression("abs", expression)

        /**
         * Returns the absolute value of the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(abs(-3.14159265359f))
         * )
         * ```
         *
         * @param number number to get absolute value from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#abs)
         */
        @JvmStatic
        fun abs(number: Number): Expression = abs(literal(number))

        /**
         * Returns the smallest integer that is greater than or equal to the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(ceil(pi()))
         * )
         * ```
         *
         * @param expression number expression to get value from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#abs)
         */
        @JvmStatic
        fun ceil(expression: Expression): Expression = Expression("ceil", expression)

        /**
         * Returns the smallest integer that is greater than or equal to the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(ceil(3.14159265359))
         * )
         * ```
         *
         * @param number number to get value from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#abs)
         */
        @JvmStatic
        fun ceil(number: Number): Expression = ceil(literal(number))

        /**
         * Returns the largest integer that is less than or equal to the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(floor(pi()))
         * )
         * ```
         *
         * @param expression number expression to get value from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#abs)
         */
        @JvmStatic
        fun floor(expression: Expression): Expression = Expression("floor", expression)

        /**
         * Returns the largest integer that is less than or equal to the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(floor(pi()))
         * )
         * ```
         *
         * @param number number to get value from
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#abs)
         */
        @JvmStatic
        fun floor(number: Number): Expression = floor(literal(number))

        /**
         * Returns the IETF language tag of the locale being used by the provided collator.
         * This can be used to determine the default system locale,
         * or to determine if a requested locale was successfully loaded.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         * circleColor(switchCase(
         * eq(literal("it"), resolvedLocale(collator(true, true, Locale.ITALY))), literal(ColorUtils.colorToRgbaString
         * (Color.GREEN)),
         * literal(ColorUtils.colorToRgbaString(Color.RED))))
         * )
         * ```
         *
         * @param collator the collator expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#resolved-locale)
         */
        @JvmStatic
        fun resolvedLocale(collator: Expression): Expression = Expression("resolved-locale", collator)

        /**
         * Returns true if the input string is expected to render legibly.
         * Returns false if the input string contains sections that cannot be rendered without potential loss of meaning
         * (e.g. Indic scripts that require complex text shaping,
         * or right-to-left scripts if the the mapbox-gl-rtl-text plugin is not in use in MapLibre GL JS).
         *
         * Example usage:
         *
         * ```kotlin
         * maplibreMap.style!!.addLayer(SymbolLayer("layer-id", "source-id")
         *   .withProperties(
         *     textField(
         *       switchCase(
         *         isSupportedScript(get("name_property")), get("name_property"),
         *         literal("not-compatible")
         *       )
         *     )
         *   ))
         * ```
         *
         * @param expression the expression to evaluate
         * @return expression
         * @see <a href="https://maplibre.org/maplibre-style-spec/expressions/#is-supported-script">Style
         * specification</a>
         */
        @JvmStatic
        fun isSupportedScript(expression: Expression): Expression = Expression("is-supported-script", expression)

        /**
         * Returns true if the input string is expected to render legibly.
         * Returns false if the input string contains sections that cannot be rendered without potential loss of meaning
         * (e.g. Indic scripts that require complex text shaping,
         * or right-to-left scripts if the the mapbox-gl-rtl-text plugin is not in use in MapLibre GL JS).
         *
         * Example usage:
         *
         * ```kotlin
         * maplibreMap.style!!.addLayer(SymbolLayer("layer-id", "source-id")
         * .withProperties(
         *   textField(
         *     switchCase(
         *       isSupportedScript("ಗೌರವಾರ್ಥವಾಗಿ"), literal("ಗೌರವಾರ್ಥವಾಗಿ"),
         *       literal("not-compatible"))
         *     )
         *   )
         * )
         * ```
         *
         * @param string the string to evaluate
         * @return expression
         * @see <a href="https://maplibre.org/maplibre-style-spec/expressions/#is-supported-script">Style
         * specification</a>
         */
        @JvmStatic
        fun isSupportedScript(string: String): Expression = Expression("is-supported-script", literal(string))

        /**
         * Returns the input string converted to uppercase.
         *
         * Follows the Unicode Default Case Conversion algorithm
         * and the locale-insensitive case mappings in the Unicode Character Database.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(upcase(get("key-to-string-value")))
         * )
         * ```
         *
         * @param string the string to upcase
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#upcase)
         */
        @JvmStatic
        fun upcase(string: Expression): Expression = Expression("upcase", string)

        /**
         * Returns the input string converted to uppercase.
         *
         * Follows the Unicode Default Case Conversion algorithm
         * and the locale-insensitive case mappings in the Unicode Character Database.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(upcase("text"))
         * )
         * ```
         *
         * @param string string to upcase
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#upcase)
         */
        @JvmStatic
        fun upcase(string: String): Expression = upcase(literal(string))

        /**
         * Returns the input string converted to lowercase.
         *
         * Follows the Unicode Default Case Conversion algorithm
         * and the locale-insensitive case mappings in the Unicode Character Database.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(downcase(get("key-to-string-value")))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#downcase)
         */
        @JvmStatic
        fun downcase(input: Expression): Expression = Expression("downcase", input)

        /**
         * Returns the input string converted to lowercase.
         *
         * Follows the Unicode Default Case Conversion algorithm
         * and the locale-insensitive case mappings in the Unicode Character Database.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(upcase("key-to-string-value"))
         * )
         * ```
         *
         * @param input string to downcase
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#downcase)
         */
        @JvmStatic
        fun downcase(input: String): Expression = downcase(literal(input))

        /**
         * Returns a string consisting of the concatenation of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(concat(get("key-to-string-value"), literal("other string")))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#concat)
         */
        @JvmStatic
        fun concat(vararg input: Expression): Expression = Expression("concat", *input)

        /**
         * Returns a string consisting of the concatenation of the inputs.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(concat("foo", "bar"))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#concat)
         */
        @JvmStatic
        fun concat(vararg input: String): Expression = concat(*input.map { literal(it) }.toTypedArray())

        /**
         * Asserts that the input is an array (optionally with a specific item type and length).
         * If, when the input expression is evaluated, it is not of the asserted type,
         * then this assertion will cause the whole expression to be aborted.
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-array)
         */
        @JvmStatic
        fun array(input: Expression): Expression = Expression("array", input)

        /**
         * Returns a string describing the type of the given value.
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-typeof)
         */
        @JvmStatic
        fun typeOf(input: Expression): Expression = Expression("typeof", input)

        /**
         * Asserts that the input value is a string.
         * If multiple values are provided, each one is evaluated in order until a string value is obtained.
         * If none of the inputs are strings, the expression is an error.
         * The asserted input value is returned as result.
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-string)
         */
        @JvmStatic
        fun string(vararg input: Expression): Expression = Expression("string", *input)

        /**
         * Asserts that the input value is a number.
         * If multiple values are provided, each one is evaluated in order until a number value is obtained.
         * If none of the inputs are numbers, the expression is an error.
         * The asserted input value is returned as result.
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-number)
         */
        @JvmStatic
        fun number(vararg input: Expression): Expression = Expression("number", *input)

        /**
         * Converts the input number into a string representation using the providing formatting rules.
         * If set, the locale argument specifies the locale to use, as a BCP 47 language tag.
         * If set, the currency argument specifies an ISO 4217 code to use for currency-style formatting.
         * If set, the min-fraction-digits and max-fraction-digits arguments specify the minimum and maximum number
         * of fractional digits to include.
         *
         * @param number  number expression
         * @param options number formatting options
         * @return expression
         */
        @JvmStatic
        fun numberFormat(
            number: Expression,
            vararg options: NumberFormatOption,
        ): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            for (option in options) {
                map[option.type] = option.value
            }
            return Expression("number-format", number, ExpressionMap(map))
        }

        /**
         * Converts the input number into a string representation using the providing formatting rules.
         * If set, the locale argument specifies the locale to use, as a BCP 47 language tag.
         * If set, the currency argument specifies an ISO 4217 code to use for currency-style formatting.
         * If set, the min-fraction-digits and max-fraction-digits arguments specify the minimum and maximum number
         * of fractional digits to include.
         *
         * @param number  number expression
         * @param options number formatting options
         * @return expression
         */
        @JvmStatic
        fun numberFormat(
            number: Number,
            vararg options: NumberFormatOption,
        ): Expression = numberFormat(literal(number), *options)

        /**
         * Asserts that the input value is a boolean.
         * If multiple values are provided, each one is evaluated in order until a boolean value is obtained.
         * If none of the inputs are booleans, the expression is an error.
         * The asserted input value is returned as result.
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-boolean)
         */
        @JvmStatic
        fun bool(vararg input: Expression): Expression = Expression("boolean", *input)

        /**
         * Returns a collator for use in locale-dependent comparison operations.
         * The case-sensitive and diacritic-sensitive options default to false.
         * The locale argument specifies the IETF language tag of the locale to use.
         * If none is provided, the default locale is used. If the requested locale is not available,
         * the collator will use a system-defined fallback locale.
         * Use resolved-locale to test the results of locale fallback behavior.
         *
         * @param caseSensitive      case sensitive flag
         * @param diacriticSensitive diacritic sensitive flag
         * @param locale             locale
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-collator)
         */
        @JvmStatic
        fun collator(
            caseSensitive: Boolean,
            diacriticSensitive: Boolean,
            locale: Locale,
        ): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            map["case-sensitive"] = literal(caseSensitive)
            map["diacritic-sensitive"] = literal(diacriticSensitive)
            val localeStringBuilder = StringBuilder()
            val language = locale.language
            if (!language.isNullOrEmpty()) {
                localeStringBuilder.append(language)
            }
            val country = locale.country
            if (!country.isNullOrEmpty()) {
                localeStringBuilder.append("-")
                localeStringBuilder.append(country)
            }
            map["locale"] = literal(localeStringBuilder.toString())
            return Expression("collator", ExpressionMap(map))
        }

        /**
         * Returns a collator for use in locale-dependent comparison operations.
         * The case-sensitive and diacritic-sensitive options default to false.
         * The locale argument specifies the IETF language tag of the locale to use.
         * If none is provided, the default locale is used. If the requested locale is not available,
         * the collator will use a system-defined fallback locale.
         * Use resolved-locale to test the results of locale fallback behavior.
         *
         * @param caseSensitive      case sensitive flag
         * @param diacriticSensitive diacritic sensitive flag
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-collator)
         */
        @JvmStatic
        fun collator(
            caseSensitive: Boolean,
            diacriticSensitive: Boolean,
        ): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            map["case-sensitive"] = literal(caseSensitive)
            map["diacritic-sensitive"] = literal(diacriticSensitive)
            return Expression("collator", ExpressionMap(map))
        }

        /**
         * Returns a collator for use in locale-dependent comparison operations.
         * The case-sensitive and diacritic-sensitive options default to false.
         * The locale argument specifies the IETF language tag of the locale to use.
         * If none is provided, the default locale is used. If the requested locale is not available,
         * the collator will use a system-defined fallback locale.
         * Use resolved-locale to test the results of locale fallback behavior.
         *
         * @param caseSensitive      case sensitive flag
         * @param diacriticSensitive diacritic sensitive flag
         * @param locale             locale
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-collator)
         */
        @JvmStatic
        fun collator(
            caseSensitive: Expression,
            diacriticSensitive: Expression,
            locale: Expression,
        ): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            map["case-sensitive"] = caseSensitive
            map["diacritic-sensitive"] = diacriticSensitive
            map["locale"] = locale
            return Expression("collator", ExpressionMap(map))
        }

        /**
         * Returns a collator for use in locale-dependent comparison operations.
         * The case-sensitive and diacritic-sensitive options default to false.
         * The locale argument specifies the IETF language tag of the locale to use.
         * If none is provided, the default locale is used. If the requested locale is not available,
         * the collator will use a system-defined fallback locale.
         * Use resolved-locale to test the results of locale fallback behavior.
         *
         * @param caseSensitive      case sensitive flag
         * @param diacriticSensitive diacritic sensitive flag
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-collator)
         */
        @JvmStatic
        fun collator(
            caseSensitive: Expression,
            diacriticSensitive: Expression,
        ): Expression {
            val map: MutableMap<String, Expression> = HashMap()
            map["case-sensitive"] = caseSensitive
            map["diacritic-sensitive"] = diacriticSensitive
            return Expression("collator", ExpressionMap(map))
        }

        /**
         * Returns formatted text containing annotations for use in mixed-format text-field entries.
         *
         * To build the expression, use [formatEntry].
         *
         * "format" expression can be used, for example, with the [PropertyFactory.textField]
         * and accepts unlimited numbers of formatted sections.
         *
         * Each section consist of the input, the displayed text, and options, like font-scale and text-font.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *   textField(
         *     format(
         *       formatEntry(
         *         get("header_property"),
         *         formatFontScale(2.0),
         *         formatTextFont(arrayOf("DIN Offc Pro Regular", "Arial Unicode MS Regular"))
         *       ),
         *       formatEntry(concat(literal("\n"), get("description_property")), formatFontScale(1.5))
         *     )
         *   )
         * )
         * ```
         *
         * @param formatEntries format entries
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-format)
         */
        @JvmStatic
        fun format(vararg formatEntries: FormatEntry): Expression {
            // for each entry we are going to build an input and parameters
            val mappedExpressions = arrayOfNulls<Expression>(formatEntries.size * 2)
            var mappedIndex = 0
            for (formatEntry in formatEntries) {
                // input
                mappedExpressions[mappedIndex++] = formatEntry.text
                // parameters
                val map: MutableMap<String, Expression> = HashMap()
                formatEntry.options?.forEach { option ->
                    map[option.type] = option.value
                }
                mappedExpressions[mappedIndex++] = ExpressionMap(map)
            }
            @Suppress("UNCHECKED_CAST")
            return Expression("format", *(mappedExpressions as kotlin.Array<Expression>))
        }

        /**
         * Returns a format entry that can be used in [format] to create formatted text fields.
         *
         * Text is required to be of a resulting type string.
         *
         * Text is required to be passed; [FormatOption]s are optional and will default to the base values defined
         * for the symbol.
         *
         * @param text          displayed text
         * @param formatOptions format options
         * @return format entry
         */
        @JvmStatic
        fun formatEntry(
            text: Expression,
            vararg formatOptions: FormatOption,
        ): FormatEntry = FormatEntry(text, formatOptions)

        /**
         * Returns a format entry that can be used in [format] to create formatted text fields.
         *
         * Text is required to be of a resulting type string.
         *
         * Text is required to be passed; [FormatOption]s are optional and will default to the base values defined
         * for the symbol.
         *
         * @param text displayed text
         * @return format entry
         */
        @JvmStatic
        fun formatEntry(text: Expression): FormatEntry = FormatEntry(text, null)

        /**
         * Returns a format entry that can be used in [format] to create formatted text fields.
         *
         * Text is required to be of a resulting type string.
         *
         * Text is required to be passed; [FormatOption]s are optional and will default to the base values defined
         * for the symbol.
         *
         * @param text          displayed text
         * @param formatOptions format options
         * @return format entry
         */
        @JvmStatic
        fun formatEntry(
            text: String,
            vararg formatOptions: FormatOption,
        ): FormatEntry = FormatEntry(literal(text), formatOptions)

        /**
         * Returns a format entry that can be used in [format] to create formatted text fields.
         *
         * Text is required to be of a resulting type string.
         *
         * Text is required to be passed; [FormatOption]s are optional and will default to the base values defined
         * for the symbol.
         *
         * @param text displayed text
         * @return format entry
         */
        @JvmStatic
        fun formatEntry(text: String): FormatEntry = FormatEntry(literal(text), null)

        /**
         * Returns image expression for use in '*-pattern' and 'icon-image' layer properties. Compared to
         * string literals that can be used to represent an image, image expression allows to determine an
         * image's availability at runtime, thus, can be used in conditional [coalesce
         * operator](https://docs.mapbox.com/mapbox-gl-js/style-spec/#expressions-coalesce).
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     iconImage(image(get("key-to-feature")))
         * )
         * ```
         *
         * Example usage with coalesce operator:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     iconImage(
         *         coalesce(
         *             image(literal("maki-11")),
         *             image(literal("bicycle-15")),
         *             image(literal("default-icon"))
         *         )
         *     )
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Image expression](https://docs.mapbox.com/mapbox-gl-js/style-spec/#expressions-types-image)
         */
        @JvmStatic
        fun image(input: Expression): Expression = Expression("image", input)

        /**
         * Asserts that the input value is an object. If it is not, the expression is an error
         * The asserted input value is returned as result.
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-object)
         */
        @JvmStatic
        fun `object`(input: Expression): Expression = Expression("object", input)

        /**
         * Converts the input value to a string.
         * If the input is null, the result is null.
         * If the input is a boolean, the result is true or false.
         * If the input is a number, it is converted to a string by NumberToString in the ECMAScript Language Specification.
         * If the input is a color, it is converted to a string of the form "rgba(r,g,b,a)",
         * where `r`, `g`, and `b` are numerals ranging from 0 to 255, and `a` ranges from 0 to 1.
         * Otherwise, the input is converted to a string in the format specified by the JSON.stringify in the ECMAScript
         * Language Specification.
         *
         * Example usage:
         *
         * ```kotlin
         * val symbolLayer = SymbolLayer("layer-id", "source-id")
         * symbolLayer.setProperties(
         *     textField(get("key-to-number-value"))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-to-string)
         */
        @JvmStatic
        fun toString(input: Expression): Expression = Expression("to-string", input)

        /**
         * Converts the input value to a number, if possible.
         * If the input is null or false, the result is 0.
         * If the input is true, the result is 1.
         * If the input is a string, it is converted to a number as specified by the ECMAScript Language Specification.
         * If multiple values are provided, each one is evaluated in order until the first successful conversion is obtained.
         * If none of the inputs can be converted, the expression is an error.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(toNumber(get("key-to-string-value")))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-to-number)
         */
        @JvmStatic
        fun toNumber(input: Expression): Expression = Expression("to-number", input)

        /**
         * Converts the input value to a boolean. The result is `false` when then input is an empty string, 0, false,
         * null, or NaN; otherwise it is true.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(toBool(get("key-to-value")))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-to-boolean)
         */
        @JvmStatic
        fun toBool(input: Expression): Expression = Expression("to-boolean", input)

        /**
         * Converts the input value to a color. If multiple values are provided,
         * each one is evaluated in order until the first successful conversion is obtained.
         * If none of the inputs can be converted, the expression is an error.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(toColor(get("keyStringValue")))
         * )
         * ```
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#types-to-color)
         */
        @JvmStatic
        fun toColor(input: Expression): Expression = Expression("to-color", input)

        /**
         * Converts input value to a padding.
         *
         * If the input is a number or an array of numbers padding is created following
         * the same pattern as CSS padding. See [Style specification](https://maplibre.org/maplibre-style-spec/types/#padding).
         * Otherwise, the expression is an error.
         *
         * @param input expression input
         * @return expression
         */
        @JvmStatic
        fun toPadding(input: Expression): Expression = Expression("to-padding", input)

        /**
         * Binds input to named variables,
         * which can then be referenced in the result expression using [var] or [var].
         *
         * @param input expression input
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#let)
         */
        @JvmStatic
        fun let(
            @Size(min = 1) vararg input: Expression,
        ): Expression = Expression("let", *input)

        /**
         * References variable bound using let.
         *
         * @param expression the variable naming expression that was bound with using let
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#var)
         */
        @JvmStatic
        fun `var`(expression: Expression): Expression = Expression("var", expression)

        /**
         * References variable bound using let.
         *
         * @param variableName the variable naming that was bound with using let
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#var)
         */
        @JvmStatic
        fun `var`(variableName: String): Expression = `var`(literal(variableName))

        /**
         * Gets the current zoom level.
         *
         * Note that in style layout and paint properties,
         * zoom may only appear as the input to a top-level step or interpolate expression.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         exponential(0.5f), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#zoom)
         */
        @JvmStatic
        fun zoom(): Expression = Expression("zoom")

        /**
         * Produces a stop value.
         *
         * Can be used for [stop] as part of varargs parameter in
         * [step] or [interpolate].
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), literal(0.0f),
         *         stop(1.0f, 2.5f),
         *         stop(10.0f, 5.0f))
         *     )
         * )
         * ```
         *
         * @param stop  the stop input
         * @param value the stop output
         * @return the stop
         */
        @JvmStatic
        fun stop(
            stop: Any,
            value: Any,
        ): Stop = Stop(stop, value)

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), literal(0.0f),
         *         literal(1.0f), literal(2.5f),
         *         literal(10.0f), literal(5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input value
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Number,
            defaultOutput: Expression,
            vararg stops: Expression,
        ): Expression = step(literal(input), defaultOutput, *stops)

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), literal(0.0f),
         *         literal(1.0f), literal(2.5f),
         *         literal(10.0f), literal(5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input expression
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Expression,
            defaultOutput: Expression,
            vararg stops: Expression,
        ): Expression = Expression("step", *join(arrayOf(input, defaultOutput), stops))

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), literal(0.0f),
         *         stop(1, 2.5f),
         *         stop(10, 5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input value
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Number,
            defaultOutput: Expression,
            vararg stops: Stop,
        ): Expression = step(literal(input), defaultOutput, *Stop.toExpressionArray(*stops))

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), literal(0.0f),
         *         stop(1, 2.5f),
         *         stop(10, 5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input value
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Expression,
            defaultOutput: Expression,
            vararg stops: Stop,
        ): Expression = step(input, defaultOutput, *Stop.toExpressionArray(*stops))

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(1.0f, 0.0f,
         *         literal(1.0f), literal(2.5f),
         *         literal(10.0f), literal(5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input value
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Number,
            defaultOutput: Number,
            vararg stops: Expression,
        ): Expression = step(literal(input), defaultOutput, *stops)

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), 0.0f,
         *         literal(1.0f), literal(2.5f),
         *         literal(10.0f), literal(5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input expression
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Expression,
            defaultOutput: Number,
            vararg stops: Expression,
        ): Expression = step(input, literal(defaultOutput), *stops)

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), 0.0f,
         *         stop(1, 2.5f),
         *         stop(10, 5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input value
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Number,
            defaultOutput: Number,
            vararg stops: Stop,
        ): Expression = step(literal(input), defaultOutput, *Stop.toExpressionArray(*stops))

        /**
         * Produces discrete, stepped results by evaluating a piecewise-constant function defined by pairs of
         * input and output values (\"stops\"). The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * Returns the output value of the stop just less than the input,
         * or the first input if the input is less than the first stop.
         *
         * Example usage:
         *
         * ```kotlin
         * val circleLayer = CircleLayer("layer-id", "source-id")
         * circleLayer.setProperties(
         *     circleRadius(
         *         step(zoom(), 0.0f,
         *         stop(1, 2.5f),
         *         stop(10, 5.0f))
         *     )
         * )
         * ```
         *
         * @param input         the input value
         * @param defaultOutput the default output expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#step)
         */
        @JvmStatic
        fun step(
            input: Expression,
            defaultOutput: Number,
            vararg stops: Stop,
        ): Expression = step(input, defaultOutput, *Stop.toExpressionArray(*stops))

        /**
         * Produces continuous, smooth results by interpolating between pairs of input and output values (\"stops\").
         * The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * The output type must be `number`, `array&lt;number&gt;`, or `color`.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *   fillColor(
         *     interpolate(exponential(0.5f), zoom(),
         *        stop(1.0f, color(Color.RED)),
         *        stop(5.0f, color(Color.BLUE)),
         *        stop(10.0f, color(Color.GREEN)
         *       )
         *     )
         *   )
         * )
         * ```
         *
         * @param interpolation type of interpolation
         * @param number        the input expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun interpolate(
            interpolation: Interpolator,
            number: Expression,
            vararg stops: Expression,
        ): Expression = Expression("interpolate", *join(arrayOf(interpolation, number), stops))

        /**
         * Produces continuous, smooth results by interpolating between pairs of input and output values (\"stops\").
         * The `input` may be any numeric expression (e.g., `[\"get\", \"population\"]`).
         * Stop inputs must be numeric literals in strictly ascending order.
         * The output type must be `number`, `array&lt;number&gt;`, or `color`.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         exponential(0.5f), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @param interpolation type of interpolation
         * @param number        the input expression
         * @param stops         pair of input and output values
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun interpolate(
            interpolation: Interpolator,
            number: Expression,
            vararg stops: Stop,
        ): Expression = interpolate(interpolation, number, *Stop.toExpressionArray(*stops))

        /**
         * interpolates linearly between the pair of stops just less than and just greater than the input.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         linear(), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun linear(): Interpolator = Interpolator("linear")

        /**
         * Interpolates exponentially between the stops just less than and just greater than the input.
         * `base` controls the rate at which the output increases:
         * higher values make the output increase more towards the high end of the range.
         * With values close to 1 the output increases linearly.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         exponential(0.5f), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @param base value controlling the route at which the output increases
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun exponential(base: Number): Interpolator = exponential(literal(base))

        /**
         * Interpolates exponentially between the stops just less than and just greater than the input.
         * The parameter controls the rate at which the output increases:
         * higher values make the output increase more towards the high end of the range.
         * With values close to 1 the output increases linearly.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         exponential(get("keyToValue")), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @param expression base number expression
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun exponential(expression: Expression): Interpolator = Interpolator("exponential", expression)

        /**
         * Interpolates using the cubic bezier curve defined by the given control points.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         cubicBezier(0.42f, 0.0f, 1.0f, 1.0f), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @param x1 x value of the first point of a cubic bezier, ranges from 0 to 1
         * @param y1 y value of the first point of a cubic bezier, ranges from 0 to 1
         * @param x2 x value of the second point of a cubic bezier, ranges from 0 to 1
         * @param y2 y value fo the second point of a cubic bezier, ranges from 0 to 1
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun cubicBezier(
            x1: Expression,
            y1: Expression,
            x2: Expression,
            y2: Expression,
        ): Interpolator = Interpolator("cubic-bezier", x1, y1, x2, y2)

        /**
         * Interpolates using the cubic bezier curve defined by the given control points.
         *
         * Example usage:
         *
         * ```kotlin
         * val fillLayer = FillLayer("layer-id", "source-id")
         * fillLayer.setProperties(
         *     fillColor(
         *       interpolate(
         *         cubicBezier(0.42f, 0.0f, 1.0f, 1.0f), zoom(),
         *         stop(1.0f, color(Color.RED)),
         *         stop(5.0f, color(Color.BLUE)),
         *         stop(10.0f, color(Color.GREEN))
         *       )
         *     )
         * )
         * ```
         *
         * @param x1 x value of the first point of a cubic bezier, ranges from 0 to 1
         * @param y1 y value of the first point of a cubic bezier, ranges from 0 to 1
         * @param x2 x value of the second point of a cubic bezier, ranges from 0 to 1
         * @param y2 y value fo the second point of a cubic bezier, ranges from 0 to 1
         * @return expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/expressions/#interpolate)
         */
        @JvmStatic
        fun cubicBezier(
            x1: Number,
            y1: Number,
            x2: Number,
            y2: Number,
        ): Interpolator = cubicBezier(literal(x1), literal(y1), literal(x2), literal(y2))

        /**
         * Joins two expressions arrays.
         *
         * This flattens the object array output of an expression from a nested expression hierarchy.
         *
         * @param left  the left part of an expression
         * @param right the right part of an expression
         * @return the joined expression
         */
        private fun join(
            left: kotlin.Array<out Expression>,
            right: kotlin.Array<out Expression>,
        ): kotlin.Array<Expression> {
            val output = arrayOfNulls<Expression>(left.size + right.size)
            System.arraycopy(left, 0, output, 0, left.size)
            System.arraycopy(right, 0, output, left.size, right.size)
            @Suppress("UNCHECKED_CAST")
            return output as kotlin.Array<Expression>
        }

        /**
         * Returns a DSL equivalent of a raw expression.
         *
         * If your raw expression contains a coma (, ) delimited literal it has to be enclosed with double quotes ("),
         * for example
         *
         * ```kotlin
         * ["to-color", "rgba(255, 0, 0, 255)"]
         * ```
         *
         * @param rawExpression the raw expression
         * @return the resulting expression
         * @see [Style specification](https://maplibre.org/maplibre-style-spec/)
         */
        @JvmStatic
        fun raw(rawExpression: String): Expression = Converter.convert(rawExpression)

        /**
         * Converts an object that is a primitive array to an Object[]
         *
         * @param object the object to convert to an object array
         * @return the converted object array
         */
        private fun toObjectArray(`object`: Any): kotlin.Array<Any?> {
            // object is a primitive array
            val len =
                java.lang.reflect.Array
                    .getLength(`object`)
            val objects = arrayOfNulls<Any>(len)
            for (i in 0 until len) {
                objects[i] =
                    java.lang.reflect.Array
                        .get(`object`, i)
            }
            return objects
        }
    }
}
