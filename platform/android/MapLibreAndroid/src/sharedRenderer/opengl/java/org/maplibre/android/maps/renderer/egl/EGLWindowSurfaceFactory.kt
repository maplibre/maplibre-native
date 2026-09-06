package org.maplibre.android.maps.renderer.egl

import android.opengl.GLSurfaceView
import android.util.Log
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLDisplay
import javax.microedition.khronos.egl.EGLSurface

class EGLWindowSurfaceFactory : GLSurfaceView.EGLWindowSurfaceFactory {
    override fun createWindowSurface(
        egl: EGL10,
        display: EGLDisplay?,
        config: EGLConfig?,
        nativeWindow: Any?,
    ): EGLSurface? {
        var result: EGLSurface? = null
        if (display != null && config != null && nativeWindow != null) {
            try {
                result = egl.eglCreateWindowSurface(display, config, nativeWindow, null)
            } catch (exception: Exception) {
                // This exception indicates that the surface flinger surface
                // is not valid. This can happen if the surface flinger surface has
                // been torn down, but the application has not yet been
                // notified via SurfaceHolder.Callback.surfaceDestroyed.
                // In theory the application should be notified first,
                // but in practice sometimes it is not. See b/4588890
                Log.e(TAG, "eglCreateWindowSurface", exception)
            }
        }
        return result
    }

    override fun destroySurface(
        egl: EGL10,
        display: EGLDisplay?,
        surface: EGLSurface?,
    ) {
        egl.eglDestroySurface(display, surface)
    }

    private companion object {
        private const val TAG = "EGLWindowSurfaceFactory"
    }
}
