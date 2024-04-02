package org.maplibre.android.testapp.utils

import android.content.Context
import android.graphics.Typeface
import timber.log.Timber
import java.lang.Exception
import java.util.*

object FontCache {
    private val fontCache = Hashtable<String, Typeface?>()

    @JvmStatic
    operator fun get(name: String, context: Context): Typeface? {
        var tf = fontCache[name]
        if (tf == null) {
            try {
                tf = Typeface.createFromAsset(context.assets, name)
                fontCache[name] = tf
            } catch (exception: Exception) {
                Timber.e("Font not found")
            }
        }
        return tf
    }
}
