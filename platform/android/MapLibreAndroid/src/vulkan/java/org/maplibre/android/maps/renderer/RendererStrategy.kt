package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.TextureView
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer

/**
 * Vulkan flavor glue. Delegates straight to [VulkanRendererStrategy].
 * Exists only so the shared [MapRendererFactory] in main can call
 * `RendererStrategy.X(...)` without a per-flavor import.
 */
internal object RendererStrategy {
    fun attachTextureRenderThread(
        textureView: TextureView,
        renderer: TextureViewMapRenderer,
    ) {
        VulkanRendererStrategy.attachTextureRenderThread(textureView, renderer)
    }

    fun createSurfaceViewRenderer(
        context: Context,
        localFontFamily: String?,
        renderSurfaceOnTop: Boolean,
        initCallback: Runnable,
    ): SurfaceViewMapRenderer =
        VulkanRendererStrategy.createSurfaceViewRenderer(
            context,
            localFontFamily,
            renderSurfaceOnTop,
            initCallback,
        )
}
