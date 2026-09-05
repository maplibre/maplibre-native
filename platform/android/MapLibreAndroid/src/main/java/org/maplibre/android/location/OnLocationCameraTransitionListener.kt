package org.maplibre.android.location

import org.maplibre.android.location.modes.CameraMode

/**
 * Callback for [CameraMode] transition state.
 */
interface OnLocationCameraTransitionListener {
    /**
     * Invoked when the camera mode transition animation has been finished.
     *
     * @param cameraMode camera mode change that initiated the transition
     */
    fun onLocationCameraTransitionFinished(
        @CameraMode.Mode cameraMode: Int,
    )

    /**
     * Invoked when the camera mode transition animation has been canceled.
     *
     * The camera mode is set regardless of the cancellation of the transition animation.
     *
     * @param cameraMode camera mode change that initiated the transition
     */
    fun onLocationCameraTransitionCanceled(
        @CameraMode.Mode cameraMode: Int,
    )
}
