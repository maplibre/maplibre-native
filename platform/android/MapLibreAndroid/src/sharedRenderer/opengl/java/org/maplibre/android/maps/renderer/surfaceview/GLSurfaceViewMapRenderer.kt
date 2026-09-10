package org.maplibre.android.maps.renderer.surfaceview

import android.content.Context
import org.maplibre.android.maps.renderer.MapRenderer
import org.maplibre.android.maps.renderer.egl.EGLConfigChooser
import org.maplibre.android.maps.renderer.egl.EGLContextFactory
import org.maplibre.android.maps.renderer.egl.EGLWindowSurfaceFactory

open class GLSurfaceViewMapRenderer(
    context: Context,
    surfaceView: MapLibreGLSurfaceView,
    localIdeographFontFamily: String?,
) : SurfaceViewMapRenderer(context, surfaceView, localIdeographFontFamily) {
    init {
        surfaceView.setEGLContextFactory(EGLContextFactory())
        surfaceView.setEGLWindowSurfaceFactory(EGLWindowSurfaceFactory())
        surfaceView.setEGLConfigChooser(EGLConfigChooser())
        surfaceView.setRenderer(this)
        surfaceView.setRenderingRefreshMode(MapRenderer.RenderingRefreshMode.WHEN_DIRTY)
        surfaceView.preserveEGLContextOnPause = true
    }
}
