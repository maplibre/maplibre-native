package org.maplibre.android.module.loader;

import org.maplibre.android.LibraryLoader;
import org.maplibre.android.LibraryLoaderProvider;

/**
 * OpenGL flavor: loads the single-backend libmaplibre.so via System.loadLibrary.
 */
public class LibraryLoaderProviderImpl implements LibraryLoaderProvider {

  @Override
  public LibraryLoader getDefaultLibraryLoader() {
    return new SystemLibraryLoader();
  }

  private static class SystemLibraryLoader extends LibraryLoader {
    @Override
    public void load(String name) {
      System.loadLibrary(name);
    }
  }
}
