package org.maplibre.android.location

import org.maplibre.android.location.modes.RenderMode

/**
 * Listener that gets invoked when layer render mode changes.
 */
fun interface OnRenderModeChangedListener {
    /**
     * Invoked on every [RenderMode] change.
     *
     * @param currentMode current active [RenderMode].
     */
    fun onRenderModeChanged(
        @RenderMode.Mode currentMode: Int,
    )
}
