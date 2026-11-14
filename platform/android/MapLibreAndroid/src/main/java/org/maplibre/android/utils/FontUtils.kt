package org.maplibre.android.utils

import android.graphics.Typeface
import android.os.Build
import androidx.annotation.RequiresApi
import org.maplibre.android.MapStrictMode
import org.maplibre.android.constants.MapLibreConstants.DEFAULT_FONT
import org.maplibre.android.log.Logger

/**
 * Utility class to select a font from a range of font names based on the availability of fonts on the device.
 */
object FontUtils {
    private const val TAG = "Mbgl-FontUtils"
    private const val TYPEFACE_FONTMAP_FIELD_NAME = "sSystemFontMap"
    private val DEFAULT_FONT_STACKS = listOf("sans-serif", "serif", "monospace")

    /**
     * Select a font from a range of font names to match the availability of fonts on the device.
     *
     * @param fontNames the range of font names to select from
     * @return the selected fon
     */
    @JvmStatic
    fun extractValidFont(vararg fontNames: String?): String? {
        if (fontNames.isEmpty()) {
            return null
        }

        val validFonts =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                getDeviceFonts()
            } else {
                DEFAULT_FONT_STACKS
            }

        for (fontName in fontNames) {
            if (fontName != null && validFonts.contains(fontName)) {
                return fontName
            }
        }

        Logger.i(
            TAG,
            String.format("Couldn't map font family for local ideograph, using %s instead", DEFAULT_FONT),
        )
        return DEFAULT_FONT
    }

    @Suppress("JavaReflectionMemberAccess", "UNCHECKED_CAST")
    @RequiresApi(Build.VERSION_CODES.LOLLIPOP)
    private fun getDeviceFonts(): List<String> {
        val fonts = mutableListOf<String>()
        try {
            val typeface = Typeface.create(Typeface.DEFAULT, Typeface.NORMAL)
            val field = Typeface::class.java.getDeclaredField(TYPEFACE_FONTMAP_FIELD_NAME)
            field.isAccessible = true
            val fontMap = field.get(typeface) as Map<String, Typeface>
            fonts.addAll(fontMap.keys)
        } catch (exception: Exception) {
            Logger.e(TAG, "Couldn't load fonts from Typeface", exception)
            MapStrictMode.strictModeViolation("Couldn't load fonts from Typeface", exception)
        }
        return fonts
    }
}
