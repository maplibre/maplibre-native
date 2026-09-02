package org.maplibre.android.maps.renderer.textureview

import android.content.Context
import android.view.Surface
import android.view.TextureView
import android.view.View
import org.maplibre.android.maps.renderer.MapRenderer

/**
 * The [TextureViewMapRenderer] encapsulates the render thread and
 * [TextureView] specifics to render the map.
 *
 * @param context                  the current Context
 * @param textureView              the TextureView
 * @param localIdeographFontFamily the local font family
 * @param translucentSurface       the translucency flag
 *
 * @see MapRenderer
 */
open class TextureViewMapRenderer(
    context: Context,
    private val textureView: TextureView,
    localIdeographFontFamily: String?,
    val isTranslucentSurface: Boolean,
) : MapRenderer(context, localIdeographFontFamily) {
    private lateinit var renderThread: TextureViewRenderThread

    fun setRenderThread(thread: TextureViewRenderThread) {
        renderThread = thread
        renderThread.name = "TextureViewRenderer"
        renderThread.start()
    }

    override val view: View
        get() = textureView

    /**
     * Overridden to widen visibility for the render threads.
     */
    public override fun onSurfaceCreated(surface: Surface?) {
        super.onSurfaceCreated(surface)
    }

    /**
     * Overridden to widen visibility for the render threads.
     */
    public override fun onSurfaceChanged(
        width: Int,
        height: Int,
    ) {
        super.onSurfaceChanged(width, height)
    }

    /**
     * Overridden to widen visibility for the render threads.
     */
    public override fun onSurfaceDestroyed() {
        super.onSurfaceDestroyed()
    }

    /**
     * Overridden to widen visibility for the render threads.
     */
    public override fun onDrawFrame() {
        super.onDrawFrame()
    }

    override fun requestRender() {
        renderThread.requestRender()
    }

    override fun queueEvent(runnable: Runnable) {
        renderThread.queueEvent(runnable)
    }

    override fun waitForEmpty() {
        renderThread.waitForEmpty()
    }

    override fun onStop() {
        renderThread.onPause()
    }

    override fun onStart() {
        renderThread.onResume()
    }

    override fun onDestroy() {
        renderThread.onDestroy()
    }

    override fun setRenderingRefreshMode(mode: RenderingRefreshMode): Unit =
        throw RuntimeException(
            "setRenderingRefreshMode is not supported for TextureViewMapRenderer. " +
                "Use SurfaceViewMapRenderer to set the rendering refresh mode.",
        )

    override fun getRenderingRefreshMode(): RenderingRefreshMode =
        throw RuntimeException(
            "getRenderingRefreshMode is not supported for TextureViewMapRenderer. " +
                "Use SurfaceViewMapRenderer to set the rendering refresh mode.",
        )
}
