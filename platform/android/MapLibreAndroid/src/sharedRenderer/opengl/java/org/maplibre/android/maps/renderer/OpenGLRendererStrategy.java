package org.maplibre.android.maps.renderer;

import android.content.Context;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.surfaceview.GLSurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.surfaceview.MapLibreGLSurfaceView;
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer;
import org.maplibre.android.maps.renderer.textureview.GLTextureViewRenderThread;
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer;

/**
 * OpenGL concrete impl behind {@link MapRendererFactory}. Lives alongside the
 * other GL renderer helpers in src/sharedRenderer/opengl/.
 */
final class OpenGLRendererStrategy {

  private OpenGLRendererStrategy() {}

  static void attachTextureRenderThread(@NonNull TextureView textureView,
                                        @NonNull TextureViewMapRenderer renderer) {
    renderer.setRenderThread(new GLTextureViewRenderThread(textureView, renderer));
  }

  static SurfaceViewMapRenderer createSurfaceViewRenderer(@NonNull Context context,
                                                          String localFontFamily,
                                                          boolean renderSurfaceOnTop,
                                                          Runnable initCallback) {
    MapLibreGLSurfaceView surfaceView = new MapLibreGLSurfaceView(context);
    surfaceView.setZOrderMediaOverlay(renderSurfaceOnTop);
    return new GLSurfaceViewMapRenderer(context, surfaceView, localFontFamily) {
      @Override
      public void onSurfaceCreated(Surface surface) {
        initCallback.run();
        super.onSurfaceCreated(surface);
      }
    };
  }
}
