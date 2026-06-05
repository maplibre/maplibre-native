package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.TextureView;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer;

/**
 * OpenGL flavor glue. Delegates straight to {@link OpenGLRendererStrategy}.
 * Exists only so the shared {@link MapRendererFactory} in main can call
 * {@code RendererStrategy.X(...)} without a per-flavor import.
 */
final class RendererStrategy {

  private RendererStrategy() {}

  static void attachTextureRenderThread(@NonNull TextureView textureView,
                                        @NonNull TextureViewMapRenderer renderer) {
    OpenGLRendererStrategy.attachTextureRenderThread(textureView, renderer);
  }

  static SurfaceViewMapRenderer createSurfaceViewRenderer(@NonNull Context context,
                                                          String localFontFamily,
                                                          boolean renderSurfaceOnTop,
                                                          Runnable initCallback) {
    return OpenGLRendererStrategy.createSurfaceViewRenderer(
        context, localFontFamily, renderSurfaceOnTop, initCallback);
  }
}
