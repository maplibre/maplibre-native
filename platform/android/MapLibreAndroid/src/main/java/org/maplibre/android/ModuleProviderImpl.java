package org.maplibre.android;

import android.content.Context;
import androidx.annotation.NonNull;

import org.maplibre.android.http.HttpRequest;
import org.maplibre.android.module.http.HttpRequestImpl;
import org.maplibre.android.module.loader.LibraryLoaderProviderImpl;
import org.maplibre.android.maps.NativeMap;
import org.maplibre.android.maps.NativeMapView;
import org.maplibre.android.maps.NativeMapView.ViewCallback;
import org.maplibre.android.maps.NativeMapView.StateCallback;
import org.maplibre.android.maps.renderer.MapRenderer;

public class ModuleProviderImpl implements ModuleProvider {

  @Override
  @NonNull
  public HttpRequest createHttpRequest() {
    return new HttpRequestImpl();
  }

  @NonNull
  @Override
  public LibraryLoaderProvider createLibraryLoaderProvider() {
    return new LibraryLoaderProviderImpl();
  }

  @NonNull
  @Override
  public NativeMap createNativeMapView(Context context, float pixelRatio, boolean crossSourceCollisions,
                                       ViewCallback viewCallback, StateCallback stateCallback,
                                       MapRenderer mapRenderer) {
    return new NativeMapView(context, pixelRatio, crossSourceCollisions, viewCallback, stateCallback, mapRenderer);
  }
}
