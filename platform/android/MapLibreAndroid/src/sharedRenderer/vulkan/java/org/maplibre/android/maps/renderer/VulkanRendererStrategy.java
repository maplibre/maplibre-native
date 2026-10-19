package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.surfaceview.MapLibreVulkanSurfaceView;
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.surfaceview.VulkanSurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.VulkanTextureViewRenderThread;

/**
 * Vulkan concrete impl behind {@link MapRendererFactory}. Lives alongside the
 * other Vulkan renderer helpers in src/sharedRenderer/vulkan/.
 */
final class VulkanRendererStrategy {

  private VulkanRendererStrategy() {}

  static void attachTextureRenderThread(@NonNull TextureView textureView,
                                        @NonNull TextureViewMapRenderer renderer) {
    renderer.setRenderThread(new VulkanTextureViewRenderThread(textureView, renderer));
  }

  static SurfaceViewMapRenderer createSurfaceViewRenderer(@NonNull Context context,
                                                          String localFontFamily,
                                                          boolean renderSurfaceOnTop,
                                                          Runnable initCallback) {
    MapLibreVulkanSurfaceView surfaceView = new MapLibreVulkanSurfaceView(context);
    surfaceView.setZOrderMediaOverlay(renderSurfaceOnTop);
    return new VulkanSurfaceViewMapRenderer(context, surfaceView, localFontFamily) {
      @Override
      public void onSurfaceCreated(Surface surface) {
        initCallback.run();
        super.onSurfaceCreated(surface);
      }
    };
  }
}
