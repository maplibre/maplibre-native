package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer;

/**
 * Shared factory used by MapRenderer.create(). The shape of renderer construction
 * (anonymous subclass wiring of initCallback, render-thread attachment) lives here;
 * the backend-specific concrete-type instantiation is delegated to the
 * flavor-provided {@link RendererStrategy}.
 */
@Keep
public final class MapRendererFactory {

  private MapRendererFactory() {}

  public static TextureViewMapRenderer newTextureViewMapRenderer(@NonNull Context context,
                                                                 TextureView textureView,
                                                                 String localFontFamily,
                                                                 boolean translucentSurface,
                                                                 Runnable initCallback) {
    TextureViewMapRenderer mapRenderer = new TextureViewMapRenderer(context, textureView,
            localFontFamily, translucentSurface) {
      @Override
      protected void onSurfaceCreated(Surface surface) {
        initCallback.run();
        super.onSurfaceCreated(surface);
      }
    };
    RendererStrategy.attachTextureRenderThread(textureView, mapRenderer);
    return mapRenderer;
  }

  public static SurfaceViewMapRenderer newSurfaceViewMapRenderer(@NonNull Context context,
                                                                 String localFontFamily,
                                                                 boolean renderSurfaceOnTop,
                                                                 Runnable initCallback) {
    return RendererStrategy.createSurfaceViewRenderer(
        context, localFontFamily, renderSurfaceOnTop, initCallback);
  }
}
