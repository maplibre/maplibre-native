package org.maplibre.android.maps

import android.graphics.PointF
import android.os.Handler
import android.os.Looper
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.ViewConfiguration

/**
 * Manages key events on a MapView.
 *
 *  - Uses [Transform] to change the map state
 *  - Uses [UiSettings] to verify validity of user restricted movement.
 */
internal class MapKeyListener(
    private val transform: Transform,
    private val uiSettings: UiSettings,
    private val mapGestureDetector: MapGestureDetector,
) {
    private var currentTrackballLongPressTimeOut: TrackballLongPressTimeOut? = null

    /**
     * Called when the user presses a key, alse called for repeated keys held down.
     *
     * @param keyCode the id of the pressed key
     * @param event   the related key event
     * @return true if the wevent is handled
     */
    fun onKeyDown(
        keyCode: Int,
        event: KeyEvent,
    ): Boolean {
        // If the user has held the scroll key down for a while then accelerate
        // the scroll speed
        val scrollDist = if (event.repeatCount >= 5) 50.0 else 10.0

        // Check which key was pressed via hardware/real key code
        when (keyCode) {
            // Tell the system to track these keys for long presses on
            // onKeyLongPress is fired
            KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_DPAD_CENTER -> {
                event.startTracking()
                return true
            }

            KeyEvent.KEYCODE_DPAD_LEFT -> {
                if (!uiSettings.isScrollGesturesEnabled) {
                    return false
                }

                // Cancel any animation
                transform.cancelTransitions()

                // Move left
                transform.moveBy(scrollDist, 0.0, 0)
                return true
            }

            KeyEvent.KEYCODE_DPAD_RIGHT -> {
                if (!uiSettings.isScrollGesturesEnabled) {
                    return false
                }

                // Cancel any animation
                transform.cancelTransitions()

                // Move right
                transform.moveBy(-scrollDist, 0.0, 0)
                return true
            }

            KeyEvent.KEYCODE_DPAD_UP -> {
                if (!uiSettings.isScrollGesturesEnabled) {
                    return false
                }

                // Cancel any animation
                transform.cancelTransitions()

                // Move up
                transform.moveBy(0.0, scrollDist, 0)
                return true
            }

            KeyEvent.KEYCODE_DPAD_DOWN -> {
                if (!uiSettings.isScrollGesturesEnabled) {
                    return false
                }

                // Cancel any animation
                transform.cancelTransitions()

                // Move down
                transform.moveBy(0.0, -scrollDist, 0)
                return true
            }

            else -> {
                // We are not interested in this key
                return false
            }
        }
    }

    /**
     * Called when the user long presses a key that is being tracked.
     *
     * @param keyCode the id of the long pressed key
     * @param event   the related key event
     * @return true if event is handled
     */
    fun onKeyLongPress(
        keyCode: Int,
        event: KeyEvent,
    ): Boolean {
        // Check which key was pressed via hardware/real key code
        when (keyCode) {
            // Tell the system to track these keys for long presses on
            // onKeyLongPress is fired
            KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_DPAD_CENTER -> {
                if (!uiSettings.isZoomGesturesEnabled) {
                    return false
                }

                // Zoom out
                mapGestureDetector.zoomOutAnimated(centerPoint(), true)
                return true
            }

            else -> {
                // We are not interested in this key
                return false
            }
        }
    }

    /**
     * Called when the user releases a key.
     *
     * @param keyCode the id of the released key
     * @param event   the related key event
     * @return true if the event is handled
     */
    fun onKeyUp(
        keyCode: Int,
        event: KeyEvent,
    ): Boolean {
        // Check if the key action was canceled (used for virtual keyboards)
        if (event.isCanceled) {
            return false
        }

        // Check which key was pressed via hardware/real key code
        // Note if keyboard does not have physical key (ie primary non-shifted
        // key) then it will not appear here
        // Must use the key character map as physical to character is not
        // fixed/guaranteed
        when (keyCode) {
            KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_DPAD_CENTER -> {
                if (!uiSettings.isZoomGesturesEnabled) {
                    return false
                }

                // Zoom in
                mapGestureDetector.zoomInAnimated(centerPoint(), true)
                return true
            }
        }

        // We are not interested in this key
        return false
    }

    /**
     * Called for trackball events, all motions are relative in device specific units.
     *
     * @param event the related motion event
     * @return true if the event is handled
     */
    fun onTrackballEvent(event: MotionEvent): Boolean {
        // Choose the action
        when (event.actionMasked) {
            // The trackball was rotated
            MotionEvent.ACTION_MOVE -> {
                if (!uiSettings.isScrollGesturesEnabled) {
                    return false
                }

                // Cancel any animation
                transform.cancelTransitions()

                // Scroll the map
                transform.moveBy(-10.0 * event.x, -10.0 * event.y, 0)
                return true
            }

            // Trackball was pushed in so start tracking and tell system we are
            // interested
            // We will then get the up action
            MotionEvent.ACTION_DOWN -> {
                // Set up a delayed callback to check if trackball is still
                // After waiting the system long press time out
                currentTrackballLongPressTimeOut?.cancel()
                val timeOut = TrackballLongPressTimeOut()
                currentTrackballLongPressTimeOut = timeOut
                Handler(Looper.getMainLooper())
                    .postDelayed(timeOut, ViewConfiguration.getLongPressTimeout().toLong())
                return true
            }

            // Trackball was released
            MotionEvent.ACTION_UP -> {
                if (!uiSettings.isZoomGesturesEnabled) {
                    return false
                }

                // Only handle if we have not already long pressed
                if (currentTrackballLongPressTimeOut != null) {
                    // Zoom in
                    mapGestureDetector.zoomInAnimated(centerPoint(), true)
                }
                return true
            }

            // Trackball was cancelled
            MotionEvent.ACTION_CANCEL -> {
                currentTrackballLongPressTimeOut?.cancel()
                currentTrackballLongPressTimeOut = null
                return true
            }

            else -> {
                // We are not interested in this event
                return false
            }
        }
    }

    private fun centerPoint(): PointF = PointF(uiSettings.width / 2, uiSettings.height / 2)

    /**
     * This class implements the trackball long press time out callback
     */
    private inner class TrackballLongPressTimeOut : Runnable {
        // Track if we have been cancelled
        private var cancelled = false

        // Cancel the timeout
        fun cancel() {
            cancelled = true
        }

        // Called when long press time out expires
        override fun run() {
            // Check if the trackball is still pressed
            if (!cancelled) {
                // Zoom out
                mapGestureDetector.zoomOutAnimated(centerPoint(), true)

                // Ensure the up action is not run
                currentTrackballLongPressTimeOut = null
            }
        }
    }
}
