package org.maplibre.android.testapp.utils

import android.content.Context
import org.maplibre.android.MapLibre
import java.lang.Exception

object ApiKeyUtils {
    /**
     *
     *
     * Returns the ApiKey set in the app resources.
     *
     * It will first search for a api key in the MapLibre object. If not found it
     * will then attempt to load the api key from the
     * `res/values/dev.xml` development file.
     *
     * @param context The [Context] of the [android.app.Activity] or [android.app.Fragment].
     * @return The api key or null if not found.
     */
    fun getApiKey(context: Context): String? {
        return try {
            // Read out AndroidManifest
            val apiKey = MapLibre.getApiKey()
            require(!(apiKey == null || apiKey.isEmpty()))
            apiKey
        } catch (exception: Exception) {
            // Use fallback on string resource, used for development
            // TODO:PP
            val apiKeyResId = context.resources
                .getIdentifier("api_key", "string", context.packageName)
            if (apiKeyResId != 0) context.getString(apiKeyResId) else null
        }
    }
}
