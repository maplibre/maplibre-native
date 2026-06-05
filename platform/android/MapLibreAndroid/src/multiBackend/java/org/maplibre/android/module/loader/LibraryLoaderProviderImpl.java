package org.maplibre.android.module.loader;

import org.maplibre.android.LibraryLoader;
import org.maplibre.android.LibraryLoaderProvider;
import org.maplibre.android.RenderingEngine;

/**
 * multiBackend flavor: when asked to load "maplibre", routes to either
 * libmaplibre.so (OpenGL, the default name) or libmaplibre-vulkan.so based on
 * {@link RenderingEngine#getCurrentType()}. Any other library name is loaded
 * verbatim.
 */
public class LibraryLoaderProviderImpl implements LibraryLoaderProvider {

  @Override
  public LibraryLoader getDefaultLibraryLoader() {
    return new MultiBackendLibraryLoader();
  }

  private static class MultiBackendLibraryLoader extends LibraryLoader {
    @Override
    public void load(String name) {
      if ("maplibre".equals(name)
          && RenderingEngine.getCurrentType() == RenderingEngine.Type.VULKAN) {
        System.loadLibrary("maplibre-vulkan");
      } else {
        System.loadLibrary(name);
      }
    }
  }
}
