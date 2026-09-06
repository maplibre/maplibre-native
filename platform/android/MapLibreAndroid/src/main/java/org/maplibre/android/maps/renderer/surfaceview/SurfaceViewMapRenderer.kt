package org.maplibre.android.maps.renderer.surfaceview

import android.content.Context
import android.view.Surface
import android.view.View
import org.maplibre.android.maps.renderer.MapRenderer

/**
 * The [SurfaceViewMapRenderer] encapsulates the render thread and
 * [MapLibreSurfaceView] specifics to render the map.
 *
 * @see MapRenderer
 */
open class SurfaceViewMapRenderer(
    context: Context,
    @JvmField protected val surfaceView: MapLibreSurfaceView,
    localIdeographFontFamily: String?,
) : MapRenderer(context, localIdeographFontFamily) {
    init {
        surfaceView.setDetachedListener {
            // because the render thread is destroyed when the view is detached from window,
            // we need to ensure releasing the native renderer as well.
            // This avoids releasing it only when the view is being recreated, which is already
            // on a new render thread, and leads to JNI crashes like
            // https://github.com/mapbox/mapbox-gl-native/issues/14618
            nativeReset()
        }
    }

    override val view: View
        get() = surfaceView

    override fun onStop() {
        surfaceView.onPause()
    }

    override fun onPause() {
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    override fun onStart() {
        surfaceView.onResume()
    }

    override fun onResume() {
        super.onResume()
    }

    public override fun onSurfaceCreated(surface: Surface?) {
        super.onSurfaceCreated(surface)
    }

    public override fun onSurfaceDestroyed() {
        super.onSurfaceDestroyed()
    }

    public override fun onSurfaceChanged(
        width: Int,
        height: Int,
    ) {
        super.onSurfaceChanged(width, height)
    }

    public override fun onDrawFrame() {
        super.onDrawFrame()
    }

    /**
     * May be called from any thread.
     *
     * Called from the renderer frontend to schedule a render.
     */
    override fun requestRender() {
        surfaceView.requestRender()
    }

    /**
     * May be called from any thread.
     *
     * Schedules work to be performed on the MapRenderer thread.
     *
     * @param runnable the runnable to execute
     */
    override fun queueEvent(runnable: Runnable) {
        surfaceView.queueEvent(runnable)
    }

    override fun waitForEmpty() {
        surfaceView.waitForEmpty()
    }

    override fun setRenderingRefreshMode(mode: RenderingRefreshMode) {
        surfaceView.setRenderingRefreshMode(mode)
    }

    override fun getRenderingRefreshMode(): RenderingRefreshMode = surfaceView.getRenderingRefreshMode()
}
