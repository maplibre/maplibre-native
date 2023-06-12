package org.maplibre.android.style.sources

import android.graphics.Bitmap
import android.graphics.drawable.BitmapDrawable
import androidx.annotation.DrawableRes
import androidx.annotation.Keep
import androidx.annotation.UiThread
import org.maplibre.android.MapLibre
import org.maplibre.android.geometry.LatLngQuad
import org.maplibre.android.utils.BitmapUtils
import java.net.URI
import java.net.URL
/**
 * Image source, allows a georeferenced raster image to be shown on the map.
 *
 *
 * The georeferenced image scales and rotates as the user zooms and rotates the map.
 * The geographic location of the raster image content, supplied with `LatLngQuad`,
 * can be non-axis aligned.
 *
 * * @see [the style specification](https://maplibre.org/maplibre-style-spec/#sources-image)
 */
@UiThread
class ImageSource : Source {
    /**
     * Internal use
     *
     * @param nativePtr - pointer to native peer
     */
    @Keep
    internal constructor(nativePtr: Long) : super(nativePtr) {
    }

    /**
     * Create an ImageSource from coordinates and an image URL
     *
     * @param id          The source id
     * @param coordinates The Latitude and Longitude of the four corners of the image
     * @param url         remote json file
     */
    @Deprecated("use {@link ImageSource#ImageSource(String, LatLngQuad, URI)} instead")
    constructor(id: String?, coordinates: LatLngQuad?, url: URL) : super() {
        initialize(id, coordinates)
        setUrl(url)
    }

    /**
     * Create an ImageSource from coordinates and an image URI
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param id          The source id
     * @param coordinates The Latitude and Longitude of the four corners of the image
     * @param uri         json file uri
     */
    constructor(id: String?, coordinates: LatLngQuad?, uri: URI) : super() {
        initialize(id, coordinates)
        setUri(uri)
    }

    /**
     * Create an ImageSource from coordinates and a bitmap image
     *
     * @param id          The source id
     * @param coordinates The Latitude and Longitude of the four corners of the image
     * @param bitmap      A Bitmap image
     */
    constructor(id: String?, coordinates: LatLngQuad?, bitmap: Bitmap) : super() {
        initialize(id, coordinates)
        setImage(bitmap)
    }

    /**
     * Create an ImageSource from coordinates and a bitmap image resource
     *
     * @param id          The source id
     * @param coordinates The Latitude and Longitude of the four corners of the image
     * @param resourceId  The resource ID of a Bitmap image
     */
    constructor(id: String?, coordinates: LatLngQuad?, @DrawableRes resourceId: Int) : super() {
        initialize(id, coordinates)
        setImage(resourceId)
    }

    /**
     * Updates the source image url
     *
     * @param url An Image url
     */
    @Deprecated("use {@link #setUri(URI)} instead")
    fun setUrl(url: URL) {
        setUrl(url.toExternalForm())
    }

    /**
     * Updates the source image url
     *
     * @param url An image url
     */
    @Deprecated("use {@link #setUri(String)} instead")
    fun setUrl(url: String?) {
        checkThread()
        nativeSetUrl(url)
    }

    /**
     * Updates the source image URI.
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param uri An Image uri
     */
    fun setUri(uri: URI) {
        checkThread()
        nativeSetUrl(uri.toString())
    }

    /**
     * Updates the source image URI.
     *
     *
     * An URI is a combination of a protocol and a resource path.
     * The following URI protocol schemes are supported:
     *
     *
     *  * http://
     *
     *  * load resources using HyperText Transfer Protocol
     *
     *  * file://
     *
     *  * load resources from the Android file system
     *
     *  * asset://
     *
     *  * load resources from the binary packaged assets folder
     *
     *
     *
     * @param uri An image uri
     */
    fun setUri(uri: String?) {
        checkThread()
        nativeSetUrl(uri)
    }

    /**
     * Updates the source image to a bitmap
     *
     * @param bitmap A Bitmap image
     */
    fun setImage(bitmap: Bitmap) {
        checkThread()
        nativeSetImage(bitmap)
    }

    /**
     * Updates the source image to a bitmap image resource
     *
     * @param resourceId The resource ID of a Bitmap image
     */
    @Throws(IllegalArgumentException::class)
    fun setImage(@DrawableRes resourceId: Int) {
        checkThread()
        val context = MapLibre.getApplicationContext()
        val drawable = BitmapUtils.getDrawableFromRes(context, resourceId)
        if (drawable is BitmapDrawable) {
            nativeSetImage(drawable.bitmap)
        } else {
            throw IllegalArgumentException("Failed to decode image. The resource provided must be a Bitmap.")
        }
    }

    /**
     * @return The url or null
     */
    @get:Deprecated("use {@link #getUri()} instead")
    val url: String?
        get() {
            checkThread()
            return nativeGetUrl()
        }

    /**
     * Get the source URI.
     *
     * @return The uri or null
     */
    val uri: String?
        get() {
            checkThread()
            return nativeGetUrl()
        }

    /**
     * Updates the latitude and longitude of the four corners of the image
     *
     * @param latLngQuad latitude and longitude of the four corners of the image
     */
    fun setCoordinates(latLngQuad: LatLngQuad?) {
        checkThread()
        nativeSetCoordinates(latLngQuad)
    }

    @Keep
    protected external fun initialize(layerId: String?, payload: LatLngQuad?)

    @Keep
    protected external fun nativeSetUrl(url: String?)

    @Keep
    protected external fun nativeGetUrl(): String

    @Keep
    protected external fun nativeSetImage(bitmap: Bitmap?)

    @Keep
    protected external fun nativeSetCoordinates(latLngQuad: LatLngQuad?)

    @Keep
    @Throws(Throwable::class)
    protected external fun finalize()
}
