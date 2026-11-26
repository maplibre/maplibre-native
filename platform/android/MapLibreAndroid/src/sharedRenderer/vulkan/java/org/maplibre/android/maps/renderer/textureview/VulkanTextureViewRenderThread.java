package org.maplibre.android.maps.renderer.textureview;

import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.annotation.UiThread;

public class VulkanTextureViewRenderThread extends TextureViewRenderThread {

  private Surface surface;

  @UiThread
  public VulkanTextureViewRenderThread(@NonNull TextureView textureView, @NonNull TextureViewMapRenderer mapRenderer) {
    super(textureView, mapRenderer);
  }

  void cleanup() {
    if (surface != null) {
      mapRenderer.onSurfaceDestroyed();
      surface.release();
      surface = null;
    }

    hasNativeSurface = false;
  }

  // Thread implementation

  @Override
  public void run() {
    try {

      while (true) {
        Runnable event = null;
        boolean createSurface = false;
        boolean sizeChanged = false;
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

            if (this.destroySurface) {
              if (surface != null) {
                mapRenderer.onSurfaceDestroyed();
                surface.release();
                surface = null;
              }

              this.hasNativeSurface = false;
              this.destroySurface = false;
              lock.notifyAll();
              break;
            }

            if (surfaceTexture != null && !paused && requestRender) {

              w = width;
              h = height;

              if (surface == null) {
                surface = new Surface(surfaceTexture);
                this.hasNativeSurface = true;
                createSurface = true;
              }

              if (this.sizeChanged) {
                sizeChanged = true;
                this.sizeChanged = false;
              }

              // Reset the request render flag now, so we can catch new requests
              // while rendering
              requestRender = false;

              // Break the guarded loop and continue to process
              break;
            }

            // Wait until needed
            lock.wait();

          } // end guarded while loop

        } // end guarded block

        // Run event, if any
        if (event != null) {
          event.run();
          continue;
        }

        if (createSurface) {
          mapRenderer.onSurfaceCreated(surface);
          mapRenderer.onSurfaceChanged(w, h);
          createSurface = false;
          continue;
        }

        // If the surface size has changed inform the map renderer.
        if (sizeChanged) {
          mapRenderer.onSurfaceChanged(w, h);
          sizeChanged = false;
          continue;
        }

        if (surface == null) {
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
        cleanup();
        this.exited = true;
        lock.notifyAll();
      }
    }
  }
}
