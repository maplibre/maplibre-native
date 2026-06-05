package org.maplibre.android;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

/**
 * multiBackend flavor: the rendering engine is selectable at runtime.
 *
 * <p>Call {@link #setCurrentType(Type)} before the first call to
 * {@link MapLibre#getInstance(android.content.Context)} (or any other path that
 * triggers native library loading). Once the native library is loaded the
 * selection is effectively locked for the process lifetime — subsequent
 * {@code setCurrentType} calls update this field, but the loaded .so does
 * not change.</p>
 */
@Keep
public final class RenderingEngine {

  public enum Type {
    OPENGL,
    VULKAN
  }

  private static volatile Type currentType = Type.OPENGL;

  private RenderingEngine() {}

  @NonNull
  public static Type getCurrentType() {
    return currentType;
  }

  public static void setCurrentType(@NonNull Type type) {
    currentType = type;
  }
}
