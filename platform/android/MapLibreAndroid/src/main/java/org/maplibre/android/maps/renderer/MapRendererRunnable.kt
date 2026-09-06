package org.maplibre.android.maps.renderer

import androidx.annotation.Keep

/**
 * Peer class for [Runnable]s to be scheduled on the [MapRenderer] thread.
 * The actual work is performed in the native peer.
 *
 * Constructed from the native peer constructor.
 *
 * @param nativePtr the native peer's memory address
 */
@Keep
internal class MapRendererRunnable(
    // Holds the pointer to the native peer after initialisation
    private val nativePtr: Long,
) : Runnable {
    external override fun run()

    @Suppress("unused", "ProtectedMemberInFinalClass")
    protected external fun finalize()

    @Suppress("unused")
    private external fun nativeInitialize()
}
