package org.maplibre.android.style.types

import androidx.annotation.ColorInt
import androidx.annotation.Keep
import org.maplibre.android.utils.ColorUtils
import java.util.Arrays

/**
 * A component of the [Formatted].
 *
 * @constructor Creates a formatted section.
 * @param text      displayed string
 * @param fontScale scale of the font, setting to null will fall back to style's default settings
 * @param fontStack main and fallback fonts that are a part of the style,
 *                  setting null will fall back to style's default settings.
 *                  The requested font stack has to be a part of the used style.
 *                  For more information see
 *                  [the documentation](https://www.mapbox.com/help/define-font-stack/).
 * @param textColor text color, setting to null will fall back to style's default settings.
 *                  Value of red, green, blue components must range between 0 and 255,
 *                  an alpha component must range between 0 and 1.
 *
 *                  For more information see
 *                  [the documentation](https://docs.mapbox.com/mapbox-gl-js/style-spec/#types-color).
 */
@Keep
class FormattedSection(
    private val text: String,
    private var fontScale: Number?,
    private var fontStack: Array<String>?,
    private var textColor: String?,
) {
    /**
     * Creates a formatted section.
     *
     * @param text displayed string
     */
    constructor(text: String) : this(text, null, null, null)

    /**
     * Creates a formatted section.
     *
     * @param text      displayed string
     * @param fontScale scale of the font, setting to null will fall back to style's default settings
     * @param fontStack main and fallback fonts that are a part of the style,
     *                  setting null will fall back to style's default settings
     */
    @Deprecated(
        "use FormattedSection(String) and setters or FormattedSection(String, Number, String[], String) instead",
    )
    constructor(text: String, fontScale: Number?, fontStack: Array<String>?) : this(text, fontScale, fontStack, null)

    /**
     * Creates a formatted section.
     *
     * @param text      displayed string
     * @param fontScale scale of the font, setting to null will fall back to style's default settings
     */
    @Deprecated(
        "use FormattedSection(String) and setters or FormattedSection(String, Number, String[], String) instead",
    )
    constructor(text: String, fontScale: Number?) : this(text, fontScale, null, null)

    /**
     * Creates a formatted section.
     *
     * @param text      displayed string
     * @param fontStack main and fallback fonts that are a part of the style,
     *                  setting null will fall back to style's default settings
     */
    @Deprecated(
        "use FormattedSection(String) and setters or FormattedSection(String, Number, String[], String) instead",
    )
    constructor(text: String, fontStack: Array<String>?) : this(text, null, fontStack, null)

    /**
     * Returns the displayed text.
     *
     * @return text
     */
    fun getText(): String = text

    /**
     * Returns displayed text's font scale.
     *
     * @return font scale
     */
    fun getFontScale(): Number? = fontScale

    /**
     * Returns the font stack with main and fallback fonts.
     *
     * @return font stack
     */
    fun getFontStack(): Array<String>? = fontStack

    /**
     * Returns the text color.
     *
     * @return text color
     */
    fun getTextColor(): String? = textColor

    /**
     * Set font scale. Setting to null will fall back to style's default settings.
     *
     * @param fontScale fontScale
     */
    fun setFontScale(fontScale: Number?) {
        // called from JNI
        this.fontScale = fontScale
    }

    /**
     * Set main and fallback fonts that are a part of the style.
     * Setting null will fall back to style's default settings.
     *
     * The requested font stack has to be a part of the used style.
     * For more information see [the documentation](https://www.mapbox.com/help/define-font-stack/).
     *
     * @param fontStack fontStack
     */
    fun setFontStack(fontStack: Array<String>?) {
        // called from JNI
        this.fontStack = fontStack
    }

    /**
     * Set text color. Setting to null will fall back to style's default settings.
     * Value of red, green, blue components must range between 0 and 255,
     * an alpha component must range between 0 and 1.
     *
     * For more information see
     * [the documentation](https://docs.mapbox.com/mapbox-gl-js/style-spec/#types-color).
     *
     * @param textColor text color
     */
    fun setTextColor(textColor: String?) {
        this.textColor = textColor
    }

    /**
     * Set the text color.
     *
     * @param textColor text color.
     */
    fun setTextColor(
        @ColorInt textColor: Int,
    ) {
        this.textColor = ColorUtils.colorToRgbaString(textColor)
    }

    @JvmName("setTextColor")
    internal fun setTextColor(textColor: Any) {
        // called from JNI
        this.textColor = textColor as String
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        val that = other as FormattedSection

        if (text != that.text) {
            return false
        }
        if (fontScale != that.fontScale) {
            return false
        }
        // Probably incorrect - comparing Object[] arrays with Arrays.equals
        if (!Arrays.equals(fontStack, that.fontStack)) {
            return false
        }
        return textColor == that.textColor
    }

    override fun hashCode(): Int {
        var result = text.hashCode()
        result = 31 * result + (fontScale?.hashCode() ?: 0)
        result = 31 * result + Arrays.hashCode(fontStack)
        result = 31 * result + (textColor?.hashCode() ?: 0)
        return result
    }

    internal fun toArray(): Array<Any?> {
        val params: MutableMap<String, Any?> = HashMap()
        params["font-scale"] = fontScale
        params["text-font"] = fontStack
        params["text-color"] = textColor
        return arrayOf(text, params)
    }

    override fun toString(): String =
        (
            "FormattedSection{" +
                "text='" + text + '\'' +
                ", fontScale=" + fontScale +
                ", fontStack=" + Arrays.toString(fontStack) +
                ", textColor='" + textColor + '\'' +
                '}'
        )
}
