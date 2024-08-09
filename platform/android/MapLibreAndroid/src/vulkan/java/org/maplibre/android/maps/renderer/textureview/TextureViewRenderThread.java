package org.maplibre.android.maps.renderer.textureview;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import java.util.ArrayList;

/**
 * The render thread is responsible for managing the communication between the
 * ui thread and the render thread it creates
 */
class TextureViewRenderThread extends Thread implements TextureView.SurfaceTextureListener {

  private static final String TAG = "Mbgl-TextureViewRenderThread";

  @NonNull
  private final TextureViewMapRenderer mapRenderer;

  // Lock used for synchronization
  private final Object lock = new Object();

  // Guarded by lock
  private final ArrayList<Runnable> eventQueue = new ArrayList<>();
  @Nullable
  private Surface surface;
  private boolean hasNativeSurface;
  private int width;
  private int height;
  private boolean requestRender;
  private boolean sizeChanged;
  private boolean paused;
  private boolean destroySurface;
  private boolean shouldExit;
  private boolean exited;

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
      this.surface = new Surface(surfaceTexture);
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
      this.surface = null;
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

  // Thread implementation

  @Override
  public void run() {
    try {

      while (true) {
        Runnable event = null;
        boolean initialize = false;
        int w = -1;
        int h = -1;

        // Guarded block
        synchronized (lock) {
          while (true) {

            if (shouldExit) {
              return;
            }

            // If any events are scheduled, pop one for processing
            if (!eventQueue.isEmpty()) {
              event = eventQueue.remove(0);
              break;
            }

            if (destroySurface) {
              destroySurface = false;
              mapRenderer.onSurfaceDestroyed();
              hasNativeSurface = false;
              break;
            }

            if (surface != null && !paused && requestRender && !hasNativeSurface) {

              w = width;
              h = height;

              initialize = true;

              // Reset the request render flag now, so we can catch new requests
              // while rendering
              requestRender = false;

              // Break the guarded loop and continue to process
              break;
            }

            if (requestRender && !paused)
              break;

            // Wait until needed
            lock.wait();

          } // end guarded while loop

        } // end guarded block

        // Run event, if any
        if (event != null) {
          event.run();
          continue;
        }

        if (initialize) {
          mapRenderer.onSurfaceCreated(surface);
          mapRenderer.onSurfaceChanged( w, h);

          hasNativeSurface = true;
          initialize = false;
          continue;
        }

        // If the surface size has changed inform the map renderer.
        if (sizeChanged) {
          mapRenderer.onSurfaceChanged(w, h);
          sizeChanged = false;
          continue;
        }

        // Time to render a frame
        mapRenderer.onDrawFrame();
      }

    } catch (InterruptedException err) {
      // To be expected
    } finally {

      // Signal we're done
      synchronized (lock) {
        this.exited = true;
        lock.notifyAll();
      }
    }
  }
}
