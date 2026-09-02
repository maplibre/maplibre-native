package org.maplibre.android.utils

import androidx.annotation.Keep
import java.text.Normalizer

/**
 * String utility class used by core from jni.
 */
@Keep
internal object StringUtils {
    /**
     * Normalises String input and strip diacritics from it.
     *
     * @return normalised String with stripped diacritics.
     */
    @Keep
    @JvmStatic
    fun unaccent(value: String): String =
        Normalizer
            .normalize(value, Normalizer.Form.NFD)
            .replace(
                (
                    "(\\p{InCombiningDiacriticalMarks}" +
                        "|\\p{InCombiningDiacriticalMarksForSymbols}" +
                        "|\\p{InCombiningDiacriticalMarksSupplement})+"
                ).toRegex(),
                "",
            )
}
