// This file is generated. Edit scripts/generate-style-code.mjs, then run `make style-code`.

package org.maplibre.android.style.sky;

import androidx.annotation.ColorInt;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import org.maplibre.android.LibraryLoader;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.PropertyValue;
import org.maplibre.android.style.layers.TransitionOptions;
import org.maplibre.android.utils.ColorUtils;
import org.maplibre.android.utils.ThreadUtils;

/**
 * The map's sky configuration.
 *
 * <p>A {@code Sky} is a snapshot. After changing one of its properties, call
 * {@link org.maplibre.android.maps.Style#setSky(Sky)} to apply it to the style.</p>
 *
 * @see <a href="https://maplibre.org/maplibre-style-spec/sky/">The online documentation</a>
 */
@UiThread
public class Sky {

  private static final String TAG = "Mbgl-Sky";

  static {
    LibraryLoader.load();
  }

  @Keep
  private long nativePtr;

  /**
   * Creates a sky configuration with no explicitly set properties.
   * Unspecified properties resolve to their style specification defaults when applied.
   */
  public Sky() {
    checkThread();
    initialize();
  }

  /**
   * Creates a sky configuration backed by a native snapshot.
   *
   * @param nativePtr pointer used by core
   */
  @Keep
  Sky(long nativePtr) {
    checkThread();
    this.nativePtr = nativePtr;
  }

  /**
   * Sets sky-color. The base color for the sky.
   *
   * @param value an Android color integer
   */
  public void setSkyColor(@ColorInt int value) {
    setSkyColor(ColorUtils.colorToRgbaString(value));
  }

  /**
   * Sets sky-color. The base color for the sky.
   *
   * @param value a CSS color string
   */
  public void setSkyColor(@NonNull String value) {
    setProperty("sky-color", value);
  }

  /**
   * Sets sky-color from a zoom expression. The base color for the sky.
   *
   * @param expression a zoom expression that evaluates to a color
   */
  public void setSkyColor(@NonNull Expression expression) {
    setProperty("sky-color", expression);
  }

  /**
   * Gets sky-color.
   *
   * @return a property value containing a CSS color string or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getSkyColor() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("sky-color", nativeGetSkyColor());
  }

  /**
   * Gets sky-color as an Android color integer.
   *
   * @return the Android color integer
   * @throws IllegalStateException if the property is an expression or undefined
   */
  @ColorInt
  public int getSkyColorAsInt() {
    PropertyValue<String> property = getSkyColor();
    if (!property.isValue()) {
      throw new IllegalStateException("sky-color is not a constant value");
    }
    return ColorUtils.rgbaToColor(property.getValue());
  }

  /** Gets the transition options for sky-color. */
  @NonNull
  public TransitionOptions getSkyColorTransition() {
    checkThread();
    return nativeGetSkyColorTransition();
  }

  /** Sets the transition options for sky-color. */
  public void setSkyColorTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetSkyColorTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Sets horizon-color. The base color at the horizon.
   *
   * @param value an Android color integer
   */
  public void setHorizonColor(@ColorInt int value) {
    setHorizonColor(ColorUtils.colorToRgbaString(value));
  }

  /**
   * Sets horizon-color. The base color at the horizon.
   *
   * @param value a CSS color string
   */
  public void setHorizonColor(@NonNull String value) {
    setProperty("horizon-color", value);
  }

  /**
   * Sets horizon-color from a zoom expression. The base color at the horizon.
   *
   * @param expression a zoom expression that evaluates to a color
   */
  public void setHorizonColor(@NonNull Expression expression) {
    setProperty("horizon-color", expression);
  }

  /**
   * Gets horizon-color.
   *
   * @return a property value containing a CSS color string or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getHorizonColor() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("horizon-color", nativeGetHorizonColor());
  }

  /**
   * Gets horizon-color as an Android color integer.
   *
   * @return the Android color integer
   * @throws IllegalStateException if the property is an expression or undefined
   */
  @ColorInt
  public int getHorizonColorAsInt() {
    PropertyValue<String> property = getHorizonColor();
    if (!property.isValue()) {
      throw new IllegalStateException("horizon-color is not a constant value");
    }
    return ColorUtils.rgbaToColor(property.getValue());
  }

  /** Gets the transition options for horizon-color. */
  @NonNull
  public TransitionOptions getHorizonColorTransition() {
    checkThread();
    return nativeGetHorizonColorTransition();
  }

  /** Sets the transition options for horizon-color. */
  public void setHorizonColorTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetHorizonColorTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Sets fog-color. The base color for the fog. Requires 3D terrain.
   *
   * @param value an Android color integer
   */
  public void setFogColor(@ColorInt int value) {
    setFogColor(ColorUtils.colorToRgbaString(value));
  }

  /**
   * Sets fog-color. The base color for the fog. Requires 3D terrain.
   *
   * @param value a CSS color string
   */
  public void setFogColor(@NonNull String value) {
    setProperty("fog-color", value);
  }

  /**
   * Sets fog-color from a zoom expression. The base color for the fog. Requires 3D terrain.
   *
   * @param expression a zoom expression that evaluates to a color
   */
  public void setFogColor(@NonNull Expression expression) {
    setProperty("fog-color", expression);
  }

  /**
   * Gets fog-color.
   *
   * @return a property value containing a CSS color string or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<String> getFogColor() {
    checkThread();
    return (PropertyValue<String>) new PropertyValue("fog-color", nativeGetFogColor());
  }

  /**
   * Gets fog-color as an Android color integer.
   *
   * @return the Android color integer
   * @throws IllegalStateException if the property is an expression or undefined
   */
  @ColorInt
  public int getFogColorAsInt() {
    PropertyValue<String> property = getFogColor();
    if (!property.isValue()) {
      throw new IllegalStateException("fog-color is not a constant value");
    }
    return ColorUtils.rgbaToColor(property.getValue());
  }

  /** Gets the transition options for fog-color. */
  @NonNull
  public TransitionOptions getFogColorTransition() {
    checkThread();
    return nativeGetFogColorTransition();
  }

  /** Sets the transition options for fog-color. */
  public void setFogColorTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetFogColorTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Sets fog-ground-blend. How to blend the fog over the 3D terrain. Where 0 is the map center and 1 is the horizon.
   *
   * @param value a value between 0 and 1
   */
  public void setFogGroundBlend(float value) {
    setProperty("fog-ground-blend", value);
  }

  /**
   * Sets fog-ground-blend from a zoom expression. How to blend the fog over the 3D terrain. Where 0 is the map center and 1 is the horizon.
   *
   * @param expression a zoom expression that evaluates to a number
   */
  public void setFogGroundBlend(@NonNull Expression expression) {
    setProperty("fog-ground-blend", expression);
  }

  /**
   * Gets fog-ground-blend.
   *
   * @return a property value containing a float or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getFogGroundBlend() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("fog-ground-blend", nativeGetFogGroundBlend());
  }

  /** Gets the transition options for fog-ground-blend. */
  @NonNull
  public TransitionOptions getFogGroundBlendTransition() {
    checkThread();
    return nativeGetFogGroundBlendTransition();
  }

  /** Sets the transition options for fog-ground-blend. */
  public void setFogGroundBlendTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetFogGroundBlendTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Sets horizon-fog-blend. How to blend the fog color and the horizon color. Where 0 is using the horizon color only and 1 is using the fog color only.
   *
   * @param value a value between 0 and 1
   */
  public void setHorizonFogBlend(float value) {
    setProperty("horizon-fog-blend", value);
  }

  /**
   * Sets horizon-fog-blend from a zoom expression. How to blend the fog color and the horizon color. Where 0 is using the horizon color only and 1 is using the fog color only.
   *
   * @param expression a zoom expression that evaluates to a number
   */
  public void setHorizonFogBlend(@NonNull Expression expression) {
    setProperty("horizon-fog-blend", expression);
  }

  /**
   * Gets horizon-fog-blend.
   *
   * @return a property value containing a float or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getHorizonFogBlend() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("horizon-fog-blend", nativeGetHorizonFogBlend());
  }

  /** Gets the transition options for horizon-fog-blend. */
  @NonNull
  public TransitionOptions getHorizonFogBlendTransition() {
    checkThread();
    return nativeGetHorizonFogBlendTransition();
  }

  /** Sets the transition options for horizon-fog-blend. */
  public void setHorizonFogBlendTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetHorizonFogBlendTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Sets sky-horizon-blend. How to blend the sky color and the horizon color. Where 1 is blending the color at the middle of the sky and 0 is not blending at all and using the sky color only.
   *
   * @param value a value between 0 and 1
   */
  public void setSkyHorizonBlend(float value) {
    setProperty("sky-horizon-blend", value);
  }

  /**
   * Sets sky-horizon-blend from a zoom expression. How to blend the sky color and the horizon color. Where 1 is blending the color at the middle of the sky and 0 is not blending at all and using the sky color only.
   *
   * @param expression a zoom expression that evaluates to a number
   */
  public void setSkyHorizonBlend(@NonNull Expression expression) {
    setProperty("sky-horizon-blend", expression);
  }

  /**
   * Gets sky-horizon-blend.
   *
   * @return a property value containing a float or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getSkyHorizonBlend() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("sky-horizon-blend", nativeGetSkyHorizonBlend());
  }

  /** Gets the transition options for sky-horizon-blend. */
  @NonNull
  public TransitionOptions getSkyHorizonBlendTransition() {
    checkThread();
    return nativeGetSkyHorizonBlendTransition();
  }

  /** Sets the transition options for sky-horizon-blend. */
  public void setSkyHorizonBlendTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetSkyHorizonBlendTransition(options.getDuration(), options.getDelay());
  }

  /**
   * Sets atmosphere-blend. How to blend the atmosphere. Where 1 is visible atmosphere and 0 is hidden. It is best to interpolate this expression when using globe projection.
   *
   * @param value a value between 0 and 1
   */
  public void setAtmosphereBlend(float value) {
    setProperty("atmosphere-blend", value);
  }

  /**
   * Sets atmosphere-blend from a zoom expression. How to blend the atmosphere. Where 1 is visible atmosphere and 0 is hidden. It is best to interpolate this expression when using globe projection.
   *
   * @param expression a zoom expression that evaluates to a number
   */
  public void setAtmosphereBlend(@NonNull Expression expression) {
    setProperty("atmosphere-blend", expression);
  }

  /**
   * Gets atmosphere-blend.
   *
   * @return a property value containing a float or expression
   */
  @NonNull
  @SuppressWarnings("unchecked")
  public PropertyValue<Float> getAtmosphereBlend() {
    checkThread();
    return (PropertyValue<Float>) new PropertyValue("atmosphere-blend", nativeGetAtmosphereBlend());
  }

  /** Gets the transition options for atmosphere-blend. */
  @NonNull
  public TransitionOptions getAtmosphereBlendTransition() {
    checkThread();
    return nativeGetAtmosphereBlendTransition();
  }

  /** Sets the transition options for atmosphere-blend. */
  public void setAtmosphereBlendTransition(@NonNull TransitionOptions options) {
    checkThread();
    nativeSetAtmosphereBlendTransition(options.getDuration(), options.getDelay());
  }

  private void setProperty(@NonNull String name, @NonNull Object value) {
    checkThread();
    nativeSetProperty(name, value instanceof Expression ? ((Expression) value).toArray() : value);
  }

  private void checkThread() {
    ThreadUtils.checkThread(TAG);
  }

  @Keep
  private native void initialize();

  @Keep
  private native void nativeSetProperty(String name, Object value);

  @Nullable
  @Keep
  private native Object nativeGetSkyColor();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetSkyColorTransition();

  @Keep
  private native void nativeSetSkyColorTransition(long duration, long delay);

  @Nullable
  @Keep
  private native Object nativeGetHorizonColor();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetHorizonColorTransition();

  @Keep
  private native void nativeSetHorizonColorTransition(long duration, long delay);

  @Nullable
  @Keep
  private native Object nativeGetFogColor();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetFogColorTransition();

  @Keep
  private native void nativeSetFogColorTransition(long duration, long delay);

  @Nullable
  @Keep
  private native Object nativeGetFogGroundBlend();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetFogGroundBlendTransition();

  @Keep
  private native void nativeSetFogGroundBlendTransition(long duration, long delay);

  @Nullable
  @Keep
  private native Object nativeGetHorizonFogBlend();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetHorizonFogBlendTransition();

  @Keep
  private native void nativeSetHorizonFogBlendTransition(long duration, long delay);

  @Nullable
  @Keep
  private native Object nativeGetSkyHorizonBlend();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetSkyHorizonBlendTransition();

  @Keep
  private native void nativeSetSkyHorizonBlendTransition(long duration, long delay);

  @Nullable
  @Keep
  private native Object nativeGetAtmosphereBlend();

  @NonNull
  @Keep
  private native TransitionOptions nativeGetAtmosphereBlendTransition();

  @Keep
  private native void nativeSetAtmosphereBlendTransition(long duration, long delay);

  @Override
  @Keep
  protected native void finalize() throws Throwable;
}
