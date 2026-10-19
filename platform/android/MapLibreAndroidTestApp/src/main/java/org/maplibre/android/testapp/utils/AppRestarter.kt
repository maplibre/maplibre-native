package org.maplibre.android.testapp.utils

import android.content.Context
import android.content.Intent

/**
 * Restarts the app: relaunches its launcher activity in a fresh task, then kills this
 * process. Used after changing process-lifetime state (like the rendering engine) that
 * only takes effect on the next launch.
 */
object AppRestarter {
    fun restart(context: Context) {
        val intent = context.packageManager.getLaunchIntentForPackage(context.packageName)
        requireNotNull(intent) { "No launch intent for package ${context.packageName}" }
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        context.startActivity(intent)
        Runtime.getRuntime().exit(0)
    }
}
