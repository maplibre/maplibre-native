package org.maplibre.android.annotations;

import android.annotation.SuppressLint;
import android.graphics.PointF;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;
import androidx.annotation.VisibleForTesting;

import com.mapbox.android.gestures.AndroidGesturesManager;
import com.mapbox.android.gestures.MoveDistancesObject;
import com.mapbox.android.gestures.MoveGestureDetector;
import com.mapbox.geojson.Geometry;

import org.maplibre.android.maps.MapView;
import org.maplibre.android.maps.MapLibreMap;

import java.util.ArrayList;
import java.util.List;

@UiThread
final class DraggableAnnotationController {

  private static DraggableAnnotationController INSTANCE = null;

  public static DraggableAnnotationController getInstance(MapView mapView, MapLibreMap mapboxMap) {
    if (INSTANCE == null || INSTANCE.mapView != mapView || INSTANCE.maplibreMap != mapboxMap) {
      INSTANCE = new DraggableAnnotationController(mapView, mapboxMap);
    }
    return INSTANCE;
  }

  private static void clearInstance() {
    if (INSTANCE != null) {
      INSTANCE.mapView = null;
      INSTANCE.maplibreMap = null;
      INSTANCE = null;
    }
  }

  private MapView mapView;
  private MapLibreMap maplibreMap;
  private List<AnnotationManager> annotationManagers = new ArrayList<>();

  private final int touchAreaShiftX;
  private final int touchAreaShiftY;
  private final int touchAreaMaxX;
  private final int touchAreaMaxY;

  @Nullable
  private AbstractAnnotation draggedAnnotation;
  @Nullable
  private AnnotationManager draggedAnnotationManager;

  @SuppressLint("ClickableViewAccessibility")
  DraggableAnnotationController(MapView mapView, MapLibreMap maplibreMap) {
    this(mapView, maplibreMap, new AndroidGesturesManager(mapView.getContext(), false),
      mapView.getScrollX(), mapView.getScrollY(), mapView.getMeasuredWidth(), mapView.getMeasuredHeight());
  }

  @VisibleForTesting
  public DraggableAnnotationController(MapView mapView, MapLibreMap maplibreMap,
                                       final AndroidGesturesManager androidGesturesManager,
                                       int touchAreaShiftX, int touchAreaShiftY,
                                       int touchAreaMaxX, int touchAreaMaxY) {
    this.mapView = mapView;
    this.maplibreMap = maplibreMap;
    this.touchAreaShiftX = touchAreaShiftX;
    this.touchAreaShiftY = touchAreaShiftY;
    this.touchAreaMaxX = touchAreaMaxX;
    this.touchAreaMaxY = touchAreaMaxY;

    androidGesturesManager.setMoveGestureListener(new AnnotationMoveGestureListener());

    mapView.setOnTouchListener(new View.OnTouchListener() {
      @Override
      public boolean onTouch(View v, MotionEvent event) {
        // Using active gesture manager
        AbstractAnnotation oldAnnotation = draggedAnnotation;
        androidGesturesManager.onTouchEvent(event);
        // if drag is started or drag is finished, don't pass motion events further
        return draggedAnnotation != null || oldAnnotation != null;
      }
    });
  }

  void addAnnotationManager(AnnotationManager annotationManager) {
    this.annotationManagers.add(annotationManager);
  }

  void removeAnnotationManager(AnnotationManager annotationManager) {
    this.annotationManagers.remove(annotationManager);
    if (annotationManagers.isEmpty()) {
      clearInstance();
    }
  }

  void onAnnotationDeleted(AbstractAnnotation annotation) {
    if (annotation == draggedAnnotation) {
      stopDragging(draggedAnnotation, draggedAnnotationManager);
    }
  }

  boolean onMoveBegin(MoveGestureDetector detector) {
    for (AnnotationManager annotationManager : annotationManagers) {
      if (detector.getPointersCount() == 1) {
        AbstractAnnotation annotation = annotationManager.queryMapForFeatures(detector.getFocalPoint());
        if (annotation != null && startDragging(annotation, annotationManager)) {
          return true;
        }
      }
    }
    return false;
  }

  boolean onMove(MoveGestureDetector detector) {
    if (draggedAnnotation != null && (detector.getPointersCount() > 1 || !draggedAnnotation.isDraggable())) {
      // Stopping the drag when we don't work with a simple, on-pointer move anymore
      stopDragging(draggedAnnotation, draggedAnnotationManager);
      return true;
    }

    // Updating symbol's position
    if (draggedAnnotation != null) {
      MoveDistancesObject moveObject = detector.getMoveObject(0);

      float x = moveObject.getCurrentX() - touchAreaShiftX;
      float y = moveObject.getCurrentY() - touchAreaShiftY;

      PointF pointF = new PointF(x, y);

      if (pointF.x < 0 || pointF.y < 0 || pointF.x > touchAreaMaxX || pointF.y > touchAreaMaxY) {
        stopDragging(draggedAnnotation, draggedAnnotationManager);
        return true;
      }

      Geometry shiftedGeometry = draggedAnnotation.getOffsetGeometry(
        maplibreMap.getProjection(), moveObject, touchAreaShiftX, touchAreaShiftY
      );

      if (shiftedGeometry != null) {
        draggedAnnotation.setGeometry(
          shiftedGeometry
        );
        draggedAnnotationManager.updateSource();
        for (
          OnAnnotationDragListener d : (List<OnAnnotationDragListener>) draggedAnnotationManager.getDragListeners()
        ) {
          d.onAnnotationDrag(draggedAnnotation);
        }
        return true;
      }
    }

    return false;
  }

  void onMoveEnd() {
    // Stopping the drag when move ends
    stopDragging(draggedAnnotation, draggedAnnotationManager);
  }

  boolean startDragging(@NonNull AbstractAnnotation annotation, @NonNull AnnotationManager annotationManager) {
    if (annotation.isDraggable()) {
      for (OnAnnotationDragListener d : (List<OnAnnotationDragListener>) annotationManager.getDragListeners()) {
        d.onAnnotationDragStarted(annotation);
      }
      draggedAnnotation = annotation;
      draggedAnnotationManager = annotationManager;
      return true;
    }
    return false;
  }

  void stopDragging(@Nullable AbstractAnnotation annotation, @Nullable AnnotationManager annotationManager) {
    if (annotation != null && annotationManager != null) {
      for (OnAnnotationDragListener d : (List<OnAnnotationDragListener>) annotationManager.getDragListeners()) {
        d.onAnnotationDragFinished(annotation);
      }
    }
    draggedAnnotation = null;
    draggedAnnotationManager = null;
  }

  private class AnnotationMoveGestureListener implements MoveGestureDetector.OnMoveGestureListener {

    @Override
    public boolean onMoveBegin(MoveGestureDetector detector) {
      return DraggableAnnotationController.this.onMoveBegin(detector);
    }

    @Override
    public boolean onMove(MoveGestureDetector detector, float distanceX, float distanceY) {
      return DraggableAnnotationController.this.onMove(detector);
    }

    @Override
    public void onMoveEnd(MoveGestureDetector detector, float velocityX, float velocityY) {
      DraggableAnnotationController.this.onMoveEnd();
    }
  }
}
