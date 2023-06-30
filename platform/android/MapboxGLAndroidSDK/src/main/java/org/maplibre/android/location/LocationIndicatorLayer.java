// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.location;

import androidx.annotation.ColorInt;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import static org.maplibre.android.utils.ColorUtils.rgbaToColor;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.TransitionOptions;
import org.maplibre.android.style.layers.Layer;
import org.maplibre.android.style.layers.PropertyValue;

/**
 * 
 *
 * @see <a href="https://maplibre.org/maplibre-style-spec/#layers-location-indicator">The online documentation</a>
 */
@UiThread
class LocationIndicatorLayer extends Layer {

  /**
   * Creates a LocationIndicatorLayer.
   *
   * @param nativePtr pointer used by core
   */
  @Keep
  LocationIndicatorLayer(long nativePtr) {
    super(nativePtr);
  }

  /**
   * Creates a LocationIndicatorLayer.
   *
   * @param layerId the id of the layer
   */
  public LocationIndicatorLayer(String layerId) {
    super();
    initialize(layerId);
  }

  @Keep
  protected native void initialize(String layerId);

  /**
   * Set a property or properties.
   *
   * @param properties the var-args properties
   * @return This
   */
  @NonNull
  public LocationIndicatorLayer withProperties(@NonNull PropertyValue<?>... properties) {
    setProperties(properties);
    return this;
  }

  // Property getters

  /**
   * Get the TopImage property
   *
   * @return property wrapper value around String
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getTopImage() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("top-image", nativeGetTopImage());
  }

  /**
   * Get the BearingImage property
   *
   * @return property wrapper value around String
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getBearingImage() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("bearing-image", nativeGetBearingImage());
  }

  /**
   * Get the ShadowImage property
   *
   * @return property wrapper value around String
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getShadowImage() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("shadow-image", nativeGetShadowImage());
  }

  /**
   * Get the PerspectiveCompensation property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getPerspectiveCompensation() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("perspective-compensation", nativeGetPerspectiveCompensation());
  }

  /**
   * Get the ImageTiltDisplacement property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getImageTiltDisplacement() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("image-tilt-displacement", nativeGetImageTiltDisplacement());
  }

  /**
   * Get the Bearing property
   *
   * @return property wrapper value around Double
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Double> getBearing() {
    checkThread();
    return (PropertyValue<Double>) new PropertyValue("bearing", nativeGetBearing());
  }

  /**
   * Get the Location property
   *
   * @return property wrapper value around Double[]
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Double[]> getLocation() {
    checkThread();
    return (PropertyValue<Double[]>) new PropertyValue("location", nativeGetLocation());
  }

  /**
   * Get the Location property transition options
   *
   * @return transition options for Double[]
   */
  @NonNull
  public TransitionOptions getLocationTransition() {
    checkThread();
    return nativeGetLocationTransition();
  }

  /**
   * Set the Location property transition options
   *
   * @param options transition options for Double[]
   */
  public void setLocationTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetLocationTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the AccuracyRadius property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getAccuracyRadius() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("accuracy-radius", nativeGetAccuracyRadius());
  }

  /**
   * Get the AccuracyRadius property transition options
   *
   * @return transition options for Float
   */
  @NonNull
  public TransitionOptions getAccuracyRadiusTransition() {
    checkThread();
    return nativeGetAccuracyRadiusTransition();
  }

  /**
   * Set the AccuracyRadius property transition options
   *
   * @param options transition options for Float
   */
  public void setAccuracyRadiusTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetAccuracyRadiusTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the TopImageSize property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getTopImageSize() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("top-image-size", nativeGetTopImageSize());
  }

  /**
   * Get the TopImageSize property transition options
   *
   * @return transition options for Float
   */
  @NonNull
  public TransitionOptions getTopImageSizeTransition() {
    checkThread();
    return nativeGetTopImageSizeTransition();
  }

  /**
   * Set the TopImageSize property transition options
   *
   * @param options transition options for Float
   */
  public void setTopImageSizeTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetTopImageSizeTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the BearingImageSize property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getBearingImageSize() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("bearing-image-size", nativeGetBearingImageSize());
  }

  /**
   * Get the BearingImageSize property transition options
   *
   * @return transition options for Float
   */
  @NonNull
  public TransitionOptions getBearingImageSizeTransition() {
    checkThread();
    return nativeGetBearingImageSizeTransition();
  }

  /**
   * Set the BearingImageSize property transition options
   *
   * @param options transition options for Float
   */
  public void setBearingImageSizeTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetBearingImageSizeTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the ShadowImageSize property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getShadowImageSize() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("shadow-image-size", nativeGetShadowImageSize());
  }

  /**
   * Get the ShadowImageSize property transition options
   *
   * @return transition options for Float
   */
  @NonNull
  public TransitionOptions getShadowImageSizeTransition() {
    checkThread();
    return nativeGetShadowImageSizeTransition();
  }

  /**
   * Set the ShadowImageSize property transition options
   *
   * @param options transition options for Float
   */
  public void setShadowImageSizeTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetShadowImageSizeTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the AccuracyRadiusColor property
   *
   * @return property wrapper value around String
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getAccuracyRadiusColor() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("accuracy-radius-color", nativeGetAccuracyRadiusColor());
  }

  /**
   * The color for drawing the accuracy radius, as a circle. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @return int representation of a rgba string color
   * @throws RuntimeException thrown if property isn't a value
   */
  @ColorInt
  public int getAccuracyRadiusColorAsInt() {
    checkThread();
    PropertyValue<String> value = getAccuracyRadiusColor();
    if (value.isValue()) {
      return rgbaToColor(value.getValue());
    } else {
      throw new RuntimeException("accuracy-radius-color was set as a Function");
    }
  }

  /**
   * Get the AccuracyRadiusColor property transition options
   *
   * @return transition options for String
   */
  @NonNull
  public TransitionOptions getAccuracyRadiusColorTransition() {
    checkThread();
    return nativeGetAccuracyRadiusColorTransition();
  }

  /**
   * Set the AccuracyRadiusColor property transition options
   *
   * @param options transition options for String
   */
  public void setAccuracyRadiusColorTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetAccuracyRadiusColorTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the AccuracyRadiusBorderColor property
   *
   * @return property wrapper value around String
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getAccuracyRadiusBorderColor() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("accuracy-radius-border-color", nativeGetAccuracyRadiusBorderColor());
  }

  /**
   * The color for drawing the accuracy radius border. To adjust transparency, set the alpha component of the color accordingly.
   *
   * @return int representation of a rgba string color
   * @throws RuntimeException thrown if property isn't a value
   */
  @ColorInt
  public int getAccuracyRadiusBorderColorAsInt() {
    checkThread();
    PropertyValue<String> value = getAccuracyRadiusBorderColor();
    if (value.isValue()) {
      return rgbaToColor(value.getValue());
    } else {
      throw new RuntimeException("accuracy-radius-border-color was set as a Function");
    }
  }

  /**
   * Get the AccuracyRadiusBorderColor property transition options
   *
   * @return transition options for String
   */
  @NonNull
  public TransitionOptions getAccuracyRadiusBorderColorTransition() {
    checkThread();
    return nativeGetAccuracyRadiusBorderColorTransition();
  }

  /**
   * Set the AccuracyRadiusBorderColor property transition options
   *
   * @param options transition options for String
   */
  public void setAccuracyRadiusBorderColorTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetAccuracyRadiusBorderColorTransition(options.getDuration(), options.getDelay());
  }

  @NonNull
  @Keep
  private native Object nativeGetTopImage();

  @NonNull
  @Keep
  private native Object nativeGetBearingImage();

  @NonNull
  @Keep
  private native Object nativeGetShadowImage();

  @NonNull
  @Keep
  private native Object nativeGetPerspectiveCompensation();

  @NonNull
  @Keep
  private native Object nativeGetImageTiltDisplacement();

  @NonNull
  @Keep
  private native Object nativeGetBearing();

  @NonNull
  @Keep
  private native Object nativeGetLocation();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetLocationTransition();

  @Keep
  private native void nativeSetLocationTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetAccuracyRadius();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetAccuracyRadiusTransition();

  @Keep
  private native void nativeSetAccuracyRadiusTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetTopImageSize();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetTopImageSizeTransition();

  @Keep
  private native void nativeSetTopImageSizeTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetBearingImageSize();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetBearingImageSizeTransition();

  @Keep
  private native void nativeSetBearingImageSizeTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetShadowImageSize();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetShadowImageSizeTransition();

  @Keep
  private native void nativeSetShadowImageSizeTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetAccuracyRadiusColor();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetAccuracyRadiusColorTransition();

  @Keep
  private native void nativeSetAccuracyRadiusColorTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetAccuracyRadiusBorderColor();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetAccuracyRadiusBorderColorTransition();

  @Keep
  private native void nativeSetAccuracyRadiusBorderColorTransition(long duration, long delay);

  @Override
  @Keep
  protected native void finalize() throws Throwable;

}
