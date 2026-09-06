package org.maplibre.android.annotations

import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView

/**
 * Annotation is an overlay on top of a Map.
 *
 * Known subclasses are [Polygon], [Polyline] and [Marker].
 *
 * This class manages attachment to a map and identification, but does not require
 * content to be placed at a geographical point.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
abstract class Annotation protected constructor() : Comparable<Annotation> {
    /**
     * The annotation's unique ID.
     *
     * This ID is unique for a MapView instance and is suitable for associating your own extra
     * data with. It is `-1` unless the annotation was added to a MapView.
     *
     * Internal C++ id is stored as unsigned int.
     */
    var id: Long = -1

    @JvmField
    protected var maplibreMap: MapLibreMap? = null

    @JvmField
    protected var mapView: MapView? = null

    /**
     * Do not use this method, used internally by the SDK.
     */
    fun remove() {
        maplibreMap?.removeAnnotation(this)
    }

    /**
     * Do not use this method, used internally by the SDK.
     *
     * @param maplibreMap the hosting MapLibreMap
     */
    fun setMapLibreMap(maplibreMap: MapLibreMap?) {
        this.maplibreMap = maplibreMap
    }

    /**
     * Gets the hosting MapLibreMap.
     *
     * @return the MapLibreMap
     */
    protected fun getMapLibreMap(): MapLibreMap? = maplibreMap

    /**
     * Do not use this method, used internally by the SDK.
     *
     * @param mapView the hosting map view
     */
    fun setMapView(mapView: MapView?) {
        this.mapView = mapView
    }

    /**
     * Gets the hosting map view.
     *
     * @return The MapView
     */
    protected fun getMapView(): MapView? = mapView

    /**
     * Compares this Annotation object with another Annotation.
     *
     * @param other Another Annotation to compare with this object.
     * @return returns 0 if id's match, 1 if id is lower, -1 if id is higher of another Annotation
     */
    override fun compareTo(other: Annotation): Int {
        if (id < other.id) {
            return 1
        } else if (id > other.id) {
            return -1
        }
        return 0
    }

    /**
     * Checks if this Annotation object is equal to another Annotation.
     *
     * @param other Another Annotation to check equality with this object.
     * @return returns true both id's match else returns false.
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other !is Annotation) {
            return false
        }
        return id == other.id
    }

    /**
     * Gives an integer which can be used as the bucket number for storing elements of the set/map.
     * This bucket number is the address of the element inside the set/map. There's no guarantee
     * that this hash value will be consistent between different Java implementations, or even
     * between different execution runs of the same program.
     *
     * @return integer value you can use for storing element.
     */
    override fun hashCode(): Int = (id xor (id ushr 32)).toInt()
}
