package org.maplibre.android.net

import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.ConnectivityManager
import androidx.annotation.UiThread
import org.maplibre.android.MapLibre
import org.maplibre.android.log.Logger
import java.util.concurrent.CopyOnWriteArrayList

/**
 * Interface definition for a callback to be invoked when connectivity changes.
 * Not public api.
 */
class ConnectivityReceiver private constructor(
    private val context: Context,
) : BroadcastReceiver() {
    private val listeners: MutableList<ConnectivityListener> = CopyOnWriteArrayList()
    private var activationCounter = 0
    private var connected: Boolean? = null

    /**
     * Activates the connectivity receiver.
     *
     * if the underlying connectivity receiver isn't active, register the connectivity receiver.
     */
    @UiThread
    fun activate() {
        if (activationCounter == 0) {
            context.registerReceiver(this, IntentFilter("android.net.conn.CONNECTIVITY_CHANGE"))
        }
        activationCounter++
    }

    /**
     * Deactivates the connectivity receiver.
     *
     * if no other components are listening, unregister the underlying connectivity receiver.
     */
    @UiThread
    fun deactivate() {
        activationCounter--
        if (activationCounter == 0) {
            context.unregisterReceiver(INSTANCE)
        }
    }

    /**
     * {@inheritDoc}
     */
    override fun onReceive(
        context: Context,
        intent: Intent?,
    ) {
        if (connected != null) {
            // Connectivity state overridden by app
            return
        }

        notifyListeners(isNetworkActive())
    }

    /**
     * Overwrites system connectivity state. To set, use [MapLibre.setConnected].
     *
     * @param connected flag to determine the connectivity state, true for connected, false for
     *                  disconnected, and null for ConnectivityManager to determine.
     */
    fun setConnected(connected: Boolean?) {
        this.connected = connected
        notifyListeners(connected ?: isNetworkActive())
    }

    private fun notifyListeners(isConnected: Boolean) {
        Logger.v(TAG, if (isConnected) LOG_CONNECTED else LOG_NOT_CONNECTED)

        // Loop over listeners
        for (listener in listeners) {
            listener.onNetworkStateChanged(isConnected)
        }
    }

    /**
     * Add a listener to be notified
     *
     * @param listener the listener to add
     */
    fun addListener(listener: ConnectivityListener) {
        listeners.add(listener)
    }

    /**
     * Remove a listener
     *
     * @param listener the listener to remove
     */
    fun removeListener(listener: ConnectivityListener) {
        listeners.remove(listener)
    }

    /**
     * Get current connectivity state
     *
     * @return true if connected
     */
    val isConnected: Boolean
        get() = connected ?: isNetworkActive()

    @Suppress("DEPRECATION")
    private fun isNetworkActive(): Boolean {
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val activeNetwork = cm.activeNetworkInfo
        return activeNetwork != null && activeNetwork.isConnected
    }

    companion object {
        private const val TAG = "Mbgl-ConnectivityReceiver"
        private const val LOG_CONNECTED = "connected - true"
        private const val LOG_NOT_CONNECTED = "connected - false"

        @SuppressLint("StaticFieldLeak")
        @Suppress("ktlint:standard:property-naming")
        private var INSTANCE: ConnectivityReceiver? = null

        /**
         * Get a single instance of ConnectivityReceiver.
         *
         * @param context the context to extract the application context from
         * @return single instance of ConnectivityReceiver
         */
        @JvmStatic
        @Synchronized
        fun instance(context: Context): ConnectivityReceiver {
            var instance = INSTANCE
            if (instance == null) {
                // Register new instance
                instance = ConnectivityReceiver(context.applicationContext)
                INSTANCE = instance
                // Add default listeners
                instance.addListener(NativeConnectivityListener())
            }

            return instance
        }
    }
}
