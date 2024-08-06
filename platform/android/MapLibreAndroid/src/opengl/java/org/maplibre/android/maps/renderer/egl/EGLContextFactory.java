package org.maplibre.android.maps.renderer.egl;

import android.opengl.GLSurfaceView;
import android.util.Log;

import androidx.annotation.Nullable;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

public class EGLContextFactory implements GLSurfaceView.EGLContextFactory {
  private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

  public EGLContext createContext(EGL10 egl, @Nullable EGLDisplay display, @Nullable EGLConfig config) {
    if (display == null || config == null) {
      return EGL10.EGL_NO_CONTEXT;
    }
    // Try and get a GLES 3 context
    int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL10.EGL_NONE};
    EGLContext context = egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attrib_list);
    if (context == EGL10.EGL_NO_CONTEXT) {
      Log.e("DefaultContextFactory", "Failed to create an OpenGL ES 3 context. Retrying with OpenGL ES 2...");
      attrib_list[1] = 2;
      context = egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attrib_list);
    }
    if (context == EGL10.EGL_NO_CONTEXT) {
      Log.e("DefaultContextFactory", "Failed to create an OpenGL ES 3 or OpenGL ES 2 context.");
    }
    return context;
  }

  public void destroyContext(EGL10 egl, EGLDisplay display,
                             EGLContext context) {
    if (!egl.eglDestroyContext(display, context)) {
      Log.e("DefaultContextFactory", "display:" + display + " context: " + context);
      Log.i("DefaultContextFactory", "tid=" + Thread.currentThread().getId());
    }
  }
}
