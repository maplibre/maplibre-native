package org.maplibre.android.maps.renderer.surfaceview

import android.content.Context

open class VulkanSurfaceViewMapRenderer(
    context: Context,
    surfaceView: MapLibreVulkanSurfaceView,
    localIdeographFontFamily: String?,
) : SurfaceViewMapRenderer(context, surfaceView, localIdeographFontFamily) {
    init {
        this.surfaceView.setRenderer(this)
    }
}
