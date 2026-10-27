package org.maplibre.plugins.android

import android.content.Context
import org.json.JSONObject
import java.lang.reflect.InvocationTargetException

/**
 * A plugin demo. Every plugin module carries `assets/demo/<id>/demo.json`, which plugins/tools/generate.ts writes from
 * the `demo` block of the plugin's plugin.json, so a new plugin shows up here without any change to this app.
 */
class Demo(
    val id: String,
    val displayName: String,
    val style: String,
    val camera: JSONObject?,
    private val registerClass: String,
) {
    /** Registers the plugin's layer types. Returns false when the active renderer does not implement the plugin. */
    fun register(): Boolean = try {
        Class.forName(registerClass).getMethod("register").invoke(null) as Boolean
    } catch (error: InvocationTargetException) {
        throw error.targetException
    }

    companion object {
        fun all(context: Context): List<Demo> =
            (context.assets.list("demo") ?: emptyArray()).sorted().map { load(context, it) }

        fun find(context: Context, id: String): Demo? = all(context).firstOrNull { it.id == id }

        private fun load(context: Context, id: String): Demo {
            val json = JSONObject(context.assets.open("demo/$id/demo.json").bufferedReader().use { it.readText() })
            return Demo(
                id = id,
                displayName = json.getString("displayName"),
                style = json.getString("style"),
                camera = json.optJSONObject("camera"),
                registerClass = json.getString("registerClass"),
            )
        }
    }
}
