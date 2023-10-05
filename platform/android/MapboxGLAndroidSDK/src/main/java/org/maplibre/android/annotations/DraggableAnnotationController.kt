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
) {
    private var mapView: MapView?
    private var maplibreMap: MapLibreMap?
    private val annotationManagers: MutableList<AnnotationManager<*, *>> = ArrayList()
    private val touchAreaShiftX: Int
    private val touchAreaShiftY: Int
    private val touchAreaMaxX: Int
    private val touchAreaMaxY: Int
    private var draggedAnnotation: KAnnotation<*>? = null
    private var draggedAnnotationManager: AnnotationManager<*, *>? = null

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
        androidGesturesManager.setMoveGestureListener(AnnotationMoveGestureListener())
        mapView.setOnTouchListener { v: View?, event: MotionEvent? ->
            // Using active gesture manager
            val oldAnnotation = draggedAnnotation
            androidGesturesManager.onTouchEvent(event)
            draggedAnnotation != null || oldAnnotation != null
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

    fun onAnnotationDeleted(annotation: KAnnotation<*>) {
        if (annotation === draggedAnnotation) {
            stopDragging(draggedAnnotation, draggedAnnotationManager)
        }
    }

    fun onMoveBegin(detector: MoveGestureDetector): Boolean {
        for (annotationManager in annotationManagers) {
            if (detector.pointersCount == 1) {
                val annotation = annotationManager.queryMapForFeatures(detector.focalPoint)
                if (annotation != null && startDragging(annotation, annotationManager)) {
                    return true
                }
            }
        }
        return false
    }

    fun onMove(detector: MoveGestureDetector): Boolean {
        if (draggedAnnotation != null && (detector.pointersCount > 1 || !draggedAnnotation!!.draggable)) {
            // Stopping the drag when we don't work with a simple, on-pointer move anymore
            stopDragging(draggedAnnotation, draggedAnnotationManager)
            return true
        }

        // Updating symbol's position
        if (draggedAnnotation != null) {
            val moveObject = detector.getMoveObject(0)
            val x = moveObject.currentX - touchAreaShiftX
            val y = moveObject.currentY - touchAreaShiftY
            val pointF = PointF(x, y)
            if (pointF.x < 0 || pointF.y < 0 || pointF.x > touchAreaMaxX || pointF.y > touchAreaMaxY) {
                stopDragging(draggedAnnotation, draggedAnnotationManager)
                return true
            }
            val shiftedGeometry = draggedAnnotation!!.getOffsetGeometry(
                maplibreMap!!.projection, moveObject, touchAreaShiftX.toFloat(), touchAreaShiftY
                    .toFloat()
            )
            if (shiftedGeometry != null) {
                draggedAnnotation!!.geometry = shiftedGeometry
                draggedAnnotationManager!!.updateSource()
                for (d in draggedAnnotationManager!!.getDragListeners()) {
                    d.onAnnotationDrag(draggedAnnotation)
                }
                return true
            }
        }
        return false
    }

    fun onMoveEnd() {
        // Stopping the drag when move ends
        stopDragging(draggedAnnotation, draggedAnnotationManager)
    }

    fun startDragging(annotation: KAnnotation<*>, annotationManager: AnnotationManager<*, *>): Boolean {
        if (annotation.draggable) {
            for (d in annotationManager.getDragListeners()) {
                d.onAnnotationDragStarted(annotation)
            }
            draggedAnnotation = annotation
            draggedAnnotationManager = annotationManager
            return true
        }
        return false
    }

    fun stopDragging(annotation: KAnnotation<*>?, annotationManager: AnnotationManager<*, *>?) {
        if (annotation != null && annotationManager != null) {
            for (d in annotationManager.getDragListeners()) {
                d.onAnnotationDragFinished(annotation)
            }
        }
        draggedAnnotation = null
        draggedAnnotationManager = null
    }

    private inner class AnnotationMoveGestureListener : OnMoveGestureListener {
        override fun onMoveBegin(detector: MoveGestureDetector): Boolean {
            return this@DraggableAnnotationController.onMoveBegin(detector)
        }

        override fun onMove(detector: MoveGestureDetector, distanceX: Float, distanceY: Float): Boolean {
            return this@DraggableAnnotationController.onMove(detector)
        }

        override fun onMoveEnd(detector: MoveGestureDetector, velocityX: Float, velocityY: Float) {
            this@DraggableAnnotationController.onMoveEnd()
        }
    }

    companion object {
        private var INSTANCE: DraggableAnnotationController? = null
        fun getInstance(mapView: MapView, maplibreMap: MapLibreMap): DraggableAnnotationController? {
            if (INSTANCE == null || INSTANCE!!.mapView !== mapView || INSTANCE!!.maplibreMap != maplibreMap) {
                INSTANCE = DraggableAnnotationController(mapView, maplibreMap)
            }
            return INSTANCE
        }

        private fun clearInstance() {
            if (INSTANCE != null) {
                INSTANCE!!.mapView = null
                INSTANCE!!.maplibreMap = null
                INSTANCE = null
            }
        }
    }
}