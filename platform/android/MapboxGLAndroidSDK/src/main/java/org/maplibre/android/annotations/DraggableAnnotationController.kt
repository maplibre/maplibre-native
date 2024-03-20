package org.maplibre.android.annotations

import android.annotation.SuppressLint
import android.graphics.PointF
import android.view.MotionEvent
import android.view.View
import androidx.annotation.UiThread
import androidx.annotation.VisibleForTesting
import com.mapbox.android.gestures.AndroidGesturesManager
import com.mapbox.android.gestures.MoveGestureDetector
import com.mapbox.android.gestures.MoveGestureDetector.OnMoveGestureListener
import com.mapbox.geojson.Geometry
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView

@UiThread
internal class DraggableAnnotationController @SuppressLint("ClickableViewAccessibility") @VisibleForTesting constructor(
    mapView: MapView,
    maplibreMap: MapLibreMap?,
    androidGesturesManager: AndroidGesturesManager,
    touchAreaShiftX: Int,
    touchAreaShiftY: Int,
    touchAreaMaxX: Int,
    touchAreaMaxY: Int
) : OnMoveGestureListener {
    private var mapView: MapView?
    private var maplibreMap: MapLibreMap?
    private val annotationManagers: MutableList<AnnotationManager<*, *>> = ArrayList()
    private val touchAreaShiftX: Int
    private val touchAreaShiftY: Int
    private val touchAreaMaxX: Int
    private val touchAreaMaxY: Int
    private var dragPair: DragPair<*>? = null

    constructor(mapView: MapView, maplibreMap: MapLibreMap?) : this(
        mapView, maplibreMap, AndroidGesturesManager(mapView.context, false),
        mapView.scrollX, mapView.scrollY, mapView.measuredWidth, mapView.measuredHeight
    )

    init {
        this.mapView = mapView
        this.maplibreMap = maplibreMap
        this.touchAreaShiftX = touchAreaShiftX
        this.touchAreaShiftY = touchAreaShiftY
        this.touchAreaMaxX = touchAreaMaxX
        this.touchAreaMaxY = touchAreaMaxY
        androidGesturesManager.setMoveGestureListener(this)
        mapView.setOnTouchListener { v: View?, event: MotionEvent? ->
            // Using active gesture manager
            val oldPair = dragPair
            androidGesturesManager.onTouchEvent(event)
            dragPair != null || oldPair != null
        }
    }

    fun addAnnotationManager(annotationManager: AnnotationManager<*, *>) {
        annotationManagers.add(annotationManager)
    }

    fun removeAnnotationManager(annotationManager: AnnotationManager<*, *>) {
        annotationManagers.remove(annotationManager)
        if (annotationManagers.isEmpty()) {
            clearInstance()
        }
    }

    fun <T : KAnnotation<*>> onAnnotationDeleted(annotation: T) {
        val dragPair = dragPair
        if (annotation === dragPair?.annotation) {
            stopDragging(dragPair)
        }
    }

    override fun onMoveBegin(detector: MoveGestureDetector): Boolean = annotationManagers.any { onMoveBegin(detector, it) }

    private fun <T : KAnnotation<*>> onMoveBegin(
        detector: MoveGestureDetector, annotationManager: AnnotationManager<*, T>
    ): Boolean {
        if (detector.pointersCount == 1) {
            annotationManager.queryMapForFeatures(detector.focalPoint)?.let { annotation ->
                return startDragging(
                    DragPair(annotation, annotationManager)
                )
            }
        }
        return false
    }

    override fun onMove(detector: MoveGestureDetector, distanceX: Float, distanceY: Float): Boolean =
        dragPair?.let { onMove(detector, it) } ?: false

    private fun <T : KAnnotation<*>> onMove(detector: MoveGestureDetector, drag: DragPair<T>): Boolean {
        if (detector.pointersCount > 1 || !drag.annotation.draggable) {
            // Stopping the drag when we don't work with a simple, one-pointer move anymore
            stopDragging(drag)
            return true
        }

        // Updating symbol's position
        val moveObject = detector.getMoveObject(0)
        val x = moveObject.currentX - touchAreaShiftX
        val y = moveObject.currentY - touchAreaShiftY
        val pointF = PointF(x, y)
        if (pointF.x < 0 || pointF.y < 0 || pointF.x > touchAreaMaxX || pointF.y > touchAreaMaxY) {
            stopDragging(drag)
            return true
        }
        val geometryUpdate = drag.annotation.offsetGeometry(
            maplibreMap!!.projection, moveObject, touchAreaShiftX.toFloat(), touchAreaShiftY
                .toFloat()
        )
        if (geometryUpdate) {
            (drag.annotation.dragListener as OnAnnotationDragListener<T>?)?.onAnnotationDrag(drag.annotation)
            for (dragListener in drag.annotationManager.getDragListeners()) {
                dragListener.onAnnotationDrag(drag.annotation)
            }
            return true
        }
        return false

    }

    override fun onMoveEnd(detector: MoveGestureDetector, velocityX: Float, velocityY: Float) {
        // Stopping the drag when move ends
        stopDragging(dragPair)
    }

    @VisibleForTesting
    internal fun <T : KAnnotation<*>> startDragging(dragPair: DragPair<T>): Boolean {
        if (dragPair.annotation.draggable) {
            (dragPair.annotation.dragListener as OnAnnotationDragListener<T>?)?.onAnnotationDragStarted(dragPair.annotation)
            for (dragListener in dragPair.annotationManager.getDragListeners()) {
                dragListener.onAnnotationDragStarted(dragPair.annotation)
            }
            this.dragPair = dragPair
            return true
        }
        return false
    }

    @VisibleForTesting
    internal fun <T : KAnnotation<*>> stopDragging(dragPair: DragPair<T>?) {
        if (dragPair != null) {
            (dragPair.annotation.dragListener as OnAnnotationDragListener<T>?)?.onAnnotationDragFinished(dragPair.annotation)
            for (d in dragPair.annotationManager.getDragListeners()) {
                d.onAnnotationDragFinished(dragPair.annotation)
            }
        }
        this.dragPair = null
    }

    companion object {
        private var INSTANCE: DraggableAnnotationController? = null

        fun getInstance(mapView: MapView, maplibreMap: MapLibreMap): DraggableAnnotationController = INSTANCE.let {
            if (it == null || it.mapView !== mapView || it.maplibreMap != maplibreMap) {
                DraggableAnnotationController(mapView, maplibreMap).also { newInstance ->
                    INSTANCE = newInstance
                }
            } else {
                it
            }
        }

        private fun clearInstance() {
            if (INSTANCE != null) {
                INSTANCE!!.mapView = null
                INSTANCE!!.maplibreMap = null
                INSTANCE = null
            }
        }
    }

    @VisibleForTesting
    internal data class DragPair<T : KAnnotation<*>>(
        val annotation: T,
        val annotationManager: AnnotationManager<*, T>
    )

}