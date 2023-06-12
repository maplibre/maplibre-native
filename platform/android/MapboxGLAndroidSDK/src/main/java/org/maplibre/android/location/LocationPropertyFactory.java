// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.location;

import androidx.annotation.ColorInt;

import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.types.Formatted;
import static org.maplibre.android.utils.ColorUtils.colorToRgbaString;
import org.maplibre.android.style.layers.LayoutPropertyValue;
import org.maplibre.android.style.layers.PaintPropertyValue;
import org.maplibre.android.style.layers.Property;
import org.maplibre.android.style.layers.PropertyValue;

/**
 * Constructs paint/layout properties for Layers
 *
 * @see <a href="https://maplibre.org/maplibre-style-spec/#layers">Layer style documentation</a>
 */
class LocationPropertyFactory {

  /**
   * Set the property visibility.
   *
   * @param value the visibility value
   * @return property wrapper around visibility
   */
  public static PropertyValue<String> visibility(@Property.VISIBILITY String value) {
    return new LayoutPropertyValue<>("visibility", value);
  }

  /**
   * The amount of the perspective compensation, between 0 and 1. A value of 1 produces a location indicator of constant width across the screen. A value of 0 makes it scale naturally according to the viewing projection.
   *
   * @param value a Float value
   * @return property wrapper around Float
   */
  public static PropertyValue<Float> perspectiveCompensation(Float value) {
    return new PaintPropertyValue<>("perspective-compensation", value);
  }

  /**
   * The amount of the perspective compensation, between 0 and 1. A value of 1 produces a location indicator of constant width across the screen. A value of 0 makes it scale naturally according to the viewing projection.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> perspectiveCompensation(Expression expression) {
    return new PaintPropertyValue<>("perspective-compensation", expression);
  }

  /**
   * The displacement off the center of the top image and the shadow image when the pitch of the map is greater than 0. This helps producing a three-dimensional appearence.
   *
   * @param value a Float value
   * @return property wrapper around Float
   */
  public static PropertyValue<Float> imageTiltDisplacement(Float value) {
    return new PaintPropertyValue<>("image-tilt-displacement", value);
  }

  /**
   * The displacement off the center of the top image and the shadow image when the pitch of the map is greater than 0. This helps producing a three-dimensional appearence.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> imageTiltDisplacement(Expression expression) {
    return new PaintPropertyValue<>("image-tilt-displacement", expression);
  }

  /**
   * The bearing of the location indicator.
   *
   * @param value a Double value
   * @return property wrapper around Double
   */
  public static PropertyValue<Double> bearing(Double value) {
    return new PaintPropertyValue<>("bearing", value);
  }

  /**
   * The bearing of the location indicator.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> bearing(Expression expression) {
    return new PaintPropertyValue<>("bearing", expression);
  }

  /**
   * An array of [latitude, longitude, altitude] position of the location indicator.
   *
   * @param value a Double[] value
   * @return property wrapper around Double[]
   */
  public static PropertyValue<Double[]> location(Double[] value) {
    return new PaintPropertyValue<>("location", value);
  }

  /**
   * An array of [latitude, longitude, altitude] position of the location indicator.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> location(Expression expression) {
    return new PaintPropertyValue<>("location", expression);
  }

  /**
   * The accuracy, in meters, of the position source used to retrieve the position of the location indicator.
   *
   * @param value a Float value
   * @return property wrapper around Float
   */
  public static PropertyValue<Float> accuracyRadius(Float value) {
    return new PaintPropertyValue<>("accuracy-radius", value);
  }

  /**
   * The accuracy, in meters, of the position source used to retrieve the position of the location indicator.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> accuracyRadius(Expression expression) {
    return new PaintPropertyValue<>("accuracy-radius", expression);
  }

  /**
   * The size of the top image, as a scale factor applied to the size of the specified image.
   *
   * @param value a Float value
   * @return property wrapper around Float
   */
  public static PropertyValue<Float> topImageSize(Float value) {
    return new PaintPropertyValue<>("top-image-size", value);
  }

  /**
   * The size of the top image, as a scale factor applied to the size of the specified image.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> topImageSize(Expression expression) {
    return new PaintPropertyValue<>("top-image-size", expression);
  }

  /**
   * The size of the bearing image, as a scale factor applied to the size of the specified image.
   *
   * @param value a Float value
   * @return property wrapper around Float
   */
  public static PropertyValue<Float> bearingImageSize(Float value) {
    return new PaintPropertyValue<>("bearing-image-size", value);
  }

  /**
   * The size of the bearing image, as a scale factor applied to the size of the specified image.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> bearingImageSize(Expression expression) {
    return new PaintPropertyValue<>("bearing-image-size", expression);
  }

  /**
   * The size of the shadow image, as a scale factor applied to the size of the specified image.
   *
   * @param value a Float value
   * @return property wrapper around Float
   */
  public static PropertyValue<Float> shadowImageSize(Float value) {
    return new PaintPropertyValue<>("shadow-image-size", value);
  }

  /**
   * The size of the shadow image, as a scale factor applied to the size of the specified image.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> shadowImageSize(Expression expression) {
    return new PaintPropertyValue<>("shadow-image-size", expression);
  }

  /**
   * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @param value a int color value
   * @return property wrapper around String color
   */
  public static PropertyValue<String> accuracyRadiusColor(@ColorInt int value) {
    return new PaintPropertyValue<>("accuracy-radius-color", colorToRgbaString(value));
  }

  /**
   * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<String> accuracyRadiusColor(String value) {
    return new PaintPropertyValue<>("accuracy-radius-color", value);
  }

  /**
   * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> accuracyRadiusColor(Expression expression) {
    return new PaintPropertyValue<>("accuracy-radius-color", expression);
  }

  /**
   * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @param value a int color value
   * @return property wrapper around String color
   */
  public static PropertyValue<String> accuracyRadiusBorderColor(@ColorInt int value) {
    return new PaintPropertyValue<>("accuracy-radius-border-color", colorToRgbaString(value));
  }

  /**
   * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<String> accuracyRadiusBorderColor(String value) {
    return new PaintPropertyValue<>("accuracy-radius-border-color", value);
  }

  /**
   * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @param expression an expression statement
   * @return property wrapper around an expression statement
   */
  public static PropertyValue<Expression> accuracyRadiusBorderColor(Expression expression) {
    return new PaintPropertyValue<>("accuracy-radius-border-color", expression);
  }

  /**
   * Name of image in sprite to use as the top of the location indicator.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<String> topImage(String value) {
    return new LayoutPropertyValue<>("top-image", value);
  }

  /**
   * Name of image in sprite to use as the top of the location indicator.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<Expression> topImage(Expression value) {
    return new LayoutPropertyValue<>("top-image", value);
  }

  /**
   * Name of image in sprite to use as the middle of the location indicator.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<String> bearingImage(String value) {
    return new LayoutPropertyValue<>("bearing-image", value);
  }

  /**
   * Name of image in sprite to use as the middle of the location indicator.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<Expression> bearingImage(Expression value) {
    return new LayoutPropertyValue<>("bearing-image", value);
  }

  /**
   * Name of image in sprite to use as the background of the location indicator.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<String> shadowImage(String value) {
    return new LayoutPropertyValue<>("shadow-image", value);
  }

  /**
   * Name of image in sprite to use as the background of the location indicator.
   *
   * @param value a String value
   * @return property wrapper around String
   */
  public static PropertyValue<Expression> shadowImage(Expression value) {
    return new LayoutPropertyValue<>("shadow-image", value);
  }

}
