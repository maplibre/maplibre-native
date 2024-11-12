package org.maplibre.android.testapp.utils

import android.content.Context
import android.os.Build
import android.os.Environment
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.contentOrNull
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import org.maplibre.android.MapLibre
import java.lang.Exception
import java.io.File

fun readFromJSON(): String? {
    val jsonFile = File("${Environment.getExternalStorageDirectory()}/instrumentation-test-input.json")
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R && Environment.isExternalStorageManager() && jsonFile.isFile) {
        val jsonFileContents = jsonFile.readText()
        val jsonElement = Json.parseToJsonElement(jsonFileContents)
        return jsonElement.jsonObject["apiKey"]?.jsonPrimitive?.contentOrNull
    }
    return null
}

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

        val fromJSON = readFromJSON()
        if (fromJSON !== null) return fromJSON

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
