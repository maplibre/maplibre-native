package org.maplibre.android.net

import androidx.annotation.Keep
import org.maplibre.android.LibraryLoader

/**
 * Updates the native library's connectivity state
 */
internal class NativeConnectivityListener
    @Keep
    constructor(
        @field:Keep private var nativePtr: Long,
    ) : ConnectivityListener {
        @Keep
        private var invalidated = false

        constructor() : this(0) {
            initialize()
        }

        override fun onNetworkStateChanged(connected: Boolean) {
            nativeOnConnectivityStateChanged(connected)
        }

        @Keep
        protected external fun nativeOnConnectivityStateChanged(connected: Boolean)

        @Keep
        protected external fun initialize()

        @Keep
        @Throws(Throwable::class)
        protected external fun finalize()

        companion object {
            init {
                LibraryLoader.load()
            }
        }
    }
