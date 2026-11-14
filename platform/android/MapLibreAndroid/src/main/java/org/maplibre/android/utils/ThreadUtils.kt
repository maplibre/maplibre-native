package org.maplibre.android.utils

import android.content.Context
import android.content.pm.ApplicationInfo
import android.os.Looper
import org.maplibre.android.exceptions.CalledFromWorkerThreadException

/**
 * Utility class to verify if execution is running on the main thread.
 *
 * Verification only runs for debug builds.
 */
object ThreadUtils {
    private var debug: Boolean? = null

    /**
     * Initialises the thread utils, verifies debug state of the consuming app.
     *
     * @param context Context hosting the MapLibre Maps SDK for Android
     * @return this
     */
    @JvmStatic
    fun init(context: Context): ThreadUtils? {
        debug = (0 != (context.applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE))
        return null
    }

    /**
     * Validates if execution is running on the main thread.
     *
     * @param origin the origin of the execution
     */
    @JvmStatic
    fun checkThread(origin: String) {
        val isDebug = debug ?: throw IllegalStateException("ThreadUtils isn't correctly initialised")

        if (isDebug) {
            if (Looper.myLooper() != Looper.getMainLooper()) {
                throw CalledFromWorkerThreadException(
                    String.format("%s interactions should happen on the UI thread.", origin),
                )
            }
        }
    }
}
