package org.maplibre.android.module.loader;

import org.maplibre.android.LibraryLoader;
import org.maplibre.android.LibraryLoaderProvider;

/**
 * Concrete implementation of a native library loader.
 * <p>
 * </p>
 */
public class LibraryLoaderProviderImpl implements LibraryLoaderProvider {

  /**
   * Creates and returns a the default Library Loader.
   *
   * @return the default library loader
   */
  @Override
  public LibraryLoader getDefaultLibraryLoader() {
    return new SystemLibraryLoader();
  }

  /**
   * Concrete implementation of a LibraryLoader using System.loadLibrary.
   */
  private static class SystemLibraryLoader extends LibraryLoader {
    @Override
    public void load(String name) {
      System.loadLibrary(name);
    }
  }
}
