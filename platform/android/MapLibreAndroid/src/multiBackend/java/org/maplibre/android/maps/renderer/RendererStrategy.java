package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.TextureView;

import androidx.annotation.NonNull;

import org.maplibre.android.RenderingEngine;
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer;

/**
 * multiBackend flavor glue. Dispatches to {@link OpenGLRendererStrategy} or
 * {@link VulkanRendererStrategy} based on {@link RenderingEngine#getCurrentType()}
 * at the moment the renderer is constructed.
 */
final class RendererStrategy {

  private RendererStrategy() {}

  static void attachTextureRenderThread(@NonNull TextureView textureView,
                                        @NonNull TextureViewMapRenderer renderer) {
    if (RenderingEngine.getCurrentType() == RenderingEngine.Type.VULKAN) {
      VulkanRendererStrategy.attachTextureRenderThread(textureView, renderer);
    } else {
      OpenGLRendererStrategy.attachTextureRenderThread(textureView, renderer);
    }
  }

  static SurfaceViewMapRenderer createSurfaceViewRenderer(@NonNull Context context,
                                                          String localFontFamily,
                                                          boolean renderSurfaceOnTop,
                                                          Runnable initCallback) {
    if (RenderingEngine.getCurrentType() == RenderingEngine.Type.VULKAN) {
      return VulkanRendererStrategy.createSurfaceViewRenderer(
          context, localFontFamily, renderSurfaceOnTop, initCallback);
    }
    return OpenGLRendererStrategy.createSurfaceViewRenderer(
        context, localFontFamily, renderSurfaceOnTop, initCallback);
  }
}
