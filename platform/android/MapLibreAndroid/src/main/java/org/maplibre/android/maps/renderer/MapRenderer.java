package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;

import androidx.annotation.CallSuper;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import org.maplibre.android.LibraryLoader;
import org.maplibre.android.log.Logger;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.maps.MapLibreMapOptions;

/**
 * The {@link MapRenderer} encapsulates the render thread.
 * <p>
 * Performs actions on the render thread to manage the resources and
 * render on the one end and acts as a scheduler to request work to
 * be performed on the render thread on the other.
 */
@Keep
public abstract class MapRenderer implements MapRendererScheduler {

  static {
    LibraryLoader.load();
  }

  private static final String TAG = "Mbgl-MapRenderer";

  // Holds the pointer to the native peer after initialisation
  private long nativePtr = 0;
  private double expectedRenderTime = 0;
  private MapLibreMap.OnFpsChangedListener onFpsChangedListener;

  public static MapRenderer create(MapLibreMapOptions options, @NonNull Context context, Runnable initCallback) {

    MapRenderer renderer = null;
    String localFontFamily = options.getLocalIdeographFontFamily();

    if (options.getTextureMode()) {
      TextureView textureView = new TextureView(context);
      boolean translucentSurface = options.getTranslucentTextureSurface();
      renderer = MapRendererFactory.newTextureViewMapRenderer(context, textureView, localFontFamily,
              translucentSurface, initCallback);
    } else {
      boolean renderSurfaceOnTop = options.getRenderSurfaceOnTop();
      renderer = MapRendererFactory.newSurfaceViewMapRenderer(context, localFontFamily,
              renderSurfaceOnTop, initCallback);
    }

    return renderer;
  }

  public MapRenderer(@NonNull Context context, String localIdeographFontFamily) {
    float pixelRatio = context.getResources().getDisplayMetrics().density;

    // Initialise native peer
    nativeInitialize(this, pixelRatio, localIdeographFontFamily);
  }

  public abstract View getView();

  public void onStart() {
    // Implement if needed
  }

  public void onPause() {
    // Implement if needed
  }

  public void onResume() {
    // Implement if needed
  }

  public void onStop() {
    // Implement if needed
  }

  public void onDestroy() {
    // Implement if needed
  }

  public void setOnFpsChangedListener(MapLibreMap.OnFpsChangedListener listener) {
    onFpsChangedListener = listener;
  }

  @CallSuper
  protected void onSurfaceCreated(Surface surface) {
    nativeOnSurfaceCreated(surface);
  }

  @CallSuper
  protected void onSurfaceChanged(int width, int height) {
    nativeOnSurfaceChanged(width, height);
  }

  @CallSuper
  protected void onSurfaceDestroyed() {
    nativeOnSurfaceDestroyed();
  }

  @CallSuper
  protected void onDrawFrame() {
    long startTime = System.nanoTime();
    try {
      nativeRender();
    } catch (java.lang.Error error) {
      Logger.e(TAG, error.getMessage());
    }
    long renderTime = System.nanoTime() - startTime;
    if (renderTime < expectedRenderTime) {
      try {
        Thread.sleep((long) ((expectedRenderTime - renderTime) / 1E6));
      } catch (InterruptedException ex) {
        Logger.e(TAG, ex.getMessage());
      }
    }
    if (onFpsChangedListener != null) {
      updateFps();
    }
  }

  public void setSwapBehaviorFlush(boolean flush) {
    nativeSetSwapBehaviorFlush(flush);
  }

  /**
   * May be called from any thread.
   * <p>
   * Called from the native peer to schedule work on the GL
   * thread. Explicit override for easier to read jni code.
   *
   * @param runnable the runnable to execute
   * @see MapRendererRunnable
   */
  @CallSuper
  void queueEvent(MapRendererRunnable runnable) {
    this.queueEvent((Runnable) runnable);
  }

  private native void nativeInitialize(MapRenderer self,
                                       float pixelRatio,
                                       String localIdeographFontFamily);

  @CallSuper
  @Override
  protected native void finalize() throws Throwable;

  private native void nativeOnSurfaceCreated(Surface surface);

  private native void nativeOnSurfaceChanged(int width, int height);

  private native void nativeOnSurfaceDestroyed();

  protected native void nativeReset();

  private native void nativeRender();

  private native void nativeSetSwapBehaviorFlush(boolean flush);

  private long timeElapsed;

  private void updateFps() {
    long currentTime = System.nanoTime();
    if (timeElapsed > 0) {
      double fps = 1E9 / ((currentTime - timeElapsed));
      onFpsChangedListener.onFpsChanged(fps);
    }
    timeElapsed = currentTime;
  }

  /**
   * The max frame rate at which this render is rendered,
   * but it can't excess the ability of device hardware.
   *
   * @param maximumFps Can be set to arbitrary integer values.
   */
  public void setMaximumFps(int maximumFps) {
    if (maximumFps <= 0) {
      // Not valid, just return
      return;
    }
    expectedRenderTime = 1E9 / maximumFps;
  }
}
