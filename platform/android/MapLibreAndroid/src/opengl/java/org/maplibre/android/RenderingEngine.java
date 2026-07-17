package org.maplibre.android;

import android.content.Context;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

/**
 * The rendering engine baked into this build of the SDK.
 *
 * <p>In single-backend flavors (opengl, vulkan, ...) the engine is fixed at build
 * time and {@link #setCurrentType(Type)} only accepts the matching value. In the
 * multiBackend flavor this class is mutable and {@code setCurrentType(...)} must
 * be called before {@link MapLibre#getInstance(android.content.Context)} to pick
 * a backend for the process lifetime.</p>
 */
@Keep
public final class RenderingEngine {

  /** Supported rendering backends. WebGPU flavors report the closest match. */
  public enum Type {
    OPENGL,
    VULKAN
  }

  private static volatile Type currentType = Type.OPENGL;

  private RenderingEngine() {}

  /**
   * @return the rendering backend used by the currently loaded library.
   */
  @NonNull
  public static Type getCurrentType() {
    return currentType;
  }

  /**
   * This flavor's engine is fixed at compile time, so there's nothing to select —
   * always {@link Type#OPENGL}.
   */
  @NonNull
  static Type getDefaultRenderingEngine(@NonNull Context context) {
    return Type.OPENGL;
  }

  /**
   * Single-backend flavor: this is a no-op when {@code type} matches the
   * compiled-in backend, and throws otherwise. Use the multiBackend flavor of
   * the SDK if you need to switch backends at runtime.
   *
   * @param type the desired backend
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
    if (type != Type.OPENGL) {
      throw new UnsupportedOperationException(
        "This MapLibre Android build supports only " + Type.OPENGL
          + ". Use the multiBackend flavor to switch backends at runtime.");
    }
    currentType = type;
  }
}
