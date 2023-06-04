package org.maplibre.android;

import androidx.annotation.NonNull;

import org.maplibre.android.http.HttpRequest;
import org.maplibre.android.module.http.HttpRequestImpl;
import org.maplibre.android.module.loader.LibraryLoaderProviderImpl;

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
}
