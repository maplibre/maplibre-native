package org.maplibre.android.location;

import android.animation.Animator;
import android.location.Location;
import android.os.SystemClock;
import android.util.SparseArray;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.LinearInterpolator;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;
import androidx.annotation.VisibleForTesting;

import org.maplibre.android.log.Logger;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.maps.Projection;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import static org.maplibre.android.location.LocationComponentConstants.ACCURACY_RADIUS_ANIMATION_DURATION;
import static org.maplibre.android.location.LocationComponentConstants.COMPASS_UPDATE_RATE_MS;
import static org.maplibre.android.location.LocationComponentConstants.MAX_ANIMATION_DURATION_MS;
import static org.maplibre.android.location.LocationComponentConstants.TRANSITION_ANIMATION_DURATION_MS;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_CAMERA_COMPASS_BEARING;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_CAMERA_GPS_BEARING;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_CAMERA_LATLNG;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_LAYER_ACCURACY;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_LAYER_COMPASS_BEARING;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_LAYER_GPS_BEARING;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_LAYER_LATLNG;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_PADDING;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_PULSING_CIRCLE;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_TILT;
import static org.maplibre.android.location.MapLibreAnimator.ANIMATOR_ZOOM;
import static org.maplibre.android.location.Utils.immediateAnimation;
import static org.maplibre.android.location.Utils.normalize;
import static org.maplibre.android.location.Utils.shortestRotation;

import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.geometry.LatLng;

final class LocationAnimatorCoordinator {

  private static final String TAG = "Mbgl-LocationAnimatorCoordinator";

  @VisibleForTesting
  final SparseArray<MapLibreAnimator> animatorArray = new SparseArray<>();

  private final Projection projection;
  private Location previousLocation;
  private float previousAccuracyRadius = -1;
  private float previousCompassBearing = -1;
  private long locationUpdateTimestamp = -1;
  private float durationMultiplier;
  private final MapLibreAnimatorProvider animatorProvider;
  private final MapLibreAnimatorSetProvider animatorSetProvider;
  private boolean compassAnimationEnabled;
  private boolean accuracyAnimationEnabled;

  @VisibleForTesting
  int maxAnimationFps = Integer.MAX_VALUE;

  @VisibleForTesting
  final SparseArray<MapLibreAnimator.AnimationsValueChangeListener> listeners = new SparseArray<>();

  LocationAnimatorCoordinator(@NonNull Projection projection, @NonNull MapLibreAnimatorSetProvider animatorSetProvider,
                              @NonNull MapLibreAnimatorProvider animatorProvider) {
    this.projection = projection;
    this.animatorProvider = animatorProvider;
    this.animatorSetProvider = animatorSetProvider;
  }

  void updateAnimatorListenerHolders(@NonNull Set<AnimatorListenerHolder> listenerHolders) {
    listeners.clear();
    for (AnimatorListenerHolder holder : listenerHolders) {
      listeners.append(holder.getAnimatorType(), holder.getListener());
    }

    for (int i = 0; i < animatorArray.size(); i++) {
      @MapLibreAnimator.Type int animatorType = animatorArray.keyAt(i);
      if (listeners.get(animatorType) == null) {
        MapLibreAnimator animator = animatorArray.get(animatorType);
        if (animator != null) {
          animator.makeInvalid();
        }
      }
    }
  }

  void feedNewLocation(@NonNull Location newLocation, @NonNull CameraPosition currentCameraPosition,
                       boolean isGpsNorth) {
    feedNewLocation(new Location[] {newLocation}, currentCameraPosition, isGpsNorth, false);
  }

  void feedNewLocation(@NonNull @Size(min = 1) Location[] newLocations,
                       @NonNull CameraPosition currentCameraPosition, boolean isGpsNorth, boolean lookAheadUpdate) {
    Location newLocation = newLocations[newLocations.length - 1];
    if (previousLocation == null) {
      previousLocation = newLocation;
      locationUpdateTimestamp = SystemClock.elapsedRealtime() - TRANSITION_ANIMATION_DURATION_MS;
    }

    LatLng previousLayerLatLng = getPreviousLayerLatLng();
    float previousLayerBearing = getPreviousLayerGpsBearing();
    LatLng previousCameraLatLng = currentCameraPosition.target;
    float previousCameraBearing = normalize((float) currentCameraPosition.bearing);

    // generate targets for layer
    LatLng[] latLngValues = getLatLngValues(previousLayerLatLng, newLocations);
    Float[] bearingValues = getBearingValues(previousLayerBearing, newLocations);
    updateLayerAnimators(latLngValues, bearingValues);

    // replace the animation start with the camera's previous value
    latLngValues[0] = previousCameraLatLng;
    if (isGpsNorth) {
      bearingValues = new Float[] {previousCameraBearing, shortestRotation(0f, previousCameraBearing)};
    } else {
      bearingValues = getBearingValues(previousCameraBearing, newLocations);
    }
    updateCameraAnimators(latLngValues, bearingValues);

    LatLng targetLatLng = new LatLng(newLocation);
    boolean snap = immediateAnimation(projection, previousCameraLatLng, targetLatLng)
      || immediateAnimation(projection, previousLayerLatLng, targetLatLng);

    long animationDuration = 0;
    if (!snap) {
      long previousUpdateTimeStamp = locationUpdateTimestamp;
      locationUpdateTimestamp = SystemClock.elapsedRealtime();

      if (previousUpdateTimeStamp == 0) {
        animationDuration = 0;
      } else if (lookAheadUpdate) {
        long currentTimestamp = System.currentTimeMillis();
        if (currentTimestamp > newLocation.getTime()) {
          animationDuration = 0;
          Logger.e("LocationAnimatorCoordinator",
            "Lookahead enabled, but the target location's timestamp is smaller than current timestamp");
        } else {
          animationDuration = newLocation.getTime() - currentTimestamp;
        }
      } else {
        animationDuration = (long) ((locationUpdateTimestamp - previousUpdateTimeStamp) * durationMultiplier)
        /* make animation slightly longer with durationMultiplier, defaults to 1.1f */;
      }

      animationDuration = Math.min(animationDuration, MAX_ANIMATION_DURATION_MS);
    }

    playAnimators(animationDuration,
      ANIMATOR_LAYER_LATLNG,
      ANIMATOR_LAYER_GPS_BEARING,
      ANIMATOR_CAMERA_LATLNG,
      ANIMATOR_CAMERA_GPS_BEARING);

    previousLocation = newLocation;
  }

  void feedNewCompassBearing(float targetCompassBearing, @NonNull CameraPosition currentCameraPosition) {
    if (previousCompassBearing < 0) {
      previousCompassBearing = targetCompassBearing;
    }

    float previousLayerBearing = getPreviousLayerCompassBearing();
    float previousCameraBearing = (float) currentCameraPosition.bearing;

    updateCompassAnimators(targetCompassBearing, previousLayerBearing, previousCameraBearing);
    playAnimators(
      compassAnimationEnabled ? COMPASS_UPDATE_RATE_MS : 0,
      ANIMATOR_LAYER_COMPASS_BEARING,
      ANIMATOR_CAMERA_COMPASS_BEARING);

    previousCompassBearing = targetCompassBearing;
  }

  void feedNewAccuracyRadius(float targetAccuracyRadius, boolean noAnimation) {
    if (previousAccuracyRadius < 0) {
      previousAccuracyRadius = targetAccuracyRadius;
    }

    float previousAccuracyRadius = getPreviousAccuracyRadius();
    updateAccuracyAnimators(targetAccuracyRadius, previousAccuracyRadius);
    playAnimators(
      noAnimation || !accuracyAnimationEnabled ? 0 : ACCURACY_RADIUS_ANIMATION_DURATION,
      ANIMATOR_LAYER_ACCURACY);

    this.previousAccuracyRadius = targetAccuracyRadius;
  }

  /**
   * Initializes the {@link PulsingLocationCircleAnimator}, which is a type of {@link MapLibreAnimator}.
   * This method also adds the animator to this class' animator array.
   *
   * @param options the {@link LocationComponentOptions} passed to this class upstream from the
   *                {@link LocationComponent}.
   */
  void startLocationComponentCirclePulsing(LocationComponentOptions options) {
    cancelAnimator(ANIMATOR_PULSING_CIRCLE);
    MapLibreAnimator.AnimationsValueChangeListener listener = listeners.get(ANIMATOR_PULSING_CIRCLE);
    if (listener != null) {
      PulsingLocationCircleAnimator pulsingLocationCircleAnimator = animatorProvider.pulsingCircleAnimator(
        listener,
        maxAnimationFps,
        options.pulseSingleDuration(),
        options.pulseMaxRadius(),
        options.pulseInterpolator() == null ? new DecelerateInterpolator() : options.pulseInterpolator());
      animatorArray.put(ANIMATOR_PULSING_CIRCLE, pulsingLocationCircleAnimator);
      playPulsingAnimator();
    }
  }

  void feedNewZoomLevel(double targetZoomLevel, @NonNull CameraPosition currentCameraPosition, long animationDuration,
                        @Nullable MapLibreMap.CancelableCallback callback) {
    updateZoomAnimator((float) targetZoomLevel, (float) currentCameraPosition.zoom, callback);
    playAnimators(animationDuration, ANIMATOR_ZOOM);
  }

  void feedNewPadding(double[] padding, @NonNull CameraPosition currentCameraPosition, long animationDuration,
                      @Nullable MapLibreMap.CancelableCallback callback) {
    updatePaddingAnimator(padding, currentCameraPosition.padding, callback);
    playAnimators(animationDuration, ANIMATOR_PADDING);
  }

  void feedNewTilt(double targetTilt, @NonNull CameraPosition currentCameraPosition, long animationDuration,
                   @Nullable MapLibreMap.CancelableCallback callback) {
    updateTiltAnimator((float) targetTilt, (float) currentCameraPosition.tilt, callback);
    playAnimators(animationDuration, ANIMATOR_TILT);
  }

  private LatLng getPreviousLayerLatLng() {
    LatLng previousLatLng;
    MapLibreAnimator latLngAnimator = animatorArray.get(ANIMATOR_LAYER_LATLNG);
    if (latLngAnimator != null) {
      previousLatLng = (LatLng) latLngAnimator.getAnimatedValue();
    } else {
      previousLatLng = new LatLng(previousLocation);
    }
    return previousLatLng;
  }

  private float getPreviousLayerGpsBearing() {
    MapLibreFloatAnimator animator = (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_LAYER_GPS_BEARING);
    float previousBearing;
    if (animator != null) {
      previousBearing = (float) animator.getAnimatedValue();
    } else {
      previousBearing = previousLocation.getBearing();
    }
    return previousBearing;
  }

  private float getPreviousLayerCompassBearing() {
    MapLibreFloatAnimator animator = (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_LAYER_COMPASS_BEARING);

    float previousBearing;
    if (animator != null) {
      previousBearing = (float) animator.getAnimatedValue();
    } else {
      previousBearing = previousCompassBearing;
    }
    return previousBearing;
  }

  private float getPreviousAccuracyRadius() {
    MapLibreAnimator animator = animatorArray.get(ANIMATOR_LAYER_ACCURACY);
    float previousRadius;
    if (animator != null) {
      previousRadius = (float) animator.getAnimatedValue();
    } else {
      previousRadius = previousAccuracyRadius;
    }
    return previousRadius;
  }

  private LatLng[] getLatLngValues(LatLng previousLatLng, Location[] targetLocations) {
    LatLng[] latLngs = new LatLng[targetLocations.length + 1];
    latLngs[0] = previousLatLng;
    for (int i = 1; i < latLngs.length; i++) {
      latLngs[i] = new LatLng(targetLocations[i - 1]);
    }
    return latLngs;
  }

  private Float[] getBearingValues(Float previousBearing, Location[] targetLocations) {
    Float[] bearings = new Float[targetLocations.length + 1];

    // Because Location bearing values are normalized to [0, 360]
    // we need to do the same for the previous bearing value to determine the shortest path
    bearings[0] = normalize(previousBearing);
    for (int i = 1; i < bearings.length; i++) {
      bearings[i] = shortestRotation(targetLocations[i - 1].getBearing(), bearings[i - 1]);
    }
    return bearings;
  }

  private void updateLayerAnimators(LatLng[] latLngValues, Float[] bearingValues) {
    createNewLatLngAnimator(ANIMATOR_LAYER_LATLNG, latLngValues);
    createNewFloatAnimator(ANIMATOR_LAYER_GPS_BEARING, bearingValues);
  }

  private void updateCameraAnimators(LatLng[] latLngValues, Float[] bearingValues) {
    createNewLatLngAnimator(ANIMATOR_CAMERA_LATLNG, latLngValues);
    createNewFloatAnimator(ANIMATOR_CAMERA_GPS_BEARING, bearingValues);
  }

  private void updateCompassAnimators(float targetCompassBearing, float previousLayerBearing,
                                      float previousCameraBearing) {
    float normalizedLayerBearing = Utils.shortestRotation(targetCompassBearing, previousLayerBearing);
    createNewFloatAnimator(ANIMATOR_LAYER_COMPASS_BEARING, previousLayerBearing, normalizedLayerBearing);

    float normalizedCameraBearing = Utils.shortestRotation(targetCompassBearing, previousCameraBearing);
    createNewFloatAnimator(ANIMATOR_CAMERA_COMPASS_BEARING, previousCameraBearing, normalizedCameraBearing);
  }

  private void updateAccuracyAnimators(float targetAccuracyRadius, float previousAccuracyRadius) {
    createNewFloatAnimator(ANIMATOR_LAYER_ACCURACY, previousAccuracyRadius, targetAccuracyRadius);
  }

  private void updateZoomAnimator(float targetZoomLevel, float previousZoomLevel,
                                  @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    createNewCameraAdapterAnimator(ANIMATOR_ZOOM, new Float[] {previousZoomLevel, targetZoomLevel}, cancelableCallback);
  }

  private void updatePaddingAnimator(double[] targetPadding, double[] previousPadding,
                                     @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    createNewPaddingAnimator(ANIMATOR_PADDING, new double[][] {previousPadding, targetPadding}, cancelableCallback);
  }

  private void updateTiltAnimator(float targetTilt, float previousTiltLevel,
                                  @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    createNewCameraAdapterAnimator(ANIMATOR_TILT, new Float[] {previousTiltLevel, targetTilt}, cancelableCallback);
  }

  private void createNewLatLngAnimator(@MapLibreAnimator.Type int animatorType, LatLng previous, LatLng target) {
    createNewLatLngAnimator(animatorType, new LatLng[] {previous, target});
  }

  private void createNewLatLngAnimator(@MapLibreAnimator.Type int animatorType, LatLng[] values) {
    cancelAnimator(animatorType);
    MapLibreAnimator.AnimationsValueChangeListener listener = listeners.get(animatorType);
    if (listener != null) {
      animatorArray.put(animatorType, animatorProvider.latLngAnimator(values, listener, maxAnimationFps));
    }
  }

  private void createNewFloatAnimator(@MapLibreAnimator.Type int animatorType, float previous, float target) {
    createNewFloatAnimator(animatorType, new Float[] {previous, target});
  }

  private void createNewFloatAnimator(@MapLibreAnimator.Type int animatorType, @NonNull @Size(min = 2) Float[] values) {
    cancelAnimator(animatorType);
    MapLibreAnimator.AnimationsValueChangeListener listener = listeners.get(animatorType);
    if (listener != null) {
      animatorArray.put(animatorType, animatorProvider.floatAnimator(values, listener, maxAnimationFps));
    }
  }

  private void createNewCameraAdapterAnimator(@MapLibreAnimator.Type int animatorType,
                                              @NonNull @Size(min = 2) Float[] values,
                                              @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    cancelAnimator(animatorType);
    MapLibreAnimator.AnimationsValueChangeListener listener = listeners.get(animatorType);
    if (listener != null) {
      animatorArray.put(animatorType, animatorProvider.cameraAnimator(values, listener, cancelableCallback));
    }
  }

  private void createNewPaddingAnimator(@MapLibreAnimator.Type int animatorType,
                                        @NonNull @Size(min = 2) double[][] values,
                                        @Nullable MapLibreMap.CancelableCallback cancelableCallback) {
    cancelAnimator(animatorType);
    MapLibreAnimator.AnimationsValueChangeListener listener = listeners.get(animatorType);
    if (listener != null) {
      animatorArray.put(animatorType, animatorProvider.paddingAnimator(values, listener, cancelableCallback));
    }
  }

  private float checkGpsNorth(boolean isGpsNorth, float targetCameraBearing) {
    if (isGpsNorth) {
      targetCameraBearing = 0;
    }
    return targetCameraBearing;
  }

  private void playAnimators(long duration, @MapLibreAnimator.Type int... animatorTypes) {
    List<Animator> animators = new ArrayList<>();
    for (@MapLibreAnimator.Type int animatorType : animatorTypes) {
      Animator animator = animatorArray.get(animatorType);
      if (animator != null) {
        animators.add(animator);
      }
    }
    animatorSetProvider.startAnimation(animators, new LinearInterpolator(), duration);
  }

  /**
   * Starts the {@link PulsingLocationCircleAnimator} in the animator array. This method is separate
   * from {@link #playAnimators(long, int...)} because the MapboxAnimatorSetProvider has many more
   * customizable animation parameters than the other {@link MapLibreAnimator}s.
   */
  private void playPulsingAnimator() {
    Animator animator = animatorArray.get(ANIMATOR_PULSING_CIRCLE);
    if (animator != null) {
      animator.start();
    }
  }

  void resetAllCameraAnimations(@NonNull CameraPosition currentCameraPosition, boolean isGpsNorth) {
    resetCameraCompassAnimation(currentCameraPosition);
    boolean snap = resetCameraLocationAnimations(currentCameraPosition, isGpsNorth);
    playAnimators(
      snap ? 0 : TRANSITION_ANIMATION_DURATION_MS,
      ANIMATOR_CAMERA_LATLNG,
      ANIMATOR_CAMERA_GPS_BEARING);
  }

  private boolean resetCameraLocationAnimations(@NonNull CameraPosition currentCameraPosition, boolean isGpsNorth) {
    resetCameraGpsBearingAnimation(currentCameraPosition, isGpsNorth);
    return resetCameraLatLngAnimation(currentCameraPosition);
  }

  private boolean resetCameraLatLngAnimation(@NonNull CameraPosition currentCameraPosition) {
    MapLibreLatLngAnimator animator = (MapLibreLatLngAnimator) animatorArray.get(ANIMATOR_CAMERA_LATLNG);
    if (animator == null) {
      return false;
    }

    LatLng currentTarget = animator.getTarget();
    LatLng previousCameraTarget = currentCameraPosition.target;
    createNewLatLngAnimator(ANIMATOR_CAMERA_LATLNG, previousCameraTarget, currentTarget);

    return immediateAnimation(projection, previousCameraTarget, currentTarget);
  }

  private void resetCameraGpsBearingAnimation(@NonNull CameraPosition currentCameraPosition, boolean isGpsNorth) {
    MapLibreFloatAnimator animator = (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_CAMERA_GPS_BEARING);
    if (animator == null) {
      return;
    }

    float currentTargetBearing = animator.getTarget();
    currentTargetBearing = checkGpsNorth(isGpsNorth, currentTargetBearing);
    float previousCameraBearing = (float) currentCameraPosition.bearing;
    float normalizedCameraBearing = Utils.shortestRotation(currentTargetBearing, previousCameraBearing);
    createNewFloatAnimator(ANIMATOR_CAMERA_GPS_BEARING, previousCameraBearing, normalizedCameraBearing);
  }

  private void resetCameraCompassAnimation(@NonNull CameraPosition currentCameraPosition) {
    MapLibreFloatAnimator animator =
      (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_CAMERA_COMPASS_BEARING);
    if (animator == null) {
      return;
    }

    float currentTargetBearing = animator.getTarget();
    float previousCameraBearing = (float) currentCameraPosition.bearing;
    float normalizedCameraBearing = Utils.shortestRotation(currentTargetBearing, previousCameraBearing);
    createNewFloatAnimator(ANIMATOR_CAMERA_COMPASS_BEARING, previousCameraBearing, normalizedCameraBearing);
  }

  void resetAllLayerAnimations() {
    MapLibreLatLngAnimator latLngAnimator = (MapLibreLatLngAnimator) animatorArray.get(ANIMATOR_LAYER_LATLNG);
    MapLibreFloatAnimator gpsBearingAnimator = (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_LAYER_GPS_BEARING);
    MapLibreFloatAnimator compassBearingAnimator =
      (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_LAYER_COMPASS_BEARING);
    MapLibreFloatAnimator accuracyAnimator =
      (MapLibreFloatAnimator) animatorArray.get(ANIMATOR_LAYER_ACCURACY);

    if (latLngAnimator != null && gpsBearingAnimator != null) {
      LatLng currentLatLng = (LatLng) latLngAnimator.getAnimatedValue();
      LatLng currentLatLngTarget = latLngAnimator.getTarget();
      createNewLatLngAnimator(ANIMATOR_LAYER_LATLNG, currentLatLng, currentLatLngTarget);

      float currentGpsBearing = (float) gpsBearingAnimator.getAnimatedValue();
      float currentGpsBearingTarget = gpsBearingAnimator.getTarget();
      createNewFloatAnimator(ANIMATOR_LAYER_GPS_BEARING, currentGpsBearing, currentGpsBearingTarget);

      long duration = latLngAnimator.getDuration() - latLngAnimator.getCurrentPlayTime();

      playAnimators(duration, ANIMATOR_LAYER_LATLNG, ANIMATOR_LAYER_GPS_BEARING);
    }

    if (compassBearingAnimator != null) {
      float currentLayerBearing = getPreviousLayerCompassBearing();
      float currentLayerBearingTarget = compassBearingAnimator.getTarget();
      createNewFloatAnimator(ANIMATOR_LAYER_COMPASS_BEARING, currentLayerBearing, currentLayerBearingTarget);
      playAnimators(
        compassAnimationEnabled ? COMPASS_UPDATE_RATE_MS : 0,
        ANIMATOR_LAYER_COMPASS_BEARING);
    }

    if (accuracyAnimator != null) {
      feedNewAccuracyRadius(previousAccuracyRadius, false);
    }
  }

  void cancelZoomAnimation() {
    cancelAnimator(ANIMATOR_ZOOM);
  }

  void cancelPaddingAnimation() {
    cancelAnimator(ANIMATOR_PADDING);
  }

  void cancelTiltAnimation() {
    cancelAnimator(ANIMATOR_TILT);
  }

  void cancelAndRemoveGpsBearingAnimation() {
    cancelAnimator(ANIMATOR_LAYER_GPS_BEARING);
    animatorArray.remove(ANIMATOR_LAYER_GPS_BEARING);
  }

  /**
   * Cancel the pulsing circle location animator.
   */
  void stopPulsingCircleAnimation() {
    cancelAnimator(ANIMATOR_PULSING_CIRCLE);
  }

  void cancelAllAnimations() {
    for (int i = 0; i < animatorArray.size(); i++) {
      @MapLibreAnimator.Type int animatorType = animatorArray.keyAt(i);
      cancelAnimator(animatorType);
    }
  }

  private void cancelAnimator(@MapLibreAnimator.Type int animatorType) {
    MapLibreAnimator animator = animatorArray.get(animatorType);
    if (animator != null) {
      animator.cancel();
      animator.removeAllUpdateListeners();
      animator.removeAllListeners();
    }
  }

  void setTrackingAnimationDurationMultiplier(float trackingAnimationDurationMultiplier) {
    this.durationMultiplier = trackingAnimationDurationMultiplier;
  }

  void setCompassAnimationEnabled(boolean compassAnimationEnabled) {
    this.compassAnimationEnabled = compassAnimationEnabled;
  }

  void setAccuracyAnimationEnabled(boolean accuracyAnimationEnabled) {
    this.accuracyAnimationEnabled = accuracyAnimationEnabled;
  }

  void setMaxAnimationFps(int maxAnimationFps) {
    if (maxAnimationFps <= 0) {
      Logger.e(TAG, "Max animation FPS cannot be less or equal to 0.");
      return;
    }
    this.maxAnimationFps = maxAnimationFps;
  }
}
