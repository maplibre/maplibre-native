package org.maplibre.android.maps.renderer.egl;

import android.opengl.GLSurfaceView;
import android.util.Log;

import androidx.annotation.Nullable;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

public class EGLContextFactory implements GLSurfaceView.EGLContextFactory {

  public EGLContext createContext(EGL10 egl, @Nullable EGLDisplay display, @Nullable EGLConfig config) {
    if (display == null || config == null) {
      return EGL10.EGL_NO_CONTEXT;
    }
    int[] attrib_list = {0x3098, 2, EGL10.EGL_NONE};
    return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attrib_list);
  }

  public void destroyContext(EGL10 egl, EGLDisplay display,
                             EGLContext context) {
    if (!egl.eglDestroyContext(display, context)) {
      Log.e("DefaultContextFactory", "display:" + display + " context: " + context);
      Log.i("DefaultContextFactory", "tid=" + Thread.currentThread().getId());
    }
  }
}
