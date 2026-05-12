package org.maplibre.android;

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

  private RenderingEngine() {}

  @NonNull
  public static Type getCurrentType() {
    return Type.VULKAN;
  }

  public static void setCurrentType(@NonNull Type type) {
    if (type != Type.VULKAN) {
      throw new UnsupportedOperationException(
        "This MapLibre Android build supports only " + Type.VULKAN
          + ". Use the multiBackend flavor to switch backends at runtime.");
    }
  }
}
