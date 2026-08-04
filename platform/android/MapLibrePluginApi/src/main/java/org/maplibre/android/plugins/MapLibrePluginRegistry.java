package org.maplibre.android.plugins;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/** Process-wide MapLibre native plugin registration and inspection API. */
@Keep
public final class MapLibrePluginRegistry {

  private MapLibrePluginRegistry() {
  }

  /** Loads the renderer-selected MapLibre native library through its configured loader. */
  public static void ensureMapLibreLoaded() {
    try {
      Class<?> loader = Class.forName("org.maplibre.android.LibraryLoader");
      loader.getMethod("load").invoke(null);
    } catch (ClassNotFoundException | NoSuchMethodException | IllegalAccessException error) {
      throw new IllegalStateException("A compatible MapLibre Android renderer is not installed", error);
    } catch (InvocationTargetException error) {
      Throwable cause = error.getCause();
      if (cause instanceof RuntimeException) throw (RuntimeException) cause;
      if (cause instanceof Error) throw (Error) cause;
      throw new IllegalStateException("MapLibre Android could not be loaded", cause);
    }
  }

  public static boolean isRegistered(@NonNull String pluginId) {
    ensureMapLibreLoaded();
    return nativeIsRegistered(pluginId);
  }

  @NonNull
  public static List<String> registeredPluginIds() {
    ensureMapLibreLoaded();
    int count = nativeCount();
    List<String> result = new ArrayList<>(count);
    for (int i = 0; i < count; i++) {
      result.add(nativeIdAt(i));
    }
    return Collections.unmodifiableList(result);
  }

  /**
   * Returns the process-lifetime address of the v1 C registration function.
   * Android plugin wrappers pass this value to their own JNI entry point so
   * neither DSO needs to link against the other's C++ runtime.
   */
  public static long registrationFunctionAddress() {
    ensureMapLibreLoaded();
    return nativeRegistrationFunctionAddress();
  }

  private static native boolean nativeIsRegistered(String pluginId);

  private static native int nativeCount();

  private static native String nativeIdAt(int index);

  private static native long nativeRegistrationFunctionAddress();
}
