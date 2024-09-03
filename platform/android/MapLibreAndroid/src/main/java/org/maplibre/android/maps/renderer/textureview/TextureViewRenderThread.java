package org.maplibre.android.maps.renderer.textureview;

import android.graphics.SurfaceTexture;
import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import java.util.ArrayList;

/**
 * The render thread is responsible for managing the communication between the
 * ui thread and the render thread it creates
 */
abstract class TextureViewRenderThread extends Thread implements TextureView.SurfaceTextureListener {

  protected static final String TAG = "Mbgl-TextureViewRenderThread";

  @NonNull
  protected final TextureViewMapRenderer mapRenderer;

  // Lock used for synchronization
  protected final Object lock = new Object();

  // Guarded by lock
  protected final ArrayList<Runnable> eventQueue = new ArrayList<>();
  @Nullable
  protected SurfaceTexture surfaceTexture;
  protected boolean hasNativeSurface;
  protected int width;
  protected int height;
  protected boolean requestRender;
  protected boolean sizeChanged;
  protected boolean paused;
  protected boolean destroySurface;
  protected boolean shouldExit;
  protected boolean exited;

  /**
   * Create a render thread for the given TextureView / Maprenderer combination.
   *
   * @param textureView the TextureView
   * @param mapRenderer the MapRenderer
   */
  @UiThread
  TextureViewRenderThread(@NonNull TextureView textureView, @NonNull TextureViewMapRenderer mapRenderer) {
    textureView.setOpaque(!mapRenderer.isTranslucentSurface());
    textureView.setSurfaceTextureListener(this);
    this.mapRenderer = mapRenderer;
  }

  // SurfaceTextureListener methods

  @UiThread
  @Override
  public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture, final int width, final int height) {
    synchronized (lock) {
      this.surfaceTexture = surfaceTexture;
      this.width = width;
      this.height = height;
      this.requestRender = true;
      lock.notifyAll();
    }
  }

  @Override
  @UiThread
  public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, final int width, final int height) {
    synchronized (lock) {
      this.width = width;
      this.height = height;
      this.sizeChanged = true;
      this.requestRender = true;
      lock.notifyAll();
    }
  }

  @Override
  @UiThread
  public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
    synchronized (lock) {
      this.surfaceTexture = null;
      this.destroySurface = true;
      this.requestRender = false;
      lock.notifyAll();
    }
    return true;
  }

  @Override
  @UiThread
  public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {
    // Ignored
  }

  // MapRenderer delegate methods

  /**
   * May be called from any thread
   */
  void requestRender() {
    synchronized (lock) {
      requestRender = true;
      lock.notifyAll();
    }
  }

  /**
   * May be called from any thread
   */
  void queueEvent(@NonNull Runnable runnable) {
    if (runnable == null) {
      throw new IllegalArgumentException("runnable must not be null");
    }
    synchronized (lock) {
      eventQueue.add(runnable);
      lock.notifyAll();
    }
  }

  /**
   * Wait for the queue to be empty.
   * @return The number of items remaining in the queue
   */
  @UiThread
  void waitForEmpty() {
    synchronized (lock) {
      // Wait for the queue to be empty
      while (!this.eventQueue.isEmpty()) {
        try {
          lock.wait(0);
        } catch (InterruptedException ex) {
          Thread.currentThread().interrupt();
        }
      }
    }
  }

  @UiThread
  void onPause() {
    synchronized (lock) {
      this.paused = true;
      lock.notifyAll();
    }
  }

  @UiThread
  void onResume() {
    synchronized (lock) {
      this.paused = false;
      lock.notifyAll();
    }
  }

  @UiThread
  void onDestroy() {
    synchronized (lock) {
      this.shouldExit = true;
      lock.notifyAll();

      // Wait for the thread to exit
      while (!this.exited) {
        try {
          lock.wait();
        } catch (InterruptedException ex) {
          Thread.currentThread().interrupt();
        }
      }
    }
  }
}
