package org.maplibre.android.annotations;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.graphics.PointF;

import com.mapbox.android.gestures.AndroidGesturesManager;
import com.mapbox.android.gestures.MoveDistancesObject;
import com.mapbox.android.gestures.MoveGestureDetector;
import com.mapbox.geojson.Geometry;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.maps.MapView;
import org.maplibre.android.maps.Projection;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.ArrayList;
import java.util.List;

@RunWith(RobolectricTestRunner.class)
public class DraggableAnnotationControllerTest {

  private static final int touchAreaMaxX = 100;
  private static final int touchAreaMaxY = 100;

  @Mock
  private MapView mapView;

  @Mock
  private MapLibreMap maplibreMap;

  @Mock
  private Projection projection;

  @Mock
  private AndroidGesturesManager androidGesturesManager;

  @Mock
  private MoveGestureDetector moveGestureDetector;

  @Mock
  private MoveDistancesObject moveObject;

  @Mock
  private AnnotationManager annotationManager;

  @Mock
  private AbstractAnnotation annotation;

  @Mock
  private Geometry geometry;

  @Mock
  private OnAnnotationDragListener dragListener;

  private List<OnAnnotationDragListener> dragListenerList;

  private DraggableAnnotationController draggableAnnotationController;

  @Before
  public void before() {
    MockitoAnnotations.initMocks(this);
    draggableAnnotationController = new DraggableAnnotationController(mapView, maplibreMap, androidGesturesManager,
      0, 0, touchAreaMaxX, touchAreaMaxY);
    draggableAnnotationController.addAnnotationManager(annotationManager);
    dragListenerList = new ArrayList<>();
    dragListenerList.add(dragListener);
  }

  @Test
  public void annotationNotDraggableTest() {
    when(annotation.isDraggable()).thenReturn(false);
    draggableAnnotationController.startDragging(annotation, annotationManager);
    verify(dragListener, times(0)).onAnnotationDragStarted(annotation);
  }

  @Test
  public void annotationDragStartOnAdditionalManagerTest() {
    AnnotationManager additionalAnnotationManager = mock(AnnotationManager.class);
    draggableAnnotationController.addAnnotationManager(additionalAnnotationManager);

    when(annotation.isDraggable()).thenReturn(true);
    when(additionalAnnotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, additionalAnnotationManager);
    verify(dragListener, times(1)).onAnnotationDragStarted(annotation);
    draggableAnnotationController.stopDragging(annotation, additionalAnnotationManager);
    verify(dragListener, times(1)).onAnnotationDragFinished(annotation);

    draggableAnnotationController.removeAnnotationManager(additionalAnnotationManager);
  }

  @Test
  public void annotationDragStartTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);
    verify(dragListener, times(1)).onAnnotationDragStarted(annotation);
  }

  @Test
  public void annotationDragStopNoneTest() {
    draggableAnnotationController.stopDragging(null, null);
    verify(dragListener, times(0)).onAnnotationDragFinished(annotation);
  }

  @Test
  public void annotationDragStopTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.stopDragging(annotation, annotationManager);
    verify(dragListener, times(1)).onAnnotationDragFinished(annotation);
  }

  @Test
  public void annotationDragStopOnDeleteTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);
    draggableAnnotationController.onAnnotationDeleted(annotation);
    verify(dragListener, times(1)).onAnnotationDragFinished(annotation);
  }

  @Test
  public void gestureOnMoveBeginWrongPointersTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);

    PointF pointF = new PointF();
    when(annotationManager.queryMapForFeatures(pointF)).thenReturn(annotation);
    when(moveGestureDetector.getFocalPoint()).thenReturn(pointF);
    when(moveGestureDetector.getPointersCount()).thenReturn(0);

    boolean moveBegan1 = draggableAnnotationController.onMoveBegin(moveGestureDetector);
    assertFalse(moveBegan1);

    when(moveGestureDetector.getPointersCount()).thenReturn(2);
    boolean moveBegan2 = draggableAnnotationController.onMoveBegin(moveGestureDetector);
    assertFalse(moveBegan2);
    verify(dragListener, times(0)).onAnnotationDragStarted(annotation);
  }

  @Test
  public void gestureOnMoveBeginTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);

    PointF pointF = new PointF();
    when(annotationManager.queryMapForFeatures(pointF)).thenReturn(annotation);
    when(moveGestureDetector.getFocalPoint()).thenReturn(pointF);
    when(moveGestureDetector.getPointersCount()).thenReturn(1);

    boolean moveBegan = draggableAnnotationController.onMoveBegin(moveGestureDetector);
    assertTrue(moveBegan);
    verify(dragListener, times(1)).onAnnotationDragStarted(annotation);
  }

  @Test
  public void gestureOnMoveMoveWrongPointersTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    when(moveGestureDetector.getPointersCount()).thenReturn(2);

    boolean moved = draggableAnnotationController.onMove(moveGestureDetector);
    assertTrue(moved);
    verify(dragListener, times(0)).onAnnotationDrag(annotation);
    verify(dragListener, times(1)).onAnnotationDragFinished(annotation);
  }

  @Test
  public void gestureOnMoveBeginNonDraggableAnnotationTest() {
    when(annotation.isDraggable()).thenReturn(false);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);

    PointF pointF = new PointF();
    when(annotationManager.queryMapForFeatures(pointF)).thenReturn(annotation);
    when(moveGestureDetector.getFocalPoint()).thenReturn(pointF);
    when(moveGestureDetector.getPointersCount()).thenReturn(1);

    boolean moveBegan = draggableAnnotationController.onMoveBegin(moveGestureDetector);
    assertFalse(moveBegan);
    verify(dragListener, times(0)).onAnnotationDragStarted(annotation);
  }

  @Test
  public void gestureOnMoveMoveOutOfBoundsTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    when(moveGestureDetector.getPointersCount()).thenReturn(1);
    when(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject);

    when(moveObject.getCurrentX()).thenReturn(touchAreaMaxX + 1f);
    when(moveObject.getCurrentY()).thenReturn(10f);
    draggableAnnotationController.onMove(moveGestureDetector);

    when(moveObject.getCurrentX()).thenReturn(10f);
    when(moveObject.getCurrentY()).thenReturn(touchAreaMaxY + 1f);
    draggableAnnotationController.startDragging(annotation, annotationManager);
    draggableAnnotationController.onMove(moveGestureDetector);

    when(moveObject.getCurrentX()).thenReturn(-1f);
    when(moveObject.getCurrentY()).thenReturn(10f);
    draggableAnnotationController.startDragging(annotation, annotationManager);
    draggableAnnotationController.onMove(moveGestureDetector);

    when(moveObject.getCurrentX()).thenReturn(10f);
    when(moveObject.getCurrentY()).thenReturn(-1f);
    draggableAnnotationController.startDragging(annotation, annotationManager);
    draggableAnnotationController.onMove(moveGestureDetector);

    verify(dragListener, times(0)).onAnnotationDrag(annotation);
    verify(dragListener, times(4)).onAnnotationDragFinished(annotation);
  }

  @Test
  public void gestureOnMoveMoveNoGeometryTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    when(moveGestureDetector.getPointersCount()).thenReturn(1);
    when(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject);

    when(moveObject.getCurrentX()).thenReturn(10f);
    when(moveObject.getCurrentY()).thenReturn(10f);

    when(maplibreMap.getProjection()).thenReturn(projection);
    when(annotation.getOffsetGeometry(projection, moveObject, 0, 0)).thenReturn(null);

    boolean moved = draggableAnnotationController.onMove(moveGestureDetector);

    assertFalse(moved);
    verify(dragListener, times(0)).onAnnotationDrag(annotation);
  }

  @Test
  public void gestureOnMoveTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    when(moveGestureDetector.getPointersCount()).thenReturn(1);
    when(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject);

    when(moveObject.getCurrentX()).thenReturn(10f);
    when(moveObject.getCurrentY()).thenReturn(10f);

    when(maplibreMap.getProjection()).thenReturn(projection);
    when(annotation.getOffsetGeometry(projection, moveObject, 0, 0)).thenReturn(geometry);

    boolean moved = draggableAnnotationController.onMove(moveGestureDetector);

    assertTrue(moved);
    verify(annotation).setGeometry(geometry);
    verify(annotationManager).updateSource();
    verify(dragListener, times(1)).onAnnotationDrag(annotation);
  }

  @Test
  public void gestureOnMoveEndTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    draggableAnnotationController.onMoveEnd();
    verify(dragListener, times(1)).onAnnotationDragFinished(annotation);
  }

  @Test
  public void startedNotDraggableTest() {
    when(annotation.isDraggable()).thenReturn(false);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    when(moveGestureDetector.getPointersCount()).thenReturn(1);
    when(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject);

    when(moveObject.getCurrentX()).thenReturn(10f);
    when(moveObject.getCurrentY()).thenReturn(10f);

    when(maplibreMap.getProjection()).thenReturn(projection);
    when(annotation.getOffsetGeometry(projection, moveObject, 0, 0)).thenReturn(geometry);

    boolean moved = draggableAnnotationController.onMove(moveGestureDetector);

    assertFalse(moved);
    verify(dragListener, times(0)).onAnnotationDrag(annotation);
  }

  @Test
  public void moveNotDraggableTest() {
    when(annotation.isDraggable()).thenReturn(true);
    when(annotationManager.getDragListeners()).thenReturn(dragListenerList);
    draggableAnnotationController.startDragging(annotation, annotationManager);

    when(moveGestureDetector.getPointersCount()).thenReturn(1);
    when(moveGestureDetector.getMoveObject(0)).thenReturn(moveObject);

    when(moveObject.getCurrentX()).thenReturn(10f);
    when(moveObject.getCurrentY()).thenReturn(10f);

    when(maplibreMap.getProjection()).thenReturn(projection);
    when(annotation.getOffsetGeometry(projection, moveObject, 0, 0)).thenReturn(geometry);

    when(annotation.isDraggable()).thenReturn(false);
    boolean moved = draggableAnnotationController.onMove(moveGestureDetector);

    assertTrue(moved);
    verify(dragListener, times(0)).onAnnotationDrag(annotation);
  }
}
