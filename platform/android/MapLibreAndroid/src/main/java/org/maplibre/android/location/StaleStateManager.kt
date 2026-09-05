package org.maplibre.android.location

import android.os.Handler
import android.os.Message
import java.lang.ref.WeakReference

/**
 * Class controls the location stale state when the [android.location.Location] hasn't
 * been updated in 'x' amount of time. [LocationComponentOptions.staleStateTimeout] can be used to
 * control the amount of time before the location's considered stale.
 * [LocationComponentOptions.enableStaleState] is available for disabling this behaviour.
 */
internal class StaleStateManager(
    private val innerOnLocationStaleListeners: OnLocationStaleListener,
    options: LocationComponentOptions,
) {
    private var isEnabled: Boolean = options.enableStaleState()
    private val handler: StaleMessageHandler = StaleMessageHandler(this)

    var isStale = true
        private set

    private var delayTime: Long = options.staleStateTimeout()

    private val staleStateMessage = 1

    fun setEnabled(enabled: Boolean) {
        if (enabled) {
            setState(isStale)
        } else if (isEnabled) {
            onStop()
            innerOnLocationStaleListeners.onStaleStateChange(false)
        }
        isEnabled = enabled
    }

    fun updateLatestLocationTime() {
        setState(false)
        postTheCallback()
    }

    fun setDelayTime(delayTime: Long) {
        this.delayTime = delayTime
        if (handler.hasMessages(staleStateMessage)) {
            postTheCallback()
        }
    }

    fun onStart() {
        if (!isStale) {
            postTheCallback()
        }
    }

    fun onStop() {
        handler.removeCallbacksAndMessages(null)
    }

    private fun postTheCallback() {
        handler.removeCallbacksAndMessages(null)
        handler.sendEmptyMessageDelayed(staleStateMessage, delayTime)
    }

    private fun setState(stale: Boolean) {
        if (stale != isStale) {
            isStale = stale
            if (isEnabled) {
                innerOnLocationStaleListeners.onStaleStateChange(stale)
            }
        }
    }

    private class StaleMessageHandler(
        staleStateManager: StaleStateManager,
    ) : Handler() {
        private val managerWeakReference = WeakReference(staleStateManager)

        override fun handleMessage(msg: Message) {
            managerWeakReference.get()?.setState(true)
        }
    }
}
