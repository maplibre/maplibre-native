package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.TextureView
import org.maplibre.android.RenderingEngine
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer

/**
 * multiBackend flavor glue. Dispatches to [OpenGLRendererStrategy] or
 * [VulkanRendererStrategy] based on [RenderingEngine.currentType]
 * at the moment the renderer is constructed.
 */
internal object RendererStrategy {
    fun attachTextureRenderThread(
        textureView: TextureView,
        renderer: TextureViewMapRenderer,
    ) {
        if (RenderingEngine.currentType == RenderingEngine.Type.VULKAN) {
            VulkanRendererStrategy.attachTextureRenderThread(textureView, renderer)
        } else {
            OpenGLRendererStrategy.attachTextureRenderThread(textureView, renderer)
        }
    }

    fun createSurfaceViewRenderer(
        context: Context,
        localFontFamily: String?,
        renderSurfaceOnTop: Boolean,
        initCallback: Runnable,
    ): SurfaceViewMapRenderer =
        if (RenderingEngine.currentType == RenderingEngine.Type.VULKAN) {
            VulkanRendererStrategy.createSurfaceViewRenderer(context, localFontFamily, renderSurfaceOnTop, initCallback)
        } else {
            OpenGLRendererStrategy.createSurfaceViewRenderer(context, localFontFamily, renderSurfaceOnTop, initCallback)
        }
}
