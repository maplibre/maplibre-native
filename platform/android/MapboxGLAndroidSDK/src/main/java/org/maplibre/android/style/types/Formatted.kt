package org.maplibre.android.style.types

import androidx.annotation.Keep
import java.util.*

/**
 * Represents a string broken into sections annotated with separate formatting options.
 *
 * @see [Style specification](https://maplibre.org/maplibre-style-spec/.types-formatted)
 */
@Keep
class Formatted(vararg formattedSections: FormattedSection) {
    /**
     * Returns sections with separate formatting options that are a part of this formatted text.
     *
     * @return formatted sections
     */
    val formattedSections: Array<out FormattedSection>

    /**
     * Create a new formatted text.
     *
     * @param formattedSections sections with formatting options
     */
    init {
        this.formattedSections = formattedSections
    }

    fun toArray(): Array<Any?> {
        val sections = arrayOfNulls<Any>(formattedSections.size)
        for (i in formattedSections.indices) {
            sections[i] = formattedSections[i].toArray()
        }
        return sections
    }

    override fun equals(o: Any?): Boolean {
        if (this === o) {
            return true
        }
        if (o == null || javaClass != o.javaClass) {
            return false
        }
        val formatted = o as Formatted
        return Arrays.equals(formattedSections, formatted.formattedSections)
    }

    override fun hashCode(): Int {
        return Arrays.hashCode(formattedSections)
    }

    override fun toString(): String {
        return ("Formatted{" + "formattedSections=" + Arrays.toString(formattedSections) + '}')
    }
}
