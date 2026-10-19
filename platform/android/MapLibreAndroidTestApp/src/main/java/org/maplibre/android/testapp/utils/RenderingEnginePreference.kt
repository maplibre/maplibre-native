package org.maplibre.android.testapp.utils

import android.content.Context
import org.maplibre.android.RenderingEngine

/**
 * Persists the user's chosen [RenderingEngine.Type] across process restarts.
 *
 * Single-backend flavors lock the compiled-in backend and throw from
 * [RenderingEngine.setCurrentType] for any other value; the multiBackend
 * flavor accepts either value but only applies it on the next native library
 * load. Either way the choice needs to survive an app restart, which is what
 * this class is for.
 */
object RenderingEnginePreference {

  private const val PREFS_NAME = "rendering_engine_prefs"
  private const val KEY_ENGINE_TYPE = "engine_type"

  /** Returns the persisted choice, or null if the user never picked one. */
  fun load(context: Context): RenderingEngine.Type? {
    val stored = prefs(context).getString(KEY_ENGINE_TYPE, null) ?: return null
    return runCatching { RenderingEngine.Type.valueOf(stored) }.getOrNull()
  }

  fun save(context: Context, type: RenderingEngine.Type) {
    prefs(context).edit().putString(KEY_ENGINE_TYPE, type.name).commit()
  }

  fun clear(context: Context) {
      prefs(context).edit().clear().commit()
  }

  private fun prefs(context: Context) =
    context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
}
