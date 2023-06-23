package org.maplibre.android.style.layers;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.JsonArray;
import org.maplibre.android.MapStrictMode;
import org.maplibre.android.exceptions.ConversionException;
import org.maplibre.android.log.Logger;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.utils.ColorUtils;

import java.util.Arrays;

/**
 * Properties for Layer
 */
public class PropertyValue<T> {

  private static final String TAG = "Mbgl-PropertyValue";

  @NonNull
  public final String name;
  public final T value;

  /**
   * Not part of the public API.
   *
   * @param name  the property name
   * @param value the property value
   * @see PropertyFactory for construction of {@link PropertyValue}s
   */
  public PropertyValue(@NonNull String name, T value) {
    this.name = name;
    this.value = value;
  }

  /**
   * Returns if this is null
   *
   * @return true if this is null, false if not
   */
  public boolean isNull() {
    return value == null;
  }

  /**
   * Returns if this is a expression.
   *
   * @return true if this is a expression, false if not
   */
  public boolean isExpression() {
    return !isNull() && (value instanceof JsonArray || value instanceof Expression);
  }

  /**
   * Get the expression of the property.
   *
   * @return the property expression
   */
  @Nullable
  public Expression getExpression() {
    if (isExpression()) {
      return value instanceof JsonArray
        ? Expression.Converter.convert((JsonArray) value)
        : (Expression) value;
    } else {
      Logger.w(TAG, String.format("%s not an expression, try PropertyValue#getValue()", name));
      return null;
    }
  }

  /**
   * Returns if this is a value.
   *
   * @return true if is a value, false if not
   */
  public boolean isValue() {
    return !isNull() && !isExpression();
  }

  /**
   * Get the value of the property.
   *
   * @return the property value
   */
  @Nullable
  public T getValue() {
    if (isValue()) {
      // noinspection unchecked
      return value;
    } else {
      Logger.w(TAG, String.format("%s not a value, try PropertyValue#getExpression()", name));
      return null;
    }
  }

  /**
   * Get the color int value of the property if the value is a color.
   *
   * @return the color int value of the property, null if not a color value
   */
  @ColorInt
  @Nullable
  public Integer getColorInt() {
    if (!isValue() || !(value instanceof String)) {
      Logger.e(TAG, String.format("%s is not a String value and can not be converted to a color it", name));
      return null;
    }

    try {
      return ColorUtils.rgbaToColor((String) value);
    } catch (ConversionException ex) {
      Logger.e(TAG, String.format("%s could not be converted to a Color int: %s", name, ex.getMessage()));
      MapStrictMode.strictModeViolation(ex);
      return null;
    }
  }

  /**
   * Get the string representation of a property value.
   *
   * @return the string representation
   */
  @Override
  public String toString() {
    return String.format("%s: %s", name, value);
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) {
      return true;
    }
    if (o == null || getClass() != o.getClass()) {
      return false;
    }

    PropertyValue<?> that = (PropertyValue<?>) o;

    if (!name.equals(that.name)) {
      return false;
    }
    if (value != null) {
      if (value instanceof Object[]) {
        return Arrays.deepEquals((Object[]) value, (Object[]) that.value);
      }
      return value.equals(that.value);
    } else {
      return that.value == null;
    }
  }

  @Override
  public int hashCode() {
    int result = name.hashCode();
    result = 31 * result + (value != null ? value.hashCode() : 0);
    return result;
  }
}
