package org.maplibre.android.style.layers

import androidx.annotation.ColorInt
import com.google.gson.JsonArray
import org.maplibre.android.MapStrictMode
import org.maplibre.android.exceptions.ConversionException
import org.maplibre.android.log.Logger
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.utils.ColorUtils
import java.util.Arrays

/**
 * Properties for Layer
 *
 * Not part of the public API.
 *
 * @param name the property name
 * @param value the property value
 * @see PropertyFactory for construction of [PropertyValue]s
 */
open class PropertyValue<T>(
    @JvmField val name: String,
    @JvmField val value: T?,
) {
    /**
     * Returns if this is null
     *
     * @return true if this is null, false if not
     */
    fun isNull(): Boolean = value == null

    /**
     * Returns if this is a expression.
     *
     * @return true if this is a expression, false if not
     */
    val isExpression: Boolean
        get() = !isNull() && (value is JsonArray || value is Expression)

    /**
     * Get the expression of the property.
     *
     * @return the property expression
     */
    val expression: Expression?
        get() =
            if (isExpression) {
                if (value is JsonArray) Expression.Converter.convert(value) else value as Expression
            } else {
                Logger.w(TAG, String.format("%s not an expression, try PropertyValue#getValue()", name))
                null
            }

    /**
     * Returns if this is a value.
     *
     * @return true if is a value, false if not
     */
    fun isValue(): Boolean = !isNull() && !isExpression

    /**
     * Get the value of the property.
     *
     * @return the property value
     */
    fun getValue(): T? =
        if (isValue()) {
            value
        } else {
            Logger.w(TAG, String.format("%s not a value, try PropertyValue#getExpression()", name))
            null
        }

    /**
     * Get the color int value of the property if the value is a color.
     *
     * @return the color int value of the property, null if not a color value
     */
    @ColorInt
    fun getColorInt(): Int? {
        if (!isValue() || value !is String) {
            Logger.e(TAG, String.format("%s is not a String value and can not be converted to a color it", name))
            return null
        }

        return try {
            ColorUtils.rgbaToColor(value)
        } catch (ex: ConversionException) {
            Logger.e(TAG, String.format("%s could not be converted to a Color int: %s", name, ex.message))
            MapStrictMode.strictModeViolation(ex)
            null
        }
    }

    /**
     * Get the string representation of a property value.
     *
     * @return the string representation
     */
    override fun toString(): String = String.format("%s: %s", name, value)

    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        val that = other as PropertyValue<*>

        if (name != that.name) {
            return false
        }
        return if (value != null) {
            if (value is Array<*>) {
                Arrays.deepEquals(value, that.value as Array<*>?)
            } else {
                value == that.value
            }
        } else {
            that.value == null
        }
    }

    override fun hashCode(): Int {
        var result = name.hashCode()
        result = 31 * result + (value?.hashCode() ?: 0)
        return result
    }

    private companion object {
        const val TAG = "Mbgl-PropertyValue"
    }
}
