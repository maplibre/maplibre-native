package org.maplibre.android.maps.renderer

import androidx.annotation.Keep

/**
 * Can be used to schedule work on the map renderer
 * thread or request a render.
 */
interface MapRendererScheduler {
    @Keep
    fun requestRender()

    @Keep
    fun queueEvent(runnable: Runnable)

    @Keep
    fun waitForEmpty()
}
