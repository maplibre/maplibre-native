package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.Surface
import android.view.TextureView
import org.maplibre.android.maps.renderer.surfaceview.MapLibreVulkanSurfaceView
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.surfaceview.VulkanSurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.VulkanTextureViewRenderThread

/**
 * Vulkan concrete impl behind [MapRendererFactory]. Lives alongside the
 * other Vulkan renderer helpers in src/sharedRenderer/vulkan/.
 */
internal object VulkanRendererStrategy {
    fun attachTextureRenderThread(
        textureView: TextureView,
        renderer: TextureViewMapRenderer,
    ) {
        renderer.setRenderThread(VulkanTextureViewRenderThread(textureView, renderer))
    }

    fun createSurfaceViewRenderer(
        context: Context,
        localFontFamily: String?,
        renderSurfaceOnTop: Boolean,
        initCallback: Runnable,
    ): SurfaceViewMapRenderer {
        val surfaceView = MapLibreVulkanSurfaceView(context)
        surfaceView.setZOrderMediaOverlay(renderSurfaceOnTop)
        return object : VulkanSurfaceViewMapRenderer(context, surfaceView, localFontFamily) {
            override fun onSurfaceCreated(surface: Surface?) {
                initCallback.run()
                super.onSurfaceCreated(surface)
            }
        }
    }
}
