package org.maplibre.android.net;

import androidx.annotation.Keep;

import org.maplibre.android.LibraryLoader;

/**
 * Updates the native library's connectivity state
 */
class NativeConnectivityListener implements ConnectivityListener {

  static {
    LibraryLoader.load();
  }

  @Keep
  private long nativePtr;
  @Keep
  private boolean invalidated;

  @Keep
  NativeConnectivityListener(long nativePtr) {
    this.nativePtr = nativePtr;
  }

  NativeConnectivityListener() {
    initialize();
  }

  @Override
  public void onNetworkStateChanged(boolean connected) {
    nativeOnConnectivityStateChanged(connected);
  }

  @Keep
  protected native void nativeOnConnectivityStateChanged(boolean connected);

  @Keep
  protected native void initialize();

  @Override
  @Keep
  protected native void finalize() throws Throwable;
}
