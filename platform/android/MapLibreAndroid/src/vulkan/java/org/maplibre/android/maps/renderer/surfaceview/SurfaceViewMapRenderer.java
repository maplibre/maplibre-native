package org.maplibre.android.maps.renderer.surfaceview;

import android.content.Context;
import android.view.View;

import androidx.annotation.NonNull;
import org.maplibre.android.maps.renderer.MapRenderer;

public class SurfaceViewMapRenderer extends MapRenderer {

  @NonNull
  private final MapLibreSurfaceView surfaceView;

  public SurfaceViewMapRenderer(Context context,
                                @NonNull MapLibreSurfaceView surfaceView,
                                String localIdeographFontFamily) {
    super(context, localIdeographFontFamily);
    this.surfaceView = surfaceView;
    this.surfaceView.setRenderer(this);
    this.surfaceView.setDetachedListener(new MapLibreSurfaceView.OnSurfaceViewDetachedListener() {
      @Override
      public void onSurfaceViewDetached() {
        // because the render thread is destroyed when the view is detached from window,
        // we need to ensure releasing the native renderer as well.
        // This avoids releasing it only when the view is being recreated, which is already on a new thread,
        // and leads to JNI crashes like https://github.com/mapbox/mapbox-gl-native/issues/14618
        nativeReset();
      }
    });
  }

  @Override
  public View getView() {
    return this.surfaceView;
  }

  @Override
  public void onStop() {
    surfaceView.onPause();
  }

  @Override
  public void onPause() {
    super.onPause();
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
  }

  @Override
  public void onStart() {
    surfaceView.onResume();
  }

  @Override
  public void onResume() {
    super.onResume();
  }

  public void onSurfaceCreated() {
    super.onSurfaceCreated(this.surfaceView.getHolder().getSurface());
  }

  @Override
  protected void onSurfaceDestroyed() {
    super.onSurfaceDestroyed();
  }

  @Override
  public void onSurfaceChanged(int width, int height) {
    super.onSurfaceChanged(width, height);
  }

  @Override
  public void onDrawFrame() {
    super.onDrawFrame();
  }

  /**
   * May be called from any thread.
   * <p>
   * Called from the renderer frontend to schedule a render.
   */
  @Override
  public void requestRender() {
    surfaceView.requestRender();
  }

  /**
   * May be called from any thread.
   * <p>
   * Schedules work to be performed on the MapRenderer thread.
   *
   * @param runnable the runnable to execute
   */
  @Override
  public void queueEvent(Runnable runnable) {
    surfaceView.queueEvent(runnable);
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void waitForEmpty() {
    surfaceView.waitForEmpty();
  }
}