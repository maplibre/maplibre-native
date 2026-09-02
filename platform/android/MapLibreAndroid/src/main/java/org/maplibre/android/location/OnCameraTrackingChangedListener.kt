package org.maplibre.android.location

import org.maplibre.android.location.modes.CameraMode

/**
 * Listener that gets invoked when camera tracking state changes.
 */
interface OnCameraTrackingChangedListener {
    /**
     * Invoked whenever camera tracking is broken.
     * This callback gets invoked just after [onCameraTrackingChanged], if needed.
     */
    fun onCameraTrackingDismissed()

    /**
     * Invoked on every [CameraMode] change.
     *
     * @param currentMode current active [CameraMode].
     */
    fun onCameraTrackingChanged(
        @CameraMode.Mode currentMode: Int,
    )
}
