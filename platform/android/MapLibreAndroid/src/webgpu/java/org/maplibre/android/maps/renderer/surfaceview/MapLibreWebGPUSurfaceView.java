package org.maplibre.android.maps.renderer.surfaceview;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;

import java.lang.ref.WeakReference;

public class MapLibreWebGPUSurfaceView extends MapLibreSurfaceView {

  protected final WeakReference<MapLibreWebGPUSurfaceView> viewWeakReference = new WeakReference<>(this);

  public MapLibreWebGPUSurfaceView(Context context) {
    super(context);
  }

  public MapLibreWebGPUSurfaceView(Context context, AttributeSet attrs) {
    super(context, attrs);
  }

  @Override
  protected void createRenderThread() {
    renderThread = new WebGPUThread(viewWeakReference);
  }

  static class WebGPUThread extends MapLibreSurfaceView.RenderThread {
    WebGPUThread(WeakReference<MapLibreWebGPUSurfaceView> surfaceViewWeakRef) {
      super(surfaceViewWeakRef.get().renderThreadManager);
      mSurfaceViewWeakRef = surfaceViewWeakRef;
    }

    @Override
    protected void guardedRun() throws InterruptedException {
      wantRenderNotification = false;

      boolean sizeChanged = false;
      boolean initSurface = false;
      boolean destroySurface = false;
      boolean wantRenderNotification = false;
      boolean doRenderNotification = false;
      int w = 0;
      int h = 0;
      Runnable event = null;
      Runnable finishDrawingRunnable = null;

      while (true) {
        synchronized (renderThreadManager) {
          while (true) {
            if (shouldExit) {
              return;
            }

            if (!eventQueue.isEmpty()) {
              event = eventQueue.remove(0);
              break;
            }

            // Update the pause state.
            if (paused != requestPaused) {
              paused = requestPaused;
              renderThreadManager.notifyAll();
            }

            // lost surface
            if (!hasSurface && !waitingForSurface) {
              MapLibreWebGPUSurfaceView view = mSurfaceViewWeakRef.get();
              if (view != null) {
                destroySurface = true;
                graphicsSurfaceCreated = false;
              }
              waitingForSurface = true;
              renderThreadManager.notifyAll();
            }

            // acquired surface
            if (hasSurface && waitingForSurface) {
              MapLibreWebGPUSurfaceView view = mSurfaceViewWeakRef.get();
              if (view != null) {
                initSurface = true;
                graphicsSurfaceCreated = true;
              }
              waitingForSurface = false;
              renderThreadManager.notifyAll();
            }

            if (doRenderNotification) {
              this.wantRenderNotification = false;
              doRenderNotification = false;
              renderComplete = true;
              renderThreadManager.notifyAll();
            }

            if (this.finishDrawingRunnable != null) {
              finishDrawingRunnable = this.finishDrawingRunnable;
              this.finishDrawingRunnable = null;
            }

            // Ready to draw?
            if (readyToDraw() && graphicsSurfaceCreated) {
              if (this.sizeChanged) {
                sizeChanged = true;
                w = width;
                h = height;
                this.wantRenderNotification = true;
                this.sizeChanged = false;
              }
              requestRender = false;
              renderThreadManager.notifyAll();
              if (this.wantRenderNotification) {
                wantRenderNotification = true;
              }
              break;
            } else {
              if (finishDrawingRunnable != null) {
                Log.w(TAG, "Warning, !readyToDraw() but waiting for "
                  + "draw finished! Early reporting draw finished.");
                finishDrawingRunnable.run();
                finishDrawingRunnable = null;
              }
            }
            // By design, this is the only place in a RenderThread thread where we wait().
            renderThreadManager.wait();
          }
        } // end of synchronized(sRenderThreadManager)

        if (event != null) {
          event.run();
          event = null;
          continue;
        }

        if (destroySurface) {
          MapLibreWebGPUSurfaceView view = mSurfaceViewWeakRef.get();
          if (view != null) {
            view.renderer.onSurfaceDestroyed();
            destroySurface = false;
            continue;
          }
        }

        if (initSurface) {
          MapLibreWebGPUSurfaceView view = mSurfaceViewWeakRef.get();
          if (view != null) {
            view.renderer.onSurfaceCreated(view.getHolder().getSurface());
            initSurface = false;
          }
        }

        if (sizeChanged) {
          MapLibreWebGPUSurfaceView view = mSurfaceViewWeakRef.get();
          if (view != null) {
            view.renderer.onSurfaceChanged(w, h);
            sizeChanged = false;
          }
        }

        {
          MapLibreWebGPUSurfaceView view = mSurfaceViewWeakRef.get();
          if (view != null) {
            view.renderer.onDrawFrame();
            if (finishDrawingRunnable != null) {
              finishDrawingRunnable.run();
              finishDrawingRunnable = null;
            }
          }
        }

        if (wantRenderNotification) {
          doRenderNotification = true;
          wantRenderNotification = false;
        }
      }
    }

    private boolean graphicsSurfaceCreated;

    private final WeakReference<MapLibreWebGPUSurfaceView> mSurfaceViewWeakRef;
  }
}
