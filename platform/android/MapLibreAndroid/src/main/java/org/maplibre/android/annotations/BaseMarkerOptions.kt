package org.maplibre.android.annotations

import android.os.Parcelable
import org.maplibre.android.geometry.LatLng

/**
 * Abstract builder class for composing custom Marker objects.
 *
 * Extending this class requires implementing the Parcelable interface.
 *
 * @param U Type of the marker to be composed
 * @param T Type of the builder to be used for composing a custom Marker
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
abstract class BaseMarkerOptions<U : Marker?, T : BaseMarkerOptions<U, T>?> : Parcelable {
    @JvmField
    protected var position: LatLng? = null

    @JvmField
    protected var snippet: String? = null

    @JvmField
    protected var title: String? = null

    @JvmField
    protected var icon: Icon? = null

    /**
     * Set the geographical location of the Marker.
     *
     * @param position the location to position the [Marker].
     * @return the object for which the method was called.
     */
    fun position(position: LatLng?): T {
        this.position = position
        return getThis()
    }

    /**
     * Set the snippet of the Marker.
     *
     * @param snippet the snippet of the [Marker].
     * @return the object for which the method was called.
     */
    fun snippet(snippet: String?): T {
        this.snippet = snippet
        return getThis()
    }

    /**
     * Set the title of the Marker.
     *
     * @param title the title of the [Marker].
     * @return the object for which the method was called.
     */
    fun title(title: String?): T {
        this.title = title
        return getThis()
    }

    /**
     * Set the icon of the Marker.
     *
     * @param icon the icon of the [Marker].
     * @return the object for which the method was called.
     */
    fun icon(icon: Icon?): T {
        this.icon = icon
        return getThis()
    }

    /**
     * Set the icon of the Marker.
     *
     * @param icon the icon of the [Marker].
     * @return the object for which the method was called.
     */
    fun setIcon(icon: Icon?): T = icon(icon)

    /**
     * Set the geographical location of the Marker.
     *
     * @param position the location to position the [Marker].
     * @return the object for which the method was called.
     */
    fun setPosition(position: LatLng?): T = position(position)

    /**
     * Set the snippet of the Marker.
     *
     * @param snippet the snippet of the [Marker].
     * @return the object for which the method was called.
     */
    fun setSnippet(snippet: String?): T = snippet(snippet)

    /**
     * Set the title of the Marker.
     *
     * @param title the title of the [Marker].
     * @return the object for which the method was called.
     */
    fun setTitle(title: String?): T = title(title)

    /**
     * Get the instance of the object for which this method was called.
     *
     * @return the object for which the this method was called.
     */
    abstract fun getThis(): T

    /**
     * The Marker created from this builder.
     */
    abstract val marker: U

    internal val markerPosition: LatLng?
        get() = position

    internal val markerSnippet: String?
        get() = snippet

    internal val markerTitle: String?
        get() = title

    internal val markerIcon: Icon?
        get() = icon
}
