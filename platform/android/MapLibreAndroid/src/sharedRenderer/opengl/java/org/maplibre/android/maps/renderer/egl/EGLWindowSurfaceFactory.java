package org.maplibre.android.maps.renderer.egl;

import android.opengl.GLSurfaceView;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

public class EGLWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory {
  public EGLSurface createWindowSurface(@NonNull EGL10 egl, @Nullable EGLDisplay display, @Nullable EGLConfig config,
                                        @Nullable Object nativeWindow) {
    EGLSurface result = null;
    if (display != null && config != null && nativeWindow != null) {
      try {
        result = egl.eglCreateWindowSurface(display, config, nativeWindow, null);
      } catch (Exception exception) {
        // This exception indicates that the surface flinger surface
        // is not valid. This can happen if the surface flinger surface has
        // been torn down, but the application has not yet been
        // notified via SurfaceHolder.Callback.surfaceDestroyed.
        // In theory the application should be notified first,
        // but in practice sometimes it is not. See b/4588890
        Log.e("EGLWindowSurfaceFactory", "eglCreateWindowSurface", exception);
      }
    }
    return result;
  }

  public void destroySurface(EGL10 egl, EGLDisplay display,
                             EGLSurface surface) {
    egl.eglDestroySurface(display, surface);
  }
}
