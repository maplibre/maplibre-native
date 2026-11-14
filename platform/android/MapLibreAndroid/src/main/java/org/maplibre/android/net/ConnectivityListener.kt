package org.maplibre.android.net

/**
 * Receives updates on connectivity state
 */
interface ConnectivityListener {
    fun onNetworkStateChanged(connected: Boolean)
}
