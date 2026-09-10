package org.maplibre.android.maps

import android.os.Handler
import android.os.Message
import androidx.annotation.IntDef
import org.maplibre.android.maps.MapLibreMap.OnCameraIdleListener
import org.maplibre.android.maps.MapLibreMap.OnCameraMoveCanceledListener
import org.maplibre.android.maps.MapLibreMap.OnCameraMoveListener
import org.maplibre.android.maps.MapLibreMap.OnCameraMoveStartedListener
import java.lang.ref.WeakReference
import java.util.concurrent.CopyOnWriteArrayList

/**
 * Class responsible for dispatching camera change events to registered listeners.
 */
@Suppress("TooManyFunctions")
internal class CameraChangeDispatcher :
    MapLibreMap.OnCameraMoveStartedListener,
    MapLibreMap.OnCameraMoveListener,
    MapLibreMap.OnCameraMoveCanceledListener,
    OnCameraIdleListener {
    private val handler = CameraChangeHandler(this)

    private var idle = true
    private var moveStartedReason = 0

    private val onCameraMoveStarted = CopyOnWriteArrayList<OnCameraMoveStartedListener>()
    private val onCameraMoveCanceled = CopyOnWriteArrayList<OnCameraMoveCanceledListener>()
    private val onCameraMove = CopyOnWriteArrayList<OnCameraMoveListener>()
    private val onCameraIdle = CopyOnWriteArrayList<OnCameraIdleListener>()

    @Retention(AnnotationRetention.SOURCE)
    @IntDef(MOVE_STARTED, MOVE, MOVE_CANCELED, IDLE)
    internal annotation class CameraChange

    override fun onCameraMoveStarted(reason: Int) {
        moveStartedReason = reason
        handler.scheduleMessage(MOVE_STARTED)
    }

    override fun onCameraMove() {
        handler.scheduleMessage(MOVE)
    }

    override fun onCameraMoveCanceled() {
        handler.scheduleMessage(MOVE_CANCELED)
    }

    override fun onCameraIdle() {
        handler.scheduleMessage(IDLE)
    }

    fun addOnCameraIdleListener(listener: OnCameraIdleListener) {
        onCameraIdle.add(listener)
    }

    fun removeOnCameraIdleListener(listener: OnCameraIdleListener) {
        onCameraIdle.remove(listener)
    }

    fun addOnCameraMoveCancelListener(listener: OnCameraMoveCanceledListener) {
        onCameraMoveCanceled.add(listener)
    }

    fun removeOnCameraMoveCancelListener(listener: OnCameraMoveCanceledListener) {
        onCameraMoveCanceled.remove(listener)
    }

    fun addOnCameraMoveStartedListener(listener: OnCameraMoveStartedListener) {
        onCameraMoveStarted.add(listener)
    }

    fun removeOnCameraMoveStartedListener(listener: OnCameraMoveStartedListener) {
        onCameraMoveStarted.remove(listener)
    }

    fun addOnCameraMoveListener(listener: OnCameraMoveListener) {
        onCameraMove.add(listener)
    }

    fun removeOnCameraMoveListener(listener: OnCameraMoveListener) {
        onCameraMove.remove(listener)
    }

    private fun executeOnCameraMoveStarted() {
        if (!idle) {
            return
        }
        idle = false
        for (cameraMoveStartedListener in onCameraMoveStarted) {
            cameraMoveStartedListener.onCameraMoveStarted(moveStartedReason)
        }
    }

    private fun executeOnCameraMove() {
        if (!idle) {
            for (cameraMoveListener in onCameraMove) {
                cameraMoveListener.onCameraMove()
            }
        }
    }

    private fun executeOnCameraMoveCancelled() {
        if (!idle) {
            for (cameraMoveCanceledListener in onCameraMoveCanceled) {
                cameraMoveCanceledListener.onCameraMoveCanceled()
            }
        }
    }

    private fun executeOnCameraIdle() {
        if (idle) {
            return
        }
        idle = true
        for (cameraIdleListener in onCameraIdle) {
            cameraIdleListener.onCameraIdle()
        }
    }

    fun onDestroy() {
        handler.removeCallbacksAndMessages(null)
        onCameraMoveStarted.clear()
        onCameraMoveCanceled.clear()
        onCameraMove.clear()
        onCameraIdle.clear()
    }

    @Suppress("DEPRECATION")
    private class CameraChangeHandler(
        dispatcher: CameraChangeDispatcher,
    ) : Handler() {
        private val dispatcherWeakReference = WeakReference(dispatcher)

        override fun handleMessage(msg: Message) {
            val dispatcher = dispatcherWeakReference.get() ?: return
            when (msg.what) {
                MOVE_STARTED -> dispatcher.executeOnCameraMoveStarted()
                MOVE -> dispatcher.executeOnCameraMove()
                MOVE_CANCELED -> dispatcher.executeOnCameraMoveCancelled()
                IDLE -> dispatcher.executeOnCameraIdle()
            }
        }

        fun scheduleMessage(
            @CameraChange change: Int,
        ) {
            val dispatcher = dispatcherWeakReference.get() ?: return

            // if there is a movement that is cancelled/stopped and restarted in the same code block
            // we can safely assume that the movement will continue,
            // no need for dispatching unnecessary callbacks sequence
            if (change == MOVE_STARTED) {
                val shouldReturn = !dispatcher.idle && (hasMessages(IDLE) || hasMessages(MOVE_CANCELED))
                removeMessages(IDLE)
                removeMessages(MOVE_CANCELED)

                if (shouldReturn) {
                    return
                }
            }

            val message = Message()
            message.what = change
            this.sendMessage(message)
        }
    }

    private companion object {
        const val MOVE_STARTED = 0
        const val MOVE = 1
        const val MOVE_CANCELED = 2
        const val IDLE = 3
    }
}
