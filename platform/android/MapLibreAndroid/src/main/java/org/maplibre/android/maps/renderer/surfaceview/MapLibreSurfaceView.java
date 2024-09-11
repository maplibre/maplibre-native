package org.maplibre.android.maps.renderer.surfaceview;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import java.util.ArrayList;

public abstract class MapLibreSurfaceView extends SurfaceView implements SurfaceHolder.Callback2 {

  protected static final String TAG = "MapLibreSurfaceView";
  protected static final RenderThreadManager renderThreadManager = new RenderThreadManager();

  protected SurfaceViewMapRenderer renderer;
  protected RenderThread renderThread;
  protected OnSurfaceViewDetachedListener detachedListener;

  protected boolean detached;

  /**
   * The renderer only renders
   * when the surface is created, or when {@link #requestRender} is called.
   *
   * @see #getRenderMode()
   * @see #setRenderMode(int)
   * @see #requestRender()
   */
  public static final int RENDERMODE_WHEN_DIRTY = 0;
  /**
   * The renderer is called
   * continuously to re-render the scene.
   *
   * @see #getRenderMode()
   * @see #setRenderMode(int)
   */
  public static final int RENDERMODE_CONTINUOUSLY = 1;

  /**
   * Standard View constructor. In order to render something, you
   * must call {@link #setRenderer} to register a renderer.
   */
  public MapLibreSurfaceView(Context context) {
    super(context);
    init();
  }

  /**
   * Standard View constructor. In order to render something, you
   * must call {@link #setRenderer} to register a renderer.
   */
  public MapLibreSurfaceView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  private void init() {
    // Install a SurfaceHolder.Callback so we get notified when the
    // underlying surface is created and destroyed
    SurfaceHolder holder = getHolder();
    holder.setFormat(PixelFormat.TRANSPARENT);
    holder.addCallback(this);
  }

  @Override
  protected void finalize() throws Throwable {
    try {
      if (renderThread != null) {
        // thread may still be running if this view was never
        // attached to a window.
        renderThread.requestExitAndWait();
      }
    } finally {
      super.finalize();
    }
  }

  /**
   * Set a listener that gets notified when the view is detached from window.
   *
   * @param detachedListener listener
   */
  public void setDetachedListener(@NonNull OnSurfaceViewDetachedListener detachedListener) {
    if (this.detachedListener != null) {
      throw new IllegalArgumentException("Detached from window listener has been already set.");
    }
    this.detachedListener = detachedListener;
  }

  /**
  /**
   * Set the renderer associated with this view. Also starts the thread that
   * will call the renderer, which in turn causes the rendering to start.
   * <p>This method should be called once and only once in the life-cycle of
   * a SurfaceView.
   * The following SurfaceView methods can only be called <em>after</em>
   * setRenderer is called:
   * <ul>
   * <li>{@link #getRenderMode()}
   * <li>{@link #onPause()}
   * <li>{@link #onResume()}
   * <li>{@link #queueEvent(Runnable)}
   * <li>{@link #requestRender()}
   * <li>{@link #setRenderMode(int)}
   * </ul>
   *
   * @param renderer the renderer to use to perform drawing.
   */
  public void setRenderer(SurfaceViewMapRenderer renderer) {
    checkRenderThreadState();
    this.renderer = renderer;

    createRenderThread();
    renderThread.start();
  }

  /**
   * Set the rendering mode. When renderMode is
   * RENDERMODE_CONTINUOUSLY, the renderer is called
   * repeatedly to re-render the scene. When renderMode
   * is RENDERMODE_WHEN_DIRTY, the renderer only rendered when the surface
   * is created, or when {@link #requestRender} is called. Defaults to RENDERMODE_CONTINUOUSLY.
   * <p>
   * Using RENDERMODE_WHEN_DIRTY can improve battery life and overall system performance
   * by allowing the GPU and CPU to idle when the view does not need to be updated.
   * <p>
   * This method can only be called after {@link #setRenderer(SurfaceViewMapRenderer)}
   *
   * @param renderMode one of the RENDERMODE_X constants
   */
  public void setRenderMode(int renderMode) {
    renderThread.setRenderMode(renderMode);
  }

  /**
   * Get the current rendering mode. May be called
   * from any thread. Must not be called before a renderer has been set.
   *
   * @return the current rendering mode.
   */
  public int getRenderMode() {
    return renderThread.getRenderMode();
  }

  /**
   * Request that the renderer render a frame.
   * This method is typically used when the render mode has been set to
   * RENDERMODE_WHEN_DIRTY, so that frames are only rendered on demand.
   * May be called
   * from any thread. Must not be called before a renderer has been set.
   */
  public void requestRender() {
    renderThread.requestRender();
  }

  /**
   * This method is part of the SurfaceHolder.Callback interface, and is
   * not normally called or subclassed by clients of MapLibreSurfaceView.
   */
  public void surfaceCreated(SurfaceHolder holder) {
    renderThread.surfaceCreated();
  }

  /**
   * This method is part of the SurfaceHolder.Callback interface, and is
   * not normally called or subclassed by clients of MapLibreSurfaceView.
   */
  public void surfaceDestroyed(SurfaceHolder holder) {
    // Surface will be destroyed when we return
    renderThread.surfaceDestroyed();
  }

  /**
   * This method is part of the SurfaceHolder.Callback interface, and is
   * not normally called or subclassed by clients of MapLibreSurfaceView.
   */
  public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
    renderThread.onWindowResize(w, h);
  }

  /**
   * This method is part of the SurfaceHolder.Callback2 interface, and is
   * not normally called or subclassed by clients of MapLibreSurfaceView.
   */
  @Override
  public void surfaceRedrawNeededAsync(SurfaceHolder holder, Runnable finishDrawing) {
    if (renderThread != null) {
      renderThread.requestRenderAndNotify(finishDrawing);
    }
  }

  /**
   * This method is part of the SurfaceHolder.Callback2 interface, and is
   * not normally called or subclassed by clients of MapLibreSurfaceView.
   */
  @Deprecated
  @Override
  public void surfaceRedrawNeeded(SurfaceHolder holder) {
    // Since we are part of the framework we know only surfaceRedrawNeededAsync
    // will be called.
  }

  /**
   * Pause the rendering thread.
   * <p>
   * Must not be called before a renderer has been set.
   */
  public void onPause() {
    renderThread.onPause();
  }

  /**
   * Resumes the rendering thread. It
   * is the counterpart to {@link #onPause()}.
   * <p>
   * Must not be called before a renderer has been set.
   */
  public void onResume() {
    renderThread.onResume();
  }

  /**
   * Queue a runnable to be run on the rendering thread. This can be used
   * to communicate with the Renderer on the rendering thread.
   * Must not be called before a renderer has been set.
   *
   * @param r the runnable to be run on the rendering thread.
   */
  public void queueEvent(Runnable r) {
    renderThread.queueEvent(r);
  }

  /**
   * Wait for the queue to become empty
   */
  public void waitForEmpty() {
    renderThread.waitForEmpty();
  }


  /**
   * This method is used as part of the View class and is not normally
   * called or subclassed by clients of MapLibreSurfaceView.
   */
  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    if (detached && (renderer != null)) {
      int renderMode = RENDERMODE_CONTINUOUSLY;
      if (renderThread != null) {
        renderMode = renderThread.getRenderMode();
      }
      createRenderThread();
      if (renderMode != RENDERMODE_CONTINUOUSLY) {
        renderThread.setRenderMode(renderMode);
      }
      renderThread.start();
    }
    detached = false;
  }

  @Override
  protected void onDetachedFromWindow() {
    if (detachedListener != null) {
      detachedListener.onSurfaceViewDetached();
    }
    if (renderThread != null) {
      renderThread.requestExitAndWait();
    }
    detached = true;
    super.onDetachedFromWindow();
  }

  /**
  /**
   * A generic render Thread. Delegates
   * to a Renderer instance to do the actual drawing. Can be configured to
   * render continuously or on request.
   * <p>
   * All potentially blocking synchronization is done through the
   * sRenderThreadManager object. This avoids multiple-lock ordering issues.
   */
  abstract static class RenderThread extends Thread {
    RenderThread() {
      super();
      width = 0;
      height = 0;
      requestRender = true;
      renderMode = RENDERMODE_CONTINUOUSLY;
      wantRenderNotification = false;
    }

    @Override
    public void run() {
      setName("RenderThread " + getId());

      try {
        guardedRun();
      } catch (InterruptedException exception) {
        // fall thru and exit normally
      } finally {
        renderThreadManager.threadExiting(this);
      }
    }

    protected abstract void guardedRun() throws InterruptedException;

    protected boolean readyToDraw() {
      return (!paused) && hasSurface
              && (width > 0) && (height > 0)
              && (requestRender || (renderMode == RENDERMODE_CONTINUOUSLY));
    }

    public boolean ableToDraw() {
      return readyToDraw();
    }

    public void setRenderMode(int renderMode) {
      if ( !((RENDERMODE_WHEN_DIRTY <= renderMode) && (renderMode <= RENDERMODE_CONTINUOUSLY)) ) {
        throw new IllegalArgumentException("renderMode");
      }
      synchronized (renderThreadManager) {
        this.renderMode = renderMode;
        renderThreadManager.notifyAll();
      }
    }

    public int getRenderMode() {
      synchronized (renderThreadManager) {
        return renderMode;
      }
    }

    public void requestRender() {
      synchronized (renderThreadManager) {
        requestRender = true;
        renderThreadManager.notifyAll();
      }
    }

    public void requestRenderAndNotify(Runnable finishDrawing) {
      synchronized (renderThreadManager) {
        // If we are already on the render thread, this means a client callback
        // has caused reentrancy, for example via updating the SurfaceView parameters.
        // We will return to the client rendering code, so here we don't need to
        // do anything.
        if (Thread.currentThread() == this) {
          return;
        }

        wantRenderNotification = true;
        requestRender = true;
        renderComplete = false;
        finishDrawingRunnable = finishDrawing;

        renderThreadManager.notifyAll();
      }
    }

    public void surfaceCreated() {
      synchronized (renderThreadManager) {
        hasSurface = true;
        renderThreadManager.notifyAll();
        while (!exited && waitingForSurface) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void surfaceDestroyed() {
      synchronized (renderThreadManager) {
        hasSurface = false;
        renderThreadManager.notifyAll();
        while (!exited && !waitingForSurface) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void onPause() {
      synchronized (renderThreadManager) {
        requestPaused = true;
        renderThreadManager.notifyAll();
        while ((!exited) && (!paused)) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void onResume() {
      synchronized (renderThreadManager) {
        requestPaused = false;
        requestRender = true;
        renderComplete = false;
        renderThreadManager.notifyAll();
        while ((!exited) && paused && (!renderComplete)) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void onWindowResize(int w, int h) {
      synchronized (renderThreadManager) {
        width = w;
        height = h;
        sizeChanged = true;
        requestRender = true;
        renderComplete = false;

        // If we are already on the render thread, this means a client callback
        // has caused reentrancy, for example via updating the SurfaceView parameters.
        // We need to process the size change eventually though and update our surface.
        // So we set the parameters and return so they can be processed on our
        // next iteration.
        if (Thread.currentThread() == this) {
          return;
        }

        renderThreadManager.notifyAll();

        // Wait for thread to react to resize and render a frame
        while (!exited && !paused && !renderComplete
          && ableToDraw()) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void requestExitAndWait() {
      // don't call this from renderThread thread or it is a guaranteed
      // deadlock!
      synchronized (renderThreadManager) {
        shouldExit = true;
        renderThreadManager.notifyAll();
        while (!exited) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    /**
     * Queue an "event" to be run on the rendering thread.
     *
     * @param r the runnable to be run on the rendering thread.
     */
    public void queueEvent(@NonNull Runnable r) {
      synchronized (renderThreadManager) {
        eventQueue.add(r);
        renderThreadManager.notifyAll();
      }
    }

    /**
     * Wait for the queue to become empty
    */
    public void waitForEmpty() {
      synchronized (renderThreadManager) {
        // Wait for the queue to be empty
        while (!this.eventQueue.isEmpty()) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    // Once the thread is started, all accesses to the following member
    // variables are protected by the sRenderThreadManager monitor
    protected boolean shouldExit;
    protected boolean exited;
    protected boolean requestPaused;
    protected boolean paused;
    protected boolean hasSurface;
    protected boolean waitingForSurface;
    protected int width;
    protected int height;
    protected int renderMode;
    protected boolean requestRender;
    protected boolean wantRenderNotification;
    protected boolean renderComplete;
    protected ArrayList<Runnable> eventQueue = new ArrayList<>();
    protected boolean sizeChanged = true;
    protected Runnable finishDrawingRunnable = null;
    // End of member variables protected by the sRenderThreadManager monitor.

  }

  protected abstract void createRenderThread();

  protected void checkRenderThreadState() {
    if (renderThread != null) {
      throw new IllegalStateException("setRenderer has already been called for this instance.");
    }
  }

  protected static class RenderThreadManager {

    synchronized void threadExiting(RenderThread thread) {
      thread.exited = true;
      notifyAll();
    }
  }

  /**
   * Listener interface that notifies when a {@link MapLibreSurfaceView} is detached from window.
   */
  public interface OnSurfaceViewDetachedListener {

    /**
     * Called when a {@link MapLibreSurfaceView} is detached from window.
     */
    void onSurfaceViewDetached();
  }
}
