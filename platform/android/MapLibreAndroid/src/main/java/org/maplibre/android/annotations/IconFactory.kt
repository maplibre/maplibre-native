package org.maplibre.android.annotations

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.drawable.BitmapDrawable
import android.os.Build
import android.util.DisplayMetrics
import android.view.WindowManager
import androidx.annotation.DrawableRes
import org.maplibre.android.MapStrictMode
import org.maplibre.android.R
import org.maplibre.android.exceptions.TooManyIconsException
import org.maplibre.android.utils.BitmapUtils
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.IOException
import java.io.InputStream

/**
 * Factory for creating Icons from bitmap images.
 *
 * icon is used to display bitmaps on top of the map using [Marker].
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
class IconFactory private constructor(
    private val context: Context,
) {
    private var defaultMarker: Icon? = null
    private val options: BitmapFactory.Options
    private var nextId = 0

    init {
        var realMetrics: DisplayMetrics? = null
        val metrics = DisplayMetrics()
        val wm = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            realMetrics = DisplayMetrics()
            wm.defaultDisplay.getRealMetrics(realMetrics)
        }
        wm.defaultDisplay.getMetrics(metrics)

        options = BitmapFactory.Options()
        options.inScaled = true
        options.inDensity = DisplayMetrics.DENSITY_DEFAULT
        options.inTargetDensity = metrics.densityDpi
        if (realMetrics != null) {
            options.inScreenDensity = realMetrics.densityDpi
        }
    }

    /**
     * Creates an icon from a given Bitmap image.
     *
     * @param bitmap image used for creating the Icon.
     * @return The icon using the given Bitmap image.
     */
    fun fromBitmap(bitmap: Bitmap): Icon = iconFromBitmap(bitmap)

    /**
     * Creates an icon using the resource ID of a Bitmap image.
     *
     * @param resourceId The resource ID of a Bitmap image.
     * @return The icon that was loaded from the asset or `null` if failed to load.
     */
    fun fromResource(
        @DrawableRes resourceId: Int,
    ): Icon {
        val drawable = BitmapUtils.getDrawableFromRes(context, resourceId)
        if (drawable is BitmapDrawable) {
            return fromBitmap(drawable.bitmap)
        } else {
            throw IllegalArgumentException("Failed to decode image. The resource provided must be a Bitmap.")
        }
    }

    /**
     * Provides an icon using the default marker icon used for [Marker].
     *
     * @return An icon with the default [Marker] icon.
     */
    fun defaultMarker(): Icon =
        defaultMarker ?: fromResource(R.drawable.maplibre_marker_icon_default).also {
            defaultMarker = it
        }

    /**
     * Creates an Icon using the name of a Bitmap image in the assets directory.
     *
     * @param assetName The name of a Bitmap image in the assets directory.
     * @return The Icon that was loaded from the asset or null if failed to load.
     */
    fun fromAsset(assetName: String): Icon? {
        val stream: InputStream =
            try {
                context.assets.open(assetName)
            } catch (ioException: IOException) {
                MapStrictMode.strictModeViolation(ioException)
                return null
            }
        return fromInputStream(stream)
    }

    /**
     * Creates an Icon using the absolute file path of a Bitmap image.
     *
     * @param absolutePath The absolute path of the Bitmap image.
     * @return The Icon that was loaded from the absolute path or null if failed to load.
     */
    fun fromPath(absolutePath: String): Icon = iconFromBitmap(BitmapFactory.decodeFile(absolutePath, options))

    /**
     * Create an Icon using the name of a Bitmap image file located in the internal storage.
     * In particular, this calls Context#openFileInput(String).
     *
     * @param fileName The name of the Bitmap image file.
     * @return The Icon that was loaded from the asset or null if failed to load.
     * @see [Using the Internal Storage](https://developer.android.com/guide/topics/data/data-storage.html#filesInternal)
     */
    fun fromFile(fileName: String): Icon? {
        val stream: FileInputStream =
            try {
                context.openFileInput(fileName)
            } catch (fileNotFoundException: FileNotFoundException) {
                MapStrictMode.strictModeViolation(fileNotFoundException)
                return null
            }
        return fromInputStream(stream)
    }

    private fun fromInputStream(stream: InputStream): Icon = iconFromBitmap(BitmapFactory.decodeStream(stream, null, options))

    private fun iconFromBitmap(bitmap: Bitmap?): Icon {
        if (nextId < 0) {
            throw TooManyIconsException()
        }
        val id = ICON_ID_PREFIX + ++nextId
        return Icon(id, bitmap)
    }

    companion object {
        private const val ICON_ID_PREFIX = "com.mapbox.icons.icon_"

        @SuppressLint("StaticFieldLeak")
        private var instance: IconFactory? = null

        /**
         * Get a single instance of IconFactory.
         *
         * @param context the context to derive the application context from
         * @return the single instance of IconFactory
         */
        @JvmStatic
        @Synchronized
        fun getInstance(context: Context): IconFactory = instance ?: IconFactory(context.applicationContext).also { instance = it }

        /**
         * Create an Icon using a previously created icon identifier along with a provided Bitmap.
         *
         * @param iconId The Icon identifier you'd like to recreate.
         * @param bitmap a Bitmap used to replace the current one.
         * @return The Icon using the new Bitmap.
         */
        @JvmStatic
        fun recreate(
            iconId: String,
            bitmap: Bitmap,
        ): Icon = Icon(iconId, bitmap)
    }
}
