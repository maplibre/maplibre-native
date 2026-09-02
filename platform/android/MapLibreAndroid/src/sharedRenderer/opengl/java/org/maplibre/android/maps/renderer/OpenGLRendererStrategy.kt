package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.Surface
import android.view.TextureView
import org.maplibre.android.maps.renderer.surfaceview.GLSurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.surfaceview.MapLibreGLSurfaceView
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.GLTextureViewRenderThread
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer

/**
 * OpenGL concrete impl behind [MapRendererFactory]. Lives alongside the
 * other GL renderer helpers in src/sharedRenderer/opengl/.
 */
internal object OpenGLRendererStrategy {
    fun attachTextureRenderThread(
        textureView: TextureView,
        renderer: TextureViewMapRenderer,
    ) {
        renderer.setRenderThread(GLTextureViewRenderThread(textureView, renderer))
    }

    fun createSurfaceViewRenderer(
        context: Context,
        localFontFamily: String?,
        renderSurfaceOnTop: Boolean,
        initCallback: Runnable,
    ): SurfaceViewMapRenderer {
        val surfaceView = MapLibreGLSurfaceView(context)
        surfaceView.setZOrderMediaOverlay(renderSurfaceOnTop)
        return object : GLSurfaceViewMapRenderer(context, surfaceView, localFontFamily) {
            override fun onSurfaceCreated(surface: Surface?) {
                initCallback.run()
                super.onSurfaceCreated(surface)
            }
        }
    }
}
