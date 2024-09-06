package org.maplibre.android.maps.renderer.surfaceview;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

import org.maplibre.android.maps.renderer.egl.EGLLogWrapper;

import java.lang.ref.WeakReference;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;

public class MapLibreGLSurfaceView extends MapLibreSurfaceView {

  protected final WeakReference<MapLibreGLSurfaceView> viewWeakReference = new WeakReference<>(this);

  private GLSurfaceView.EGLConfigChooser eglConfigChooser;
  private GLSurfaceView.EGLContextFactory eglContextFactory;
  private GLSurfaceView.EGLWindowSurfaceFactory eglWindowSurfaceFactory;

  private boolean preserveEGLContextOnPause;

  public MapLibreGLSurfaceView(Context context) {
    super(context);
  }

  public MapLibreGLSurfaceView(Context context, AttributeSet attrs) {
    super(context, attrs);
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

  public void setRenderer(SurfaceViewMapRenderer renderer) {
    if (eglConfigChooser == null) {
      throw new IllegalStateException("No eglConfigChooser provided");
    }
    if (eglContextFactory == null) {
      throw new IllegalStateException("No eglContextFactory provided");
    }
    if (eglWindowSurfaceFactory == null) {
      throw new IllegalStateException("No eglWindowSurfaceFactory provided");
    }

    super.setRenderer(renderer);
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

  @Override
  protected void createRenderThread() {
    renderThread = new GLThread(viewWeakReference);
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
  static class GLThread extends MapLibreSurfaceView.RenderThread {
    GLThread(WeakReference<MapLibreGLSurfaceView> surfaceViewWeakRef) {
      super();

      mSurfaceViewWeakRef = surfaceViewWeakRef;
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
        renderThreadManager.notifyAll();
      }
    }

    @Override
    protected void guardedRun() throws InterruptedException {
      eglHelper = new EglHelper(mSurfaceViewWeakRef);
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
              boolean pausing = false;
              if (paused != requestPaused) {
                pausing = requestPaused;
                paused = requestPaused;
                renderThreadManager.notifyAll();
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
                MapLibreGLSurfaceView view = mSurfaceViewWeakRef.get();
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
                renderThreadManager.notifyAll();
              }

              // Have we acquired the surface view surface?
              if (hasSurface && waitingForSurface) {
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
              if (readyToDraw()) {

                // If we don't have an EGL context, try to acquire one.
                if (!haveEglContext) {
                  if (askedToReleaseEglContext) {
                    askedToReleaseEglContext = false;
                  } else {
                    try {
                      eglHelper.start();
                    } catch (RuntimeException exception) {
                      renderThreadManager.notifyAll();
                      return;
                    }
                    haveEglContext = true;
                    createEglContext = true;

                    renderThreadManager.notifyAll();
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
                  renderThreadManager.notifyAll();
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
              renderThreadManager.wait();
            }
          } // end of synchronized(sGLThreadManager)

          if (event != null) {
            event.run();
            event = null;
            continue;
          }

          if (createEglSurface) {
            if (eglHelper.createSurface()) {
              synchronized (renderThreadManager) {
                finishedCreatingEglSurface = true;
                renderThreadManager.notifyAll();
              }
            } else {
              synchronized (renderThreadManager) {
                finishedCreatingEglSurface = true;
                surfaceIsBad = true;
                renderThreadManager.notifyAll();
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
            MapLibreSurfaceView view = mSurfaceViewWeakRef.get();
            if (view != null) {
              view.renderer.onSurfaceCreated(null);
            }
            createEglContext = false;
          }

          if (sizeChanged) {
            MapLibreSurfaceView view = mSurfaceViewWeakRef.get();
            if (view != null) {
              view.renderer.onSurfaceChanged(w, h);
            }
            sizeChanged = false;
          }

          MapLibreSurfaceView view = mSurfaceViewWeakRef.get();
          if (view != null) {
            view.renderer.onDrawFrame();
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

              synchronized (renderThreadManager) {
                surfaceIsBad = true;
                renderThreadManager.notifyAll();
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
        synchronized (renderThreadManager) {
          stopEglSurfaceLocked();
          stopEglContextLocked();
        }
      }
    }

    @Override
    protected boolean readyToDraw() {
      return super.readyToDraw() && !surfaceIsBad;
    }

    @Override
    public boolean ableToDraw() {
      return haveEglContext && haveEglSurface && readyToDraw();
    }

    public void surfaceCreated() {
      synchronized (renderThreadManager) {
        hasSurface = true;
        finishedCreatingEglSurface = false;
        renderThreadManager.notifyAll();
        while (waitingForSurface
          && !finishedCreatingEglSurface
          && !exited) {
          try {
            renderThreadManager.wait();
          } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
          }
        }
      }
    }

    public void requestReleaseEglContextLocked() {
      shouldReleaseEglContext = true;
      renderThreadManager.notifyAll();
    }

    private boolean surfaceIsBad;
    private boolean haveEglContext;
    private boolean haveEglSurface;
    private boolean finishedCreatingEglSurface;
    private boolean shouldReleaseEglContext;

    private EglHelper eglHelper;

    /**
     * Set once at thread construction time, nulled out when the parent view is garbage
     * called. This weak reference allows the SurfaceView to be garbage collected while
     * the RenderThread is still alive.
     */
    protected WeakReference<MapLibreGLSurfaceView> mSurfaceViewWeakRef;
  }
}
