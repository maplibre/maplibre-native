package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.TextureView;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer;

/**
 * Vulkan flavor glue. Delegates straight to {@link VulkanRendererStrategy}.
 * Exists only so the shared {@link MapRendererFactory} in main can call
 * {@code RendererStrategy.X(...)} without a per-flavor import.
 */
final class RendererStrategy {

  private RendererStrategy() {}

  static void attachTextureRenderThread(@NonNull TextureView textureView,
                                        @NonNull TextureViewMapRenderer renderer) {
    VulkanRendererStrategy.attachTextureRenderThread(textureView, renderer);
  }

  static SurfaceViewMapRenderer createSurfaceViewRenderer(@NonNull Context context,
                                                          String localFontFamily,
                                                          boolean renderSurfaceOnTop,
                                                          Runnable initCallback) {
    return VulkanRendererStrategy.createSurfaceViewRenderer(
        context, localFontFamily, renderSurfaceOnTop, initCallback);
  }
}
