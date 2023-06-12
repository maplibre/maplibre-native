package org.maplibre.android.testapp.utils

import android.content.Context
import android.util.TypedValue
import androidx.annotation.RawRes
import java.io.*

object ResourceUtils {
    @JvmStatic
    fun readRawResource(context: Context?, @RawRes rawResource: Int): String {
        var json = ""
        if (context != null) {
            val writer: Writer = StringWriter()
            val buffer = CharArray(1024)
            context.resources.openRawResource(rawResource).use { `is` ->
                val reader: Reader = BufferedReader(InputStreamReader(`is`, "UTF-8"))
                var numRead: Int
                while (reader.read(buffer).also { numRead = it } != -1) {
                    writer.write(buffer, 0, numRead)
                }
            }
            json = writer.toString()
        }
        return json
    }

    fun convertDpToPx(context: Context, dp: Float): Float {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            dp,
            context.resources.displayMetrics
        )
    }
}
