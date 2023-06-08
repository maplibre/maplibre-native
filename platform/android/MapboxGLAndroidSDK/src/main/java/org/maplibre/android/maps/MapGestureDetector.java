package org.maplibre.android.maps;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.PointF;
import android.os.Handler;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.animation.DecelerateInterpolator;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.mapbox.android.gestures.AndroidGesturesManager;
import com.mapbox.android.gestures.MoveGestureDetector;
import com.mapbox.android.gestures.MultiFingerTapGestureDetector;
import com.mapbox.android.gestures.RotateGestureDetector;
import com.mapbox.android.gestures.ShoveGestureDetector;
import com.mapbox.android.gestures.StandardGestureDetector;
import com.mapbox.android.gestures.StandardScaleGestureDetector;
import org.maplibre.android.R;
import org.maplibre.android.constants.MapLibreConstants;
import org.maplibre.android.utils.MathUtils;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;

import static org.maplibre.android.constants.MapLibreConstants.MAXIMUM_ANGULAR_VELOCITY;
import static org.maplibre.android.constants.MapLibreConstants.MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE;
import static org.maplibre.android.constants.MapLibreConstants.QUICK_ZOOM_MAX_ZOOM_CHANGE;
import static org.maplibre.android.constants.MapLibreConstants.ROTATE_VELOCITY_RATIO_THRESHOLD;
import static org.maplibre.android.constants.MapLibreConstants.SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER;
import static org.maplibre.android.constants.MapLibreConstants.SCALE_VELOCITY_RATIO_THRESHOLD;
import static org.maplibre.android.constants.MapLibreConstants.ZOOM_RATE;
import static org.maplibre.android.maps.MapLibreMap.OnCameraMoveStartedListener.REASON_API_GESTURE;
import static org.maplibre.android.utils.MathUtils.normalize;

import timber.log.Timber;

/**
 * Manages gestures events on a MapView.
 */
final class MapGestureDetector {

  private final Transform transform;
  private final Projection projection;
  private final UiSettings uiSettings;
  private final AnnotationManager annotationManager;
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

  private final CopyOnWriteArrayList<MapLibreMap.OnRotateListener> onRotateListenerList
    = new CopyOnWriteArrayList<>();

  private final CopyOnWriteArrayList<MapLibreMap.OnScaleListener> onScaleListenerList
    = new CopyOnWriteArrayList<>();

  private final CopyOnWriteArrayList<MapLibreMap.OnShoveListener> onShoveListenerList
    = new CopyOnWriteArrayList<>();

  /**
   * User-set focal point.
   */
  @Nullable
  private PointF constantFocalPoint;

  @NonNull
  private PointF doubleTapFocalPoint = new PointF();

  private AndroidGesturesManager gesturesManager;

  private Animator scaleAnimator;
  private Animator rotateAnimator;
  private final List<Animator> scheduledAnimators = new ArrayList<>();

  /**
   * Cancels scheduled velocity animations if user doesn't lift fingers within
   * {@link MapLibreConstants#SCHEDULED_ANIMATION_TIMEOUT}
   */
  @NonNull
  private Handler animationsTimeoutHandler = new Handler();

  private boolean doubleTapRegistered;

  MapGestureDetector(@Nullable Context context, Transform transform, Projection projection, UiSettings uiSettings,
                     AnnotationManager annotationManager, CameraChangeDispatcher cameraChangeDispatcher) {
    this.annotationManager = annotationManager;
    this.transform = transform;
    this.projection = projection;
    this.uiSettings = uiSettings;
    this.cameraChangeDispatcher = cameraChangeDispatcher;

    // Checking for context != null for testing purposes
    if (context != null) {
      // Initialize gestures manager
      AndroidGesturesManager androidGesturesManager = new AndroidGesturesManager(context);
      initializeGesturesManager(androidGesturesManager, true);

      // Initialize gesture listeners
      initializeGestureListeners(context, true);
    }
  }

  private void initializeGestureListeners(@NonNull Context context, boolean attachDefaultListeners) {
    if (attachDefaultListeners) {
      StandardGestureListener standardGestureListener = new StandardGestureListener(
        context.getResources().getDimension(
          com.mapbox.android.gestures.R.dimen.mapbox_defaultScaleSpanSinceStartThreshold));
      MoveGestureListener moveGestureListener = new MoveGestureListener();
      ScaleGestureListener scaleGestureListener = new ScaleGestureListener(
        context.getResources().getDimension(R.dimen.maplibre_density_constant),
        context.getResources().getDimension(R.dimen.maplibre_minimum_scale_speed),
        context.getResources().getDimension(R.dimen.maplibre_minimum_angled_scale_speed),
        context.getResources().getDimension(R.dimen.maplibre_minimum_scale_velocity)
      );
      RotateGestureListener rotateGestureListener = new RotateGestureListener(
        context.getResources().getDimension(R.dimen.maplibre_minimum_scale_span_when_rotating),
        context.getResources().getDimension(R.dimen.maplibre_density_constant),
        context.getResources().getDimension(R.dimen.maplibre_angular_velocity_multiplier),
        context.getResources().getDimension(R.dimen.maplibre_minimum_angular_velocity),
        context.getResources().getDimension(
          com.mapbox.android.gestures.R.dimen.mapbox_defaultScaleSpanSinceStartThreshold));
      ShoveGestureListener shoveGestureListener = new ShoveGestureListener();
      TapGestureListener tapGestureListener = new TapGestureListener();

      gesturesManager.setStandardGestureListener(standardGestureListener);
      gesturesManager.setMoveGestureListener(moveGestureListener);
      gesturesManager.setStandardScaleGestureListener(scaleGestureListener);
      gesturesManager.setRotateGestureListener(rotateGestureListener);
      gesturesManager.setShoveGestureListener(shoveGestureListener);
      gesturesManager.setMultiFingerTapGestureListener(tapGestureListener);
    }
  }

  private void initializeGesturesManager(@NonNull AndroidGesturesManager androidGesturesManager,
                                         boolean setDefaultMutuallyExclusives) {
    if (setDefaultMutuallyExclusives) {
      Set<Integer> shoveScaleSet = new HashSet<>();
      shoveScaleSet.add(AndroidGesturesManager.GESTURE_TYPE_SHOVE);
      shoveScaleSet.add(AndroidGesturesManager.GESTURE_TYPE_SCALE);

      Set<Integer> shoveRotateSet = new HashSet<>();
      shoveRotateSet.add(AndroidGesturesManager.GESTURE_TYPE_SHOVE);
      shoveRotateSet.add(AndroidGesturesManager.GESTURE_TYPE_ROTATE);

      Set<Integer> ScaleLongPressSet = new HashSet<>();
      ScaleLongPressSet.add(AndroidGesturesManager.GESTURE_TYPE_SCALE);
      ScaleLongPressSet.add(AndroidGesturesManager.GESTURE_TYPE_LONG_PRESS);

      androidGesturesManager.setMutuallyExclusiveGestures(shoveScaleSet, shoveRotateSet, ScaleLongPressSet);
    }

    gesturesManager = androidGesturesManager;
    gesturesManager.getRotateGestureDetector().setAngleThreshold(3f);
  }

  /**
   * Set the gesture focal point.
   * <p>
   * This is the center point used for calculate transformations from gestures, value is
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
    this.constantFocalPoint = focalPoint;
  }

  /**
   * Called when user touches the screen, all positions are absolute.
   * <p>
   * Forwards event to the related gesture detectors.
   * </p>
   *
   * @param motionEvent the MotionEvent
   * @return True if touch event is handled
   */
  boolean onTouchEvent(@Nullable MotionEvent motionEvent) {
    // Framework can return null motion events in edge cases #9432
    if (motionEvent == null) {
      return false;
    }

    // Check and ignore non touch or left clicks
    if ((motionEvent.getButtonState() != 0) && (motionEvent.getButtonState() != MotionEvent.BUTTON_PRIMARY)) {
      return false;
    }

    if (motionEvent.getActionMasked() == MotionEvent.ACTION_DOWN) {
      cancelAnimators();
      transform.setGestureInProgress(true);
    }

    boolean result = gesturesManager.onTouchEvent(motionEvent);

    switch (motionEvent.getActionMasked()) {
      case MotionEvent.ACTION_POINTER_DOWN:
        doubleTapFinished();
        break;

      case MotionEvent.ACTION_UP:
        doubleTapFinished();
        transform.setGestureInProgress(false);

        if (!scheduledAnimators.isEmpty()) {
          // Start all awaiting velocity animations
          animationsTimeoutHandler.removeCallbacksAndMessages(null);
          for (Animator animator : scheduledAnimators) {
            animator.start();
          }
          scheduledAnimators.clear();
        }
        break;

      case MotionEvent.ACTION_CANCEL:
        scheduledAnimators.clear();
        transform.setGestureInProgress(false);
        doubleTapFinished();
        break;
    }

    return result;
  }

  void cancelAnimators() {
    animationsTimeoutHandler.removeCallbacksAndMessages(null);
    scheduledAnimators.clear();

    cancelAnimator(scaleAnimator);
    cancelAnimator(rotateAnimator);

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
   * Schedules a velocity animator to be executed when user lifts fingers,
   * unless canceled by the {@link #cancelAnimatorsRunnable}.
   *
   * @param animator animator ot be scheduled
   */
  private void scheduleAnimator(Animator animator) {
    scheduledAnimators.add(animator);
    animationsTimeoutHandler.removeCallbacksAndMessages(null);
    animationsTimeoutHandler.postDelayed(cancelAnimatorsRunnable, MapLibreConstants.SCHEDULED_ANIMATION_TIMEOUT);
  }

  /**
   * Called for events that don't fit the other handlers.
   * <p>
   * Examples of such events are mouse scroll events, mouse moves, joystick & trackpad.
   * </p>
   *
   * @param event The MotionEvent occurred
   * @return True is the event is handled
   */
  boolean onGenericMotionEvent(MotionEvent event) {
    // Mouse events
    // if (event.isFromSource(InputDevice.SOURCE_CLASS_POINTER)) { // this is not available before API 18
    if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) == InputDevice.SOURCE_CLASS_POINTER) {
      // Choose the action
      switch (event.getActionMasked()) {
        // Mouse scrolls
        case MotionEvent.ACTION_SCROLL:
          if (!uiSettings.isZoomGesturesEnabled()) {
            return false;
          }

          // Cancel any animation
          transform.cancelTransitions();

          // Get the vertical scroll amount, one click = 1
          float scrollDist = event.getAxisValue(MotionEvent.AXIS_VSCROLL);

          // Scale the map by the appropriate power of two factor
          transform.zoomBy(scrollDist, new PointF(event.getX(), event.getY()));

          return true;

        default:
          // We are not interested in this event
          return false;
      }
    }

    // We are not interested in this event
    return false;
  }

  private final class StandardGestureListener extends StandardGestureDetector.SimpleStandardOnGestureListener {
    private final float doubleTapMovementThreshold;

    StandardGestureListener(float doubleTapMovementThreshold) {
      this.doubleTapMovementThreshold = doubleTapMovementThreshold;
    }

    @Override
    public boolean onDown(MotionEvent motionEvent) {
      return true;
    }

    @Override
    public boolean onSingleTapUp(MotionEvent motionEvent) {
      transform.cancelTransitions();
      return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent motionEvent) {
      PointF tapPoint = new PointF(motionEvent.getX(), motionEvent.getY());
      boolean tapHandled = annotationManager.onTap(tapPoint);

      if (!tapHandled) {
        if (uiSettings.isDeselectMarkersOnTap()) {
          // deselect any selected marker
          annotationManager.deselectMarkers();
        }

        notifyOnMapClickListeners(tapPoint);
      }

      return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent motionEvent) {
      int action = motionEvent.getActionMasked();
      if (action == MotionEvent.ACTION_DOWN) {
        doubleTapFocalPoint = new PointF(motionEvent.getX(), motionEvent.getY());
        doubleTapStarted();
      }

      if (motionEvent.getActionMasked() == MotionEvent.ACTION_UP) {
        float diffX = Math.abs(motionEvent.getX() - doubleTapFocalPoint.x);
        float diffY = Math.abs(motionEvent.getY() - doubleTapFocalPoint.y);
        if (diffX > doubleTapMovementThreshold || diffY > doubleTapMovementThreshold) {
          // Ignore double-tap event because we've started the quick-zoom. See #14013.
          return false;
        }

        if (!uiSettings.isZoomGesturesEnabled() || !uiSettings.isDoubleTapGesturesEnabled()) {
          return false;
        }

        // Single finger double tap
        if (constantFocalPoint != null) {
          // User provided focal point
          doubleTapFocalPoint = constantFocalPoint;
        }

        zoomInAnimated(doubleTapFocalPoint, false);

        return true;
      }

      return super.onDoubleTapEvent(motionEvent);
    }

    @Override
    public void onLongPress(MotionEvent motionEvent) {
      PointF longClickPoint = new PointF(motionEvent.getX(), motionEvent.getY());
      notifyOnMapLongClickListeners(longClickPoint);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
      if (!uiSettings.isScrollGesturesEnabled()) {
        // don't allow a fling if scroll is disabled
        return false;
      }

      if (!uiSettings.isFlingVelocityAnimationEnabled()) {
        return false;
      }

      float screenDensity = uiSettings.getPixelRatio();

      // calculate velocity vector for xy dimensions, independent from screen size
      double velocityXY = Math.hypot(velocityX / screenDensity, velocityY / screenDensity);
      if (velocityXY < uiSettings.getFlingThreshold()) {
        // ignore short flings, these can occur when other gestures just have finished executing
        return false;
      }

      // tilt results in a bigger translation, limiting input for #5281
      double tilt = transform.getTilt();
      double tiltFactor = 1.5 + ((tilt != 0) ? (tilt / 10) : 0);

      // calculate animation time based on displacement
      long animationTime = (long) (velocityXY / 7 / tiltFactor + uiSettings.getFlingAnimationBaseTime());

      // screenDensity and influcentcetilt come in here via animationTime
      // factor 1000 because speed is in pixels/s
      // and the factor 0.28 was determined by testing: panning the map and releasing
      //  should result in fling animation starting at same speed as the move before
      double offsetX = velocityX * animationTime * 0.28 / 1000;
      double offsetY = velocityY * animationTime * 0.28 / 1000;

      if (!uiSettings.isHorizontalScrollGesturesEnabled()) {
        // determine if angle of fling is valid for performing a vertical fling
        double angle = Math.abs(Math.toDegrees(Math.atan(offsetX / offsetY)));
        if (angle > MapLibreConstants.ANGLE_THRESHOLD_IGNORE_VERTICAL_FLING) {
          return false;
        }
        offsetX = 0.0;
      }

      transform.cancelTransitions();
      notifyOnFlingListeners();
      cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE);

      // update transformation
      transform.moveBy(offsetX, offsetY, animationTime);

      return true;
    }
  }

  private void doubleTapStarted() {
    // disable the move detector in preparation for the quickzoom,
    // so that we don't move the map's center slightly before the quickzoom is started (see #14227)
    gesturesManager.getMoveGestureDetector().setEnabled(false);
    doubleTapRegistered = true;
  }

  private void doubleTapFinished() {
    if (doubleTapRegistered) {
      // re-enable the move detector in case of double tap
      gesturesManager.getMoveGestureDetector().setEnabled(true);
      doubleTapRegistered = false;
    }
  }

  private final class MoveGestureListener extends MoveGestureDetector.SimpleOnMoveGestureListener {
    @Override
    public boolean onMoveBegin(@NonNull MoveGestureDetector detector) {
      if (!uiSettings.isScrollGesturesEnabled()) {
        return false;
      }

      cancelTransitionsIfRequired();
      notifyOnMoveBeginListeners(detector);
      return true;
    }

    @Override
    public boolean onMove(@NonNull MoveGestureDetector detector, float distanceX, float distanceY) {
      // first move event is often delivered with no displacement
      if (!Float.isNaN(distanceX) && !Float.isNaN(distanceY) && (distanceX != 0 || distanceY != 0)) {
        // dispatching camera start event only when the movement actually occurred
        cameraChangeDispatcher.onCameraMoveStarted(CameraChangeDispatcher.REASON_API_GESTURE);

        // Disable scrolling horizontal if not allowed
        if (!uiSettings.isHorizontalScrollGesturesEnabled()) {
          distanceX = 0;
        }

        // Scroll the map
        transform.moveBy(-distanceX, -distanceY, 0 /*no duration*/);

        notifyOnMoveListeners(detector);
      } else {
        Timber.e("Could not call onMove with parameters %s,%s", distanceX, distanceY);
      }
      return true;
    }

    @Override
    public void onMoveEnd(@NonNull MoveGestureDetector detector, float velocityX, float velocityY) {
      dispatchCameraIdle();
      notifyOnMoveEndListeners(detector);
    }
  }

  private final class ScaleGestureListener extends StandardScaleGestureDetector.SimpleStandardOnScaleGestureListener {

    private final float minimumGestureSpeed;
    private final float minimumAngledGestureSpeed;
    private final float minimumVelocity;
    private final double scaleVelocityRatioThreshold;
    private boolean quickZoom;
    private float spanSinceLast;
    private double screenHeight;
    private double startZoom;

    ScaleGestureListener(double densityMultiplier, float minimumGestureSpeed, float minimumAngledGestureSpeed,
                         float minimumVelocity) {
      this.minimumGestureSpeed = minimumGestureSpeed;
      this.minimumAngledGestureSpeed = minimumAngledGestureSpeed;
      this.minimumVelocity = minimumVelocity;
      this.scaleVelocityRatioThreshold = SCALE_VELOCITY_RATIO_THRESHOLD * densityMultiplier;
    }

    @Override
    public boolean onScaleBegin(@NonNull StandardScaleGestureDetector detector) {
      quickZoom = detector.getPointersCount() == 1;

      if (!uiSettings.isZoomGesturesEnabled()) {
        return false;
      }

      if (quickZoom) {
        if (!uiSettings.isQuickZoomGesturesEnabled()) {
          return false;
        }
        // re-try disabling the move detector in case double tap has been interrupted before quickzoom started
        gesturesManager.getMoveGestureDetector().setEnabled(false);
      } else {
        if (detector.getPreviousSpan() > 0) {
          float currSpan = detector.getCurrentSpan();
          float prevSpan = detector.getPreviousSpan();
          double currTime = detector.getCurrentEvent().getEventTime();
          double prevTime = detector.getPreviousEvent().getEventTime();
          if (currTime == prevTime) {
            return false;
          }
          double speed = Math.abs(currSpan - prevSpan) / (currTime - prevTime);
          if (speed < minimumGestureSpeed) {
            // do not scale if the minimal gesture speed is not met
            return false;
          } else if (!gesturesManager.getRotateGestureDetector().isInProgress()) {
            float rotationDeltaSinceLast = gesturesManager.getRotateGestureDetector().getDeltaSinceLast();
            if (Math.abs(rotationDeltaSinceLast) > 0.4 && speed < minimumAngledGestureSpeed) {
              // do not scale in case we're preferring to start rotation
              return false;
            }

            if (uiSettings.isDisableRotateWhenScaling()) {
              // disable rotate gesture when scale is detected first
              gesturesManager.getRotateGestureDetector().setEnabled(false);
            }
          }
        } else {
          return false;
        }
      }

      screenHeight = Resources.getSystem().getDisplayMetrics().heightPixels;
      startZoom = transform.getRawZoom();

      cancelTransitionsIfRequired();

      notifyOnScaleBeginListeners(detector);

      spanSinceLast = Math.abs(detector.getCurrentSpan() - detector.getPreviousSpan());

      return true;
    }

    @Override
    public boolean onScale(@NonNull StandardScaleGestureDetector detector) {
      // dispatching camera start event only when the movement actually occurred
      cameraChangeDispatcher.onCameraMoveStarted(CameraChangeDispatcher.REASON_API_GESTURE);

      PointF focalPoint = getScaleFocalPoint(detector);
      if (quickZoom) {
        double pixelDeltaChange = Math.abs(detector.getCurrentEvent().getY() - doubleTapFocalPoint.y);
        boolean zoomedOut = detector.getCurrentEvent().getY() < doubleTapFocalPoint.y;

        // normalize the pixel delta change, ranging from 0 to screen height, to a constant zoom change range
        double normalizedDeltaChange = normalize(pixelDeltaChange, 0, screenHeight, 0, QUICK_ZOOM_MAX_ZOOM_CHANGE);

        // calculate target zoom and adjust for a multiplier
        double targetZoom = (zoomedOut ? startZoom - normalizedDeltaChange : startZoom + normalizedDeltaChange);
        targetZoom *= uiSettings.getZoomRate();

        transform.setZoom(targetZoom, focalPoint);
      } else {
        double zoomBy =
          (Math.log(detector.getScaleFactor()) / Math.log(Math.PI / 2)) * ZOOM_RATE * uiSettings.getZoomRate();
        transform.zoomBy(zoomBy, focalPoint);
      }

      notifyOnScaleListeners(detector);

      spanSinceLast = Math.abs(detector.getCurrentSpan() - detector.getPreviousSpan());

      return true;
    }

    @Override
    public void onScaleEnd(@NonNull StandardScaleGestureDetector detector, float velocityX, float velocityY) {
      if (quickZoom) {
        // re-enabled the move detector if the quickzoom happened
        gesturesManager.getMoveGestureDetector().setEnabled(true);
      } else {
        // re-enable rotation in case it's been disabled
        gesturesManager.getRotateGestureDetector().setEnabled(true);
      }

      notifyOnScaleEndListeners(detector);

      float velocityXY = Math.abs(velocityX) + Math.abs(velocityY);

      if (!uiSettings.isScaleVelocityAnimationEnabled()
        || velocityXY < minimumVelocity
        || spanSinceLast / velocityXY < scaleVelocityRatioThreshold) {
        // notifying listeners that camera is idle only if there is no follow-up animation
        dispatchCameraIdle();
        return;
      }

      double zoomAddition = calculateScale(velocityXY, detector.isScalingOut());
      double currentZoom = transform.getRawZoom();
      PointF focalPoint = getScaleFocalPoint(detector);
      // (log(x + 1 / e^2) + 2) * 150, x=0 to 2.5 (MapboxConstants#MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE)
      long animationTime = (long) ((Math.log((Math.abs(zoomAddition)) + 1 / Math.pow(Math.E, 2)) + 2)
        * SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER);
      scaleAnimator = createScaleAnimator(currentZoom, zoomAddition, focalPoint, animationTime);
      scheduleAnimator(scaleAnimator);
    }

    @NonNull
    private PointF getScaleFocalPoint(@NonNull StandardScaleGestureDetector detector) {
      if (constantFocalPoint != null) {
        // around user provided focal point
        return constantFocalPoint;
      } else if (quickZoom) {
        // around center
        return new PointF(uiSettings.getWidth() / 2, uiSettings.getHeight() / 2);
      } else {
        // around gesture
        return detector.getFocalPoint();
      }
    }

    private double calculateScale(double velocityXY, boolean isScalingOut) {
      double zoomAddition = velocityXY * MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE * 1e-4;
      zoomAddition = MathUtils.clamp(zoomAddition, 0, MAX_ABSOLUTE_SCALE_VELOCITY_CHANGE);
      if (isScalingOut) {
        zoomAddition = -zoomAddition;
      }
      return zoomAddition;
    }
  }

  private final class RotateGestureListener extends RotateGestureDetector.SimpleOnRotateGestureListener {
    private final float minimumScaleSpanWhenRotating;
    private final float angularVelocityMultiplier;
    private final float minimumAngularVelocity;
    private final double rotateVelocityRatioThreshold;
    private final float defaultSpanSinceStartThreshold;

    RotateGestureListener(float minimumScaleSpanWhenRotating, double densityMultiplier, float angularVelocityMultiplier,
                          float minimumAngularVelocity, float defaultSpanSinceStartThreshold) {
      this.minimumScaleSpanWhenRotating = minimumScaleSpanWhenRotating;
      this.angularVelocityMultiplier = angularVelocityMultiplier;
      this.minimumAngularVelocity = minimumAngularVelocity;
      this.rotateVelocityRatioThreshold = ROTATE_VELOCITY_RATIO_THRESHOLD * densityMultiplier;
      this.defaultSpanSinceStartThreshold = defaultSpanSinceStartThreshold;
    }

    @Override
    public boolean onRotateBegin(@NonNull RotateGestureDetector detector) {
      if (!uiSettings.isRotateGesturesEnabled()) {
        return false;
      }

      float deltaSinceLast = Math.abs(detector.getDeltaSinceLast());
      double currTime = detector.getCurrentEvent().getEventTime();
      double prevTime = detector.getPreviousEvent().getEventTime();
      if (currTime == prevTime) {
        return false;
      }
      double speed = deltaSinceLast / (currTime - prevTime);
      float deltaSinceStart = Math.abs(detector.getDeltaSinceStart());

      // adjust the responsiveness of a rotation gesture - the higher the speed, the bigger the threshold
      if (speed < 0.04
        || (speed > 0.07 && deltaSinceStart < 5)
        || (speed > 0.15 && deltaSinceStart < 7)
        || (speed > 0.5 && deltaSinceStart < 15)
      ) {
        return false;
      }

      if (uiSettings.isIncreaseScaleThresholdWhenRotating()) {
        // when rotation starts, interrupting scale and increasing the threshold
        // to make rotation without scaling easier
        gesturesManager.getStandardScaleGestureDetector().setSpanSinceStartThreshold(minimumScaleSpanWhenRotating);
        gesturesManager.getStandardScaleGestureDetector().interrupt();
      }

      cancelTransitionsIfRequired();

      notifyOnRotateBeginListeners(detector);

      return true;
    }

    @Override
    public boolean onRotate(@NonNull RotateGestureDetector detector, float rotationDegreesSinceLast,
                            float rotationDegreesSinceFirst) {
      // dispatching camera start event only when the movement actually occurred
      cameraChangeDispatcher.onCameraMoveStarted(CameraChangeDispatcher.REASON_API_GESTURE);

      // Calculate map bearing value
      double bearing = transform.getRawBearing() + rotationDegreesSinceLast;

      // Rotate the map
      PointF focalPoint = getRotateFocalPoint(detector);
      transform.setBearing(bearing, focalPoint.x, focalPoint.y);

      notifyOnRotateListeners(detector);

      return true;
    }

    @Override
    public void onRotateEnd(@NonNull RotateGestureDetector detector, float velocityX,
                            float velocityY, float angularVelocity) {
      if (uiSettings.isIncreaseScaleThresholdWhenRotating()) {
        // resetting default scale threshold values
        gesturesManager.getStandardScaleGestureDetector().setSpanSinceStartThreshold(defaultSpanSinceStartThreshold);
      }

      notifyOnRotateEndListeners(detector);

      angularVelocity = angularVelocity * angularVelocityMultiplier;
      angularVelocity = MathUtils.clamp(angularVelocity, -MAXIMUM_ANGULAR_VELOCITY, MAXIMUM_ANGULAR_VELOCITY);

      float velocityXY = Math.abs(velocityX) + Math.abs(velocityY);
      float delta = Math.abs(detector.getDeltaSinceLast());
      double ratio = delta / velocityXY;

      if (!uiSettings.isRotateVelocityAnimationEnabled()
        || Math.abs(angularVelocity) < minimumAngularVelocity
        || (gesturesManager.getStandardScaleGestureDetector().isInProgress() && ratio < rotateVelocityRatioThreshold)) {
        // notifying listeners that camera is idle only if there is no follow-up animation
        dispatchCameraIdle();
        return;
      }

      long animationTime = (long) ((Math.log((Math.abs(angularVelocity)) + 1 / Math.pow(Math.E, 2)) + 2)
        * SCALE_VELOCITY_ANIMATION_DURATION_MULTIPLIER);

      PointF focalPoint = getRotateFocalPoint(detector);
      rotateAnimator = createRotateAnimator(angularVelocity, animationTime, focalPoint);
      scheduleAnimator(rotateAnimator);
    }

    @NonNull
    private PointF getRotateFocalPoint(@NonNull RotateGestureDetector detector) {
      if (constantFocalPoint != null) {
        // User provided focal point
        return constantFocalPoint;
      } else {
        // around gesture
        return detector.getFocalPoint();
      }
    }

    private Animator createRotateAnimator(float angularVelocity, long animationTime,
                                          @NonNull final PointF animationFocalPoint) {
      ValueAnimator animator = ValueAnimator.ofFloat(angularVelocity, 0f);
      animator.setDuration(animationTime);
      animator.setInterpolator(new DecelerateInterpolator());
      animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
        @Override
        public void onAnimationUpdate(@NonNull ValueAnimator animation) {
          transform.setBearing(
            transform.getRawBearing() + (float) animation.getAnimatedValue(),
            animationFocalPoint.x, animationFocalPoint.y,
            0L
          );
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
  }

  private final class ShoveGestureListener extends ShoveGestureDetector.SimpleOnShoveGestureListener {
    @Override
    public boolean onShoveBegin(@NonNull ShoveGestureDetector detector) {
      if (!uiSettings.isTiltGesturesEnabled()) {
        return false;
      }

      cancelTransitionsIfRequired();

      // disabling move gesture during shove
      gesturesManager.getMoveGestureDetector().setEnabled(false);

      notifyOnShoveBeginListeners(detector);

      return true;
    }

    @Override
    public boolean onShove(@NonNull ShoveGestureDetector detector,
                           float deltaPixelsSinceLast, float deltaPixelsSinceStart) {
      // dispatching camera start event only when the movement actually occurred
      cameraChangeDispatcher.onCameraMoveStarted(CameraChangeDispatcher.REASON_API_GESTURE);

      // Get tilt value (scale and clamp)
      double pitch = transform.getTilt();
      pitch -= MapLibreConstants.SHOVE_PIXEL_CHANGE_FACTOR * deltaPixelsSinceLast;
      pitch = MathUtils.clamp(pitch, MapLibreConstants.MINIMUM_TILT, MapLibreConstants.MAXIMUM_TILT);

      // Tilt the map
      transform.setTilt(pitch);

      notifyOnShoveListeners(detector);

      return true;
    }

    @Override
    public void onShoveEnd(@NonNull ShoveGestureDetector detector, float velocityX, float velocityY) {
      dispatchCameraIdle();

      // re-enabling move gesture
      gesturesManager.getMoveGestureDetector().setEnabled(true);

      notifyOnShoveEndListeners(detector);
    }
  }

  private final class TapGestureListener implements MultiFingerTapGestureDetector.OnMultiFingerTapGestureListener {
    @Override
    public boolean onMultiFingerTap(@NonNull MultiFingerTapGestureDetector detector, int pointersCount) {
      if (!uiSettings.isZoomGesturesEnabled() || pointersCount != 2) {
        return false;
      }

      transform.cancelTransitions();
      cameraChangeDispatcher.onCameraMoveStarted(REASON_API_GESTURE);

      PointF zoomFocalPoint;
      // Single finger double tap
      if (constantFocalPoint != null) {
        // User provided focal point
        zoomFocalPoint = constantFocalPoint;
      } else {
        // Zoom in on gesture
        zoomFocalPoint = detector.getFocalPoint();
      }

      zoomOutAnimated(zoomFocalPoint, false);

      return true;
    }
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
    // we need to dispatch camera idle callback only if there is no other gestures in progress
    if (noGesturesInProgress()) {
      // invalidate the camera position, so that it's valid when fetched from the #onIdle event
      // and doesn't rely on the last frame being rendered
      transform.invalidateCameraPosition();
      cameraChangeDispatcher.onCameraIdle();
    }
  }

  private void cancelTransitionsIfRequired() {
    // we need to cancel core transitions only if there is no started gesture yet
    if (noGesturesInProgress()) {
      transform.cancelTransitions();
    }
  }

  private boolean noGesturesInProgress() {
    return (!uiSettings.isScrollGesturesEnabled() || !gesturesManager.getMoveGestureDetector().isInProgress())
      && (!uiSettings.isZoomGesturesEnabled() || !gesturesManager.getStandardScaleGestureDetector().isInProgress())
      && (!uiSettings.isRotateGesturesEnabled() || !gesturesManager.getRotateGestureDetector().isInProgress())
      && (!uiSettings.isTiltGesturesEnabled() || !gesturesManager.getShoveGestureDetector().isInProgress());
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

  void notifyOnMoveBeginListeners(@NonNull MoveGestureDetector detector) {
    for (MapLibreMap.OnMoveListener listener : onMoveListenerList) {
      listener.onMoveBegin(detector);
    }
  }

  void notifyOnMoveListeners(@NonNull MoveGestureDetector detector) {
    for (MapLibreMap.OnMoveListener listener : onMoveListenerList) {
      listener.onMove(detector);
    }
  }

  void notifyOnMoveEndListeners(@NonNull MoveGestureDetector detector) {
    for (MapLibreMap.OnMoveListener listener : onMoveListenerList) {
      listener.onMoveEnd(detector);
    }
  }

  void notifyOnRotateBeginListeners(@NonNull RotateGestureDetector detector) {
    for (MapLibreMap.OnRotateListener listener : onRotateListenerList) {
      listener.onRotateBegin(detector);
    }
  }

  void notifyOnRotateListeners(@NonNull RotateGestureDetector detector) {
    for (MapLibreMap.OnRotateListener listener : onRotateListenerList) {
      listener.onRotate(detector);
    }
  }

  void notifyOnRotateEndListeners(@NonNull RotateGestureDetector detector) {
    for (MapLibreMap.OnRotateListener listener : onRotateListenerList) {
      listener.onRotateEnd(detector);
    }
  }

  void notifyOnScaleBeginListeners(@NonNull StandardScaleGestureDetector detector) {
    for (MapLibreMap.OnScaleListener listener : onScaleListenerList) {
      listener.onScaleBegin(detector);
    }
  }

  void notifyOnScaleListeners(@NonNull StandardScaleGestureDetector detector) {
    for (MapLibreMap.OnScaleListener listener : onScaleListenerList) {
      listener.onScale(detector);
    }
  }

  void notifyOnScaleEndListeners(@NonNull StandardScaleGestureDetector detector) {
    for (MapLibreMap.OnScaleListener listener : onScaleListenerList) {
      listener.onScaleEnd(detector);
    }
  }

  void notifyOnShoveBeginListeners(@NonNull ShoveGestureDetector detector) {
    for (MapLibreMap.OnShoveListener listener : onShoveListenerList) {
      listener.onShoveBegin(detector);
    }
  }

  void notifyOnShoveListeners(@NonNull ShoveGestureDetector detector) {
    for (MapLibreMap.OnShoveListener listener : onShoveListenerList) {
      listener.onShove(detector);
    }
  }

  void notifyOnShoveEndListeners(@NonNull ShoveGestureDetector detector) {
    for (MapLibreMap.OnShoveListener listener : onShoveListenerList) {
      listener.onShoveEnd(detector);
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

  void addOnRotateListener(MapLibreMap.OnRotateListener listener) {
    onRotateListenerList.add(listener);
  }

  void removeOnRotateListener(MapLibreMap.OnRotateListener listener) {
    onRotateListenerList.remove(listener);
  }

  void addOnScaleListener(MapLibreMap.OnScaleListener listener) {
    onScaleListenerList.add(listener);
  }

  void removeOnScaleListener(MapLibreMap.OnScaleListener listener) {
    onScaleListenerList.remove(listener);
  }

  void addShoveListener(MapLibreMap.OnShoveListener listener) {
    onShoveListenerList.add(listener);
  }

  void removeShoveListener(MapLibreMap.OnShoveListener listener) {
    onShoveListenerList.remove(listener);
  }

  AndroidGesturesManager getGesturesManager() {
    return gesturesManager;
  }

  void setGesturesManager(@NonNull Context context, @NonNull AndroidGesturesManager gesturesManager,
                          boolean attachDefaultListeners, boolean setDefaultMutuallyExclusives) {
    initializeGesturesManager(gesturesManager, setDefaultMutuallyExclusives);
    initializeGestureListeners(context, attachDefaultListeners);
  }
}
