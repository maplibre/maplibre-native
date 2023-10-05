package org.maplibre.android.annotations

import android.graphics.PointF
import com.mapbox.android.gestures.AndroidGesturesManager
import com.mapbox.android.gestures.MoveDistancesObject
import com.mapbox.android.gestures.MoveGestureDetector
import com.mapbox.geojson.Geometry
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Projection
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class DraggableAnnotationControllerTest {
    @Mock
    private lateinit var mapView: MapView

    @Mock
    private lateinit var maplibreMap: MapLibreMap

    @Mock
    private lateinit var projection: Projection

    @Mock
    private lateinit var androidGesturesManager: AndroidGesturesManager

    @Mock
    private lateinit var moveGestureDetector: MoveGestureDetector

    @Mock
    private lateinit var moveObject: MoveDistancesObject

    @Mock
    private lateinit var annotationManager: AnnotationManager<*, Symbol>

    @Mock
    private lateinit var annotation: Symbol

    private lateinit var dragPair: DraggableAnnotationController.DragPair<*>

    @Mock
    private lateinit var dragListener: OnAnnotationDragListener<Symbol>
    private lateinit var dragListenerList: List<OnAnnotationDragListener<Symbol>>
    private lateinit var draggableAnnotationController: DraggableAnnotationController
    @Before
    fun before() {
        MockitoAnnotations.initMocks(this)
        dragPair = DraggableAnnotationController.DragPair(annotation, annotationManager)
        draggableAnnotationController = DraggableAnnotationController(
            mapView, maplibreMap, androidGesturesManager,
            0, 0, touchAreaMaxX, touchAreaMaxY
        )
        draggableAnnotationController.addAnnotationManager(annotationManager)
        dragListenerList = listOf(dragListener)
    }

    @Test
    fun annotationNotDraggableTest() {
        Mockito.`when`(annotation.draggable).thenReturn(false)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDragStarted(annotation)
    }

    @Test
    fun annotationDragStartOnAdditionalManagerTest() {
        val additionalAnnotationManager = Mockito.mock(
            AnnotationManager::class.java
        )
        draggableAnnotationController.addAnnotationManager(additionalAnnotationManager)
        Mockito.`when`(annotation.draggable).thenReturn(true)
        // Additional annotation manager is ignored (no way of knowing whether it handles same annotation type)
        Mockito.`when`(additionalAnnotationManager.getDragListeners()).thenReturn(dragListenerList)

        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragStarted(annotation)
        draggableAnnotationController.stopDragging(dragPair)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragFinished(annotation)
        draggableAnnotationController.removeAnnotationManager(additionalAnnotationManager)
    }

    @Test
    fun annotationDragStartTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragStarted(annotation)
    }

    @Test
    fun annotationDragStopNoneTest() {
        draggableAnnotationController.stopDragging<Symbol>(null)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDragFinished(annotation)
    }

    @Test
    fun annotationDragStopTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.stopDragging(dragPair)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragFinished(annotation)
    }

    @Test
    fun annotationDragStopOnDeleteTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        draggableAnnotationController.onAnnotationDeleted(annotation)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragFinished(annotation)
    }

    @Test
    fun gestureOnMoveBeginWrongPointersTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        val pointF = PointF()
        Mockito.`when`(annotationManager.queryMapForFeatures(pointF)).thenReturn(annotation)
        Mockito.`when`(moveGestureDetector.focalPoint).thenReturn(pointF)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(0)
        val moveBegan1 = draggableAnnotationController.onMoveBegin(moveGestureDetector)
        Assert.assertFalse(moveBegan1)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(2)
        val moveBegan2 = draggableAnnotationController.onMoveBegin(moveGestureDetector)
        Assert.assertFalse(moveBegan2)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDragStarted(annotation)
    }

    @Test
    fun gestureOnMoveBeginTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        val pointF = PointF()
        Mockito.`when`(annotationManager.queryMapForFeatures(pointF)).thenReturn(annotation)
        Mockito.`when`(moveGestureDetector.focalPoint).thenReturn(pointF)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        val moveBegan = draggableAnnotationController.onMoveBegin(moveGestureDetector)
        Assert.assertTrue(moveBegan)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragStarted(annotation)
    }

    @Test
    fun gestureOnMoveMoveWrongPointersTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(2)
        val moved = draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Assert.assertTrue(moved)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDrag(annotation)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragFinished(annotation)
    }

    @Test
    fun gestureOnMoveBeginNonDraggableAnnotationTest() {
        Mockito.`when`(annotation.draggable).thenReturn(false)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        val pointF = PointF()
        Mockito.`when`(annotationManager.queryMapForFeatures(pointF)).thenReturn(annotation)
        Mockito.`when`(moveGestureDetector.focalPoint).thenReturn(pointF)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        val moveBegan = draggableAnnotationController.onMoveBegin(moveGestureDetector)
        Assert.assertFalse(moveBegan)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDragStarted(annotation)
    }

    @Test
    fun gestureOnMoveMoveOutOfBoundsTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        Mockito.`when`(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject)
        Mockito.`when`(moveObject.currentX).thenReturn(touchAreaMaxX + 1f)
        Mockito.`when`(moveObject.currentY).thenReturn(10f)
        draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Mockito.`when`(moveObject.currentX).thenReturn(10f)
        Mockito.`when`(moveObject.currentY).thenReturn(touchAreaMaxY + 1f)
        draggableAnnotationController.startDragging(dragPair)
        draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Mockito.`when`(moveObject.currentX).thenReturn(-1f)
        Mockito.`when`(moveObject.currentY).thenReturn(10f)
        draggableAnnotationController.startDragging(dragPair)
        draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Mockito.`when`(moveObject.currentX).thenReturn(10f)
        Mockito.`when`(moveObject.currentY).thenReturn(-1f)
        draggableAnnotationController.startDragging(dragPair)
        draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDrag(annotation)
        Mockito.verify(dragListener, Mockito.times(4)).onAnnotationDragFinished(annotation)
    }

    @Test
    fun gestureOnMoveMoveNoGeometryTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        Mockito.`when`(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject)
        Mockito.`when`(moveObject.currentX).thenReturn(10f)
        Mockito.`when`(moveObject.currentY).thenReturn(10f)
        Mockito.`when`(maplibreMap.projection).thenReturn(projection)
        Mockito.`when`(annotation.offsetGeometry(projection, moveObject, 0f, 0f)).thenReturn(false)
        val moved = draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Assert.assertFalse(moved)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDrag(annotation)
    }

    @Test
    fun gestureOnMoveTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        Mockito.`when`(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject)
        Mockito.`when`(moveObject.currentX).thenReturn(10f)
        Mockito.`when`(moveObject.currentY).thenReturn(10f)
        Mockito.`when`(maplibreMap.projection).thenReturn(projection)
        Mockito.`when`(annotation.offsetGeometry(projection, moveObject, 0f, 0f)).thenReturn(true)
        val moved = draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Assert.assertTrue(moved)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDrag(annotation)
    }

    @Test
    fun gestureOnMoveEndTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        draggableAnnotationController.onMoveEnd(moveGestureDetector, 0f, 0f)
        Mockito.verify(dragListener, Mockito.times(1)).onAnnotationDragFinished(annotation)
    }

    @Test
    fun startedNotDraggableTest() {
        Mockito.`when`(annotation.draggable).thenReturn(false)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        Mockito.`when`(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject)
        Mockito.`when`(moveObject.currentX).thenReturn(10f)
        Mockito.`when`(moveObject.currentY).thenReturn(10f)
        Mockito.`when`(maplibreMap.projection).thenReturn(projection)
        Mockito.`when`(annotation.offsetGeometry(projection, moveObject, 0f, 0f)).thenReturn(true)
        val moved = draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Assert.assertFalse(moved)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDrag(annotation)
    }

    @Test
    fun moveNotDraggableTest() {
        Mockito.`when`(annotation.draggable).thenReturn(true)
        Mockito.`when`(annotationManager.getDragListeners()).thenReturn(dragListenerList)
        draggableAnnotationController.startDragging(dragPair)
        Mockito.`when`(moveGestureDetector.pointersCount).thenReturn(1)
        Mockito.`when`(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject)
        Mockito.`when`(moveObject.currentX).thenReturn(10f)
        Mockito.`when`(moveObject.currentY).thenReturn(10f)
        Mockito.`when`(maplibreMap.projection).thenReturn(projection)
        Mockito.`when`(annotation.offsetGeometry(projection, moveObject, 0f, 0f)).thenReturn(true)
        Mockito.`when`(annotation.draggable).thenReturn(false)
        val moved = draggableAnnotationController.onMove(moveGestureDetector, 0f, 0f)
        Assert.assertTrue(moved)
        Mockito.verify(dragListener, Mockito.times(0)).onAnnotationDrag(annotation)
    }

    companion object {
        private const val touchAreaMaxX = 100
        private const val touchAreaMaxY = 100
    }
}