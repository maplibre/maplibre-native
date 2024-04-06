package org.maplibre.android;

import org.maplibre.android.log.Logger;

/**
 * Loads the mapbox-gl shared library
 * <p>
 * By default uses System.loadLibrary
 * use {@link #setLibraryLoader(LibraryLoader)} to provide an alternative library loading hook.
 * </p>
 */
public abstract class LibraryLoader {

  private static final String TAG = "Mbgl-LibraryLoader";

  private static final LibraryLoader DEFAULT = MapLibre.getModuleProvider()
    .createLibraryLoaderProvider()
    .getDefaultLibraryLoader();

  private static volatile LibraryLoader loader = DEFAULT;

  private static boolean loaded;

  /**
   * Set the library loader that loads the shared library.
   *
   * @param libraryLoader the library loader
   */
  public static void setLibraryLoader(LibraryLoader libraryLoader) {
    loader = libraryLoader;
  }

  /**
   * Loads "libmaplibre.so" native shared library.
   * <p>
   * Catches UnsatisfiedLinkErrors and prints a warning to logcat.
   * </p>
   */
  public static synchronized void load() {
    try {
      if (!loaded) {
        loaded = true;
        loader.load("maplibre");
      }
    } catch (UnsatisfiedLinkError error) {
      loaded = false;
      String message = "Failed to load native shared library.";
      Logger.e(TAG, message, error);
      MapStrictMode.strictModeViolation(message, error);
    }
  }

  public abstract void load(String name);
}

