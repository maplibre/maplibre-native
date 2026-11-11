package org.maplibre.android.testapp.utils

import android.app.Activity
import android.app.Application
import android.os.Bundle
import android.view.View
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.updatePadding

/**
 * Implemented by activities that draw behind the system bars on purpose and handle the window insets themselves.
 */
interface EdgeToEdgeActivity

/**
 * Since targetSdk 35 every activity is drawn edge-to-edge. Keeps the content of the test app activities clear of
 * the system bars and the action bar, unless the activity is an [EdgeToEdgeActivity].
 */
object SystemBarInsets : Application.ActivityLifecycleCallbacks {

    override fun onActivityPostCreated(activity: Activity, savedInstanceState: Bundle?) {
        if (activity is EdgeToEdgeActivity) {
            return
        }
        val content = activity.findViewById<View>(android.R.id.content) ?: return
        ViewCompat.setOnApplyWindowInsetsListener(content) { view, insets ->
            val bars = insets.getInsets(WindowInsetsCompat.Type.systemBars() or WindowInsetsCompat.Type.displayCutout())
            view.updatePadding(left = bars.left, top = bars.top, right = bars.right, bottom = bars.bottom)
            WindowInsetsCompat.CONSUMED
        }
    }

    override fun onActivityCreated(activity: Activity, savedInstanceState: Bundle?) {}

    override fun onActivityStarted(activity: Activity) {}

    override fun onActivityResumed(activity: Activity) {}

    override fun onActivityPaused(activity: Activity) {}

    override fun onActivityStopped(activity: Activity) {}

    override fun onActivitySaveInstanceState(activity: Activity, outState: Bundle) {}

    override fun onActivityDestroyed(activity: Activity) {}
}
