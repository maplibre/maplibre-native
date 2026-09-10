package org.maplibre.android.maps

import android.graphics.Bitmap
import android.graphics.drawable.Drawable
import android.os.AsyncTask
import android.util.DisplayMetrics
import android.util.Pair
import androidx.annotation.IntRange
import org.maplibre.android.MapLibre
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.log.Logger
import org.maplibre.android.style.layers.CannotAddLayerException
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.light.Light
import org.maplibre.android.style.sources.CannotAddSourceException
import org.maplibre.android.style.sources.Source
import org.maplibre.android.util.DefaultStyle
import org.maplibre.android.utils.BitmapUtils
import java.lang.ref.WeakReference
import java.nio.ByteBuffer

/**
 * The proxy object for current map style.
 *
 * To create new instances of this object, create a new instance using a [Builder] and load the style with
 * MapLibreMap. This object is returned from [MapLibreMap.getStyle] once the style
 * has been loaded by underlying map.
 */
@Suppress("TooManyFunctions", "LargeClass")
class Style private constructor(
    private val builder: Builder,
    private val nativeMap: NativeMap,
) {
    private val sourceMap = HashMap<String, Source>()
    private val layerMap = HashMap<String, Layer>()
    private val imageMap = HashMap<String, Bitmap>()
    private var fullyLoaded = false

    /**
     * Returns the current style url.
     *
     * @return the style url
     * @deprecated use [getUri] instead
     */
    @Deprecated("use getUri() instead", ReplaceWith("getUri()"))
    fun getUrl(): String {
        validateState("getUrl")
        return nativeMap.styleUri
    }

    /**
     * Returns the current style uri.
     *
     * @return the style uri
     */
    val uri: String
        get() {
            validateState("getUri")
            return nativeMap.styleUri
        }

    /**
     * Returns the current style json.
     *
     * @return the style json
     */
    val json: String
        get() {
            validateState("getJson")
            return nativeMap.styleJson
        }

    //
    // Source
    //

    /**
     * Retrieve all the sources in the style
     *
     * @return all the sources in the current style
     */
    val sources: List<Source>
        get() {
            validateState("getSources")
            return nativeMap.getSources()
        }

    /**
     * Adds the source to the map. The source must be newly created and not added to the map before
     *
     * @param source the source to add
     */
    fun addSource(source: Source) {
        validateState("addSource")
        nativeMap.addSource(source)
        sourceMap[source.id] = source
    }

    /**
     * Retrieve a source by id
     *
     * @param id the source's id
     * @return the source if present in the current style
     */
    fun getSource(id: String): Source? {
        validateState("getSource")
        return sourceMap[id] ?: nativeMap.getSource(id)
    }

    /**
     * Tries to cast the Source to T, throws ClassCastException if it's another type.
     *
     * @param sourceId the id used to look up a layer
     * @param T        the generic type of a Source
     * @return the casted Source, null if another type
     */
    @Suppress("UNCHECKED_CAST")
    fun <T : Source> getSourceAs(sourceId: String): T? {
        validateState("getSourceAs")
        if (sourceMap.containsKey(sourceId)) {
            return sourceMap[sourceId] as T?
        }
        return nativeMap.getSource(sourceId) as T?
    }

    /**
     * Removes the source from the style.
     *
     * @param sourceId the source to remove
     * @return true if the source was removed, false otherwise
     */
    fun removeSource(sourceId: String): Boolean {
        validateState("removeSource")
        val successful = nativeMap.removeSource(sourceId)
        if (successful) {
            sourceMap.remove(sourceId)
        }
        return successful
    }

    /**
     * Removes the source, preserving the reference for re-use
     *
     * @param source the source to remove
     * @return true if the source was removed, false otherwise
     */
    fun removeSource(source: Source): Boolean {
        validateState("removeSource")
        val successful = nativeMap.removeSource(source)
        if (successful) {
            sourceMap.remove(source.id)
        }
        return successful
    }

    //
    // Layer
    //

    /**
     * Adds the layer to the map. The layer must be newly created and not added to the map before
     *
     * @param layer the layer to add
     */
    fun addLayer(layer: Layer) {
        validateState("addLayer")
        nativeMap.addLayer(layer)
        layerMap[layer.id] = layer
    }

    /**
     * Adds the layer to the map. The layer must be newly created and not added to the map before
     *
     * @param layer the layer to add
     * @param below the layer id to add this layer before
     */
    fun addLayerBelow(
        layer: Layer,
        below: String,
    ) {
        validateState("addLayerBelow")
        nativeMap.addLayerBelow(layer, below)
        layerMap[layer.id] = layer
    }

    /**
     * Adds the layer to the map. The layer must be newly created and not added to the map before
     *
     * @param layer the layer to add
     * @param above the layer id to add this layer above
     */
    fun addLayerAbove(
        layer: Layer,
        above: String,
    ) {
        validateState("addLayerAbove")
        nativeMap.addLayerAbove(layer, above)
        layerMap[layer.id] = layer
    }

    /**
     * Adds the layer to the map at the specified index. The layer must be newly
     * created and not added to the map before
     *
     * @param layer the layer to add
     * @param index the index to insert the layer at
     */
    fun addLayerAt(
        layer: Layer,
        @IntRange(from = 0) index: Int,
    ) {
        validateState("addLayerAbove")
        nativeMap.addLayerAt(layer, index)
        layerMap[layer.id] = layer
    }

    /**
     * Get the layer by id
     *
     * @param id the layer's id
     * @return the layer, if present in the style
     */
    fun getLayer(id: String): Layer? {
        validateState("getLayer")
        return layerMap[id] ?: nativeMap.getLayer(id)
    }

    /**
     * Tries to cast the Layer to T, throws ClassCastException if it's another type.
     *
     * @param layerId the layer id used to look up a layer
     * @param T       the generic attribute of a Layer
     * @return the casted Layer, null if another type
     */
    @Suppress("UNCHECKED_CAST")
    fun <T : Layer> getLayerAs(layerId: String): T? {
        validateState("getLayerAs")
        return nativeMap.getLayer(layerId) as T?
    }

    /**
     * Retrieve all the layers in the style
     *
     * @return all the layers in the current style
     */
    val layers: List<Layer>
        get() {
            validateState("getLayers")
            return nativeMap.getLayers()
        }

    /**
     * Removes the layer. Any references to the layer become invalid and should not be used anymore
     *
     * @param layerId the layer to remove
     * @return true if the layer was removed, false otherwise
     */
    fun removeLayer(layerId: String): Boolean {
        validateState("removeLayer")
        layerMap.remove(layerId)
        return nativeMap.removeLayer(layerId)
    }

    /**
     * Removes the layer. The reference is re-usable after this and can be re-added
     *
     * @param layer the layer to remove
     * @return true if the layer was removed, false otherwise
     */
    fun removeLayer(layer: Layer): Boolean {
        validateState("removeLayer")
        layerMap.remove(layer.id)
        return nativeMap.removeLayer(layer)
    }

    /**
     * Removes the layer. Any other references to the layer become invalid and should not be used anymore
     *
     * @param index the layer index
     * @return true if the layer was removed, false otherwise
     */
    fun removeLayerAt(
        @IntRange(from = 0) index: Int,
    ): Boolean {
        validateState("removeLayerAt")
        return nativeMap.removeLayerAt(index)
    }

    //
    // Image
    //

    /**
     * Adds an image to be used in the map's style
     *
     * @param name  the name of the image
     * @param image the pre-multiplied Bitmap
     */
    fun addImage(
        name: String,
        image: Bitmap,
    ) {
        addImage(name, image, false)
    }

    /**
     * Adds an image to be used in the map's style
     *
     * @param name     the name of the image
     * @param image    the pre-multiplied Bitmap
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    fun addImage(
        name: String,
        image: Bitmap,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        addImage(name, image, false, stretchX, stretchY, content)
    }

    /**
     * Adds an drawable to be converted into a bitmap to be used in the map's style
     *
     * @param name     the name of the image
     * @param drawable the drawable instance to convert
     */
    fun addImage(
        name: String,
        drawable: Drawable,
    ) {
        val bitmap =
            BitmapUtils.getBitmapFromDrawable(drawable)
                ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
        addImage(name, bitmap, false)
    }

    /**
     * Adds an drawable to be converted into a bitmap to be used in the map's style
     *
     * @param name     the name of the image
     * @param drawable the drawable instance to convert
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    fun addImage(
        name: String,
        drawable: Drawable,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        val bitmap =
            BitmapUtils.getBitmapFromDrawable(drawable)
                ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
        addImage(name, bitmap, false, stretchX, stretchY, content)
    }

    /**
     * Adds an image to be used in the map's style
     *
     * @param name   the name of the image
     * @param bitmap the pre-multiplied Bitmap
     * @param sdf    the flag indicating image is an SDF or template image
     */
    fun addImage(
        name: String,
        bitmap: Bitmap,
        sdf: Boolean,
    ) {
        validateState("addImage")
        nativeMap.addImages(arrayOf(toImage(Builder.ImageWrapper(name, bitmap, sdf))))
    }

    /**
     * Adds an image to be used in the map's style
     *
     * @param name     the name of the image
     * @param bitmap   the pre-multiplied Bitmap
     * @param sdf      the flag indicating image is an SDF or template image
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    @Suppress("LongParameterList")
    fun addImage(
        name: String,
        bitmap: Bitmap,
        sdf: Boolean,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        validateState("addImage")
        nativeMap.addImages(arrayOf(toImage(Builder.ImageWrapper(name, bitmap, sdf, stretchX, stretchY, content))))
    }

    /**
     * Adds an image asynchronously, to be used in the map's style.
     *
     * @param name  the name of the image
     * @param image the pre-multiplied Bitmap
     */
    fun addImageAsync(
        name: String,
        image: Bitmap,
    ) {
        addImageAsync(name, image, false)
    }

    /**
     * Adds an image asynchronously, to be used in the map's style.
     *
     * @param name     the name of the image
     * @param image    the pre-multiplied Bitmap
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    fun addImageAsync(
        name: String,
        image: Bitmap,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        addImageAsync(name, image, false, stretchX, stretchY, content)
    }

    /**
     * Adds an drawable asynchronously, to be converted into a bitmap to be used in the map's style.
     *
     * @param name     the name of the image
     * @param drawable the drawable instance to convert
     */
    fun addImageAsync(
        name: String,
        drawable: Drawable,
    ) {
        val bitmap =
            BitmapUtils.getBitmapFromDrawable(drawable)
                ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
        addImageAsync(name, bitmap, false)
    }

    /**
     * Adds an drawable asynchronously, to be converted into a bitmap to be used in the map's style.
     *
     * @param name     the name of the image
     * @param drawable the drawable instance to convert
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    fun addImageAsync(
        name: String,
        drawable: Drawable,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        val bitmap =
            BitmapUtils.getBitmapFromDrawable(drawable)
                ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
        addImageAsync(name, bitmap, false, stretchX, stretchY, content)
    }

    /**
     * Adds an image asynchronously, to be used in the map's style.
     *
     * @param name   the name of the image
     * @param bitmap the pre-multiplied Bitmap
     * @param sdf    the flag indicating image is an SDF or template image
     */
    fun addImageAsync(
        name: String,
        bitmap: Bitmap,
        sdf: Boolean,
    ) {
        validateState("addImage")
        BitmapImageConversionTask(nativeMap).execute(Builder.ImageWrapper(name, bitmap, sdf))
    }

    /**
     * Adds an image asynchronously, to be used in the map's style.
     *
     * @param name     the name of the image
     * @param bitmap   the pre-multiplied Bitmap
     * @param sdf      the flag indicating image is an SDF or template image
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    @Suppress("LongParameterList")
    fun addImageAsync(
        name: String,
        bitmap: Bitmap,
        sdf: Boolean,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        validateState("addImage")
        BitmapImageConversionTask(nativeMap)
            .execute(Builder.ImageWrapper(name, bitmap, sdf, stretchX, stretchY, content))
    }

    /**
     * Adds images to be used in the map's style.
     *
     * @param images the map of images to add
     */
    fun addImages(images: HashMap<String, Bitmap>) {
        addImages(images, false)
    }

    /**
     * Adds images to be used in the map's style.
     *
     * @param images   the map of images to add
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    fun addImages(
        images: HashMap<String, Bitmap>,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        addImages(images, false, stretchX, stretchY, content)
    }

    /**
     * Adds images to be used in the map's style.
     *
     * @param images the map of images to add
     * @param sdf    the flag indicating image is an SDF or template image
     */
    fun addImages(
        images: HashMap<String, Bitmap>,
        sdf: Boolean,
    ) {
        validateState("addImage")
        val wrappers = Builder.ImageWrapper.convertToImageArray(images, sdf)
        nativeMap.addImages(Array(wrappers.size) { toImage(wrappers[it]) })
    }

    /**
     * Adds images to be used in the map's style.
     *
     * @param images   the map of images to add
     * @param sdf      the flag indicating image is an SDF or template image
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    @Suppress("LongParameterList")
    fun addImages(
        images: HashMap<String, Bitmap>,
        sdf: Boolean,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        validateState("addImage")
        val wrappers = Builder.ImageWrapper.convertToImageArray(images, sdf, stretchX, stretchY, content)
        nativeMap.addImages(Array(wrappers.size) { toImage(wrappers[it]) })
    }

    /**
     * Adds images asynchronously, to be used in the map's style.
     *
     * @param images the map of images to add
     */
    fun addImagesAsync(images: HashMap<String, Bitmap>) {
        addImagesAsync(images, false)
    }

    /**
     * Adds images asynchronously, to be used in the map's style.
     *
     * @param images   the map of images to add
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    fun addImagesAsync(
        images: HashMap<String, Bitmap>,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        addImagesAsync(images, false, stretchX, stretchY, content)
    }

    /**
     * Adds images asynchronously, to be used in the map's style.
     *
     * @param images the map of images to add
     * @param sdf    the flag indicating image is an SDF or template image
     */
    fun addImagesAsync(
        images: HashMap<String, Bitmap>,
        sdf: Boolean,
    ) {
        validateState("addImages")
        BitmapImageConversionTask(nativeMap).execute(*Builder.ImageWrapper.convertToImageArray(images, sdf))
    }

    /**
     * Adds images asynchronously, to be used in the map's style.
     *
     * @param images   the map of images to add
     * @param sdf      the flag indicating image is an SDF or template image
     * @param stretchX image stretch areas for x axix
     * @param stretchY image stretch areas for y axix
     * @param content  image content for text to fit
     */
    @Suppress("LongParameterList")
    fun addImagesAsync(
        images: HashMap<String, Bitmap>,
        sdf: Boolean,
        stretchX: List<ImageStretches>,
        stretchY: List<ImageStretches>,
        content: ImageContent?,
    ) {
        validateState("addImages")
        BitmapImageConversionTask(nativeMap)
            .execute(*Builder.ImageWrapper.convertToImageArray(images, sdf, stretchX, stretchY, content))
    }

    /**
     * Removes an image from the map's style.
     *
     * @param name the name of the image to remove
     */
    fun removeImage(name: String) {
        validateState("removeImage")
        nativeMap.removeImage(name)
    }

    /**
     * Get an image from the map's style using an id.
     *
     * @param id the id of the image
     * @return the image bitmap
     */
    fun getImage(id: String): Bitmap? {
        validateState("getImage")
        return nativeMap.getImage(id)
    }

    //
    // Transition
    //

    /**
     * The transition options for style changes.
     *
     * If not set, any changes take effect without animation, besides symbols,
     * which will fade in/out with a default duration after symbol collision detection.
     *
     * To disable symbols fade in/out animation,
     * pass transition options with [TransitionOptions.enablePlacementTransitions] equal to false.
     *
     * Both [TransitionOptions.duration] and [TransitionOptions.delay]
     * will also change the behavior of the symbols fade in/out animation if the placement transition is enabled.
     */
    var transition: TransitionOptions
        get() {
            validateState("getTransition")
            return nativeMap.transitionOptions
        }
        set(value) {
            validateState("setTransition")
            nativeMap.transitionOptions = value
        }

    //
    // Light
    //

    /**
     * Get the light source used to change lighting conditions on extruded fill layers.
     *
     * @return the global light source
     */
    val light: Light?
        get() {
            validateState("getLight")
            return nativeMap.getLight()
        }

    //
    // State
    //

    /**
     * Called when the underlying map will start loading a new style or the map is destroyed.
     * This method will clean up this style by setting the java sources and layers
     * in a detached state and removing them from core.
     */
    internal fun clear() {
        fullyLoaded = false
        for (layer in layerMap.values) {
            layer.setDetached()
        }

        for (source in sourceMap.values) {
            source.setDetached()
        }

        for ((name, bitmap) in imageMap) {
            nativeMap.removeImage(name)
            bitmap.recycle()
        }

        sourceMap.clear()
        layerMap.clear()
        imageMap.clear()
    }

    /**
     * Called when the underlying map has finished loading this style.
     * This method will add all components added to the builder that were defined with the 'with' prefix.
     */
    internal fun onDidFinishLoadingStyle() {
        if (fullyLoaded) {
            return
        }
        fullyLoaded = true
        for (source in builder.sourceList) {
            try {
                addSource(source)
            } catch (exception: CannotAddSourceException) {
                Logger.e(TAG, "Failed to add source", exception)
            }
        }

        try {
            for (layerWrapper in builder.layerList) {
                when (layerWrapper) {
                    is Builder.LayerAtWrapper -> addLayerAt(layerWrapper.layer, layerWrapper.index)

                    is Builder.LayerAboveWrapper -> addLayerAbove(layerWrapper.layer, layerWrapper.aboveLayer)

                    is Builder.LayerBelowWrapper -> addLayerBelow(layerWrapper.layer, layerWrapper.belowLayer)

                    // just add layer to map, but below annotations
                    else -> addLayerBelow(layerWrapper.layer, MapLibreConstants.LAYER_ID_ANNOTATIONS)
                }
            }
        } catch (exception: CannotAddLayerException) {
            Logger.e(TAG, "Failed to add layer", exception)
        }

        for (image in builder.imageList) {
            addImage(image.id, image.bitmap, image.sdf)
        }

        builder.transition?.let { transition = it }
    }

    /**
     * Returns true if the style is fully loaded. Returns false if style hasn't been fully loaded or a new style is
     * underway of being loaded.
     *
     * @return True if fully loaded, false otherwise
     */
    val isFullyLoaded: Boolean
        get() = fullyLoaded

    /**
     * Validates the style state, throw an IllegalArgumentException on invalid state.
     *
     * @param methodCall the calling method name
     */
    private fun validateState(methodCall: String) {
        if (!fullyLoaded) {
            throw IllegalStateException("Calling $methodCall when a newer style is loading/has loaded.")
        }
    }

    //
    // Builder
    //

    /**
     * Builder for composing a style object.
     */
    @Suppress("TooManyFunctions")
    class Builder {
        internal val sourceList = mutableListOf<Source>()
        internal val layerList = mutableListOf<LayerWrapper>()
        internal val imageList = mutableListOf<ImageWrapper>()

        internal var transition: TransitionOptions? = null
        private var styleUri: String? = null
        private var styleJson: String? = null

        /**
         * Will loads a new map style asynchronous from the specified URL.
         *
         * `url` can take the following forms:
         *
         *  - `http://...` or `https://...`: loads the style over the Internet from any web server.
         *  - `asset://...`: loads the style from the APK `assets/` directory.
         *    This is used to load a style bundled with your app.
         *  - `file://...`: loads the style from a file path. This is used to load a style from disk.
         *
         * This method is asynchronous and will return before the style finishes loading.
         * If you wish to wait for the map to finish loading, listen to the
         * [MapView.OnDidFinishLoadingStyleListener] callback or provide an [OnStyleLoaded] callback
         * when setting the style on MapLibreMap.
         *
         * If the style fails to load or an invalid style URL is set, the map view will become blank.
         * An error message will be logged in the Android logcat and [MapView.OnDidFailLoadingMapListener] callback
         * will be triggered.
         *
         * @param url The URL of the map style
         * @return this
         * @see Style
         * @deprecated use [fromUri] instead
         */
        @Deprecated("use fromUri(String) instead", ReplaceWith("fromUri(url)"))
        fun fromUrl(url: String): Builder {
            styleUri = url
            return this
        }

        /**
         * Will loads a new map style asynchronous from the specified URI.
         *
         * `uri` can take the following forms:
         *
         *  - `http://...` or `https://...`: loads the style over the Internet from any web server.
         *  - `asset://...`: loads the style from the APK `assets/` directory.
         *    This is used to load a style bundled with your app.
         *  - `file://...`: loads the style from a file path. This is used to load a style from disk.
         *
         * This method is asynchronous and will return before the style finishes loading.
         * If you wish to wait for the map to finish loading, listen to the
         * [MapView.OnDidFinishLoadingStyleListener] callback or use [MapLibreMap.setStyle] instead.
         *
         * If the style fails to load or an invalid style URI is set, the map view will become blank.
         * An error message will be logged in the Android logcat and [MapView.OnDidFailLoadingMapListener] callback
         * will be triggered.
         *
         * @param uri The URI of the map style
         * @return this
         * @see Style
         */
        fun fromUri(uri: String): Builder {
            styleUri = uri
            return this
        }

        /**
         * Will load a new map style from a json string.
         *
         * If the style fails to load or an invalid style URI is set, the map view will become blank.
         * An error message will be logged in the Android logcat and [MapView.OnDidFailLoadingMapListener] callback
         * will be triggered.
         *
         * @return this
         */
        fun fromJson(styleJson: String): Builder {
            this.styleJson = styleJson
            return this
        }

        /**
         * Will add the source when map style has loaded.
         *
         * @param source the source to add
         * @return this
         */
        fun withSource(source: Source): Builder {
            sourceList.add(source)
            return this
        }

        /**
         * Will add the sources when map style has loaded.
         *
         * @param sources the sources to add
         * @return this
         */
        fun withSources(vararg sources: Source): Builder {
            sourceList.addAll(sources)
            return this
        }

        /**
         * Will add the layer when the style has loaded.
         *
         * @param layer the layer to be added
         * @return this
         */
        fun withLayer(layer: Layer): Builder {
            layerList.add(LayerWrapper(layer))
            return this
        }

        /**
         * Will add the layers when the style has loaded.
         *
         * @param layers the layers to be added
         * @return this
         */
        fun withLayers(vararg layers: Layer): Builder {
            for (layer in layers) {
                layerList.add(LayerWrapper(layer))
            }
            return this
        }

        /**
         * Will add the layer when the style has loaded at a specified index.
         *
         * @param layer the layer to be added
         * @return this
         */
        fun withLayerAt(
            layer: Layer,
            index: Int,
        ): Builder {
            layerList.add(LayerAtWrapper(layer, index))
            return this
        }

        /**
         * Will add the layer when the style has loaded above a specified layer id.
         *
         * @param layer the layer to be added
         * @return this
         */
        fun withLayerAbove(
            layer: Layer,
            aboveLayerId: String,
        ): Builder {
            layerList.add(LayerAboveWrapper(layer, aboveLayerId))
            return this
        }

        /**
         * Will add the layer when the style has loaded below a specified layer id.
         *
         * @param layer the layer to be added
         * @return this
         */
        fun withLayerBelow(
            layer: Layer,
            belowLayerId: String,
        ): Builder {
            layerList.add(LayerBelowWrapper(layer, belowLayerId))
            return this
        }

        /**
         * Will add the transition when the map style has loaded.
         *
         * @param transition the transition to be added
         * @return this
         */
        fun withTransition(transition: TransitionOptions): Builder {
            this.transition = transition
            return this
        }

        /**
         * Will add the drawable as image when the map style has loaded.
         *
         * @param id       the id for the image
         * @param drawable the drawable to be converted and added
         * @return this
         */
        fun withImage(
            id: String,
            drawable: Drawable,
        ): Builder = withImage(id, drawable, false)

        /**
         * Will add the drawable as image when the map style has loaded.
         *
         * @param id       the id for the image
         * @param drawable the drawable to be converted and added
         * @param stretchX image stretch areas for x axix
         * @param stretchY image stretch areas for y axix
         * @param content  image content for text to fit
         * @return this
         */
        fun withImage(
            id: String,
            drawable: Drawable,
            stretchX: List<ImageStretches>,
            stretchY: List<ImageStretches>,
            content: ImageContent?,
        ): Builder = withImage(id, drawable, false, stretchX, stretchY, content)

        /**
         * Will add the drawables as images when the map style has loaded.
         *
         * @param values pairs, where first is the id for te image and second is the drawable
         * @return this
         */
        fun withDrawableImages(vararg values: Pair<String, Drawable>): Builder = withDrawableImages(false, *values)

        /**
         * Will add the image when the map style has loaded.
         *
         * @param id    the id for the image
         * @param image the image to be added
         * @return this
         */
        fun withImage(
            id: String,
            image: Bitmap,
        ): Builder = withImage(id, image, false)

        /**
         * Will add the image when the map style has loaded.
         *
         * @param id       the id for the image
         * @param image    the image to be added
         * @param stretchX image stretch areas for x axix
         * @param stretchY image stretch areas for y axix
         * @param content  image content for text to fit
         * @return this
         */
        fun withImage(
            id: String,
            image: Bitmap,
            stretchX: List<ImageStretches>,
            stretchY: List<ImageStretches>,
            content: ImageContent?,
        ): Builder = withImage(id, image, false, stretchX, stretchY, content)

        /**
         * Will add the images when the map style has loaded.
         *
         * @param values pairs, where first is the id for te image and second is the bitmap
         * @return this
         */
        fun withBitmapImages(vararg values: Pair<String, Bitmap>): Builder {
            for (value in values) {
                withImage(value.first, value.second, false)
            }
            return this
        }

        /**
         * Will add the drawable as image when the map style has loaded.
         *
         * @param id       the id for the image
         * @param drawable the drawable to be converted and added
         * @param sdf      the flag indicating image is an SDF or template image
         * @return this
         */
        fun withImage(
            id: String,
            drawable: Drawable,
            sdf: Boolean,
        ): Builder {
            val bitmap =
                BitmapUtils.getBitmapFromDrawable(drawable)
                    ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
            return withImage(id, bitmap, sdf)
        }

        /**
         * Will add the drawable as image when the map style has loaded.
         *
         * @param id       the id for the image
         * @param drawable the drawable to be converted and added
         * @param sdf      the flag indicating image is an SDF or template image
         * @param stretchX image stretch areas for x axix
         * @param stretchY image stretch areas for y axix
         * @param content  image content for text to fit
         * @return this
         */
        @Suppress("LongParameterList")
        fun withImage(
            id: String,
            drawable: Drawable,
            sdf: Boolean,
            stretchX: List<ImageStretches>,
            stretchY: List<ImageStretches>,
            content: ImageContent?,
        ): Builder {
            val bitmap =
                BitmapUtils.getBitmapFromDrawable(drawable)
                    ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
            return withImage(id, bitmap, sdf, stretchX, stretchY, content)
        }

        /**
         * Will add the drawables as images when the map style has loaded.
         *
         * @param sdf    the flag indicating image is an SDF or template image
         * @param values pairs, where first is the id for te image and second is the drawable
         * @return this
         */
        fun withDrawableImages(
            sdf: Boolean,
            vararg values: Pair<String, Drawable>,
        ): Builder {
            for (value in values) {
                val bitmap =
                    BitmapUtils.getBitmapFromDrawable(value.second)
                        ?: throw IllegalArgumentException("Provided drawable couldn't be converted to a Bitmap.")
                withImage(value.first, bitmap, sdf)
            }
            return this
        }

        /**
         * Will add the image when the map style has loaded.
         *
         * @param id    the id for the image
         * @param image the image to be added
         * @param sdf   the flag indicating image is an SDF or template image
         * @return this
         */
        fun withImage(
            id: String,
            image: Bitmap,
            sdf: Boolean,
        ): Builder {
            imageList.add(ImageWrapper(id, image, sdf))
            return this
        }

        /**
         * Will add the image when the map style has loaded.
         *
         * @param id       the id for the image
         * @param image    the image to be added
         * @param sdf      the flag indicating image is an SDF or template image
         * @param stretchX image stretch areas for x axix
         * @param stretchY image stretch areas for y axix
         * @param content  image content for text to fit
         * @return this
         */
        @Suppress("LongParameterList")
        fun withImage(
            id: String,
            image: Bitmap,
            sdf: Boolean,
            stretchX: List<ImageStretches>,
            stretchY: List<ImageStretches>,
            content: ImageContent?,
        ): Builder {
            imageList.add(ImageWrapper(id, image, sdf, stretchX, stretchY, content))
            return this
        }

        /**
         * Will add the images when the map style has loaded.
         *
         * @param sdf    the flag indicating image is an SDF or template image
         * @param values pairs, where first is the id for te image and second is the bitmap
         * @return this
         */
        fun withBitmapImages(
            sdf: Boolean,
            vararg values: Pair<String, Bitmap>,
        ): Builder {
            for (value in values) {
                withImage(value.first, value.second, sdf)
            }
            return this
        }

        val uri: String? get() = styleUri

        val json: String? get() = styleJson

        val sources: List<Source> get() = sourceList

        val layers: List<LayerWrapper> get() = layerList

        val images: List<ImageWrapper> get() = imageList

        internal val transitionOptions: TransitionOptions? get() = transition

        /**
         * Build the composed style.
         */
        internal fun build(nativeMap: NativeMap): Style = Style(this, nativeMap)

        class ImageWrapper
            @JvmOverloads
            constructor(
                val id: String,
                val bitmap: Bitmap,
                @get:JvmName("isSdf") val sdf: Boolean,
                val stretchX: List<ImageStretches>? = null,
                val stretchY: List<ImageStretches>? = null,
                val content: ImageContent? = null,
            ) {
                companion object {
                    @JvmStatic
                    fun convertToImageArray(
                        bitmapHashMap: HashMap<String, Bitmap>,
                        sdf: Boolean,
                    ): Array<ImageWrapper> {
                        val keyList = bitmapHashMap.keys.toList()
                        return Array(keyList.size) { index ->
                            val id = keyList[index]
                            ImageWrapper(id, bitmapHashMap.getValue(id), sdf)
                        }
                    }

                    @JvmStatic
                    fun convertToImageArray(
                        bitmapHashMap: HashMap<String, Bitmap>,
                        sdf: Boolean,
                        stretchX: List<ImageStretches>?,
                        stretchY: List<ImageStretches>?,
                        content: ImageContent?,
                    ): Array<ImageWrapper> {
                        val keyList = bitmapHashMap.keys.toList()
                        return Array(keyList.size) { index ->
                            val id = keyList[index]
                            ImageWrapper(id, bitmapHashMap.getValue(id), sdf, stretchX, stretchY, content)
                        }
                    }
                }
            }

        open inner class LayerWrapper internal constructor(
            val layer: Layer,
        )

        inner class LayerAboveWrapper internal constructor(
            layer: Layer,
            val aboveLayer: String,
        ) : LayerWrapper(layer)

        inner class LayerBelowWrapper internal constructor(
            layer: Layer,
            val belowLayer: String,
        ) : LayerWrapper(layer)

        inner class LayerAtWrapper internal constructor(
            layer: Layer,
            val index: Int,
        ) : LayerWrapper(layer)
    }

    @Suppress("DEPRECATION")
    private class BitmapImageConversionTask(
        nativeMap: NativeMap,
    ) : AsyncTask<Builder.ImageWrapper, Void, Array<Image>>() {
        private val nativeMap = WeakReference(nativeMap)

        override fun doInBackground(vararg params: Builder.ImageWrapper): Array<Image> = Array(params.size) { toImage(params[it]) }

        override fun onPostExecute(images: Array<Image>) {
            super.onPostExecute(images)
            val nativeMap = this.nativeMap.get()
            if (nativeMap != null && !nativeMap.isDestroyed) {
                nativeMap.addImages(images)
            }
        }
    }

    /**
     * Callback to be invoked when a style has finished loading.
     */
    fun interface OnStyleLoaded {
        /**
         * Invoked when a style has finished loading.
         *
         * @param style the style that has finished loading
         */
        fun onStyleLoaded(style: Style)
    }

    companion object {
        internal const val TAG = "Style"
        internal const val EMPTY_JSON = "{\"version\": 8,\"sources\": {},\"layers\": []}"

        @JvmStatic
        fun toImage(imageWrapper: Builder.ImageWrapper): Image {
            var bitmap = imageWrapper.bitmap
            if (bitmap.config != Bitmap.Config.ARGB_8888) {
                bitmap = bitmap.copy(Bitmap.Config.ARGB_8888, false) ?: bitmap
            }

            val buffer = ByteBuffer.allocate(bitmap.byteCount)
            bitmap.copyPixelsToBuffer(buffer)
            val pixelRatio = bitmap.density.toFloat() / DisplayMetrics.DENSITY_DEFAULT

            val stretchX = imageWrapper.stretchX
            val stretchY = imageWrapper.stretchY
            if (stretchX != null && stretchY != null) {
                val arrayX = FloatArray(stretchX.size * 2)
                for (i in stretchX.indices) {
                    arrayX[i * 2] = stretchX[i].first
                    arrayX[i * 2 + 1] = stretchX[i].second
                }

                val arrayY = FloatArray(stretchY.size * 2)
                for (i in stretchY.indices) {
                    arrayY[i * 2] = stretchY[i].first
                    arrayY[i * 2 + 1] = stretchY[i].second
                }
                return Image(
                    buffer.array(),
                    pixelRatio,
                    imageWrapper.id,
                    bitmap.width,
                    bitmap.height,
                    imageWrapper.sdf,
                    arrayX,
                    arrayY,
                    imageWrapper.content?.contentArray,
                )
            }

            return Image(buffer.array(), pixelRatio, imageWrapper.id, bitmap.width, bitmap.height, imageWrapper.sdf)
        }

        //
        // Style URL constants
        //

        /**
         * Get predefined styles
         *
         * @return The list of predefined styles
         */
        @JvmStatic
        fun getPredefinedStyles(): Array<DefaultStyle>? = MapLibre.getPredefinedStyles()

        /**
         * Get predefined style by name
         *
         * @return The predefined style definition
         */
        @JvmStatic
        fun getPredefinedStyle(name: String): String {
            val style = MapLibre.getPredefinedStyle(name)
            return style?.url ?: throw IllegalArgumentException("Could not find layer $name")
        }
    }
}
