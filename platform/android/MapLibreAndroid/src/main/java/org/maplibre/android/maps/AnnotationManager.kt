package org.maplibre.android.maps

import android.graphics.PointF
import android.graphics.RectF
import androidx.collection.LongSparseArray
import org.maplibre.android.MapLibre
import org.maplibre.android.R
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.Polygon
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.annotations.Polyline
import org.maplibre.android.annotations.PolylineOptions
import org.maplibre.android.log.Logger

/**
 * Responsible for managing and tracking state of Annotations linked to Map. All events related to
 * annotations that occur on [MapLibreMap] are forwarded to this class.
 *
 * Responsible for referencing [InfoWindowManager].
 *
 * Exposes convenience methods to add/remove/update all subtypes of annotations found in
 * org.maplibre.android.annotations.
 */
@Suppress("TooManyFunctions", "LongParameterList")
internal class AnnotationManager(
    private val mapView: MapView,
    private val annotationsArray: LongSparseArray<Annotation>,
    private val iconManager: IconManager,
    private val annotationsSource: Annotations,
    private val markers: Markers,
    private val polygons: Polygons,
    private val polylines: Polylines,
    private val shapeAnnotations: ShapeAnnotations,
) {
    private val infoWindowManager = InfoWindowManager()
    private val selectedMarkers = mutableListOf<Marker>()

    private lateinit var maplibreMap: MapLibreMap
    private var onMarkerClickListener: MapLibreMap.OnMarkerClickListener? = null
    private var onPolygonClickListener: MapLibreMap.OnPolygonClickListener? = null
    private var onPolylineClickListener: MapLibreMap.OnPolylineClickListener? = null

    // TODO refactor MapLibreMap out for Projection and Transform
    // Requires removing MapLibreMap from Annotations by using Peer model from #6912
    fun bind(maplibreMap: MapLibreMap): AnnotationManager {
        this.maplibreMap = maplibreMap
        return this
    }

    fun update() {
        infoWindowManager.update()
    }

    //
    // Annotations
    //

    fun getAnnotation(id: Long): Annotation? = annotationsSource.obtainBy(id)

    val annotations: List<Annotation> get() = annotationsSource.obtainAll()

    fun removeAnnotation(id: Long) {
        annotationsSource.removeBy(id)
    }

    fun removeAnnotation(annotation: Annotation) {
        if (annotation is Marker) {
            annotation.hideInfoWindow()
            selectedMarkers.remove(annotation)
            // do icon cleanup
            annotation.icon?.let { iconManager.iconCleanup(it) }
        }
        annotationsSource.removeBy(annotation)
    }

    fun removeAnnotations(annotationList: List<Annotation>) {
        for (annotation in annotationList) {
            if (annotation is Marker) {
                annotation.hideInfoWindow()
                selectedMarkers.remove(annotation)
                annotation.icon?.let { iconManager.iconCleanup(it) }
            }
        }
        annotationsSource.removeBy(annotationList)
    }

    fun removeAnnotations() {
        val count = annotationsArray.size()
        selectedMarkers.clear()
        for (i in 0 until count) {
            val annotation = annotationsArray.get(annotationsArray.keyAt(i))
            if (annotation is Marker) {
                annotation.hideInfoWindow()
                annotation.icon?.let { iconManager.iconCleanup(it) }
            }
        }
        annotationsSource.removeAll()
    }

    //
    // Markers
    //

    fun addMarker(
        markerOptions: BaseMarkerOptions<*, *>,
        maplibreMap: MapLibreMap,
    ): Marker = markers.addBy(markerOptions, maplibreMap)

    fun addMarkers(
        markerOptionsList: List<BaseMarkerOptions<*, *>>,
        maplibreMap: MapLibreMap,
    ): List<Marker> = markers.addBy(markerOptionsList, maplibreMap)

    fun updateMarker(
        updatedMarker: Marker,
        maplibreMap: MapLibreMap,
    ) {
        if (!isAddedToMap(updatedMarker)) {
            logNonAdded(updatedMarker)
            return
        }
        markers.update(updatedMarker, maplibreMap)
    }

    fun getMarkers(): List<Marker> = markers.obtainAll()

    fun getMarkersInRect(rectangle: RectF): List<Marker> = markers.obtainAllIn(rectangle)

    fun reloadMarkers() {
        markers.reload()
    }

    //
    // Polygons
    //

    fun addPolygon(
        polygonOptions: PolygonOptions,
        maplibreMap: MapLibreMap,
    ): Polygon = polygons.addBy(polygonOptions, maplibreMap)

    fun addPolygons(
        polygonOptionsList: List<PolygonOptions>,
        maplibreMap: MapLibreMap,
    ): List<Polygon> = polygons.addBy(polygonOptionsList, maplibreMap)

    fun updatePolygon(polygon: Polygon) {
        if (!isAddedToMap(polygon)) {
            logNonAdded(polygon)
            return
        }
        polygons.update(polygon)
    }

    fun getPolygons(): List<Polygon> = polygons.obtainAll()

    //
    // Polylines
    //

    fun addPolyline(
        polylineOptions: PolylineOptions,
        maplibreMap: MapLibreMap,
    ): Polyline = polylines.addBy(polylineOptions, maplibreMap)

    fun addPolylines(
        polylineOptionsList: List<PolylineOptions>,
        maplibreMap: MapLibreMap,
    ): List<Polyline> = polylines.addBy(polylineOptionsList, maplibreMap)

    fun updatePolyline(polyline: Polyline) {
        if (!isAddedToMap(polyline)) {
            logNonAdded(polyline)
            return
        }
        polylines.update(polyline)
    }

    fun getPolylines(): List<Polyline> = polylines.obtainAll()

    // TODO Refactor from here still in progress
    fun setOnMarkerClickListener(listener: MapLibreMap.OnMarkerClickListener?) {
        onMarkerClickListener = listener
    }

    fun setOnPolygonClickListener(listener: MapLibreMap.OnPolygonClickListener?) {
        onPolygonClickListener = listener
    }

    fun setOnPolylineClickListener(listener: MapLibreMap.OnPolylineClickListener?) {
        onPolylineClickListener = listener
    }

    fun selectMarker(marker: Marker) {
        if (selectedMarkers.contains(marker)) {
            return
        }

        // Need to deselect any currently selected annotation first
        if (!infoWindowManager.isAllowConcurrentMultipleOpenInfoWindows()) {
            deselectMarkers()
        }

        if (infoWindowManager.isInfoWindowValidForMarker(marker) || infoWindowManager.getInfoWindowAdapter() != null) {
            marker.showInfoWindow(maplibreMap, mapView)?.let { infoWindowManager.add(it) }
        }

        // only add to selected markers if user didn't handle the click event themselves #3176
        selectedMarkers.add(marker)
    }

    fun deselectMarkers() {
        if (selectedMarkers.isEmpty()) {
            return
        }

        for (marker in selectedMarkers) {
            if (marker.isInfoWindowShown) {
                marker.hideInfoWindow()
            }
        }

        // Removes all selected markers from the list
        selectedMarkers.clear()
    }

    fun deselectMarker(marker: Marker) {
        if (!selectedMarkers.contains(marker)) {
            return
        }

        if (marker.isInfoWindowShown) {
            marker.hideInfoWindow()
        }
        selectedMarkers.remove(marker)
    }

    fun getSelectedMarkers(): List<Marker> = selectedMarkers

    fun getInfoWindowManager(): InfoWindowManager = infoWindowManager

    fun adjustTopOffsetPixels(maplibreMap: MapLibreMap) {
        val count = annotationsArray.size()
        for (i in 0 until count) {
            val annotation = annotationsArray.get(i.toLong())
            if (annotation is Marker) {
                annotation.icon?.let {
                    annotation.setTopOffsetPixels(iconManager.getTopOffsetPixelsForIcon(it))
                }
            }
        }

        for (marker in selectedMarkers) {
            if (marker.isInfoWindowShown) {
                marker.hideInfoWindow()
                marker.showInfoWindow(maplibreMap, mapView)
            }
        }
    }

    private fun isAddedToMap(annotation: Annotation?): Boolean =
        annotation != null && annotation.id != -1L && annotationsArray.indexOfKey(annotation.id) > -1

    private fun logNonAdded(annotation: Annotation) {
        Logger.w(
            TAG,
            "Attempting to update non-added ${annotation.javaClass.canonicalName} with value $annotation",
        )
    }

    //
    // Click event
    //

    fun onTap(tapPoint: PointF): Boolean {
        val markerHit = getMarkerHitFromTouchArea(tapPoint)
        val markerId = MarkerHitResolver(maplibreMap).execute(markerHit)
        if (markerId != NO_ANNOTATION_ID && isClickHandledForMarker(markerId)) {
            return true
        }

        val shapeAnnotationHit = getShapeAnnotationHitFromTap(tapPoint)
        val annotation = ShapeAnnotationHitResolver(shapeAnnotations).execute(shapeAnnotationHit)
        return annotation != null && handleClickForShapeAnnotation(annotation)
    }

    private fun getShapeAnnotationHitFromTap(tapPoint: PointF): ShapeAnnotationHit {
        val touchTargetSide = MapLibre.getApplicationContext().resources.getDimension(R.dimen.maplibre_eight_dp)
        val tapRect =
            RectF(
                tapPoint.x - touchTargetSide,
                tapPoint.y - touchTargetSide,
                tapPoint.x + touchTargetSide,
                tapPoint.y + touchTargetSide,
            )
        return ShapeAnnotationHit(tapRect)
    }

    private fun handleClickForShapeAnnotation(annotation: Annotation): Boolean {
        val polygonClickListener = onPolygonClickListener
        val polylineClickListener = onPolylineClickListener
        if (annotation is Polygon && polygonClickListener != null) {
            polygonClickListener.onPolygonClick(annotation)
            return true
        } else if (annotation is Polyline && polylineClickListener != null) {
            polylineClickListener.onPolylineClick(annotation)
            return true
        }
        return false
    }

    private fun getMarkerHitFromTouchArea(tapPoint: PointF): MarkerHit {
        val touchSurfaceWidth = (iconManager.highestIconHeight * 1.5).toInt()
        val touchSurfaceHeight = (iconManager.highestIconWidth * 1.5).toInt()
        val tapRect =
            RectF(
                tapPoint.x - touchSurfaceWidth,
                tapPoint.y - touchSurfaceHeight,
                tapPoint.x + touchSurfaceWidth,
                tapPoint.y + touchSurfaceHeight,
            )
        return MarkerHit(tapRect, getMarkersInRect(tapRect))
    }

    private fun isClickHandledForMarker(markerId: Long): Boolean {
        val marker = getAnnotation(markerId) as Marker
        val handledDefaultClick = onClickMarker(marker)
        if (!handledDefaultClick) {
            toggleMarkerSelectionState(marker)
        }
        return true
    }

    private fun onClickMarker(marker: Marker): Boolean = onMarkerClickListener?.onMarkerClick(marker) == true

    private fun toggleMarkerSelectionState(marker: Marker) {
        if (!selectedMarkers.contains(marker)) {
            selectMarker(marker)
        } else {
            deselectMarker(marker)
        }
    }

    private class ShapeAnnotationHitResolver(
        private val shapeAnnotations: ShapeAnnotations,
    ) {
        fun execute(shapeHit: ShapeAnnotationHit): Annotation? = shapeAnnotations.obtainAllIn(shapeHit.tapPoint).firstOrNull()
    }

    private class MarkerHitResolver(
        maplibreMap: MapLibreMap,
    ) {
        private val projection: Projection = maplibreMap.projection
        private val minimalTouchSize: Int =
            (
                32 *
                    MapLibre
                        .getApplicationContext()
                        .resources.displayMetrics.density
            ).toInt()

        private var bitmapWidth = 0
        private var bitmapHeight = 0

        private val hitRectMarker = RectF()
        private var highestSurfaceIntersection = RectF()

        private var closestMarkerId = NO_ANNOTATION_ID

        fun execute(markerHit: MarkerHit): Long {
            resolveForMarkers(markerHit)
            return closestMarkerId
        }

        private fun resolveForMarkers(markerHit: MarkerHit) {
            for (marker in markerHit.markers) {
                resolveForMarker(markerHit, marker)
            }
        }

        private fun resolveForMarker(
            markerHit: MarkerHit,
            marker: Marker,
        ) {
            val location = projection.toScreenLocation(marker.position!!)
            val markerBitmap = marker.icon!!.bitmap!!

            bitmapHeight = markerBitmap.height
            if (bitmapHeight < minimalTouchSize) {
                bitmapHeight = minimalTouchSize
            }

            bitmapWidth = markerBitmap.width
            if (bitmapWidth < minimalTouchSize) {
                bitmapWidth = minimalTouchSize
            }

            hitRectMarker.set(0f, 0f, bitmapWidth.toFloat(), bitmapHeight.toFloat())
            hitRectMarker.offsetTo(
                location.x - bitmapWidth / 2,
                location.y - bitmapHeight / 2,
            )
            hitTestMarker(markerHit, marker, hitRectMarker)
        }

        private fun hitTestMarker(
            markerHit: MarkerHit,
            marker: Marker,
            hitRectMarker: RectF,
        ) {
            if (hitRectMarker.contains(markerHit.getTapPointX(), markerHit.getTapPointY())) {
                hitRectMarker.intersect(markerHit.tapRect)
                if (isRectangleHighestSurfaceIntersection(hitRectMarker)) {
                    highestSurfaceIntersection = RectF(hitRectMarker)
                    closestMarkerId = marker.id
                }
            }
        }

        private fun isRectangleHighestSurfaceIntersection(rectF: RectF): Boolean =
            rectF.width() * rectF.height() >
                highestSurfaceIntersection.width() * highestSurfaceIntersection.height()
    }

    private class ShapeAnnotationHit(
        val tapPoint: RectF,
    )

    private class MarkerHit(
        val tapRect: RectF,
        val markers: List<Marker>,
    ) {
        fun getTapPointX(): Float = tapRect.centerX()

        fun getTapPointY(): Float = tapRect.centerY()
    }

    private companion object {
        const val TAG = "Mbgl-AnnotationManager"
        const val NO_ANNOTATION_ID = -1L
    }
}
