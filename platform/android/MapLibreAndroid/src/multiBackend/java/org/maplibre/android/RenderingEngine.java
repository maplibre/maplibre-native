package org.maplibre.android;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

/**
 * multiBackend flavor: the rendering engine is selectable at runtime.
 *
 * <p>Call {@link #setCurrentType(Type)} before the first call to
 * {@link MapLibre#getInstance(android.content.Context)} (or any other path that
 * triggers native library loading). {@link #setCurrentType(Type)} throws once
 * {@link MapLibre#getInstance(android.content.Context)} has been called, since the
 * native library is loaded by then and the selection can no longer take effect for
 * the current process.</p>
 */
@Keep
public final class RenderingEngine {

  /**
   * Matches the android.hardware.vulkan.version <uses-feature> declared for this flavor.
   */
  private static final int REQUIRED_VULKAN_VERSION = 0x400003;

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
   * @throws IllegalStateException if {@link MapLibre#getInstance(android.content.Context)}
   *     has already been called in this process.
   */
  static void setCurrentType(@NonNull Type type) {
    if (MapLibre.hasInstance()) {
      throw new IllegalStateException(
        "RenderingEngine.setCurrentType() must be called before MapLibre.getInstance().");
    }
    currentType = type;
  }

  /**
   * Determines which rendering engine to use when none was explicitly requested:
   * Vulkan if this device's hardware supports it, OpenGL otherwise.
   */
  @NonNull
  static Type getDefaultRenderingEngine(@NonNull Context context) {
    return isVulkanSupported(context) ? Type.VULKAN : Type.OPENGL;
  }

  private static boolean isVulkanSupported(@NonNull Context context) {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
      return false;
    }
    return context.getPackageManager().hasSystemFeature(
      PackageManager.FEATURE_VULKAN_HARDWARE_VERSION, REQUIRED_VULKAN_VERSION);
  }
}
