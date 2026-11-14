package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.TextureView
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer

/**
 * OpenGL flavor glue. Delegates straight to [OpenGLRendererStrategy].
 * Exists only so the shared [MapRendererFactory] in main can call
 * `RendererStrategy.X(...)` without a per-flavor import.
 */
internal object RendererStrategy {
    fun attachTextureRenderThread(
        textureView: TextureView,
        renderer: TextureViewMapRenderer,
    ) {
        OpenGLRendererStrategy.attachTextureRenderThread(textureView, renderer)
    }

    fun createSurfaceViewRenderer(
        context: Context,
        localFontFamily: String?,
        renderSurfaceOnTop: Boolean,
        initCallback: Runnable,
    ): SurfaceViewMapRenderer =
        OpenGLRendererStrategy.createSurfaceViewRenderer(
            context,
            localFontFamily,
            renderSurfaceOnTop,
            initCallback,
        )
}
