package org.maplibre.android.maps.renderer.textureview;

import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import org.maplibre.android.log.Logger;
import org.maplibre.android.maps.renderer.egl.EGLConfigChooser;

import java.lang.ref.WeakReference;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

/**
 * The render thread is responsible for managing the communication between the
 * ui thread and the render thread it creates. Also, the EGL and GL contexts
 * are managed from here.
 */
public class GLTextureViewRenderThread extends TextureViewRenderThread {

  @NonNull
  private final EGLHolder eglHolder;

  private boolean destroyContext;

  /**
   * Create a render thread for the given TextureView / Maprenderer combination.
   *
   * @param textureView the TextureView
   * @param mapRenderer the MapRenderer
   */
  @UiThread
  public GLTextureViewRenderThread(@NonNull TextureView textureView, @NonNull TextureViewMapRenderer mapRenderer) {
    super(textureView, mapRenderer);
    this.eglHolder = new EGLHolder(new WeakReference<>(textureView), mapRenderer.isTranslucentSurface());
  }

  // Thread implementation

  @Override
  public void run() {
    try {

      while (true) {
        Runnable event = null;
        boolean initializeEGL = false;
        boolean recreateSurface = false;
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
              eglHolder.destroySurface();
              destroySurface = false;
              break;
            }

            if (destroyContext) {
              eglHolder.destroyContext();
              destroyContext = false;
              break;
            }

            if (surfaceTexture != null && !paused && requestRender) {

              w = width;
              h = height;

              // Initialize EGL if needed
              if (eglHolder.eglContext == EGL10.EGL_NO_CONTEXT) {
                initializeEGL = true;
                break;
              }

              // (re-)Initialize EGL Surface if needed
              if (eglHolder.eglSurface == EGL10.EGL_NO_SURFACE) {
                recreateSurface = true;
                break;
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

        GL10 gl = eglHolder.createGL();

        // Initialize EGL
        if (initializeEGL) {
          eglHolder.prepare();
          synchronized (lock) {
            if (!eglHolder.createSurface()) {
              // Cleanup the surface if one could not be created
              // and wait for another to be ready.
              destroySurface = true;
              continue;
            }
          }
          mapRenderer.onSurfaceCreated(null);
          mapRenderer.onSurfaceChanged(w, h);
          continue;
        }

        // If the surface size has changed inform the map renderer.
        if (recreateSurface) {
          synchronized (lock) {
            eglHolder.createSurface();
          }
          mapRenderer.onSurfaceChanged(w, h);
          continue;
        }

        if (sizeChanged) {
          mapRenderer.onSurfaceChanged(w, h);
          sizeChanged = false;
          continue;
        }

        // Don't continue without a surface
        if (eglHolder.eglSurface == EGL10.EGL_NO_SURFACE) {
          continue;
        }

        // Time to render a frame
        mapRenderer.onDrawFrame();

        // Swap and check the result
        int swapError = eglHolder.swap();
        switch (swapError) {
          case EGL10.EGL_SUCCESS:
            break;
          case EGL11.EGL_CONTEXT_LOST:
            Logger.w(TAG, "Context lost. Waiting for re-aquire");
            synchronized (lock) {
              surfaceTexture = null;
              destroySurface = true;
              destroyContext = true;
            }
            break;
          default:
            Logger.w(TAG, String.format("eglSwapBuffer error: %s. Waiting or new surface", swapError));
            // Probably lost the surface. Clear the current one and
            // wait for a new one to be set
            synchronized (lock) {
              surfaceTexture = null;
              destroySurface = true;
            }
        }

      }

    } catch (InterruptedException err) {
      // To be expected
    } finally {
      // Cleanup
      eglHolder.cleanup();

      // Signal we're done
      synchronized (lock) {
        this.exited = true;
        lock.notifyAll();
      }
    }
  }

  /**
   * Holds the EGL state and offers methods to mutate it.
   */
  private static class EGLHolder {
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private final WeakReference<TextureView> textureViewWeakRef;
    private boolean translucentSurface;

    private EGL10 egl;
    @Nullable
    private EGLConfig eglConfig;
    private EGLDisplay eglDisplay = EGL10.EGL_NO_DISPLAY;
    private EGLContext eglContext = EGL10.EGL_NO_CONTEXT;
    private EGLSurface eglSurface = EGL10.EGL_NO_SURFACE;

    EGLHolder(WeakReference<TextureView> textureViewWeakRef, boolean translucentSurface) {
      this.textureViewWeakRef = textureViewWeakRef;
      this.translucentSurface = translucentSurface;
    }

    void prepare() {
      this.egl = (EGL10) EGLContext.getEGL();

      // Only re-initialize display when needed
      if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
        this.eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

        if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
          throw new RuntimeException("eglGetDisplay failed");
        }

        int[] version = new int[2];
        if (!egl.eglInitialize(eglDisplay, version)) {
          throw new RuntimeException("eglInitialize failed");
        }
      }

      if (textureViewWeakRef == null) {
        // No texture view present
        eglConfig = null;
        eglContext = EGL10.EGL_NO_CONTEXT;
      } else if (eglContext == EGL10.EGL_NO_CONTEXT) {
        eglConfig = new EGLConfigChooser(translucentSurface).chooseConfig(egl, eglDisplay);
        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
        eglContext = egl.eglCreateContext(eglDisplay, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
      }

      if (eglContext == EGL10.EGL_NO_CONTEXT) {
        throw new RuntimeException("createContext");
      }
    }

    @NonNull
    GL10 createGL() {
      return (GL10) eglContext.getGL();
    }

    boolean createSurface() {
      // The window size has changed, so we need to create a new surface.
      destroySurface();

      // Create an EGL surface we can render into.
      TextureView view = textureViewWeakRef.get();
      if (view != null && view.getSurfaceTexture() != null) {
        int[] surfaceAttribs = {EGL10.EGL_NONE};
        eglSurface = egl.eglCreateWindowSurface(eglDisplay, eglConfig, view.getSurfaceTexture(), surfaceAttribs);
      } else {
        eglSurface = EGL10.EGL_NO_SURFACE;
      }

      if (eglSurface == null || eglSurface == EGL10.EGL_NO_SURFACE) {
        int error = egl.eglGetError();
        if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
          Logger.e(TAG, "createWindowSurface returned EGL_BAD_NATIVE_WINDOW.");
        }
        return false;
      }

      return makeCurrent();
    }

    boolean makeCurrent() {
      if (!egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        // Could not make the context current, probably because the underlying
        // SurfaceView surface has been destroyed.
        Logger.w(TAG, String.format("eglMakeCurrent: %s", egl.eglGetError()));
        return false;
      }

      return true;
    }

    int swap() {
      if (!egl.eglSwapBuffers(eglDisplay, eglSurface)) {
        return egl.eglGetError();
      }
      return EGL10.EGL_SUCCESS;
    }

    private void destroySurface() {
      if (eglSurface == EGL10.EGL_NO_SURFACE) {
        return;
      }

      if (!egl.eglDestroySurface(eglDisplay, eglSurface)) {
        Logger.w(TAG, String.format("Could not destroy egl surface. Display %s, Surface %s", eglDisplay, eglSurface));
      }

      eglSurface = EGL10.EGL_NO_SURFACE;
    }

    private void destroyContext() {
      if (eglContext == EGL10.EGL_NO_CONTEXT) {
        return;
      }

      if (!egl.eglDestroyContext(eglDisplay, eglContext)) {
        Logger.w(TAG, String.format("Could not destroy egl context. Display %s, Context %s", eglDisplay, eglContext));
      }

      eglContext = EGL10.EGL_NO_CONTEXT;
    }

    private void terminate() {
      if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
        return;
      }

      if (!egl.eglTerminate(eglDisplay)) {
        Logger.w(TAG, String.format("Could not terminate egl. Display %s", eglDisplay));
      }
      eglDisplay = EGL10.EGL_NO_DISPLAY;
    }

    void cleanup() {
      destroySurface();
      destroyContext();
      terminate();
    }
  }
}
