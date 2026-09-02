package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.Surface
import android.view.TextureView
import androidx.annotation.Keep
import org.maplibre.android.maps.renderer.surfaceview.SurfaceViewMapRenderer
import org.maplibre.android.maps.renderer.textureview.TextureViewMapRenderer

/**
 * Shared factory used by MapRenderer.create(). The shape of renderer construction
 * (anonymous subclass wiring of initCallback, render-thread attachment) lives here;
 * the backend-specific concrete-type instantiation is delegated to the
 * flavor-provided `RendererStrategy`.
 */
@Keep
object MapRendererFactory {
    @JvmStatic
    fun newTextureViewMapRenderer(
        context: Context,
        textureView: TextureView,
        localFontFamily: String?,
        translucentSurface: Boolean,
        initCallback: Runnable,
    ): TextureViewMapRenderer {
        val mapRenderer =
            object : TextureViewMapRenderer(context, textureView, localFontFamily, translucentSurface) {
                override fun onSurfaceCreated(surface: Surface?) {
                    initCallback.run()
                    super.onSurfaceCreated(surface)
                }
            }
        RendererStrategy.attachTextureRenderThread(textureView, mapRenderer)
        return mapRenderer
    }

    @JvmStatic
    fun newSurfaceViewMapRenderer(
        context: Context,
        localFontFamily: String?,
        renderSurfaceOnTop: Boolean,
        initCallback: Runnable,
    ): SurfaceViewMapRenderer =
        RendererStrategy.createSurfaceViewRenderer(
            context,
            localFontFamily,
            renderSurfaceOnTop,
            initCallback,
        )
}
