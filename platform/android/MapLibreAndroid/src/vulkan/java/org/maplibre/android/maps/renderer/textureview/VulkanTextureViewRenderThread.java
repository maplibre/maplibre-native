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
              surface = null;
              break;
            }

            if (surfaceTexture != null && !paused && requestRender && surface == null) {

              w = width;
              h = height;
              surface = new Surface(surfaceTexture);

              initialize = true;

              // Reset the request render flag now, so we can catch new requests
              // while rendering
              requestRender = false;

              // Break the guarded loop and continue to process
              break;
            }

            if (requestRender && !paused) {
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

        if (initialize) {
          mapRenderer.onSurfaceCreated(surface);
          mapRenderer.onSurfaceChanged( w, h);

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
