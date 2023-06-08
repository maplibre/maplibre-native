package org.maplibre.android.snapshotter

import android.content.Context
import android.graphics.*
import android.os.Handler
import android.text.Html
import android.text.TextUtils
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.annotation.Keep
import androidx.annotation.UiThread
import androidx.core.content.res.ResourcesCompat
import org.maplibre.android.R
import org.maplibre.android.attribution.AttributionLayout
import org.maplibre.android.attribution.AttributionMeasure
import org.maplibre.android.attribution.AttributionParser
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.Image
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.Style.Builder.*
import org.maplibre.android.storage.FileSource
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.sources.Source
import org.maplibre.android.utils.FontUtils
import org.maplibre.android.utils.ThreadUtils

/**
 * The map snapshotter creates a large of the map, rendered
 * off the UI thread. The snapshotter itself must be used on
 * the UI thread (for access to the main looper)
 */
@UiThread
open class MapSnapshotter(context: Context, options: Options) {
    // Holds the pointer to JNI NativeMapView
    @Keep
    private val nativePtr: Long = 0
    private val context: Context
    private var fullyLoaded = false
    private val options: Options
    private var callback: SnapshotReadyCallback? = null
    private var errorHandler: ErrorHandler? = null
    private var observer: Observer? = null

    /**
     * Get notified on snapshot completion.
     *
     * @see MapSnapshotter.start
     */
    fun interface SnapshotReadyCallback {
        /**
         * Called when the snapshot is complete.
         *
         * @param snapshot the snapshot
         */
        fun onSnapshotReady(snapshot: MapSnapshot)
    }

    /**
     * Can be used to get notified of errors
     * in snapshot generation
     *
     * @see MapSnapshotter.start
     */
    fun interface ErrorHandler {
        /**
         * Called on error. Snapshotting will not
         * continue
         *
         * @param error the error message
         */
        fun onError(error: String)
    }

    /**
     * Can be used to get notified on snapshotter style loading
     * completion.
     *
     * @see MapSnapshotter.setObserver
     */
    interface Observer {
        /**
         * Called when snapshotter finishes loading its style.
         */
        fun onDidFinishLoadingStyle()

        /**
         * Called when the map is missing an icon.The icon should be added synchronously with
         * [MapSnapshotter.addImage] to be rendered on the current zoom level.
         * When loading icons asynchronously, you can load a placeholder image and replace it when you icon has loaded.
         *
         * @param imageName the id of the icon that is missing
         */
        fun onStyleImageMissing(imageName: String)
    }

    /**
     * MapSnapshotter options
     */
    class Options(width: Int, height: Int) {
        /**
         * @return the pixel ratio
         */
        var pixelRatio = 1f
            private set

        /**
         * @return the width of the image
         */
        val width: Int

        /**
         * @return the height of the image
         */
        val height: Int

        /**
         * @return the region
         */
        var region: LatLngBounds? = null
            private set

        /**
         * @return the camera position
         */
        var cameraPosition: CameraPosition? = null
            private set

        var showLogo = true

        /**
         * @return the font family used for locally generating ideographs,
         * Default font for local ideograph font family is [MapLibreConstants.DEFAULT_FONT].
         */
        var localIdeographFontFamily = MapLibreConstants.DEFAULT_FONT
            private set

        /**
         * @return The base of our API endpoint
         */
        @get:Deprecated("use {@link #getApiBaseUri()} instead")
        var apiBaseUri: String? = null
            private set
            get() = field

        var builder: Style.Builder? = null
            private set

        /**
         * @param width  the width of the image
         * @param height the height of the image
         */
        init {
            require(!(width == 0 || height == 0)) { "Unable to create a snapshot with width or height set to 0" }
            this.width = width
            this.height = height
        }

        /**
         * Set a style builder to snapshotter, the contents in builder like layers/sources/images will be applied
         * to snapshotter.
         *
         * @param builder The builder will applied to snapshotter
         * @return the mutated [Options]
         */
        fun withStyleBuilder(builder: Style.Builder?): Options {
            this.builder = builder
            return this
        }

        /**
         * @param uri The style URI to use
         * @return the mutated [Options]
         */
        @Deprecated("use {@link  #withStyleBuilder(Style.Builder)} instead")
        fun withStyle(uri: String?): Options {
            withStyleBuilder(Style.Builder().fromUri(uri!!))
            return this
        }

        /**
         * @param styleJson The style json to use
         * @return the mutated [Options]
         */
        @Deprecated("use {@link  #withStyleBuilder(Style.Builder)} instead")
        fun withStyleJson(styleJson: String?): Options {
            withStyleBuilder(Style.Builder().fromJson(styleJson!!))
            return this
        }

        /**
         * @param region the region to show in the snapshot.
         * This is applied after the camera position
         * @return the mutated [Options]
         */
        fun withRegion(region: LatLngBounds?): Options {
            this.region = region
            return this
        }

        /**
         * @param pixelRatio the pixel ratio to use (default: 1)
         * @return the mutated [Options]
         */
        fun withPixelRatio(pixelRatio: Float): Options {
            this.pixelRatio = pixelRatio
            return this
        }

        /**
         * @param cameraPosition The camera position to use,
         * the [CameraPosition.target] is overridden
         * by region if set in conjunction.
         * @return the mutated [Options]
         */
        fun withCameraPosition(cameraPosition: CameraPosition?): Options {
            this.cameraPosition = cameraPosition
            return this
        }

        /**
         * @param showLogo The flag indicating to show the MapLibre logo.
         * @return the mutated [Options]
         */
        fun withLogo(showLogo: Boolean): Options {
            this.showLogo = showLogo
            return this
        }

        /**
         * Set the font family for generating glyphs locally for ideographs in the &#x27;CJK Unified Ideographs&#x27;
         * and &#x27;Hangul Syllables&#x27; ranges.
         *
         *
         * The font family argument is passed to [android.graphics.Typeface.create].
         * Default system fonts are defined in &#x27;/system/etc/fonts.xml&#x27;
         * Default font for local ideograph font family is [MapLibreConstants.DEFAULT_FONT].
         *
         *
         * @param fontFamily font family for local ideograph generation.
         * @return the mutated [Options]
         */
        fun withLocalIdeographFontFamily(fontFamily: String?): Options {
            localIdeographFontFamily = FontUtils.extractValidFont(fontFamily)
            return this
        }

        /**
         * Set a font family from range of font families for generating glyphs locally for ideographs in the
         * &#x27;CJK Unified Ideographs&#x27; and &#x27;Hangul Syllables&#x27; ranges.
         *
         *
         * The font families are checked against the default system fonts defined in
         * &#x27;/system/etc/fonts.xml&#x27;. Default font for local ideograph font family is
         * [MapLibreConstants.DEFAULT_FONT].
         *
         *
         * @param fontFamilies font families for local ideograph generation.
         * @return the mutated [Options]
         */
        fun withLocalIdeographFontFamily(vararg fontFamilies: String?): Options {
            localIdeographFontFamily = FontUtils.extractValidFont(*fontFamilies)
            return this
        }

        /**
         * Specifies the URL used for API endpoint.
         *
         * @param apiBaseUrl The base of our API endpoint
         * @return the mutated [Options]
         */
        @Deprecated("use {@link  #withApiBaseUri(String)} instead")
        fun withApiBaseUrl(apiBaseUrl: String?): Options {
            apiBaseUri = apiBaseUrl
            return this
        }

        /**
         * Specifies the URI used for API endpoint.
         *
         * @param apiBaseUri The base of our API endpoint
         * @return the mutated [Options]
         */
        fun withApiBaseUri(apiBaseUri: String?): Options {
            this.apiBaseUri = apiBaseUri
            return this
        }

        /**
         * @return the style url
         */
        @get:Deprecated("use {@link #getStyleUri()} instead")
        val styleUrl: String?
            get() = if (builder == null) null else builder!!.uri

        /**
         * @return the style uri
         */
        val styleUri: String?
            get() = if (builder == null) null else builder!!.uri

        /**
         * @return the style json
         */
        val styleJson: String?
            get() = if (builder == null) null else builder!!.json
    }

    /**
     * Creates the Map snapshotter, but doesn't start rendering or
     * loading yet.
     *
     * @param context the Context that is or contains the Application context
     * @param options the options to use for the snapshot
     */
    init {
        checkThread()
        this.context = context.applicationContext
        this.options = options
        val fileSource = FileSource.getInstance(context)
        val apiBaseUrl = options.apiBaseUri
        if (!TextUtils.isEmpty(apiBaseUrl)) {
            fileSource.setApiBaseUrl(apiBaseUrl)
        }
        nativeInitialize(
            this,
            fileSource,
            options.pixelRatio,
            options.width,
            options.height,
            options.styleUri,
            options.styleJson,
            options.region,
            options.cameraPosition,
            options.showLogo,
            options.localIdeographFontFamily
        )
    }

    /**
     * Starts loading and rendering the snapshot. The callbacks will be fired
     * on the calling thread.
     *
     * @param callback     the callback to use when the snapshot is ready
     * @param errorHandler the error handler to use on snapshot errors
     */
    @JvmOverloads
    fun start(callback: SnapshotReadyCallback, errorHandler: ErrorHandler? = null) {
        check(this.callback == null) { "Snapshotter was already started" }
        checkThread()
        this.callback = callback
        this.errorHandler = errorHandler
        nativeStart()
    }

    /**
     * Updates the snapshotter with a new size
     *
     * @param width  the width
     * @param height the height
     */
    @Keep
    external fun setSize(width: Int, height: Int)

    /**
     * Updates the snapshotter with a new [CameraPosition]
     *
     * @param cameraPosition the camera position
     */
    @Keep
    external fun setCameraPosition(cameraPosition: CameraPosition?)

    /**
     * Updates the snapshotter with a new [LatLngBounds]
     *
     * @param region the region
     */
    @Keep
    external fun setRegion(region: LatLngBounds?)

    /**
     * Updates the snapshotter with a new style url
     *
     * @param styleUrl the style url
     */
    @Keep
    external fun setStyleUrl(styleUrl: String?)

    /**
     * Updates the snapshotter with a new style json
     *
     * @param styleJson the style json
     */
    @Keep
    external fun setStyleJson(styleJson: String?)

    /**
     * Adds the layer to the map. The layer must be newly created and not added to the snapshotter before
     *
     * @param layer the layer to add
     * @param below the layer id to add this layer before
     */
    private fun addLayerBelow(layer: Layer, below: String) {
        nativeAddLayerBelow(layer.nativePtr, below)
    }

    /**
     * Adds the layer to the map. The layer must be newly created and not added to the snapshotter before
     *
     * @param layer the layer to add
     * @param above the layer id to add this layer above
     */
    private fun addLayerAbove(layer: Layer, above: String) {
        nativeAddLayerAbove(layer.nativePtr, above)
    }

    /**
     * Adds the layer to the snapshotter at the specified index. The layer must be newly
     * created and not added to the snapshotter before
     *
     * @param layer the layer to add
     * @param index the index to insert the layer at
     */
    private fun addLayerAt(layer: Layer, index: Int) {
        nativeAddLayerAt(layer.nativePtr, index)
    }

    /**
     * Adds the source to the map. The source must be newly created and not added to the map before
     *
     * @param source the source to add
     */
    private fun addSource(source: Source) {
        nativeAddSource(source, source.nativePtr)
    }

    /**
     * Adds an image to be used in the snapshotter's style
     *
     * @param name   the name of the image
     * @param bitmap the pre-multiplied Bitmap
     * @param sdf    the flag indicating image is an SDF or template image
     */
    fun addImage(name: String, bitmap: Bitmap, sdf: Boolean) {
        nativeAddImages(arrayOf(Style.toImage(ImageWrapper(name, bitmap, sdf))))
    }

    /**
     * Must be called in on the thread
     * the object was created on.
     */
    fun cancel() {
        checkThread()
        reset()
        nativeCancel()
    }

    /**
     * Sets observer for a snapshotter
     *
     * @param observer an Observer object
     */
    fun setObserver(observer: Observer?) {
        checkThread()
        this.observer = observer
    }

    /**
     * Draw an overlay on the map snapshot.
     *
     * @param mapSnapshot the map snapshot to draw the overlay on
     */
    protected open fun addOverlay(mapSnapshot: MapSnapshot) {
        val snapshot = mapSnapshot.bitmap
        val canvas = Canvas(snapshot!!)
        val margin = context.resources.displayMetrics.density.toInt() * LOGO_MARGIN_DP
        drawOverlay(mapSnapshot, snapshot, canvas, margin)
    }

    private fun drawOverlay(mapSnapshot: MapSnapshot, snapshot: Bitmap, canvas: Canvas, margin: Int) {
        val measure = getAttributionMeasure(mapSnapshot, snapshot, margin)
        val layout = measure.measure()
        drawLogo(mapSnapshot, canvas, margin, layout!!)
        drawAttribution(mapSnapshot, canvas, measure, layout)
    }

    private fun getAttributionMeasure(mapSnapshot: MapSnapshot, snapshot: Bitmap, margin: Int): AttributionMeasure {
        val logo = createScaledLogo(snapshot)
        val longText = createTextView(mapSnapshot, false, logo.scale)
        val shortText = createTextView(mapSnapshot, true, logo.scale)
        return AttributionMeasure.Builder().setSnapshot(snapshot).setLogo(logo.large).setLogoSmall(logo.small).setTextView(longText).setTextViewShort(shortText)
            .setMarginPadding(margin.toFloat()).build()
    }

    private fun drawLogo(mapSnapshot: MapSnapshot, canvas: Canvas, margin: Int, layout: AttributionLayout) {
        if (mapSnapshot.isShowLogo) {
            drawLogo(mapSnapshot.bitmap, canvas, margin, layout)
        }
    }

    private fun drawLogo(snapshot: Bitmap, canvas: Canvas, margin: Int, placement: AttributionLayout) {
        val selectedLogo = placement.logo
        if (selectedLogo != null) {
            canvas.drawBitmap(selectedLogo, margin.toFloat(), (snapshot.height - selectedLogo.height - margin).toFloat(), null)
        }
    }

    private fun drawAttribution(mapSnapshot: MapSnapshot, canvas: Canvas, measure: AttributionMeasure, layout: AttributionLayout?) {
        // draw attribution
        val anchorPoint = layout!!.anchorPoint
        if (anchorPoint != null) {
            drawAttribution(canvas, measure, anchorPoint)
        } else {
            val snapshot = mapSnapshot.bitmap
            Logger.e(
                TAG,
                String.format(
                    "Could not generate attribution for snapshot size: %s x %s." + " You are required to provide your own attribution for the used sources: %s",
                    snapshot.width,
                    snapshot.height,
                    mapSnapshot.attributions
                )
            )
        }
    }

    private fun drawAttribution(canvas: Canvas, measure: AttributionMeasure, anchorPoint: PointF) {
        canvas.save()
        canvas.translate(anchorPoint.x, anchorPoint.y)
        measure.textView.draw(canvas)
        canvas.restore()
    }

    private fun createTextView(mapSnapshot: MapSnapshot, shortText: Boolean, scale: Float): TextView {
        val textColor = ResourcesCompat.getColor(context.resources, R.color.maplibre_gray_dark, context.theme)
        val widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
        val heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
        val textView = TextView(context)
        textView.layoutParams = ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        textView.isSingleLine = true
        textView.textSize = 10 * scale
        textView.setTextColor(textColor)
        textView.setBackgroundResource(R.drawable.maplibre_rounded_corner)
        textView.text = Html.fromHtml(createAttributionString(mapSnapshot, shortText))
        textView.measure(widthMeasureSpec, heightMeasureSpec)
        textView.layout(0, 0, textView.measuredWidth, textView.measuredHeight)
        return textView
    }

    /**
     * Create the attribution string.
     *
     * @param mapSnapshot the map snapshot to create the attribution for
     * @param shortText   indicates if the short variant of the string should be parsed
     * @return the parsed attribution string
     */
    private fun createAttributionString(mapSnapshot: MapSnapshot, shortText: Boolean): String {
        val attributionParser = AttributionParser.Options(context).withAttributionData(*mapSnapshot.attributions).withCopyrightSign(false).withImproveMap(false).build()
        return attributionParser.createAttributionString(shortText)
    }

    /**
     * Create a scaled logo for a map snapshot.
     *
     * @param snapshot the map snapshot where the logo should be placed on
     * @return the scaled large logo
     */
    private fun createScaledLogo(snapshot: Bitmap): Logo {
        val logo = BitmapFactory.decodeResource(context.resources, R.drawable.maplibre_logo_icon, null)
        val scale = calculateLogoScale(snapshot, logo)
        val matrix = Matrix()
        matrix.postScale(scale, scale)
        val helmet = BitmapFactory.decodeResource(context.resources, R.drawable.maplibre_logo_helmet, null)
        val large = Bitmap.createBitmap(logo, 0, 0, logo.width, logo.height, matrix, true)
        val small = Bitmap.createBitmap(helmet, 0, 0, helmet.width, helmet.height, matrix, true)
        return Logo(large, small, scale)
    }

    /**
     * Calculates the scale of the logo, only allow downscaling.
     *
     * @param snapshot the large of the map snapshot
     * @param logo     the large of the mapbox logo
     * @return the scale value
     */
    private fun calculateLogoScale(snapshot: Bitmap, logo: Bitmap): Float {
        val displayMetrics = context.resources.displayMetrics
        val widthRatio = (displayMetrics.widthPixels / snapshot.width).toFloat()
        val heightRatio = (displayMetrics.heightPixels / snapshot.height).toFloat()
        val prefWidth = logo.width / widthRatio
        val prefHeight = logo.height / heightRatio
        var calculatedScale = Math.min(prefWidth / logo.width, prefHeight / logo.height) * 2
        if (calculatedScale > 1) {
            // don't allow over-scaling
            calculatedScale = 1.0f
        } else if (calculatedScale < 0.60f) {
            // don't scale to low either
            calculatedScale = 0.60f
        }
        return calculatedScale
    }

    /**
     * Called by JNI peer when snapshot is ready.
     * Always called on the origin (main) thread.
     *
     * @param snapshot the generated snapshot
     */
    @Keep
    protected fun onSnapshotReady(snapshot: MapSnapshot) {
        Handler().post {
            if (callback != null) {
                addOverlay(snapshot)
                callback!!.onSnapshotReady(snapshot)
                reset()
            }
        }
    }

    /**
     * Called by JNI peer when snapshot has failed.
     *
     * @param reason the exception string
     */
    @Keep
    protected fun onSnapshotFailed(reason: String) {
        Handler().post {
            if (errorHandler != null) {
                errorHandler!!.onError(reason)
                reset()
            }
        }
    }

    /**
     * Called by JNI peer when snapshot style is ready.
     */
    @Keep
    protected fun onDidFailLoadingStyle(reason: String) {
        onSnapshotFailed(reason)
    }

    /**
     * Called by JNI peer when snapshot style is loaded.
     */
    @Keep
    protected fun onDidFinishLoadingStyle() {
        if (!fullyLoaded) {
            fullyLoaded = true
            val builder = options.builder
            if (builder != null) {
                for (source in builder.sources) {
                    nativeAddSource(source, source.nativePtr)
                }
                for (layerWrapper in builder.layers) {
                    if (layerWrapper is LayerAtWrapper) {
                        addLayerAt(layerWrapper.getLayer(), layerWrapper.index)
                    } else if (layerWrapper is LayerAboveWrapper) {
                        addLayerAbove(layerWrapper.getLayer(), layerWrapper.aboveLayer)
                    } else if (layerWrapper is LayerBelowWrapper) {
                        addLayerBelow(layerWrapper.getLayer(), layerWrapper.belowLayer)
                    } else {
                        addLayerBelow(layerWrapper.layer, MapLibreConstants.LAYER_ID_ANNOTATIONS)
                    }
                }
                for (image in builder.images) {
                    addImage(image.id, image.bitmap, image.isSdf)
                }
            }
        }
        if (observer != null) {
            observer!!.onDidFinishLoadingStyle()
        }
    }

    /**
     * Returns Layer of a style that is used by a snapshotter
     *
     * @param layerId the id of a Layer
     * @return the Layer object if Layer with layerId exists, null otherwise
     */
    fun getLayer(layerId: String): Layer? {
        checkThread()
        return if (fullyLoaded) nativeGetLayer(layerId) else null
    }

    /**
     * Returns Source of a style that is used by a snapshotter
     *
     * @param sourceId the id of a Source
     * @return the Source object if a Source with sourceId exists, null otherwise
     */
    fun getSource(sourceId: String): Source? {
        checkThread()
        return if (fullyLoaded) nativeGetSource(sourceId) else null
    }

    /**
     * Called by JNI peer when snapshot style image is missing.
     */
    @Keep
    protected fun onStyleImageMissing(imageName: String) {
        if (observer != null) {
            observer!!.onStyleImageMissing(imageName)
        }
    }

    private fun checkThread() {
        ThreadUtils.checkThread(TAG)
    }

    protected fun reset() {
        callback = null
        errorHandler = null
    }

    @Keep
    protected external fun nativeInitialize(
        mapSnapshotter: MapSnapshotter?,
        fileSource: FileSource?,
        pixelRatio: Float,
        width: Int,
        height: Int,
        styleUrl: String?,
        styleJson: String?,
        region: LatLngBounds?,
        position: CameraPosition?,
        showLogo: Boolean,
        localIdeographFontFamily: String?
    )

    @Keep
    protected external fun nativeStart()

    @Keep
    protected external fun nativeCancel()

    @Keep
    private external fun nativeAddLayerBelow(layerPtr: Long, below: String)

    @Keep
    private external fun nativeAddLayerAbove(layerPtr: Long, above: String)

    @Keep
    private external fun nativeAddLayerAt(layerPtr: Long, index: Int)

    @Keep
    private external fun nativeAddSource(source: Source, sourcePtr: Long)

    @Keep
    private external fun nativeAddImages(images: Array<Image>)

    @Keep
    private external fun nativeGetLayer(layerId: String): Layer

    @Keep
    private external fun nativeGetSource(sourceId: String): Source

    @Keep
    @Throws(Throwable::class)
    protected open external fun finalize()

    private inner class Logo internal constructor(val large: Bitmap, val small: Bitmap, val scale: Float)

    companion object {
        private const val TAG = "Mbgl-MapSnapshotter"
        private const val LOGO_MARGIN_DP = 4
    }
}
