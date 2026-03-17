// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.style.layers;

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

/**
 * Client-side elevation coloring based on DEM data. The implementation supports Mapbox Terrain RGB, Mapzen Terrarium tiles and custom encodings.
 *
 * @see <a href="https://maplibre.org/maplibre-style-spec/#layers-color-relief">The online documentation</a>
 */
@UiThread
public class ColorReliefLayer extends Layer {

  /**
   * Creates a ColorReliefLayer.
   *
   * @param nativePtr pointer used by core
   */
  @Keep
  ColorReliefLayer(long nativePtr) {
    super(nativePtr);
  }

  /**
   * Creates a ColorReliefLayer.
   *
   * @param layerId  the id of the layer
   * @param sourceId the id of the source
   */
  public ColorReliefLayer(String layerId, String sourceId) {
    super();
    initialize(layerId, sourceId);
  }

  @Keep
  protected native void initialize(String layerId, String sourceId);

  /**
   * Set the source layer.
   *
   * @param sourceLayer the source layer to set
   */
  public void setSourceLayer(String sourceLayer) {
    checkThread();
    nativeSetSourceLayer(sourceLayer);
  }

  /**
   * Set the source Layer.
   *
   * @param sourceLayer the source layer to set
   * @return This
   */
  @NonNull
  public ColorReliefLayer withSourceLayer(String sourceLayer) {
    setSourceLayer(sourceLayer);
    return this;
  }

  /**
   * Get the source id.
   *
   * @return id of the source
   */
  @NonNull
  public String getSourceId() {
    checkThread();
    return nativeGetSourceId();
  }

  /**
   * Get the source layer.
   *
   * @return sourceLayer the source layer to get
   */
  @NonNull
  public String getSourceLayer() {
    checkThread();
    return nativeGetSourceLayer();
  }

  /**
   * Set a single expression filter.
   *
   * @param filter the expression filter to set
   */
  public void setFilter(@NonNull Expression filter) {
    checkThread();
    nativeSetFilter(filter.toArray());
  }

  /**
   * Set a single expression filter.
   *
   * @param filter the expression filter to set
   * @return This
   */
  @NonNull
  public ColorReliefLayer withFilter(@NonNull Expression filter) {
    setFilter(filter);
    return this;
  }

  /**
   * Get a single expression filter.
   *
   * @return the expression filter to get
   */
  @Nullable
  public Expression getFilter() {
    checkThread();
    JsonElement jsonElement = nativeGetFilter();
    if (jsonElement != null) {
      return Expression.Converter.convert(jsonElement);
    } else {
      return null;
    }
  }

  /**
   * Set a property or properties.
   *
   * @param properties the var-args properties
   * @return This
   */
  @NonNull
  public ColorReliefLayer withProperties(@NonNull PropertyValue<?>... properties) {
    setProperties(properties);
    return this;
  }

  // Property getters

  /**
   * Get the ColorReliefOpacity property
   *
   * @return property wrapper value around Float
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getColorReliefOpacity() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("color-relief-opacity", nativeGetColorReliefOpacity());
  }

  /**
   * Get the ColorReliefOpacity property transition options
   *
   * @return transition options for Float
   */
  @NonNull
  public TransitionOptions getColorReliefOpacityTransition() {
    checkThread();
    return nativeGetColorReliefOpacityTransition();
  }

  /**
   * Set the ColorReliefOpacity property transition options
   *
   * @param options transition options for Float
   */
  public void setColorReliefOpacityTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetColorReliefOpacityTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Get the ColorReliefColor property
   *
   * @return property wrapper value around String
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getColorReliefColor() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("color-relief-color", nativeGetColorReliefColor());
  }

  /**
   * Defines the color of each pixel based on its elevation. Should be an expression that uses `["elevation"]` as input.
   *
   * @return int representation of a rgba string color
   * @throws RuntimeException thrown if property isn't a value
   */
  @ColorInt
  public int getColorReliefColorAsInt() {
    checkThread();
    PropertyValue<String> value = getColorReliefColor();
    if (value.isValue()) {
      return rgbaToColor(value.getValue());
    } else {
      throw new RuntimeException("color-relief-color was set as a Function");
    }
  }

  @NonNull
  @Keep
  private native Object nativeGetColorReliefOpacity();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetColorReliefOpacityTransition();

  @Keep
  private native void nativeSetColorReliefOpacityTransition(long duration, long delay);

  @NonNull
  @Keep
  private native Object nativeGetColorReliefColor();

  @Override
  @Keep
  protected native void finalize() throws Throwable;

}
