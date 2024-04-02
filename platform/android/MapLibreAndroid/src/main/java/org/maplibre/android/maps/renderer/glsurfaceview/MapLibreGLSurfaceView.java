package org.maplibre.android.maps.renderer.glsurfaceview;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.egl.EGLLogWrapper;

import java.io.Writer;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLSurfaceView.RENDERMODE_CONTINUOUSLY;

/**
 * {@link GLSurfaceView} extension that notifies a listener when the view is detached from window,
 * which is the point of destruction of the GL thread.
 */
public class MapLibreGLSurfaceView extends SurfaceView implements SurfaceHolder.Callback2 {

  private static final String TAG = "GLSurfaceView";
  private static final GLThreadManager glThreadManager = new GLThreadManager();

  private final WeakReference<MapLibreGLSurfaceView> viewWeakReference = new WeakReference<>(this);
  private GLThread glThread;
  private GLSurfaceView.Renderer renderer;
  private GLSurfaceView.EGLConfigChooser eglConfigChooser;
  private GLSurfaceView.EGLContextFactory eglContextFactory;
  private GLSurfaceView.EGLWindowSurfaceFactory eglWindowSurfaceFactory;
  private OnGLSurfaceViewDetachedListener detachedListener;

  private boolean preserveEGLContextOnPause;
  private boolean detached;

  /**
   * Standard View constructor. In order to render something, you
   * must call {@link #setRenderer} to register a renderer.
   */
  public MapLibreGLSurfaceView(Context context) {
    super(context);
    init();
  }

  /**
   * Standard View constructor. In order to render something, you
   * must call {@link #setRenderer} to register a renderer.
   */
  public MapLibreGLSurfaceView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  private void init() {
    // Install a SurfaceHolder.Callback so we get notified when the
    // underlying surface is created and destroyed
    SurfaceHolder holder = getHolder();
    holder.addCallback(this);
  }

  @Override
  protected void finalize() throws Throwable {
    try {
      if (glThread != null) {
        // GLThread may still be running if this view was never
        // attached to a window.
        glThread.requestExitAndWait();
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
  public void setDetachedListener(@NonNull OnGLSurfaceViewDetachedListener detachedListener) {
    if (this.detachedListener != null) {
      throw new IllegalArgumentException("Detached from window listener has been already set.");
    }
    this.detachedListener = detachedListener;
  }

  /**
   * Control whether the EGL context is preserved when the GLSurfaceView is paused and
   * resumed.
   * <p>
   * If set to true, then the EGL context may be preserved when the GLSurfaceView is paused.
   * <p>
   * Prior to API level 11, whether the EGL context is actually preserved or not
   * depends upon whether the Android device can support an arbitrary number of
   * EGL contexts or not. Devices that can only support a limited number of EGL
   * contexts must release the EGL context in order to allow multiple applications
   * to share the GPU.
   * <p>
   * If set to false, the EGL context will be released when the GLSurfaceView is paused,
   * and recreated when the GLSurfaceView is resumed.
   * <p>
   * <p>
   * The default is false.
   *
   * @param preserveOnPause preserve the EGL context when paused
   */
  public void setPreserveEGLContextOnPause(boolean preserveOnPause) {
    preserveEGLContextOnPause = preserveOnPause;
  }

  /**
   * @return true if the EGL context will be preserved when paused
   */
  public boolean getPreserveEGLContextOnPause() {
    return preserveEGLContextOnPause;
  }

  /**
   * Set the renderer associated with this view. Also starts the thread that
   * will call the renderer, which in turn causes the rendering to start.
   * <p>This method should be called once and only once in the life-cycle of
   * a GLSurfaceView.
   * <p>The following GLSurfaceView methods can only be called <em>before</em>
   * setRenderer is called:
   * <ul>
   * <li>{@link #setEGLConfigChooser(GLSurfaceView.EGLConfigChooser)}
   * </ul>
   * <p>
   * The following GLSurfaceView methods can only be called <em>after</em>
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
   * @param renderer the renderer to use to perform OpenGL drawing.
   */
  public void setRenderer(GLSurfaceView.Renderer renderer) {
    checkRenderThreadState();
    if (eglConfigChooser == null) {
      throw new IllegalStateException("No eglConfigChooser provided");
    }
    if (eglContextFactory == null) {
      throw new IllegalStateException("No eglContextFactory provided");
    }
    if (eglWindowSurfaceFactory == null) {
      throw new IllegalStateException("No eglWindowSurfaceFactory provided");
    }
    this.renderer = renderer;
    glThread = new GLThread(viewWeakReference);
    glThread.start();
  }

  /**
   * Install a custom EGLContextFactory.
   * <p>If this method is
   * called, it must be called before {@link #setRenderer(GLSurfaceView.Renderer)}
   * is called.
   */
  public void setEGLContextFactory(GLSurfaceView.EGLContextFactory factory) {
    checkRenderThreadState();
    eglContextFactory = factory;
  }

  /**
   * Install a custom EGLWindowSurfaceFactory.
   * <p>If this method is
   * called, it must be called before {@link #setRenderer(GLSurfaceView.Renderer)}
   * is called.
   */
  public void setEGLWindowSurfaceFactory(GLSurfaceView.EGLWindowSurfaceFactory factory) {
    checkRenderThreadState();
    eglWindowSurfaceFactory = factory;
  }

  /**
   * Install a custom EGLConfigChooser.
   * <p>If this method is
   * called, it must be called before {@link #setRenderer(GLSurfaceView.Renderer)}
   * is called.
   */
  public void setEGLConfigChooser(GLSurfaceView.EGLConfigChooser configChooser) {
    checkRenderThreadState();
    eglConfigChooser = configChooser;
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
   * This method can only be called after {@link #setRenderer(GLSurfaceView.Renderer)}
   *
   * @param renderMode one of the RENDERMODE_X constants
   */
  public void setRenderMode(int renderMode) {
    glThread.setRenderMode(renderMode);
  }

  /**
   * Get the current rendering mode. May be called
   * from any thread. Must not be called before a renderer has been set.
   *
   * @return the current rendering mode.
   */
  public int getRenderMode() {
    return glThread.getRenderMode();
  }

  /**
   * Request that the renderer render a frame.
   * This method is typically used when the render mode has been set to
   * RENDERMODE_WHEN_DIRTY, so that frames are only rendered on demand.
   * May be called
   * from any thread. Must not be called before a renderer has been set.
   */
  public void requestRender() {
    glThread.requestRender();
  }

  /**
   * This method is part of the SurfaceHolder.Callback interface, and is
   * not normally called or subclassed by clients of GLSurfaceView.
   */
  public void surfaceCreated(SurfaceHolder holder) {
    glThread.surfaceCreated();
  }

  /**
   * This method is part of the SurfaceHolder.Callback interface, and is
   * not normally called or subclassed by clients of GLSurfaceView.
   */
  public void surfaceDestroyed(SurfaceHolder holder) {
    // Surface will be destroyed when we return
    glThread.surfaceDestroyed();
  }

  /**
   * This method is part of the SurfaceHolder.Callback interface, and is
   * not normally called or subclassed by clients of GLSurfaceView.
   */
  public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
    glThread.onWindowResize(w, h);
  }

  /**
   * This method is part of the SurfaceHolder.Callback2 interface, and is
   * not normally called or subclassed by clients of GLSurfaceView.
   */
  @Override
  public void surfaceRedrawNeededAsync(SurfaceHolder holder, Runnable finishDrawing) {
    if (glThread != null) {
      glThread.requestRenderAndNotify(finishDrawing);
    }
  }

  /**
   * This method is part of the SurfaceHolder.Callback2 interface, and is
   * not normally called or subclassed by clients of GLSurfaceView.
   */
  @Deprecated
  @Override
  public void surfaceRedrawNeeded(SurfaceHolder holder) {
    // Since we are part of the framework we know only surfaceRedrawNeededAsync
    // will be called.
  }

  /**
   * Pause the rendering thread, optionally tearing down the EGL context
   * depending upon the value of {@link #setPreserveEGLContextOnPause(boolean)}.
   * <p>
   * Must not be called before a renderer has been set.
   */
  public void onPause() {
    glThread.onPause();
  }

  /**
   * Resumes the rendering thread, re-creating the OpenGL context if necessary. It
   * is the counterpart to {@link #onPause()}.
   * <p>
   * Must not be called before a renderer has been set.
   */
  public void onResume() {
    glThread.onResume();
  }

  /**
   * Queue a runnable to be run on the GL rendering thread. This can be used
   * to communicate with the Renderer on the rendering thread.
   * Must not be called before a renderer has been set.
   *
   * @param r the runnable to be run on the GL rendering thread.
   */
  public void queueEvent(Runnable r) {
    glThread.queueEvent(r);
  }

  /**
   * Wait for the queue to become empty
   * @param timeoutMillis Timeout in milliseconds
   * @return Number of queue items remaining
   */
  public long waitForEmpty(long timeoutMillis) {
    return glThread.waitForEmpty(timeoutMillis);
  }


  /**
   * This method is used as part of the View class and is not normally
   * called or subclassed by clients of GLSurfaceView.
   */
  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    if (detached && (renderer != null)) {
      int renderMode = RENDERMODE_CONTINUOUSLY;
      if (glThread != null) {
        renderMode = glThread.getRenderMode();
      }
      glThread = new GLThread(viewWeakReference);
      if (renderMode != RENDERMODE_CONTINUOUSLY) {
        glThread.setRenderMode(renderMode);
      }
      glThread.start();
    }
    detached = false;
  }

  @Override
  protected void onDetachedFromWindow() {
    if (detachedListener != null) {
      detachedListener.onGLSurfaceViewDetached();
    }
    if (glThread != null) {
      glThread.requestExitAndWait();
    }
    detached = true;
    super.onDetachedFromWindow();
  }

  /**
   * An EGL helper class.
   */
  private static class EglHelper {
    private EglHelper(WeakReference<MapLibreGLSurfaceView> glSurfaceViewWeakRef) {
      mGLSurfaceViewWeakRef = glSurfaceViewWeakRef;
    }

    /**
     * Initialize EGL for a given configuration spec.
     */
    public void start() {
      try {
        /*
         * Get an EGL instance
         */
        mEgl = (EGL10) EGLContext.getEGL();

        /*
         * Get to the default display.
         */
        mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

        if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
          Log.e(TAG, "eglGetDisplay failed");
          return;
        }

        /*
         * We can now initialize EGL for that display
         */
        int[] version = new int[2];
        if (!mEgl.eglInitialize(mEglDisplay, version)) {
          Log.e(TAG, "eglInitialize failed");
          return;
        }
        MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
        if (view == null) {
          mEglConfig = null;
          mEglContext = null;
        } else {
          mEglConfig = view.eglConfigChooser.chooseConfig(mEgl, mEglDisplay);
          if (mEglConfig == null) {
            Log.e(TAG, "failed to select an EGL configuration");
            return;
          }

          /*
           * Create an EGL context. We want to do this as rarely as we can, because an
           * EGL context is a somewhat heavy object.
           */
          mEglContext = view.eglContextFactory.createContext(mEgl, mEglDisplay, mEglConfig);
        }
        if (mEglContext == null || mEglContext == EGL10.EGL_NO_CONTEXT) {
          mEglContext = null;
          Log.e(TAG, "createContext failed");
          return;
        }
      } catch (Exception exception) {
        Log.e(TAG, "createContext failed: ", exception);
      }
      mEglSurface = null;
    }

    /**
     * Create an egl surface for the current SurfaceHolder surface. If a surface
     * already exists, destroy it before creating the new surface.
     *
     * @return true if the surface was created successfully.
     */
    boolean createSurface() {
      /*
       * Check preconditions.
       */
      if (mEgl == null) {
        Log.e(TAG, "egl not initialized");
        return false;
      }
      if (mEglDisplay == null) {
        Log.e(TAG, "eglDisplay not initialized");
        return false;
      }
      if (mEglConfig == null) {
        Log.e(TAG, "mEglConfig not initialized");
        return false;
      }

      /*
       *  The window size has changed, so we need to create a new
       *  surface.
       */
      destroySurfaceImp();

      /*
       * Create an EGL surface we can render into.
       */
      MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
      if (view != null) {
        mEglSurface = view.eglWindowSurfaceFactory.createWindowSurface(mEgl,
          mEglDisplay, mEglConfig, view.getHolder());
      } else {
        mEglSurface = null;
      }

      if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
        int error = mEgl.eglGetError();
        if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
          Log.e(TAG, "createWindowSurface returned EGL_BAD_NATIVE_WINDOW.");
        }
        return false;
      }

      /*
       * Before we can issue GL commands, we need to make sure
       * the context is current and bound to a surface.
       */
      if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
        /*
         * Could not make the context current, probably because the underlying
         * SurfaceView surface has been destroyed.
         */
        logEglErrorAsWarning(TAG, "eglMakeCurrent", mEgl.eglGetError());
        return false;
      }

      return true;
    }

    /**
     * Create a GL object for the current EGL context.
     */
    GL createGL() {
      return mEglContext.getGL();
    }

    /**
     * Display the current render surface.
     *
     * @return the EGL error code from eglSwapBuffers.
     */
    public int swap() {
      if (!mEgl.eglSwapBuffers(mEglDisplay, mEglSurface)) {
        return mEgl.eglGetError();
      }
      return EGL10.EGL_SUCCESS;
    }

    void destroySurface() {
      destroySurfaceImp();
    }

    private void destroySurfaceImp() {
      if (mEglSurface != null && mEglSurface != EGL10.EGL_NO_SURFACE) {
        mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE,
          EGL10.EGL_NO_SURFACE,
          EGL10.EGL_NO_CONTEXT);
        MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
        if (view != null) {
          view.eglWindowSurfaceFactory.destroySurface(mEgl, mEglDisplay, mEglSurface);
        }
        mEglSurface = null;
      }
    }

    public void finish() {
      if (mEglContext != null) {
        MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
        if (view != null) {
          view.eglContextFactory.destroyContext(mEgl, mEglDisplay, mEglContext);
        }
        mEglContext = null;
      }
      if (mEglDisplay != null) {
        mEgl.eglTerminate(mEglDisplay);
        mEglDisplay = null;
      }
    }

    static void logEglErrorAsWarning(String tag, String function, int error) {
      Log.w(tag, formatEglError(function, error));
    }

    static String formatEglError(String function, int error) {
      return function + " failed: " + EGLLogWrapper.getErrorString(error);
    }

    private WeakReference<MapLibreGLSurfaceView> mGLSurfaceViewWeakRef;
    EGL10 mEgl;
    EGLDisplay mEglDisplay;
    EGLSurface mEglSurface;
    EGLConfig mEglConfig;
    EGLContext mEglContext;

  }

  /**
   * A generic GL Thread. Takes care of initializing EGL and GL. Delegates
   * to a Renderer instance to do the actual drawing. Can be configured to
   * render continuously or on request.
   * <p>
   * All potentially blocking synchronization is done through the
   * sGLThreadManager object. This avoids multiple-lock ordering issues.
   */
  static class GLThread extends Thread {
    GLThread(WeakReference<MapLibreGLSurfaceView> glSurfaceViewWeakRef) {
      super();
      width = 0;
      height = 0;
      requestRender = true;
      renderMode = RENDERMODE_CONTINUOUSLY;
      wantRenderNotification = false;
      mGLSurfaceViewWeakRef = glSurfaceViewWeakRef;
    }

    @Override
    public void run() {
      setName("GLThread " + getId());

      try {
        guardedRun();
      } catch (InterruptedException exception) {
        // fall thru and exit normally
      } finally {
        glThreadManager.threadExiting(this);
      }
    }

    /*
     * This private method should only be called inside a
     * synchronized(sGLThreadManager) block.
     */
    private void stopEglSurfaceLocked() {
      if (haveEglSurface) {
        haveEglSurface = false;
        eglHelper.destroySurface();
      }
    }

    /*
     * This private method should only be called inside a
     * synchronized(sGLThreadManager) block.
     */
    private void stopEglContextLocked() {
      if (haveEglContext) {
        eglHelper.finish();
        haveEglContext = false;
        glThreadManager.releaseEglContextLocked(this);
      }
    }

    private void guardedRun() throws InterruptedException {
      eglHelper = new EglHelper(mGLSurfaceViewWeakRef);
      haveEglContext = false;
      haveEglSurface = false;
      wantRenderNotification = false;

      try {
        GL10 gl = null;
        boolean createEglContext = false;
        boolean createEglSurface = false;
        boolean createGlInterface = false;
        boolean lostEglContext = false;
        boolean sizeChanged = false;
        boolean wantRenderNotification = false;
        boolean doRenderNotification = false;
        boolean askedToReleaseEglContext = false;
        int w = 0;
        int h = 0;
        Runnable event = null;
        Runnable finishDrawingRunnable = null;

        while (true) {
          synchronized (glThreadManager) {
            while (true) {
              if (shouldExit) {
                return;
              }

              if (!eventQueue.isEmpty()) {
                event = eventQueue.remove(0);
                break;
              }

              // Update the pause state.
              boolean pausing = false;
              if (paused != requestPaused) {
                pausing = requestPaused;
                paused = requestPaused;
                glThreadManager.notifyAll();
              }

              // Do we need to give up the EGL context?
              if (shouldReleaseEglContext) {
                stopEglSurfaceLocked();
                stopEglContextLocked();
                shouldReleaseEglContext = false;
                askedToReleaseEglContext = true;
              }

              // Have we lost the EGL context?
              if (lostEglContext) {
                stopEglSurfaceLocked();
                stopEglContextLocked();
                lostEglContext = false;
              }

              // When pausing, release the EGL surface:
              if (pausing && haveEglSurface) {
                stopEglSurfaceLocked();
              }

              // When pausing, optionally release the EGL Context:
              if (pausing && haveEglContext) {
                MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
                boolean preserveEglContextOnPause = view != null && view.preserveEGLContextOnPause;
                if (!preserveEglContextOnPause) {
                  stopEglContextLocked();
                }
              }

              // Have we lost the SurfaceView surface?
              if ((!hasSurface) && (!waitingForSurface)) {
                if (haveEglSurface) {
                  stopEglSurfaceLocked();
                }
                waitingForSurface = true;
                surfaceIsBad = false;
                glThreadManager.notifyAll();
              }

              // Have we acquired the surface view surface?
              if (hasSurface && waitingForSurface) {
                waitingForSurface = false;
                glThreadManager.notifyAll();
              }

              if (doRenderNotification) {
                this.wantRenderNotification = false;
                doRenderNotification = false;
                renderComplete = true;
                glThreadManager.notifyAll();
              }

              if (this.finishDrawingRunnable != null) {
                finishDrawingRunnable = this.finishDrawingRunnable;
                this.finishDrawingRunnable = null;
              }

              // Ready to draw?
              if (readyToDraw()) {

                // If we don't have an EGL context, try to acquire one.
                if (!haveEglContext) {
                  if (askedToReleaseEglContext) {
                    askedToReleaseEglContext = false;
                  } else {
                    try {
                      eglHelper.start();
                    } catch (RuntimeException exception) {
                      glThreadManager.releaseEglContextLocked(this);
                      return;
                    }
                    haveEglContext = true;
                    createEglContext = true;

                    glThreadManager.notifyAll();
                  }
                }

                if (haveEglContext && !haveEglSurface) {
                  haveEglSurface = true;
                  createEglSurface = true;
                  createGlInterface = true;
                  sizeChanged = true;
                }

                if (haveEglSurface) {
                  if (this.sizeChanged) {
                    sizeChanged = true;
                    w = width;
                    h = height;
                    this.wantRenderNotification = true;

                    // Destroy and recreate the EGL surface.
                    createEglSurface = true;

                    this.sizeChanged = false;
                  }
                  requestRender = false;
                  glThreadManager.notifyAll();
                  if (this.wantRenderNotification) {
                    wantRenderNotification = true;
                  }
                  break;
                }
              } else {
                if (finishDrawingRunnable != null) {
                  Log.w(TAG, "Warning, !readyToDraw() but waiting for "
                    + "draw finished! Early reporting draw finished.");
                  finishDrawingRunnable.run();
                  finishDrawingRunnable = null;
                }
              }
              // By design, this is the only place in a GLThread thread where we wait().
              glThreadManager.wait();
            }
          } // end of synchronized(sGLThreadManager)

          if (event != null) {
            event.run();
            event = null;
            continue;
          }

          if (createEglSurface) {
            if (eglHelper.createSurface()) {
              synchronized (glThreadManager) {
                finishedCreatingEglSurface = true;
                glThreadManager.notifyAll();
              }
            } else {
              synchronized (glThreadManager) {
                finishedCreatingEglSurface = true;
                surfaceIsBad = true;
                glThreadManager.notifyAll();
              }
              continue;
            }
            createEglSurface = false;
          }

          if (createGlInterface) {
            gl = (GL10) eglHelper.createGL();

            createGlInterface = false;
          }

          if (createEglContext) {
            MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
            if (view != null) {
              view.renderer.onSurfaceCreated(gl, eglHelper.mEglConfig);
            }
            createEglContext = false;
          }

          if (sizeChanged) {
            MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
            if (view != null) {
              view.renderer.onSurfaceChanged(gl, w, h);
            }
            sizeChanged = false;
          }

          MapLibreGLSurfaceView view = mGLSurfaceViewWeakRef.get();
          if (view != null) {
            view.renderer.onDrawFrame(gl);
            if (finishDrawingRunnable != null) {
              finishDrawingRunnable.run();
              finishDrawingRunnable = null;
            }
          }
          int swapError = eglHelper.swap();
          switch (swapError) {
            case EGL10.EGL_SUCCESS:
              break;
            case EGL11.EGL_CONTEXT_LOST:
              lostEglContext = true;
              break;
            default:
              // Other errors typically mean that the current surface is bad,
              // probably because the SurfaceView surface has been destroyed,
              // but we haven't been notified yet.
              // Log the error to help developers understand why rendering stopped.
              EglHelper.logEglErrorAsWarning(TAG, "eglSwapBuffers", swapError);

              synchronized (glThreadManager) {
                surfaceIsBad = true;
                glThreadManager.notifyAll();
              }
              break;
          }

          if (wantRenderNotification) {
            doRenderNotification = true;
            wantRenderNotification = false;
          }
        }

      } finally {
        /*
         * clean-up everything...
         */
        synchronized (glThreadManager) {
          stopEglSurfaceLocked();
          stopEglContextLocked();
        }
      }
    }

    public boolean ableToDraw() {
      return haveEglContext && haveEglSurface && readyToDraw();
    }

    private boolean readyToDraw() {
      return (!paused) && hasSurface && (!surfaceIsBad)
        && (width > 0) && (height > 0)
        && (requestRender || (renderMode == RENDERMODE_CONTINUOUSLY));
    }

    public void setRenderMode(int renderMode) {
      synchronized (glThreadManager) {
        this.renderMode = renderMode;
        glThreadManager.notifyAll();
      }
    }

    public int getRenderMode() {
      synchronized (glThreadManager) {
        return renderMode;
      }
    }

    public void requestRender() {
      synchronized (glThreadManager) {
        requestRender = true;
        glThreadManager.notifyAll();
      }
    }

    public void requestRenderAndNotify(Runnable finishDrawing) {
      synchronized (glThreadManager) {
        // If we are already on the GL thread, this means a client callback
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

        glThreadManager.notifyAll();
      }
    }

    public void surfaceCreated() {
      synchronized (glThreadManager) {
        hasSurface = true;
        finishedCreatingEglSurface = false;
        glThreadManager.notifyAll();
        while (waitingForSurface
          && !finishedCreatingEglSurface
          && !exited) {
          try {
            glThreadManager.wait();
          } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void surfaceDestroyed() {
      synchronized (glThreadManager) {
        hasSurface = false;
        glThreadManager.notifyAll();
        while ((!waitingForSurface) && (!exited)) {
          try {
            glThreadManager.wait();
          } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void onPause() {
      synchronized (glThreadManager) {
        requestPaused = true;
        glThreadManager.notifyAll();
        while ((!exited) && (!paused)) {
          try {
            glThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void onResume() {
      synchronized (glThreadManager) {
        requestPaused = false;
        requestRender = true;
        renderComplete = false;
        glThreadManager.notifyAll();
        while ((!exited) && paused && (!renderComplete)) {
          try {
            glThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void onWindowResize(int w, int h) {
      synchronized (glThreadManager) {
        width = w;
        height = h;
        sizeChanged = true;
        requestRender = true;
        renderComplete = false;

        // If we are already on the GL thread, this means a client callback
        // has caused reentrancy, for example via updating the SurfaceView parameters.
        // We need to process the size change eventually though and update our EGLSurface.
        // So we set the parameters and return so they can be processed on our
        // next iteration.
        if (Thread.currentThread() == this) {
          return;
        }

        glThreadManager.notifyAll();

        // Wait for thread to react to resize and render a frame
        while (!exited && !paused && !renderComplete
          && ableToDraw()) {
          try {
            glThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void requestExitAndWait() {
      // don't call this from GLThread thread or it is a guaranteed
      // deadlock!
      synchronized (glThreadManager) {
        shouldExit = true;
        glThreadManager.notifyAll();
        while (!exited) {
          try {
            glThreadManager.wait();
          } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void requestReleaseEglContextLocked() {
      shouldReleaseEglContext = true;
      glThreadManager.notifyAll();
    }

    /**
     * Queue an "event" to be run on the GL rendering thread.
     *
     * @param r the runnable to be run on the GL rendering thread.
     */
    public void queueEvent(@NonNull Runnable r) {
      synchronized (glThreadManager) {
        eventQueue.add(r);
        glThreadManager.notifyAll();
      }
    }

    /**
     * Wait for the queue to become empty
     * @param timeoutMillis Timeout in milliseconds, zero for indefinite wait
     * @return Number of queue items remaining
    */
    public int waitForEmpty(long timeoutMillis) {
      final long startTime = System.nanoTime();
      synchronized (glThreadManager) {
        // Wait for the queue to be empty
        while (!this.eventQueue.isEmpty()) {
          if (timeoutMillis > 0) {
            final long elapsedMillis = (System.nanoTime() - startTime) / 1000 / 1000;
            if (elapsedMillis < timeoutMillis) {
              try {
                glThreadManager.wait(timeoutMillis - elapsedMillis);
              } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
              }
            } else {
              break;
            }
          } else {
            try {
              glThreadManager.wait();
            } catch (InterruptedException ex) {
              Thread.currentThread().interrupt();
            }
          }
        }
        return this.eventQueue.size();
      }
    }

    // Once the thread is started, all accesses to the following member
    // variables are protected by the sGLThreadManager monitor
    private boolean shouldExit;
    private boolean exited;
    private boolean requestPaused;
    private boolean paused;
    private boolean hasSurface;
    private boolean surfaceIsBad;
    private boolean waitingForSurface;
    private boolean haveEglContext;
    private boolean haveEglSurface;
    private boolean finishedCreatingEglSurface;
    private boolean shouldReleaseEglContext;
    private int width;
    private int height;
    private int renderMode;
    private boolean requestRender;
    private boolean wantRenderNotification;
    private boolean renderComplete;
    private ArrayList<Runnable> eventQueue = new ArrayList<>();
    private boolean sizeChanged = true;
    private Runnable finishDrawingRunnable = null;
    // End of member variables protected by the sGLThreadManager monitor.

    private EglHelper eglHelper;

    /**
     * Set once at thread construction time, nulled out when the parent view is garbage
     * called. This weak reference allows the GLSurfaceView to be garbage collected while
     * the GLThread is still alive.
     */
    private WeakReference<MapLibreGLSurfaceView> mGLSurfaceViewWeakRef;

  }

  static class LogWriter extends Writer {

    @Override
    public void close() {
      flushBuilder();
    }

    @Override
    public void flush() {
      flushBuilder();
    }

    @Override
    public void write(char[] buf, int offset, int count) {
      for (int i = 0; i < count; i++) {
        char c = buf[offset + i];
        if (c == '\n') {
          flushBuilder();
        } else {
          mBuilder.append(c);
        }
      }
    }

    private void flushBuilder() {
      if (mBuilder.length() > 0) {
        Log.v("GLSurfaceView", mBuilder.toString());
        mBuilder.delete(0, mBuilder.length());
      }
    }

    private StringBuilder mBuilder = new StringBuilder();
  }

  private void checkRenderThreadState() {
    if (glThread != null) {
      throw new IllegalStateException("setRenderer has already been called for this instance.");
    }
  }

  private static class GLThreadManager {

    synchronized void threadExiting(GLThread thread) {
      thread.exited = true;
      notifyAll();
    }

    /*
     * Releases the EGL context. Requires that we are already in the
     * sGLThreadManager monitor when this is called.
     */
    void releaseEglContextLocked(GLThread thread) {
      notifyAll();
    }
  }

  /**
   * Listener interface that notifies when a {@link MapLibreGLSurfaceView} is detached from window.
   */
  public interface OnGLSurfaceViewDetachedListener {

    /**
     * Called when a {@link MapLibreGLSurfaceView} is detached from window.
     */
    void onGLSurfaceViewDetached();
  }
}
