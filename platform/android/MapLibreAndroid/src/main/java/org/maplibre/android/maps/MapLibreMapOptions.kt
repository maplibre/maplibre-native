package org.maplibre.android.maps

import android.content.Context
import android.content.res.TypedArray
import android.graphics.Bitmap
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.os.Parcel
import android.os.Parcelable
import android.text.TextUtils
import android.util.AttributeSet
import android.view.Gravity
import androidx.annotation.ColorInt
import androidx.annotation.IntRange
import androidx.annotation.VisibleForTesting
import androidx.core.content.res.ResourcesCompat
import org.maplibre.android.R
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.utils.BitmapUtils
import org.maplibre.android.utils.FontUtils

/**
 * Defines configuration MapLibreMapOptions for a MapLibreMap. These options can be used when adding a
 * map to your application programmatically (as opposed to via XML). If you are using a MapFragment,
 * you can pass these options in using the static factory method newInstance(MapLibreMapOptions).
 * If you are using a MapView, you can pass these options in using the constructor
 * MapView(Context, MapLibreMapOptions). If you add a map using XML, then you can apply these options
 * using custom XML tags.
 */
@Suppress("TooManyFunctions", "LargeClass")
open class MapLibreMapOptions
/**
     * Creates a new MapLibreMapOptions object.
     *
     * @deprecated Use [createFromAttributes] instead.
     */
    @Deprecated("Use createFromAttributes(Context, AttributeSet) instead")
    constructor() : Parcelable {
        private var cameraPosition: CameraPosition? = null

        /**
         * Get the current configured debug state for a map view.
         *
         * @return True indicates debug is enabled.
         */
        var debugActive = false
            private set

        /**
         * Get the current configured visibility state for maplibre_compass_icon for a map view.
         *
         * @return Visibility state of the maplibre_compass_icon
         */
        var compassEnabled = true
            private set
        private var fadeCompassFacingNorth = true

        /**
         * Get the current configured gravity state for maplibre_compass_icon for a map view.
         *
         * @return Gravity state of the maplibre_compass_icon
         */
        var compassGravity = Gravity.TOP or Gravity.END
            private set

        /**
         * Get the current configured margins for maplibre_compass_icon for a map view.
         *
         * @return Margins state of the maplibre_compass_icon
         */
        var compassMargins: IntArray? = null
            private set

        /**
         * Get the current configured CompassView image.
         *
         * @return the drawable used as compass image
         */
        var compassImage: Drawable? = null
            private set

        /**
         * Get the current configured visibility state for maplibre_compass_icon for a map view.
         *
         * @return Visibility state of the maplibre_compass_icon
         */
        var logoEnabled = true
            private set

        /**
         * Get the current configured gravity state for logo for a map view.
         *
         * @return Gravity state of the logo
         */
        var logoGravity = Gravity.BOTTOM or Gravity.START
            private set

        /**
         * Get the current configured margins for logo for a map view.
         *
         * @return Margins state of the logo
         */
        var logoMargins: IntArray? = null
            private set

        /**
         * Get the current configured tint color for attribution for a map view.
         *
         * @return the tint color
         */
        @get:ColorInt
        var attributionTintColor = UNDEFINED_COLOR
            private set

        /**
         * Get the current configured visibility state for attribution for a map view.
         *
         * @return Visibility state of the attribution
         */
        var attributionEnabled = true
            private set

        /**
         * Get the current configured gravity state for attribution for a map view.
         *
         * @return Gravity state of the logo
         */
        var attributionGravity = Gravity.BOTTOM or Gravity.START
            private set

        /**
         * Get the current configured margins for attribution for a map view.
         *
         * @return Margins state of the logo
         */
        var attributionMargins: IntArray? = null
            private set

        private var minZoom: Double = MapLibreConstants.MINIMUM_ZOOM.toDouble()
        private var maxZoom: Double = MapLibreConstants.MAXIMUM_ZOOM.toDouble()
        private var minPitch: Double = MapLibreConstants.MINIMUM_PITCH.toDouble()
        private var maxPitch: Double = MapLibreConstants.MAXIMUM_PITCH.toDouble()

        /**
         * Get the current configured rotate gesture state for a map view.
         *
         * @return True indicates gesture is enabled
         */
        var rotateGesturesEnabled = true
            private set

        /**
         * Get the current configured scroll gesture state for a map view.
         *
         * @return True indicates gesture is enabled
         */
        var scrollGesturesEnabled = true
            private set

        /**
         * Get the current configured horizontal scroll gesture state for a map view.
         *
         * @return True indicates horizontal scroll gesture is enabled
         */
        var horizontalScrollGesturesEnabled = true
            private set

        /**
         * Get the current configured tilt gesture state for a map view.
         *
         * @return True indicates gesture is enabled
         */
        var tiltGesturesEnabled = true
            private set

        /**
         * Get the current configured zoom gesture state for a map view.
         *
         * @return True indicates gesture is enabled
         */
        var zoomGesturesEnabled = true
            private set

        /**
         * Get the current configured double tap gesture state for a map view.
         *
         * @return True indicates gesture is enabled
         */
        var doubleTapGesturesEnabled = true
            private set

        /**
         * Get whether the user may zoom the map by tapping twice, holding and moving the pointer up and down.
         *
         * @return True indicates gesture is enabled
         */
        var quickZoomGesturesEnabled = true
            private set

        /**
         * Check whether tile pre-fetching is enabled.
         *
         * @return true if enabled
         * @deprecated Use [getPrefetchZoomDelta] instead.
         */
        @Deprecated("Use getPrefetchZoomDelta() instead.", ReplaceWith("getPrefetchZoomDelta()"))
        var prefetchesTiles = true
            private set

        /**
         * Check current pre-fetching zoom delta.
         *
         * @return current zoom delta.
         */
        @get:IntRange(from = 0)
        var prefetchZoomDelta = 4
            private set
        private var zMediaOverlay = false

        private var localIdeographFontFamilyEnabled = true

        /**
         * Returns the font-family for locally overriding generation of glyphs in the
         * &#x27;CJK Unified Ideographs&#x27; and &#x27;Hangul Syllables&#x27; ranges.
         * Default font for local ideograph font family is [MapLibreConstants.DEFAULT_FONT].
         * Returns null if local ideograph font families are disabled.
         *
         * @return Local ideograph font family name.
         */
        var localIdeographFontFamily: String? = null
            get() = if (localIdeographFontFamilyEnabled) field else null
            private set

        private var localIdeographFontFamilies: Array<String>? = null

        /**
         * Get the current configured API endpoint base URI.
         *
         * @return Base URI to be used API endpoint.
         */
        var apiBaseUri: String? = null
            private set

        /**
         * Returns true if TextureView is being used the render view.
         *
         * @return True if TextureView is used.
         */
        var textureMode = false
            private set

        /**
         * Returns true if TextureView supports a translucent surface
         *
         * @return True if translucent surface is active
         */
        var translucentTextureSurface = false
            private set

        /**
         * Returns the current configured foreground color that is used during map creation.
         *
         * @return the load color
         */
        @get:ColorInt
        var foregroundLoadColor = 0
            private set

        /**
         * Return the custom configured pixel ratio, returns 0 if not configured.
         *
         * @return the pixel ratio used by the map under construction
         */
        var pixelRatio = 0f
            private set

        /**
         * Check whether cross-source symbol collision detection is enabled.
         *
         * @return true if enabled
         */
        var crossSourceCollisions = true
            private set

        /**
         * Check whether action journal logging is enabled.
         *
         * @return true if enabled
         */
        var actionJournalEnabled = false
            private set

        /**
         * Get the current configured action journal log path.
         *
         * @return log file path
         */
        var actionJournalPath = ""
            private set

        /**
         * Get the current configured action journal log file size.
         * Total log size is equal to `actionJournalLogFileSize * actionJournalLogFileCount`.
         *
         * @return maximum file size
         */
        var actionJournalLogFileSize = (1024 * 1024).toLong()
            private set

        /**
         * Get the current configured action journal log file count.
         * Total log size is equal to `actionJournalLogFileSize * actionJournalLogFileCount`.
         *
         * @return maximum log files used
         */
        var actionJournalLogFileCount = 5L
            private set

        /**
         * Get the current configured action journal rendering stats report time interval.
         *
         * @return time interval in seconds
         */
        var actionJournalRenderingReportInterval = 60
            private set

        /**
         * Check whether the renderer backend is async
         *
         * @return true if async
         */
        var asyncRendererCleanup = false
            private set

        /**
         * Check whether FastPFOR decompression is enabled for vector tiles.
         *
         * @return true if enabled
         */
        var fastPFOREnabled = false
            private set

        @Suppress("DEPRECATION", "LongMethod")
        private constructor(parcel: Parcel) : this() {
            cameraPosition = parcel.readParcelable(CameraPosition::class.java.classLoader)
            debugActive = parcel.readBooleanByte()

            compassEnabled = parcel.readBooleanByte()
            compassGravity = parcel.readInt()
            compassMargins = parcel.createIntArray()
            fadeCompassFacingNorth = parcel.readBooleanByte()

            val compassBitmap = parcel.readParcelable<Bitmap>(javaClass.classLoader)
            if (compassBitmap != null) {
                compassImage = BitmapDrawable(compassBitmap)
            }

            logoEnabled = parcel.readBooleanByte()
            logoGravity = parcel.readInt()
            logoMargins = parcel.createIntArray()

            attributionEnabled = parcel.readBooleanByte()
            attributionGravity = parcel.readInt()
            attributionMargins = parcel.createIntArray()
            attributionTintColor = parcel.readInt()

            minZoom = parcel.readDouble()
            maxZoom = parcel.readDouble()
            minPitch = parcel.readDouble()
            maxPitch = parcel.readDouble()

            rotateGesturesEnabled = parcel.readBooleanByte()
            scrollGesturesEnabled = parcel.readBooleanByte()
            horizontalScrollGesturesEnabled = parcel.readBooleanByte()
            tiltGesturesEnabled = parcel.readBooleanByte()
            zoomGesturesEnabled = parcel.readBooleanByte()
            doubleTapGesturesEnabled = parcel.readBooleanByte()
            quickZoomGesturesEnabled = parcel.readBooleanByte()

            apiBaseUri = parcel.readString()
            textureMode = parcel.readBooleanByte()
            translucentTextureSurface = parcel.readBooleanByte()
            prefetchesTiles = parcel.readBooleanByte()
            prefetchZoomDelta = parcel.readInt()
            zMediaOverlay = parcel.readBooleanByte()
            localIdeographFontFamilyEnabled = parcel.readBooleanByte()
            localIdeographFontFamily = parcel.readString()
            localIdeographFontFamilies = parcel.createStringArray()
            pixelRatio = parcel.readFloat()
            foregroundLoadColor = parcel.readInt()
            crossSourceCollisions = parcel.readBooleanByte()

            actionJournalEnabled = parcel.readBooleanByte()
            actionJournalPath = parcel.readString() ?: ""
            actionJournalLogFileSize = parcel.readLong()
            actionJournalLogFileCount = parcel.readLong()
            actionJournalRenderingReportInterval = parcel.readInt()

            asyncRendererCleanup = parcel.readBooleanByte()

            fastPFOREnabled = parcel.readBooleanByte()
        }

        /**
         * Specifies the URL used for API endpoint.
         *
         * @param apiBaseUrl The base of our API endpoint
         * @return This
         * @deprecated use [apiBaseUri] instead
         */
        @Deprecated("use apiBaseUri instead", ReplaceWith("apiBaseUri(apiBaseUrl)"))
        fun apiBaseUrl(apiBaseUrl: String?): MapLibreMapOptions {
            apiBaseUri = apiBaseUrl
            return this
        }

        /**
         * Specifies the URI used for API endpoint.
         *
         * @param apiBaseUri The base of our API endpoint
         * @return This
         */
        fun apiBaseUri(apiBaseUri: String?): MapLibreMapOptions {
            this.apiBaseUri = apiBaseUri
            return this
        }

        /**
         * Specifies a the initial camera position for the map view.
         *
         * @param cameraPosition Inital camera position
         * @return This
         */
        fun camera(cameraPosition: CameraPosition?): MapLibreMapOptions {
            this.cameraPosition = cameraPosition
            return this
        }

        /**
         * Specifies the used debug type for a map view.
         *
         * @param enabled True is debug is enabled
         * @return This
         */
        fun debugActive(enabled: Boolean): MapLibreMapOptions {
            debugActive = enabled
            return this
        }

        /**
         * Specifies the used minimum zoom level for a map view.
         *
         * @param minZoom Zoom level to be used
         * @return This
         */
        fun minZoomPreference(minZoom: Double): MapLibreMapOptions {
            this.minZoom = minZoom
            return this
        }

        /**
         * Specifies the used maximum zoom level for a map view.
         *
         * @param maxZoom Zoom level to be used
         * @return This
         */
        fun maxZoomPreference(maxZoom: Double): MapLibreMapOptions {
            this.maxZoom = maxZoom
            return this
        }

        /**
         * Specifies the used minimum pitch for a map view.
         *
         * @param minPitch Pitch to be used
         * @return This
         */
        fun minPitchPreference(minPitch: Double): MapLibreMapOptions {
            this.minPitch = minPitch
            return this
        }

        /**
         * Specifies the used maximum pitch for a map view.
         *
         * @param maxPitch Pitch to be used
         * @return This
         */
        fun maxPitchPreference(maxPitch: Double): MapLibreMapOptions {
            this.maxPitch = maxPitch
            return this
        }

        /**
         * Specifies the visibility state of a maplibre_compass_icon for a map view.
         *
         * @param enabled True and maplibre_compass_icon is shown
         * @return This
         */
        fun compassEnabled(enabled: Boolean): MapLibreMapOptions {
            compassEnabled = enabled
            return this
        }

        /**
         * Specifies the gravity state of maplibre_compass_icon for a map view.
         *
         * @param gravity Android SDK Gravity.
         * @return This
         */
        fun compassGravity(gravity: Int): MapLibreMapOptions {
            compassGravity = gravity
            return this
        }

        /**
         * Specifies the margin state of maplibre_compass_icon for a map view
         *
         * @param margins 4 long array for LTRB margins
         * @return This
         */
        fun compassMargins(margins: IntArray?): MapLibreMapOptions {
            compassMargins = margins
            return this
        }

        /**
         * Specifies if the maplibre_compass_icon fades to invisible when facing north.
         *
         * By default this value is true.
         *
         * @param compassFadeWhenFacingNorth true is maplibre_compass_icon fades to invisble
         * @return This
         */
        fun compassFadesWhenFacingNorth(compassFadeWhenFacingNorth: Boolean): MapLibreMapOptions {
            fadeCompassFacingNorth = compassFadeWhenFacingNorth
            return this
        }

        /**
         * Specifies the image of the CompassView.
         *
         * By default this value is R.drawable.maplibre_compass_icon.
         *
         * @param compass the drawable to show as image compass
         * @return This
         */
        fun compassImage(compass: Drawable?): MapLibreMapOptions {
            compassImage = compass
            return this
        }

        /**
         * Specifies the visibility state of a logo for a map view.
         *
         * @param enabled True and logo is shown
         * @return This
         */
        fun logoEnabled(enabled: Boolean): MapLibreMapOptions {
            logoEnabled = enabled
            return this
        }

        /**
         * Specifies the gravity state of logo for a map view.
         *
         * @param gravity Android SDK Gravity.
         * @return This
         */
        fun logoGravity(gravity: Int): MapLibreMapOptions {
            logoGravity = gravity
            return this
        }

        /**
         * Specifies the margin state of logo for a map view
         *
         * @param margins 4 long array for LTRB margins
         * @return This
         */
        fun logoMargins(margins: IntArray?): MapLibreMapOptions {
            logoMargins = margins
            return this
        }

        /**
         * Specifies the visibility state of a attribution for a map view.
         *
         * @param enabled True and attribution is shown
         * @return This
         */
        fun attributionEnabled(enabled: Boolean): MapLibreMapOptions {
            attributionEnabled = enabled
            return this
        }

        /**
         * Specifies the gravity state of attribution for a map view.
         *
         * @param gravity Android SDK Gravity.
         * @return This
         */
        fun attributionGravity(gravity: Int): MapLibreMapOptions {
            attributionGravity = gravity
            return this
        }

        /**
         * Specifies the margin state of attribution for a map view
         *
         * @param margins 4 long array for LTRB margins
         * @return This
         */
        fun attributionMargins(margins: IntArray?): MapLibreMapOptions {
            attributionMargins = margins
            return this
        }

        /**
         * Specifies the tint color of the attribution for a map view
         *
         * @param color integer resembling a color
         * @return This
         */
        fun attributionTintColor(
            @ColorInt color: Int,
        ): MapLibreMapOptions {
            attributionTintColor = color
            return this
        }

        /**
         * Specifies if the rotate gesture is enabled for a map view.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun rotateGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            rotateGesturesEnabled = enabled
            return this
        }

        /**
         * Specifies if the scroll gesture is enabled for a map view.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun scrollGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            scrollGesturesEnabled = enabled
            return this
        }

        /**
         * Specifies if the horizontal scroll gesture is enabled for a map view.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun horizontalScrollGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            horizontalScrollGesturesEnabled = enabled
            return this
        }

        /**
         * Specifies if the tilt gesture is enabled for a map view.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun tiltGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            tiltGesturesEnabled = enabled
            return this
        }

        /**
         * Specifies if the zoom gesture is enabled for a map view.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun zoomGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            zoomGesturesEnabled = enabled
            return this
        }

        /**
         * Specifies if the double tap gesture is enabled for a map view.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun doubleTapGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            doubleTapGesturesEnabled = enabled
            return this
        }

        /**
         * Specifies whether the user may zoom the map by tapping twice, holding and moving the pointer up and down.
         *
         * @param enabled True and gesture will be enabled
         * @return This
         */
        fun quickZoomGesturesEnabled(enabled: Boolean): MapLibreMapOptions {
            quickZoomGesturesEnabled = enabled
            return this
        }

        /**
         * Enable [android.view.TextureView] as rendered surface.
         *
         * Since the 5.2.0 release we replaced our TextureView with an [android.opengl.GLSurfaceView]
         * implementation. Enabling this option will use the [android.view.TextureView] instead.
         * [android.view.TextureView] can be useful in situations where you need to animate, scale
         * or transform the view. This comes at a siginficant performance penalty and should not be considered
         * unless absolutely needed.
         *
         * @param textureMode True to enable texture mode
         * @return This
         */
        fun textureMode(textureMode: Boolean): MapLibreMapOptions {
            this.textureMode = textureMode
            return this
        }

        fun translucentTextureSurface(translucentTextureSurface: Boolean): MapLibreMapOptions {
            this.translucentTextureSurface = translucentTextureSurface
            return this
        }

        /**
         * Set the MapView foreground color that is used when the map surface is being created.
         *
         * @param loadColor the color to show during map creation
         * @return This
         */
        fun foregroundLoadColor(
            @ColorInt loadColor: Int,
        ): MapLibreMapOptions {
            foregroundLoadColor = loadColor
            return this
        }

        /**
         * Enable tile pre-fetching. Loads tiles at a lower zoom-level to pre-render
         * a low resolution preview while more detailed tiles are loaded.
         * Enabled by default
         *
         * @param enable true to enable
         * @return This
         * @deprecated Use [setPrefetchZoomDelta] instead.
         */
        @Deprecated("Use setPrefetchZoomDelta(int) instead.")
        fun setPrefetchesTiles(enable: Boolean): MapLibreMapOptions {
            prefetchesTiles = enable
            return this
        }

        /**
         * Set the tile pre-fetching zoom delta. Pre-fetching makes sure that a low-resolution
         * tile at the (current_zoom_level - delta) is rendered as soon as possible at the
         * expense of a little bandwidth.
         * Note: This operation will override the MapLibreMapOptions#setPrefetchesTiles(boolean)
         * Setting zoom delta to 0 will disable pre-fetching.
         * Default zoom delta is 4.
         *
         * @param delta zoom delta
         * @return This
         */
        fun setPrefetchZoomDelta(
            @IntRange(from = 0) delta: Int,
        ): MapLibreMapOptions {
            prefetchZoomDelta = delta
            return this
        }

        /**
         * Enable cross-source symbol collision detection, defaults to true.
         *
         * If set to false, symbol layers will only run collision detection against
         * other symbol layers that are part of the same source.
         *
         * @param crossSourceCollisions true to enable, false to disable
         * @return This
         */
        fun crossSourceCollisions(crossSourceCollisions: Boolean): MapLibreMapOptions {
            this.crossSourceCollisions = crossSourceCollisions
            return this
        }

        /**
         * Enable action journal event logging, defaults to false.
         *
         * If set to true, enables map event file logging (obtainable via MapView#getActionJournalLog)
         *
         * @param actionJournalEnabled true to enable, false to disable
         * @return This
         */
        fun actionJournalEnabled(actionJournalEnabled: Boolean): MapLibreMapOptions {
            this.actionJournalEnabled = actionJournalEnabled
            return this
        }

        /**
         * Set the action journal log path.
         *
         * @param actionJournalPath Path to be used
         * @return This
         */
        fun actionJournalPath(actionJournalPath: String): MapLibreMapOptions {
            this.actionJournalPath = actionJournalPath
            return this
        }

        /**
         * Set the action journal log file size.
         *
         * The action journal uses a rolling log with multiple files.
         * Total log size is equal to `actionJournalLogFileSize * actionJournalLogFileCount`.
         *
         * @param actionJournalLogFileSize maximum log file size
         * @return This
         */
        fun actionJournalLogFileSize(actionJournalLogFileSize: Long): MapLibreMapOptions {
            this.actionJournalLogFileSize = actionJournalLogFileSize
            return this
        }

        /**
         * Set the action journal log file count.
         *
         * The action journal uses a rolling log with multiple files.
         * Total log size is equal to `actionJournalLogFileSize * actionJournalLogFileCount`.
         *
         * @param actionJournalLogFileCount maximum number of log files
         * @return This
         */
        fun actionJournalLogFileCount(actionJournalLogFileCount: Long): MapLibreMapOptions {
            this.actionJournalLogFileCount = actionJournalLogFileCount
            return this
        }

        /**
         * Set the number of seconds to wait between rendering stats reports.
         *
         * @param actionJournalRenderingReportInterval time interval in seconds
         * @return This
         */
        fun actionJournalRenderingReportInterval(actionJournalRenderingReportInterval: Int): MapLibreMapOptions {
            this.actionJournalRenderingReportInterval = actionJournalRenderingReportInterval
            return this
        }

        /**
         * By default, backend cleanup executes on the render thread
         * (with the main thread waiting for the task to finish).
         * Enabling this defers cleanup work to the garbage collector/finalzer thread. This can reduce
         * shutdown stalls on the main thread, but introduces a non-deterministic cleanup timing and
         * might expose possible OpenGL driver issues.
         *
         * @param asyncRendererCleanup true to enable, false to disable
         * @return This
         */
        fun asyncRendererCleanup(asyncRendererCleanup: Boolean): MapLibreMapOptions {
            this.asyncRendererCleanup = asyncRendererCleanup
            return this
        }

        /**
         * Enable FastPFOR decompression for vector tiles, defaults to false.
         *
         * @param enable true to enable, false to disable
         * @return This
         */
        fun fastPFOREnabled(enable: Boolean): MapLibreMapOptions {
            fastPFOREnabled = enable
            return this
        }

        /**
         * Enable local ideograph font family, defaults to true.
         *
         * @param enabled true to enable, false to disable
         * @return This
         */
        fun localIdeographFontFamilyEnabled(enabled: Boolean): MapLibreMapOptions {
            localIdeographFontFamilyEnabled = enabled
            return this
        }

        /**
         * Set the font family for generating glyphs locally for ideographs in the &#x27;CJK Unified Ideographs&#x27;
         * and &#x27;Hangul Syllables&#x27; ranges.
         *
         * The font family argument is passed to [android.graphics.Typeface.create].
         * Default system fonts are defined in &#x27;/system/etc/fonts.xml&#x27;
         * Default font for local ideograph font family is [MapLibreConstants.DEFAULT_FONT].
         *
         * @param fontFamily font family for local ideograph generation.
         * @return This
         */
        fun localIdeographFontFamily(fontFamily: String?): MapLibreMapOptions {
            localIdeographFontFamily = FontUtils.extractValidFont(fontFamily)
            return this
        }

        /**
         * Set a font family from range of font families for generating glyphs locally for ideographs in the
         * &#x27;CJK Unified Ideographs&#x27; and &#x27;Hangul Syllables&#x27; ranges. The first matching font
         * will be selected. If no valid font found, it defaults to [MapLibreConstants.DEFAULT_FONT].
         *
         * The font families are checked against the default system fonts defined in
         * &#x27;/system/etc/fonts.xml&#x27; Default font for local ideograph font family is
         * [MapLibreConstants.DEFAULT_FONT].
         *
         * @param fontFamilies an array of font families for local ideograph generation.
         * @return This
         */
        fun localIdeographFontFamily(vararg fontFamilies: String?): MapLibreMapOptions {
            localIdeographFontFamily = FontUtils.extractValidFont(*fontFamilies)
            return this
        }

        /**
         * Set the custom pixel ratio configuration to override the default value from resources.
         * This ratio will be used to initialise the map with.
         *
         * @param pixelRatio the custom pixel ratio of the map under construction
         * @return This
         */
        fun pixelRatio(pixelRatio: Float): MapLibreMapOptions {
            this.pixelRatio = pixelRatio
            return this
        }

        /**
         * Set the flag to render the map surface on top of another surface.
         *
         * @param renderOnTop true if this map is shown on top of another one, false if bottom.
         */
        fun renderSurfaceOnTop(renderOnTop: Boolean) {
            zMediaOverlay = renderOnTop
        }

        /**
         * Get the flag to render the map surface on top of another surface.
         *
         * @return true if this map is
         */
        val renderSurfaceOnTop: Boolean get() = zMediaOverlay

        /**
         * Get the current configured API endpoint base URL.
         *
         * @return Base URL to be used API endpoint.
         * @deprecated use [getApiBaseUri] instead
         */
        @Deprecated("use getApiBaseUri() instead", ReplaceWith("getApiBaseUri()"))
        val apiBaseUrl: String? get() = apiBaseUri

        /**
         * Get the current configured initial camera position for a map view.
         *
         * @return CameraPosition to be initially used.
         */
        val camera: CameraPosition? get() = cameraPosition

        /**
         * Get the current configured min zoom for a map view.
         *
         * @return Mininum zoom level to be used.
         */
        val minZoomPreference: Double get() = minZoom

        /**
         * Get the current configured maximum zoom for a map view.
         *
         * @return Maximum zoom to be used.
         */
        val maxZoomPreference: Double get() = maxZoom

        /**
         * Get the current configured min pitch for a map view.
         *
         * @return Mininum pitch to be used.
         */
        val minPitchPreference: Double get() = minPitch

        /**
         * Get the current configured maximum pitch for a map view.
         *
         * @return Maximum pitch to be used.
         */
        val maxPitchPreference: Double get() = maxPitch

        /**
         * Get the current configured state for fading the maplibre_compass_icon when facing north.
         *
         * @return True if maplibre_compass_icon fades to invisible when facing north
         */
        val compassFadeFacingNorth: Boolean get() = fadeCompassFacingNorth

        /**
         * Returns true if local ideograph font family is enabled, defaults to true.
         *
         * @return True if local ideograph font family is enabled
         */
        fun isLocalIdeographFontFamilyEnabled(): Boolean = localIdeographFontFamilyEnabled

        override fun describeContents(): Int = 0

        @Suppress("LongMethod")
        override fun writeToParcel(
            dest: Parcel,
            flags: Int,
        ) {
            dest.writeParcelable(cameraPosition, flags)
            dest.writeBooleanByte(debugActive)

            dest.writeBooleanByte(compassEnabled)
            dest.writeInt(compassGravity)
            dest.writeIntArray(compassMargins)
            dest.writeBooleanByte(fadeCompassFacingNorth)
            dest.writeParcelable(compassImage?.let { BitmapUtils.getBitmapFromDrawable(it) }, flags)

            dest.writeBooleanByte(logoEnabled)
            dest.writeInt(logoGravity)
            dest.writeIntArray(logoMargins)

            dest.writeBooleanByte(attributionEnabled)
            dest.writeInt(attributionGravity)
            dest.writeIntArray(attributionMargins)
            dest.writeInt(attributionTintColor)

            dest.writeDouble(minZoom)
            dest.writeDouble(maxZoom)
            dest.writeDouble(minPitch)
            dest.writeDouble(maxPitch)

            dest.writeBooleanByte(rotateGesturesEnabled)
            dest.writeBooleanByte(scrollGesturesEnabled)
            dest.writeBooleanByte(horizontalScrollGesturesEnabled)
            dest.writeBooleanByte(tiltGesturesEnabled)
            dest.writeBooleanByte(zoomGesturesEnabled)
            dest.writeBooleanByte(doubleTapGesturesEnabled)
            dest.writeBooleanByte(quickZoomGesturesEnabled)

            dest.writeString(apiBaseUri)
            dest.writeBooleanByte(textureMode)
            dest.writeBooleanByte(translucentTextureSurface)
            dest.writeBooleanByte(prefetchesTiles)
            dest.writeInt(prefetchZoomDelta)
            dest.writeBooleanByte(zMediaOverlay)
            dest.writeBooleanByte(localIdeographFontFamilyEnabled)
            dest.writeString(localIdeographFontFamily)
            dest.writeStringArray(localIdeographFontFamilies)
            dest.writeFloat(pixelRatio)
            dest.writeInt(foregroundLoadColor)
            dest.writeBooleanByte(crossSourceCollisions)

            dest.writeBooleanByte(actionJournalEnabled)
            dest.writeString(actionJournalPath)
            dest.writeLong(actionJournalLogFileSize)
            dest.writeLong(actionJournalLogFileCount)
            dest.writeInt(actionJournalRenderingReportInterval)

            dest.writeBooleanByte(asyncRendererCleanup)

            dest.writeBooleanByte(fastPFOREnabled)
        }

        @Suppress("CyclomaticComplexMethod", "LongMethod", "ReturnCount", "EqualsAlwaysReturnsTrueOrFalse")
        override fun equals(other: Any?): Boolean {
            if (this === other) {
                return true
            }
            if (other == null || javaClass != other.javaClass) {
                return false
            }

            val options = other as MapLibreMapOptions

            if (debugActive != options.debugActive) {
                return false
            }
            if (compassEnabled != options.compassEnabled) {
                return false
            }
            if (fadeCompassFacingNorth != options.fadeCompassFacingNorth) {
                return false
            }
            if (compassImage != options.compassImage) {
                return false
            }
            if (compassGravity != options.compassGravity) {
                return false
            }
            if (logoEnabled != options.logoEnabled) {
                return false
            }
            if (logoGravity != options.logoGravity) {
                return false
            }
            if (attributionTintColor != options.attributionTintColor) {
                return false
            }
            if (attributionEnabled != options.attributionEnabled) {
                return false
            }
            if (attributionGravity != options.attributionGravity) {
                return false
            }
            if (options.minZoom.compareTo(minZoom) != 0) {
                return false
            }
            if (options.maxZoom.compareTo(maxZoom) != 0) {
                return false
            }
            if (options.minPitch.compareTo(minPitch) != 0) {
                return false
            }
            if (options.maxPitch.compareTo(maxPitch) != 0) {
                return false
            }
            if (rotateGesturesEnabled != options.rotateGesturesEnabled) {
                return false
            }
            if (scrollGesturesEnabled != options.scrollGesturesEnabled) {
                return false
            }
            if (horizontalScrollGesturesEnabled != options.horizontalScrollGesturesEnabled) {
                return false
            }
            if (tiltGesturesEnabled != options.tiltGesturesEnabled) {
                return false
            }
            if (zoomGesturesEnabled != options.zoomGesturesEnabled) {
                return false
            }
            if (doubleTapGesturesEnabled != options.doubleTapGesturesEnabled) {
                return false
            }
            if (quickZoomGesturesEnabled != options.quickZoomGesturesEnabled) {
                return false
            }
            if (cameraPosition != options.cameraPosition) {
                return false
            }
            if (!compassMargins.contentEquals(options.compassMargins)) {
                return false
            }
            if (!logoMargins.contentEquals(options.logoMargins)) {
                return false
            }
            if (!attributionMargins.contentEquals(options.attributionMargins)) {
                return false
            }
            if (apiBaseUri != options.apiBaseUri) {
                return false
            }
            if (prefetchesTiles != options.prefetchesTiles) {
                return false
            }
            if (prefetchZoomDelta != options.prefetchZoomDelta) {
                return false
            }
            if (zMediaOverlay != options.zMediaOverlay) {
                return false
            }
            if (localIdeographFontFamilyEnabled != options.localIdeographFontFamilyEnabled) {
                return false
            }
            if (localIdeographFontFamily != options.localIdeographFontFamily) {
                return false
            }
            if (!localIdeographFontFamilies.contentEquals(options.localIdeographFontFamilies)) {
                return false
            }

            if (pixelRatio != options.pixelRatio) {
                return false
            }

            if (crossSourceCollisions != options.crossSourceCollisions) {
                return false
            }

            if (actionJournalEnabled != options.actionJournalEnabled) {
                return false
            }

            if (actionJournalPath != options.actionJournalPath) {
                return false
            }

            if (actionJournalLogFileSize != options.actionJournalLogFileSize) {
                return false
            }

            if (actionJournalLogFileCount != options.actionJournalLogFileCount) {
                return false
            }

            if (actionJournalRenderingReportInterval != options.actionJournalRenderingReportInterval) {
                return false
            }

            if (asyncRendererCleanup != options.asyncRendererCleanup) {
                return false
            }

            if (fastPFOREnabled != options.fastPFOREnabled) {
                return false
            }

            return true
        }

        @Suppress("LongMethod")
        override fun hashCode(): Int {
            var result = cameraPosition?.hashCode() ?: 0
            result = 31 * result + if (debugActive) 1 else 0
            result = 31 * result + if (compassEnabled) 1 else 0
            result = 31 * result + if (fadeCompassFacingNorth) 1 else 0
            result = 31 * result + compassGravity
            result = 31 * result + (compassImage?.hashCode() ?: 0)
            result = 31 * result + compassMargins.contentHashCode()
            result = 31 * result + if (logoEnabled) 1 else 0
            result = 31 * result + logoGravity
            result = 31 * result + logoMargins.contentHashCode()
            result = 31 * result + attributionTintColor
            result = 31 * result + if (attributionEnabled) 1 else 0
            result = 31 * result + attributionGravity
            result = 31 * result + attributionMargins.contentHashCode()
            var temp = minZoom.toBits()
            result = 31 * result + (temp xor (temp ushr 32)).toInt()
            temp = maxZoom.toBits()
            result = 31 * result + (temp xor (temp ushr 32)).toInt()
            temp = minPitch.toBits()
            result = 31 * result + (temp xor (temp ushr 32)).toInt()
            temp = maxPitch.toBits()
            result = 31 * result + (temp xor (temp ushr 32)).toInt()
            result = 31 * result + if (rotateGesturesEnabled) 1 else 0
            result = 31 * result + if (scrollGesturesEnabled) 1 else 0
            result = 31 * result + if (horizontalScrollGesturesEnabled) 1 else 0
            result = 31 * result + if (tiltGesturesEnabled) 1 else 0
            result = 31 * result + if (zoomGesturesEnabled) 1 else 0
            result = 31 * result + if (doubleTapGesturesEnabled) 1 else 0
            result = 31 * result + if (quickZoomGesturesEnabled) 1 else 0
            result = 31 * result + (apiBaseUri?.hashCode() ?: 0)
            result = 31 * result + if (textureMode) 1 else 0
            result = 31 * result + if (translucentTextureSurface) 1 else 0
            result = 31 * result + if (prefetchesTiles) 1 else 0
            result = 31 * result + prefetchZoomDelta
            result = 31 * result + if (zMediaOverlay) 1 else 0
            result = 31 * result + if (localIdeographFontFamilyEnabled) 1 else 0
            result = 31 * result + (localIdeographFontFamily?.hashCode() ?: 0)
            result = 31 * result + localIdeographFontFamilies.contentHashCode()
            result = 31 * result + pixelRatio.toInt()
            result = 31 * result + if (crossSourceCollisions) 1 else 0
            result = 31 * result + if (actionJournalEnabled) 1 else 0
            result = 31 * result + actionJournalPath.hashCode()
            result = 31 * result + actionJournalLogFileSize.toInt()
            result = 31 * result + actionJournalLogFileCount.toInt()
            result = 31 * result + actionJournalRenderingReportInterval
            result = 31 * result + if (asyncRendererCleanup) 1 else 0
            result = 31 * result + if (fastPFOREnabled) 1 else 0
            return result
        }

        companion object {
            private val LIGHT_GRAY = 0xFFF0E9E1.toInt() // RGB(240, 233, 225))
            private const val FOUR_DP = 4f
            private const val NINETY_TWO_DP = 92f
            private const val UNDEFINED_COLOR = -1

            /**
             * Creates a default MapLibreMapsOptions from a given context.
             *
             * @param context Context related to a map view.
             * @return the MapLibreMapOptions created from attributes
             */
            @JvmStatic
            fun createFromAttributes(context: Context): MapLibreMapOptions = createFromAttributes(context, null)

            /**
             * Creates a MapLibreMapsOptions from the attribute set.
             *
             * @param context Context related to a map view.
             * @param attrs   Attributeset containing configuration
             * @return the MapLibreMapOptions created from attributes
             */
            @JvmStatic
            @Suppress("DEPRECATION")
            fun createFromAttributes(
                context: Context,
                attrs: AttributeSet?,
            ): MapLibreMapOptions {
                val typedArray = context.obtainStyledAttributes(attrs, R.styleable.maplibre_MapView, 0, 0)
                return createFromAttributes(MapLibreMapOptions(), context, typedArray)
            }

            @JvmStatic
            @VisibleForTesting
            @Suppress("DEPRECATION", "LongMethod")
            fun createFromAttributes(
                maplibreMapOptions: MapLibreMapOptions,
                context: Context,
                typedArray: TypedArray,
            ): MapLibreMapOptions {
                val pxlRatio = context.resources.displayMetrics.density
                maplibreMapOptions.actionJournalPath(context.filesDir.absolutePath)

                try {
                    maplibreMapOptions.camera(CameraPosition.Builder(typedArray).build())

                    // deprecated
                    maplibreMapOptions.apiBaseUrl(typedArray.getString(R.styleable.maplibre_MapView_maplibre_apiBaseUrl))

                    val baseUri = typedArray.getString(R.styleable.maplibre_MapView_maplibre_apiBaseUri)
                    if (!TextUtils.isEmpty(baseUri)) {
                        // override deprecated property if a value of the new type was provided
                        maplibreMapOptions.apiBaseUri(baseUri)
                    }

                    maplibreMapOptions.zoomGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiZoomGestures, true),
                    )
                    maplibreMapOptions.scrollGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiScrollGestures, true),
                    )
                    maplibreMapOptions.horizontalScrollGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiHorizontalScrollGestures, true),
                    )
                    maplibreMapOptions.rotateGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiRotateGestures, true),
                    )
                    maplibreMapOptions.tiltGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiTiltGestures, true),
                    )
                    maplibreMapOptions.doubleTapGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiDoubleTapGestures, true),
                    )
                    maplibreMapOptions.quickZoomGesturesEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiQuickZoomGestures, true),
                    )

                    maplibreMapOptions.maxZoomPreference(
                        typedArray
                            .getFloat(
                                R.styleable.maplibre_MapView_maplibre_cameraZoomMax,
                                MapLibreConstants.MAXIMUM_ZOOM.toFloat(),
                            ).toDouble(),
                    )
                    maplibreMapOptions.minZoomPreference(
                        typedArray
                            .getFloat(
                                R.styleable.maplibre_MapView_maplibre_cameraZoomMin,
                                MapLibreConstants.MINIMUM_ZOOM.toFloat(),
                            ).toDouble(),
                    )
                    maplibreMapOptions.maxPitchPreference(
                        typedArray
                            .getFloat(
                                R.styleable.maplibre_MapView_maplibre_cameraPitchMax,
                                MapLibreConstants.MAXIMUM_PITCH.toFloat(),
                            ).toDouble(),
                    )
                    maplibreMapOptions.minPitchPreference(
                        typedArray
                            .getFloat(
                                R.styleable.maplibre_MapView_maplibre_cameraPitchMin,
                                MapLibreConstants.MINIMUM_PITCH.toFloat(),
                            ).toDouble(),
                    )

                    maplibreMapOptions.compassEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiCompass, true),
                    )
                    maplibreMapOptions.compassGravity(
                        typedArray.getInt(
                            R.styleable.maplibre_MapView_maplibre_uiCompassGravity,
                            Gravity.TOP or Gravity.END,
                        ),
                    )
                    maplibreMapOptions.compassMargins(
                        intArrayOf(
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiCompassMarginLeft,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiCompassMarginTop,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiCompassMarginRight,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiCompassMarginBottom,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                        ),
                    )
                    maplibreMapOptions.compassFadesWhenFacingNorth(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiCompassFadeFacingNorth, true),
                    )
                    var compassDrawable = typedArray.getDrawable(R.styleable.maplibre_MapView_maplibre_uiCompassDrawable)
                    if (compassDrawable == null) {
                        compassDrawable =
                            ResourcesCompat.getDrawable(context.resources, R.drawable.maplibre_compass_icon, null)
                    }
                    maplibreMapOptions.compassImage(compassDrawable)

                    maplibreMapOptions.logoEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiLogo, true),
                    )
                    maplibreMapOptions.logoGravity(
                        typedArray.getInt(
                            R.styleable.maplibre_MapView_maplibre_uiLogoGravity,
                            Gravity.BOTTOM or Gravity.START,
                        ),
                    )
                    maplibreMapOptions.logoMargins(
                        intArrayOf(
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiLogoMarginLeft,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiLogoMarginTop,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiLogoMarginRight,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiLogoMarginBottom,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                        ),
                    )

                    maplibreMapOptions.attributionTintColor(
                        typedArray.getColor(R.styleable.maplibre_MapView_maplibre_uiAttributionTintColor, UNDEFINED_COLOR),
                    )
                    maplibreMapOptions.attributionEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_uiAttribution, true),
                    )
                    maplibreMapOptions.attributionGravity(
                        typedArray.getInt(
                            R.styleable.maplibre_MapView_maplibre_uiAttributionGravity,
                            Gravity.BOTTOM or Gravity.START,
                        ),
                    )
                    maplibreMapOptions.attributionMargins(
                        intArrayOf(
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiAttributionMarginLeft,
                                    NINETY_TWO_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiAttributionMarginTop,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiAttributionMarginRight,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                            typedArray
                                .getDimension(
                                    R.styleable.maplibre_MapView_maplibre_uiAttributionMarginBottom,
                                    FOUR_DP * pxlRatio,
                                ).toInt(),
                        ),
                    )
                    maplibreMapOptions.textureMode(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_renderTextureMode, false),
                    )
                    maplibreMapOptions.translucentTextureSurface(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_renderTextureTranslucentSurface, false),
                    )
                    maplibreMapOptions.setPrefetchesTiles(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_enableTilePrefetch, true),
                    )
                    maplibreMapOptions.setPrefetchZoomDelta(
                        typedArray.getInt(R.styleable.maplibre_MapView_maplibre_prefetchZoomDelta, 4),
                    )
                    maplibreMapOptions.renderSurfaceOnTop(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_enableZMediaOverlay, false),
                    )

                    maplibreMapOptions.localIdeographFontFamilyEnabled =
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_localIdeographEnabled, true)

                    val localIdeographFontFamiliesResId =
                        typedArray.getResourceId(R.styleable.maplibre_MapView_maplibre_localIdeographFontFamilies, 0)
                    if (localIdeographFontFamiliesResId != 0) {
                        val localIdeographFontFamilies =
                            context.resources.getStringArray(localIdeographFontFamiliesResId)
                        maplibreMapOptions.localIdeographFontFamily(*localIdeographFontFamilies)
                    } else {
                        // did user provide xml font string?
                        val localIdeographFontFamily =
                            typedArray.getString(R.styleable.maplibre_MapView_maplibre_localIdeographFontFamily)
                                ?: MapLibreConstants.DEFAULT_FONT
                        maplibreMapOptions.localIdeographFontFamily(localIdeographFontFamily)
                    }

                    maplibreMapOptions.pixelRatio(
                        typedArray.getFloat(R.styleable.maplibre_MapView_maplibre_pixelRatio, 0f),
                    )
                    maplibreMapOptions.foregroundLoadColor(
                        typedArray.getInt(R.styleable.maplibre_MapView_maplibre_foregroundLoadColor, LIGHT_GRAY),
                    )
                    maplibreMapOptions.crossSourceCollisions(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_cross_source_collisions, true),
                    )

                    maplibreMapOptions.actionJournalEnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_actionJournalEnabled, false),
                    )
                    maplibreMapOptions.actionJournalLogFileSize(
                        typedArray
                            .getInteger(
                                R.styleable.maplibre_MapView_maplibre_actionJournalLogFileSize,
                                1024 * 1024,
                            ).toLong(),
                    )
                    maplibreMapOptions.actionJournalLogFileCount(
                        typedArray.getInteger(R.styleable.maplibre_MapView_maplibre_actionJournalLogFileCount, 5).toLong(),
                    )
                    maplibreMapOptions.actionJournalRenderingReportInterval(
                        typedArray.getInteger(
                            R.styleable.maplibre_MapView_maplibre_actionJournalRenderingReportInterval,
                            60,
                        ),
                    )

                    maplibreMapOptions.asyncRendererCleanup(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_asyncRendererCleanup, false),
                    )

                    maplibreMapOptions.fastPFOREnabled(
                        typedArray.getBoolean(R.styleable.maplibre_MapView_maplibre_fastPFOREnabled, false),
                    )
                } finally {
                    typedArray.recycle()
                }
                return maplibreMapOptions
            }

            @JvmField
            val CREATOR: Parcelable.Creator<MapLibreMapOptions> =
                object : Parcelable.Creator<MapLibreMapOptions> {
                    override fun createFromParcel(parcel: Parcel): MapLibreMapOptions = MapLibreMapOptions(parcel)

                    override fun newArray(size: Int): Array<MapLibreMapOptions?> = arrayOfNulls(size)
                }
        }
    }

private fun Parcel.writeBooleanByte(value: Boolean) = writeByte(if (value) 1 else 0)

private fun Parcel.readBooleanByte(): Boolean = readByte() != 0.toByte()
