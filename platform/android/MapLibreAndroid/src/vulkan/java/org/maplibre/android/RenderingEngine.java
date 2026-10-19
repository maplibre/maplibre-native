package org.maplibre.android;

import android.content.Context;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

/**
 * Vulkan (and WebGPU) flavor: the rendering engine is fixed at build time.
 * See the opengl flavor's javadoc for the multiBackend contrast.
 */
@Keep
public final class RenderingEngine {

  public enum Type {
    OPENGL,
    VULKAN
  }

  private static volatile Type currentType = Type.VULKAN;

  private RenderingEngine() {}

  @NonNull
  public static Type getCurrentType() {
    return currentType;
  }

  /**
   * This flavor's engine is fixed at compile time, so there's nothing to select —
   * always {@link Type#VULKAN}.
   */
  @NonNull
  static Type getDefaultRenderingEngine(@NonNull Context context) {
    return Type.VULKAN;
  }

  /**
   * @throws IllegalStateException if {@link MapLibre#getInstance(android.content.Context)}
   *     has already been called in this process.
   * @throws UnsupportedOperationException if {@code type} differs from the
   *     compiled-in backend
   */
  static void setCurrentType(@NonNull Type type) {
    if (MapLibre.hasInstance()) {
      throw new IllegalStateException(
        "RenderingEngine.setCurrentType() must be called before MapLibre.getInstance().");
    }
    if (type != Type.VULKAN) {
      throw new UnsupportedOperationException(
        "This MapLibre Android build supports only " + Type.VULKAN
          + ". Use the multiBackend flavor to switch backends at runtime.");
    }
    currentType = type;
  }
}
