/**
 * Contains the Mapbox Maps Android Geometry API classes.
 */
package com.mapbox.mapboxsdk.geometry

import android.os.Parcelable
import com.mapbox.mapboxsdk.geometry.LatLng
import android.os.Parcel
import com.mapbox.mapboxsdk.constants.GeometryConstants
import com.mapbox.turf.TurfMeasurement
import com.mapbox.turf.TurfConstants
import com.mapbox.mapboxsdk.geometry.LatLngQuad
import com.mapbox.mapboxsdk.geometry.LatLngSpan
import com.mapbox.mapboxsdk.geometry.LatLngBounds
import com.mapbox.mapboxsdk.exceptions.InvalidLatLngBoundsException
import com.mapbox.mapboxsdk.geometry.VisibleRegion
import com.mapbox.mapboxsdk.geometry.ProjectedMeters
