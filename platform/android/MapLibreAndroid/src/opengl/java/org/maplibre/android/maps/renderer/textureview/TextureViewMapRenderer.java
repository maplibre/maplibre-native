package org.maplibre.android.maps.renderer.textureview;

import android.content.Context;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.MapRenderer;

/**
 * The {@link TextureViewMapRenderer} encapsulates the GL thread and
 * {@link TextureView} specifics to render the map.
 *
 * @see MapRenderer
 */
public class TextureViewMapRenderer extends MapRenderer {
  private TextureViewRenderThread renderThread;
  private boolean translucentSurface;
  private TextureView textureView;

  /**
   * Create a {@link MapRenderer} for the given {@link TextureView}
   *
   * @param context                  the current Context
   * @param textureView              the TextureView
   * @param localIdeographFontFamily the local font family
   * @param translucentSurface    the translucency flag
   */
  public TextureViewMapRenderer(@NonNull Context context,
                                @NonNull TextureView textureView,
                                String localIdeographFontFamily,
                                boolean translucentSurface) {
    super(context, localIdeographFontFamily);
    this.textureView = textureView;
    this.translucentSurface = translucentSurface;

  }

  public void setRenderThread(TextureViewRenderThread thread) {
    renderThread = thread;
    renderThread.setName("TextureViewRenderer");
    renderThread.start();
  }

  @Override
  public View getView() {
    return this.textureView;
  }

  /**
   * Overridden to provide package access
   */
  @Override
  protected void onSurfaceCreated(Surface surface) {
    super.onSurfaceCreated(surface);
  }

  /**
   * Overridden to provide package access
   */
  @Override
  protected void onSurfaceChanged(int width, int height) {
    super.onSurfaceChanged(width, height);
  }

  /**
   * Overridden to provide package access
   */
  @Override
  protected void onSurfaceDestroyed() {
    super.onSurfaceDestroyed();
  }

  /**
   * Overridden to provide package access
   */
  @Override
  protected void onDrawFrame() {
    super.onDrawFrame();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void requestRender() {
    renderThread.requestRender();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void queueEvent(Runnable runnable) {
    renderThread.queueEvent(runnable);
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void waitForEmpty() {
    renderThread.waitForEmpty();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void onStop() {
    renderThread.onPause();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void onStart() {
    renderThread.onResume();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public void onDestroy() {
    renderThread.onDestroy();
  }

  public boolean isTranslucentSurface() {
    return translucentSurface;
  }
}
