package org.maplibre.android;

import android.content.Context;
import androidx.annotation.NonNull;

import org.maplibre.android.http.HttpRequest;
import org.maplibre.android.maps.NativeMap;
import org.maplibre.android.maps.NativeMapView.ViewCallback;
import org.maplibre.android.maps.NativeMapView.StateCallback;
import org.maplibre.android.maps.renderer.MapRenderer;

/**
 * Injects concrete instances of configurable abstractions
 */
public interface ModuleProvider {

  /**
   * Create a new concrete implementation of HttpRequest.
   *
   * @return a new instance of an HttpRequest
   */
  @NonNull
  HttpRequest createHttpRequest();

  /**
   * Get the concrete implementation of LibraryLoaderProvider
   *
   * @return a new instance of LibraryLoaderProvider
   */
  @NonNull
  LibraryLoaderProvider createLibraryLoaderProvider();

  /**
   * Create and return a new NativeMap view
   *
   * @return a new instance implementing NativeMap
   */
  @NonNull
  NativeMap createNativeMapView(Context context, float pixelRatio, boolean crossSourceCollisions,
                                ViewCallback viewCallback, StateCallback stateCallback, MapRenderer mapRenderer);
}
