package org.maplibre.android.exceptions

/**
 * An InvalidLatLngBoundsException is thrown by LatLngBounds
 * when there aren't enough LatLng to create a bounds.
 */
class InvalidLatLngBoundsException(
    latLngsListSize: Int,
) : RuntimeException("Cannot create a LatLngBounds from $latLngsListSize items")
