package org.maplibre.android.http

import android.content.Context
import org.maplibre.android.MapLibre
import org.maplibre.android.MapStrictMode

object HttpIdentifier {
    /**
     * Returns the application identifier, consisting out the package name, version name and version code.
     *
     * @return the application identifier
     */
    @JvmStatic
    fun getIdentifier(): String = getIdentifier(MapLibre.getApplicationContext())

    /**
     * Returns the application identifier, consisting out the package name, version name and version code.
     *
     * @param context the context used to retrieve the package manager from
     * @return the application identifier
     */
    @Suppress("DEPRECATION")
    private fun getIdentifier(context: Context): String =
        try {
            val packageInfo = context.packageManager.getPackageInfo(context.packageName, 0)
            String.format("%s/%s (%s)", context.packageName, packageInfo.versionName, packageInfo.versionCode)
        } catch (exception: Exception) {
            MapStrictMode.strictModeViolation(exception)
            ""
        }
}
