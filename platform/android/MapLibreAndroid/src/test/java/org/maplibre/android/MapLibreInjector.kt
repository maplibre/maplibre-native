package org.maplibre.android

import android.content.Context
import org.maplibre.android.MapLibre
import org.maplibre.android.util.TileServerOptions

object MapLibreInjector {
    private const val FIELD_INSTANCE = "INSTANCE"
    @JvmStatic
    fun inject(context: Context, apiKey: String,
               options: TileServerOptions) {
        val maplibre = MapLibre(context, apiKey, options)
        try {
            val instance = MapLibre::class.java.getDeclaredField(FIELD_INSTANCE)
            instance.isAccessible = true
            instance[maplibre] = maplibre
        } catch (exception: Exception) {
            throw AssertionError()
        }
    }

    @JvmStatic
    fun clear() {
        try {
            val field = MapLibre::class.java.getDeclaredField(FIELD_INSTANCE)
            field.isAccessible = true
            field[field] = null
        } catch (exception: Exception) {
            throw AssertionError()
        }
    }
}
