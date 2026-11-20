package org.maplibre.android.utils

import android.content.Context
import android.content.res.ColorStateList
import android.graphics.Color
import android.os.Build
import android.util.TypedValue
import android.widget.ImageView
import androidx.annotation.ColorInt
import androidx.core.widget.ImageViewCompat
import org.maplibre.android.R
import org.maplibre.android.exceptions.ConversionException
import java.text.DecimalFormat
import java.text.NumberFormat
import java.util.Locale
import java.util.regex.Pattern

/**
 * Color utility class.
 */
object ColorUtils {
    /**
     * Returns a color integer associated as primary color from a theme based on a Context.
     *
     * @param context The context used to style the color attributes.
     * @return The primary color value of current theme in the form 0xAARRGGBB.
     */
    @JvmStatic
    @ColorInt
    fun getPrimaryColor(context: Context): Int =
        try {
            val typedValue = TypedValue()
            val theme = context.theme
            val id = context.resources.getIdentifier("colorPrimary", "attrs", context.packageName)
            theme.resolveAttribute(id, typedValue, true)
            typedValue.data
        } catch (exception: Exception) {
            getColorCompat(context, R.color.maplibre_blue)
        }

    /**
     * Returns a color integer associated as primary dark color from a theme based on a Context.
     *
     * @param context The context used to style the color attributes.
     * @return The primary dark color value of current theme in the form 0xAARRGGBB.
     */
    @JvmStatic
    @ColorInt
    fun getPrimaryDarkColor(context: Context): Int =
        try {
            val typedValue = TypedValue()
            val theme = context.theme
            val id = context.resources.getIdentifier("colorPrimaryDark", "attrs", context.packageName)
            theme.resolveAttribute(id, typedValue, true)
            typedValue.data
        } catch (exception: Exception) {
            getColorCompat(context, R.color.maplibre_blue)
        }

    /**
     * Returns a color integer associated as accent color from a theme based on a Context.
     *
     * @param context The context used to style the color attributes.
     * @return The accent color value of current theme in the form 0xAARRGGBB.
     */
    @JvmStatic
    @ColorInt
    fun getAccentColor(context: Context): Int =
        try {
            val typedValue = TypedValue()
            val theme = context.theme
            val id = context.resources.getIdentifier("colorAccent", "attrs", context.packageName)
            theme.resolveAttribute(id, typedValue, true)
            typedValue.data
        } catch (exception: Exception) {
            getColorCompat(context, R.color.maplibre_gray)
        }

    /**
     * Returns a color state list associated with a theme based on a Context.
     *
     * @param color The color used for tinting.
     * @return A ColorStateList object containing the primary color of a theme
     */
    @JvmStatic
    fun getSelector(
        @ColorInt color: Int,
    ): ColorStateList =
        ColorStateList(
            arrayOf(
                intArrayOf(android.R.attr.state_pressed),
                intArrayOf(),
            ),
            intArrayOf(
                color,
                color,
            ),
        )

    /**
     * Set a color tint list to the Drawable of an ImageView.
     *
     * @param imageView The view to set the default tint list.
     * @param tintColor The color to tint.
     */
    @JvmStatic
    fun setTintList(
        imageView: ImageView,
        @ColorInt tintColor: Int,
    ) {
        ImageViewCompat.setImageTintList(imageView, getSelector(tintColor))
    }

    /**
     * Convert an rgba string to a Color int.
     *
     * R, G, B color components have to be in the [0-255] range, while alpha has to be in the [0.0-1.0] range.
     * For example: "rgba(255, 128, 0, 0.7)".
     *
     * @param value the String representation of rgba
     * @return the int representation of rgba
     * @throws ConversionException on illegal input
     */
    @JvmStatic
    @ColorInt
    fun rgbaToColor(value: String): Int {
        // we need to accept and floor float values as well, as those can come from core
        val c =
            Pattern.compile(
                "rgba?\\s*\\(\\s*(\\d+\\.?\\d*)\\s*,\\s*(\\d+\\.?\\d*)\\s*,\\s*(\\d+\\.?\\d*)\\s*," +
                    "?\\s*(\\d+\\.?\\d*)?\\s*\\)",
            )
        val m = c.matcher(value)
        if (m.matches() && m.groupCount() == 3) {
            return Color.rgb(
                m.group(1)!!.toFloat().toInt(),
                m.group(2)!!.toFloat().toInt(),
                m.group(3)!!.toFloat().toInt(),
            )
        } else if (m.matches() && m.groupCount() == 4) {
            return Color.argb(
                (m.group(4)!!.toFloat() * 255).toInt(),
                m.group(1)!!.toFloat().toInt(),
                m.group(2)!!.toFloat().toInt(),
                m.group(3)!!.toFloat().toInt(),
            )
        } else {
            throw ConversionException("Not a valid rgb/rgba value")
        }
    }

    /**
     * Converts Android color int to "rbga(r, g, b, a)" String equivalent.
     *
     * Alpha value will be converted from 0-255 range to 0-1.
     *
     * @param color Android color int
     * @return String rgba color
     */
    @JvmStatic
    fun colorToRgbaString(
        @ColorInt color: Int,
    ): String {
        val numberFormat = NumberFormat.getNumberInstance(Locale.US)
        val decimalFormat = numberFormat as DecimalFormat
        decimalFormat.applyPattern("#.###")
        val alpha = decimalFormat.format(((color shr 24) and 0xFF).toFloat() / 255.0f)
        return String.format(
            Locale.US,
            "rgba(%d, %d, %d, %s)",
            (color shr 16) and 0xFF,
            (color shr 8) and 0xFF,
            color and 0xFF,
            alpha,
        )
    }

    /**
     * Converts Android color int to rgba float array.
     *
     * Returned RGB values range from 0 to 255.
     * Alpha value ranges from 0-1.
     *
     * @param color Android color int
     * @return float rgba array, rgb values range from 0-255, alpha from 0-1
     */
    @JvmStatic
    fun colorToRgbaArray(
        @ColorInt color: Int,
    ): FloatArray =
        floatArrayOf(
            ((color shr 16) and 0xFF).toFloat(), // r (0-255)
            ((color shr 8) and 0xFF).toFloat(), // g (0-255)
            (color and 0xFF).toFloat(), // b (0-255)
            ((color shr 24) and 0xFF) / 255.0f, // a (0-1)
        )

    /**
     * Converts Android color int to GL rgba float array.
     *
     * Returned values range from 0-1.
     *
     * @param color Android color int
     * @return float rgba array, values range from 0 to 1
     */
    @JvmStatic
    fun colorToGlRgbaArray(
        @ColorInt color: Int,
    ): FloatArray =
        floatArrayOf(
            ((color shr 16) and 0xFF) / 255.0f, // r (0-1)
            ((color shr 8) and 0xFF) / 255.0f, // g (0-1)
            (color and 0xFF) / 255.0f, // b (0-1)
            ((color shr 24) and 0xFF) / 255.0f, // a (0-1)
        )

    private fun getColorCompat(
        context: Context,
        id: Int,
    ): Int =
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            context.resources.getColor(id, context.theme)
        } else {
            @Suppress("DEPRECATION")
            context.resources.getColor(id)
        }
}
