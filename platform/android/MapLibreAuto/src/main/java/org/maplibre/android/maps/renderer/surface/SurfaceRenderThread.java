package org.maplibre.android.maps.renderer.surface;

import android.view.Surface;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import com.grab.mapsdk.common.log.Logger;

import org.maplibre.android.maps.renderer.egl.EGLConfigChooser;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

public class SurfaceRenderThread extends Thread {

    private static final String TAG = "Mbgl-SurfaceRenderThread";

    @NonNull
    private final SurfaceMapRenderer mapRenderer;
    @NonNull
    private final EGLHolder eglHolder;

    // Lock used for synchronization
    private final Object lock = new Object();

    // Guarded by lock
    private final ArrayList<Runnable> eventQueue = new ArrayList<>();
    @Nullable
    private Surface surface;
    private int width;
    private int height;
    private boolean requestRender;
    private boolean sizeChanged;
    private boolean paused;
    private boolean destroyContext;
    private boolean destroySurface;
    private boolean shouldExit;
    private boolean exited;

    /**
     * Create a render thread for the given TextureView / Maprenderer combination.
     *
     * @param mapRenderer the MapRenderer
     */
    SurfaceRenderThread(@NonNull SurfaceMapRenderer mapRenderer) {
        this.mapRenderer = mapRenderer;
        this.eglHolder = new EGLHolder(false);
    }

    // SurfaceTextureListener methods

    public void onSurfaceAvailable(final Surface surface, final int width, final int height) {
        synchronized (lock) {
            eglHolder.setSurface(new WeakReference<>(surface));
            this.surface = surface;
            this.width = width;
            this.height = height;
            this.requestRender = true;
            lock.notifyAll();
        }
    }

    public void onSurfaceSizeChanged(final int width, final int height) {
        synchronized (lock) {
            this.width = width;
            this.height = height;
            this.sizeChanged = true;
            this.requestRender = true;
            lock.notifyAll();
        }
    }

    public boolean onSurfaceDestroyed() {
        synchronized (lock) {
            if (this.surface != null) {
                surface.release();
            }
            this.surface = null;
            this.destroySurface = true;
            this.requestRender = false;
            lock.notifyAll();
        }
        return true;
    }

    @UiThread
    public void onSurfaceTextureUpdated(Surface surface) {
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
//              mapRenderer.onSurfaceDestroyed();
                            break;
                        }

                        if (destroyContext) {
                            eglHolder.destroyContext();
                            destroyContext = false;
                            break;
                        }

                        if (surface != null && !paused && requestRender) {

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
                    Result result = eglHolder.prepare();
                    synchronized (lock) {
                        if (!eglHolder.createSurface()) {
                            // Cleanup the surface if one could not be created
                            // and wait for another to be ready.
                            destroySurface = true;
                            continue;
                        }
                    }
                    mapRenderer.onSurfaceCreated(surface);
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
                            surface = null;
                            destroySurface = true;
                            destroyContext = true;
                        }
                        break;
                    default:
                        Logger.w(TAG, String.format("eglSwapBuffer error: %s. Waiting or new surface", swapError));
                        // Probably lost the surface. Clear the current one and
                        // wait for a new one to be set
                        synchronized (lock) {
                            surface = null;
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
        private WeakReference<Surface> surfaceWeakRef;
        private boolean translucentSurface;

        private EGL10 egl;
        @Nullable
        private EGLConfig eglConfig;
        private EGLDisplay eglDisplay = EGL10.EGL_NO_DISPLAY;
        private EGLContext eglContext = EGL10.EGL_NO_CONTEXT;
        private EGLSurface eglSurface = EGL10.EGL_NO_SURFACE;

        EGLHolder(boolean translucentSurface) {
            this.translucentSurface = translucentSurface;
        }

        void setSurface(WeakReference<Surface> surface) {
            surfaceWeakRef = surface;
        }

        Result prepare() {
            this.egl = (EGL10) EGLContext.getEGL();

            // Only re-initialize display when needed
            if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
                this.eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

                if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
                    return new Result(false, "eglGetDisplay failed");
                }

                int[] version = new int[2];
                if (!egl.eglInitialize(eglDisplay, version)) {
                    return new Result(false, "eglInitialize failed");
                }
            }

            boolean noTextureView = false;
            if (surfaceWeakRef == null) {
                // No texture view present
                eglConfig = null;
                eglContext = EGL10.EGL_NO_CONTEXT;
                noTextureView = true;
            } else if (eglContext == EGL10.EGL_NO_CONTEXT) {
                eglConfig = new EGLConfigChooser(translucentSurface).chooseConfig(egl, eglDisplay);
                int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
                eglContext = egl.eglCreateContext(eglDisplay, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            }

            if (eglContext == EGL10.EGL_NO_CONTEXT) {
                return new Result(false, "createContext failed: " + (noTextureView ? "no textureView" : "has textureView"));
            }

            return new Result();
        }

        @NonNull
        GL10 createGL() {
            return (GL10) eglContext.getGL();
        }

        boolean createSurface() {
            // The window size has changed, so we need to create a new surface.
            destroySurface();

            // Create an EGL surface we can render into.
            Surface surface = surfaceWeakRef.get();
            if (surface != null) {
                int[] surfaceAttribs = {EGL10.EGL_NONE};
                eglSurface = egl.eglCreateWindowSurface(eglDisplay, eglConfig, surface, surfaceAttribs);
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

    private static class Result {

        private boolean success = true;
        private String message = "";

        public Result() {
        }

        public Result(boolean success, String message) {
            this.success = success;
            this.message = message;
        }

        public boolean isSuccess() {
            return success;
        }

        public void setSuccess(boolean success) {
            this.success = success;
        }

        public String getMessage() {
            return message;
        }

        public void setMessage(String message) {
            this.message = message;
        }

        @Override
        public String toString() {
            return "Result{" +
                    "success=" + success +
                    ", message='" + message + '\'' +
                    '}';
        }
    }
}
