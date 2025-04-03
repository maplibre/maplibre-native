package org.maplibre.android.maps;


import static org.maplibre.android.maps.MapLibreMap.OnCameraMoveStartedListener.REASON_API_GESTURE;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.PointF;
import android.graphics.Rect;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.animation.DecelerateInterpolator;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.android.constants.MapLibreConstants;
import org.maplibre.android.gestures.AndroidGesturesManager;
import org.maplibre.android.gestures.MoveGestureDetector;
import org.maplibre.android.gestures.StandardScaleGestureDetector;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class MapSurfaceGestureDetector {

    private static final long DOUBLE_CLICK_INTERVAL = 1000;

    private static final long TOUCH_EVENT_END_DURATION = 300;

    /**
     * define like the GrabMapConstants.VELOCITY_THRESHOLD_IGNORE_FLING_FOR_AUTO
     */
    private static final long VELOCITY_THRESHOLD_IGNORE_FLING_FOR_AUTO = 100;

    private final TransformAuto transform;
    private final Projection projection;
    private final UiSettings uiSettings;
    private final CameraChangeDispatcher cameraChangeDispatcher;

    // new map touch API
    private final CopyOnWriteArrayList<MapLibreMap.OnMapClickListener> onMapClickListenerList
            = new CopyOnWriteArrayList<>();

    private final CopyOnWriteArrayList<MapLibreMap.OnMapLongClickListener> onMapLongClickListenerList
            = new CopyOnWriteArrayList<>();

    private final CopyOnWriteArrayList<MapLibreMap.OnFlingListener> onFlingListenerList
            = new CopyOnWriteArrayList<>();

    private final CopyOnWriteArrayList<MapLibreMap.OnMoveListener> onMoveListenerList
            = new CopyOnWriteArrayList<>();

    private final CopyOnWriteArrayList<MapLibreMap.OnScaleListener> onScaleListenerList
            = new CopyOnWriteArrayList<>();


    private final StandardScaleGestureDetector standardScaleGestureDetector;
    private final MoveGestureDetector moveGestureDetector;

    /**
     * User-set focal point.
     */
    @Nullable
    private PointF focalPoint;

    @Nullable
    private Rect visibleArea;

    private Animator scaleAnimator;
    private final List<Animator> scheduledAnimators = new ArrayList<>();

    private long lastClickTime = 0;

    private boolean isScrolling = false;
    private final Runnable scrollEndRunnable = () -> {
        isScrolling = false;
        dispatchCameraIdle();
        notifyOnMoveEndListeners();
    };

    private boolean isScaling = false;
    private final Runnable scaleEndRunnable = () -> {
        isScaling = false;
        dispatchCameraIdle();
        notifyOnScaleEndListeners();
    };

    @NonNull
    private final Handler animationsTimeoutHandler = new Handler();

    MapSurfaceGestureDetector(@NonNull Context context,
                              TransformAuto transform,
                              Projection projection,
                              UiSettings uiSettings,
                              CameraChangeDispatcher cameraChangeDispatcher) {
        this.transform = transform;
        this.projection = projection;
        this.uiSettings = uiSettings;
        this.cameraChangeDispatcher = cameraChangeDispatcher;

        AndroidGesturesManager androidGesturesManager = new AndroidGesturesManager(context);
        standardScaleGestureDetector = new StandardScaleGestureDetector(context, androidGesturesManager);
        moveGestureDetector = new MoveGestureDetector(context, androidGesturesManager);
    }

    public void onVisibleAreaChanged(@NonNull Rect visibleArea) {
        this.visibleArea = visibleArea;
    }

    void onScroll(float distanceX, float distanceY) {
        if (!uiSettings.isScrollGesturesEnabled()) {
            return;
        }

        //start scrolling
        if (!isScrolling) {
            isScrolling = true;
            cancelTransitionsIfRequired();
            notifyOnMoveBeginListeners();
        }

        //scrolling
        if (distanceX != 0 || distanceY != 0) {
            // dispatching camera start event only when the movement actually occurred
            cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE);

            // Scroll the map
            transform.moveBy(-distanceX, -distanceY, 0 /*no duration*/);

            notifyOnMoveListeners();
        }

        //end scroll
        animationsTimeoutHandler.removeCallbacks(scrollEndRunnable);
        animationsTimeoutHandler.postDelayed(scrollEndRunnable, TOUCH_EVENT_END_DURATION);
    }

    void onFling(float velocityX, float velocityY) {

        notifyOnFlingListeners();

        if (!uiSettings.isFlingVelocityAnimationEnabled()) {
            return;
        }

        float screenDensity = uiSettings.getPixelRatio();

        // calculate velocity vector for xy dimensions, independent from screen size
        double velocityXY = Math.hypot(velocityX / screenDensity, velocityY / screenDensity);
        if (velocityXY < VELOCITY_THRESHOLD_IGNORE_FLING_FOR_AUTO) {
            // ignore short flings, these can occur when other gestures just have finished executing
            return;
        }

        transform.cancelTransitions();
        cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE);

        // tilt results in a bigger translation, limiting input for #5281
        double tilt = transform.getTilt();
        double tiltFactor = 1.5 + ((tilt != 0) ? (tilt / 10) : 0);
        double offsetX = velocityX / tiltFactor / screenDensity;
        double offsetY = velocityY / tiltFactor / screenDensity;

        // calculate animation time based on displacement
        long animationTime = (long) (velocityXY / 7 / tiltFactor + MapLibreConstants.ANIMATION_DURATION_FLING_BASE);

        // update transformation
        transform.moveBy(offsetX, offsetY, animationTime);
    }

    void onScale(float focusX, float focusY, float scaleFactor) {
        if (!uiSettings.isZoomGesturesEnabled()) {
            return;
        }

        float x = focusX;
        float y = focusY;

        Rect visibleArea2 = visibleArea;
        if (visibleArea2 != null) {
            // If a focal point value is negative, use the center point of the visible area.
            if (x < 0) {
                x = visibleArea2.centerX();
            }
            if (y < 0) {
                y = visibleArea2.centerY();
            }
        }

        //Start scaling
        if (!isScaling) {
            isScaling = true;
            cancelTransitionsIfRequired();
            notifyOnScaleBeginListeners();
        }

        //Scaling
        cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE);
        double zoomBy = getNewZoom(scaleFactor);
        transform.zoomBy(zoomBy, new PointF(x, y));
        notifyOnScaleListeners();

        //end scroll
        animationsTimeoutHandler.removeCallbacks(scaleEndRunnable);
        animationsTimeoutHandler.postDelayed(scaleEndRunnable, TOUCH_EVENT_END_DURATION);

    }

    private double getNewZoom(float scaleFactor) {
        return (scaleFactor - 1.0);
    }


    void onClick(float x, float y) {
        long currentClickTime = System.currentTimeMillis();
        if (currentClickTime - lastClickTime <= DOUBLE_CLICK_INTERVAL) {
            lastClickTime = 0;//reset

            if (!uiSettings.isZoomGesturesEnabled() || !uiSettings.isDoubleTapGesturesEnabled()) {
                return;
            }

            PointF zoomFocalPoint;
            // Single finger double tap
            if (focalPoint != null) {
                // User provided focal point
                zoomFocalPoint = focalPoint;
            } else {
                // Zoom in on gesture
                zoomFocalPoint = new PointF(x, y);
            }

            zoomInAnimated(zoomFocalPoint, false);
        } else {
            lastClickTime = currentClickTime;
        }
    }

    /**
     * Set the gesture focal point.
     * <p>
     * this is the center point used for calculate transformations from gestures, value is
     * overridden if end user provides his own through {@link UiSettings#setFocalPoint(PointF)}.
     * </p>
     *
     * @param focalPoint the center point for gestures
     */
    void setFocalPoint(@Nullable PointF focalPoint) {
        if (focalPoint == null) {
            // resetting focal point,
            if (uiSettings.getFocalPoint() != null) {
                // using user provided one to reset
                focalPoint = uiSettings.getFocalPoint();
            }
        }
        this.focalPoint = focalPoint;
    }

    /**
     * Get the current active gesture focal point.
     * <p>
     * This could be either the user provided focal point in
     * {@link UiSettings#setFocalPoint(PointF)}or <code>null</code>.
     * If it's <code>null</code>, gestures will use focal pointed returned by the detector.
     * </p>
     *
     * @return the current active gesture focal point.
     */
    @Nullable
    PointF getFocalPoint() {
        return focalPoint;
    }


    void cancelAnimators() {
        animationsTimeoutHandler.removeCallbacksAndMessages(null);
        scheduledAnimators.clear();

        cancelAnimator(scaleAnimator);

        dispatchCameraIdle();
    }

    private void cancelAnimator(@Nullable Animator animator) {
        if (animator != null && animator.isStarted()) {
            animator.cancel();
        }
    }

    /**
     * Posted on main thread with {@link #animationsTimeoutHandler}. Cancels all scheduled animators if needed.
     */
    @NonNull
    private final Runnable cancelAnimatorsRunnable = new Runnable() {
        @Override
        public void run() {
            cancelAnimators();
        }
    };

    /**
     * Schedules a velocity animator to be executed when user lift fingers,
     * unless canceled by the {@link #cancelAnimatorsRunnable}.
     *
     * @param animator animator ot be scheduled
     */
    private void scheduleAnimator(Animator animator) {
        scheduledAnimators.add(animator);
        animationsTimeoutHandler.removeCallbacksAndMessages(null);
        animationsTimeoutHandler.postDelayed(cancelAnimatorsRunnable, MapLibreConstants.SCHEDULED_ANIMATION_TIMEOUT);
    }

    private Animator createScaleAnimator(double currentZoom, double zoomAddition,
                                         @NonNull final PointF animationFocalPoint, long animationTime) {
        ValueAnimator animator = ValueAnimator.ofFloat((float) currentZoom, (float) (currentZoom + zoomAddition));
        animator.setDuration(animationTime);
        animator.setInterpolator(new DecelerateInterpolator());
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {

            @Override
            public void onAnimationUpdate(@NonNull ValueAnimator animation) {
                transform.setZoom((Float) animation.getAnimatedValue(), animationFocalPoint);
            }
        });

        animator.addListener(new AnimatorListenerAdapter() {

            @Override
            public void onAnimationStart(Animator animation) {
                transform.cancelTransitions();
                cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE);
            }

            @Override
            public void onAnimationCancel(Animator animation) {
                transform.cancelTransitions();
            }

            @Override
            public void onAnimationEnd(Animator animation) {
                dispatchCameraIdle();
            }
        });
        return animator;
    }

    /**
     * Zoom in by 1.
     *
     * @param zoomFocalPoint focal point of zoom animation
     * @param runImmediately if true, animation will be started right away, otherwise it will wait until
     *                       {@link MotionEvent#ACTION_UP} is registered.
     */
    void zoomInAnimated(@NonNull PointF zoomFocalPoint, boolean runImmediately) {
        zoomAnimated(true, zoomFocalPoint, runImmediately);
    }

    /**
     * Zoom out by 1.
     *
     * @param zoomFocalPoint focal point of zoom animation
     * @param runImmediately if true, animation will be started right away, otherwise it will wait until
     *                       {@link MotionEvent#ACTION_UP} is registered.
     */
    void zoomOutAnimated(@NonNull PointF zoomFocalPoint, boolean runImmediately) {
        zoomAnimated(false, zoomFocalPoint, runImmediately);
    }

    private void zoomAnimated(boolean zoomIn, @NonNull PointF zoomFocalPoint, boolean runImmediately) {
        //canceling here as well, because when using a button it will not be canceled automatically by onDown()
        cancelAnimator(scaleAnimator);

        double currentZoom = transform.getRawZoom();
        scaleAnimator = createScaleAnimator(
                currentZoom,
                zoomIn ? 1 : -1,
                zoomFocalPoint,
                MapLibreConstants.ANIMATION_DURATION);
        if (runImmediately) {
            scaleAnimator.start();
        } else {
            scheduleAnimator(scaleAnimator);
        }
    }

    private void dispatchCameraIdle() {
        // invalidate the camera position, so that it's valid when fetched from the #onIdle event
        // and doesn't rely on the last frame being rendered
        transform.invalidateCameraPosition();
        cameraChangeDispatcher.onCameraIdle();

    }

    private void cancelTransitionsIfRequired() {
        // we need to cancel core transitions only if there is no started gesture yet
        transform.cancelTransitions();
    }

    private boolean isZoomValid(double mapZoom) {
        return mapZoom >= MapLibreConstants.MINIMUM_ZOOM && mapZoom <= MapLibreConstants.MAXIMUM_ZOOM;
    }

    void notifyOnMapClickListeners(@NonNull PointF tapPoint) {
        for (MapLibreMap.OnMapClickListener listener : onMapClickListenerList) {
            if (listener.onMapClick(projection.fromScreenLocation(tapPoint))) {
                return;
            }
        }
    }

    void notifyOnMapLongClickListeners(@NonNull PointF longClickPoint) {
        for (MapLibreMap.OnMapLongClickListener listener : onMapLongClickListenerList) {
            if (listener.onMapLongClick(projection.fromScreenLocation(longClickPoint))) {
                return;
            }
        }
    }

    void notifyOnFlingListeners() {
        for (MapLibreMap.OnFlingListener listener : onFlingListenerList) {
            listener.onFling();
        }
    }

    void notifyOnMoveBeginListeners() {
        for (MapLibreMap.OnMoveListener listener : onMoveListenerList) {
            listener.onMoveBegin(moveGestureDetector);
        }
    }

    void notifyOnMoveListeners() {
        for (MapLibreMap.OnMoveListener listener : onMoveListenerList) {
            listener.onMove(moveGestureDetector);
        }
    }

    void notifyOnMoveEndListeners() {
        for (MapLibreMap.OnMoveListener listener : onMoveListenerList) {
            listener.onMoveEnd(moveGestureDetector);
        }
    }

    void notifyOnScaleBeginListeners() {
        for (MapLibreMap.OnScaleListener listener : onScaleListenerList) {
            listener.onScaleBegin(standardScaleGestureDetector);
        }
    }

    void notifyOnScaleListeners() {
        for (MapLibreMap.OnScaleListener listener : onScaleListenerList) {
            listener.onScale(standardScaleGestureDetector);
        }
    }

    void notifyOnScaleEndListeners() {
        for (MapLibreMap.OnScaleListener listener : onScaleListenerList) {
            listener.onScaleEnd(standardScaleGestureDetector);
        }
    }

    void addOnMapClickListener(MapLibreMap.OnMapClickListener onMapClickListener) {
        onMapClickListenerList.add(onMapClickListener);
    }

    void removeOnMapClickListener(MapLibreMap.OnMapClickListener onMapClickListener) {
        onMapClickListenerList.remove(onMapClickListener);
    }

    void addOnMapLongClickListener(MapLibreMap.OnMapLongClickListener onMapLongClickListener) {
        onMapLongClickListenerList.add(onMapLongClickListener);
    }

    void removeOnMapLongClickListener(MapLibreMap.OnMapLongClickListener onMapLongClickListener) {
        onMapLongClickListenerList.remove(onMapLongClickListener);
    }

    void addOnFlingListener(MapLibreMap.OnFlingListener onFlingListener) {
        onFlingListenerList.add(onFlingListener);
    }

    void removeOnFlingListener(MapLibreMap.OnFlingListener onFlingListener) {
        onFlingListenerList.remove(onFlingListener);
    }

    void addOnMoveListener(MapLibreMap.OnMoveListener listener) {
        onMoveListenerList.add(listener);
    }

    void removeOnMoveListener(MapLibreMap.OnMoveListener listener) {
        onMoveListenerList.remove(listener);
    }

    void addOnScaleListener(MapLibreMap.OnScaleListener listener) {
        onScaleListenerList.add(listener);
    }

    void removeOnScaleListener(MapLibreMap.OnScaleListener listener) {
        onScaleListenerList.remove(listener);
    }
}
