package org.maplibre.android.maps.renderer.egl

import android.opengl.GLSurfaceView
import android.util.Log
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay

class EGLContextFactory : GLSurfaceView.EGLContextFactory {
    override fun createContext(
        egl: EGL10,
        display: EGLDisplay?,
        config: EGLConfig?,
    ): EGLContext {
        if (display == null || config == null) {
            return EGL10.EGL_NO_CONTEXT
        }
        val attribList = intArrayOf(EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE)
        return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attribList)
    }

    override fun destroyContext(
        egl: EGL10,
        display: EGLDisplay?,
        context: EGLContext?,
    ) {
        if (!egl.eglDestroyContext(display, context)) {
            Log.e(TAG, "display:$display context: $context")
            Log.i(TAG, "tid=" + Thread.currentThread().id)
        }
    }

    private companion object {
        private const val TAG = "DefaultContextFactory"
        private const val EGL_CONTEXT_CLIENT_VERSION = 0x3098
    }
}
