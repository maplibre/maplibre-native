package org.maplibre.android.location;

import android.annotation.SuppressLint;
import android.content.Context;
import android.hardware.SensorManager;
import android.location.Location;
import android.os.Looper;
import android.os.SystemClock;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresPermission;
import androidx.annotation.StyleRes;
import androidx.annotation.VisibleForTesting;

import org.maplibre.android.R;
import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.location.engine.LocationEngine;
import org.maplibre.android.location.engine.LocationEngineCallback;
import org.maplibre.android.location.engine.LocationEngineDefault;
import org.maplibre.android.location.engine.LocationEngineRequest;
import org.maplibre.android.location.engine.LocationEngineResult;
import org.maplibre.android.location.modes.CameraMode;
import org.maplibre.android.location.modes.RenderMode;
import org.maplibre.android.location.engine.MapLibreFusedLocationEngineImpl;
import org.maplibre.android.location.permissions.PermissionsManager;
import org.maplibre.android.log.Logger;
import org.maplibre.android.maps.MapView;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.maps.MapLibreMap.OnCameraIdleListener;
import org.maplibre.android.maps.MapLibreMap.OnCameraMoveListener;
import org.maplibre.android.maps.MapLibreMap.OnMapClickListener;
import org.maplibre.android.maps.Style;
import org.maplibre.android.maps.Transform;
import org.maplibre.android.style.layers.SymbolLayer;

import java.lang.ref.WeakReference;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;

import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static org.maplibre.android.location.LocationComponentConstants.DEFAULT_FASTEST_INTERVAL_MILLIS;
import static org.maplibre.android.location.LocationComponentConstants.DEFAULT_INTERVAL_MILLIS;
import static org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_PADDING_ANIM_DURATION;
import static org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_TILT_ANIM_DURATION;
import static org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_ZOOM_ANIM_DURATION;
import static org.maplibre.android.location.LocationComponentConstants.TRANSITION_ANIMATION_DURATION_MS;
import static org.maplibre.android.location.modes.RenderMode.GPS;


/**
 * The Location Component provides location awareness to your mobile application. Enabling this
 * component provides a contextual experience to your users by showing an icon representing the users
 * current location. A few different modes are offered to provide the right context to your users at
 * the correct time. {@link RenderMode#NORMAL} simply shows the users location on the map
 * represented as a dot. {@link RenderMode#COMPASS} mode allows you to display an arrow icon
 * (by default) that points in the direction the device is pointing in.
 * {@link RenderMode#GPS} can be used in conjunction with our Navigation SDK to
 * display a larger icon (customized with {@link LocationComponentOptions#gpsDrawable()}) we call the user puck.
 * <p>
 * This component also offers the ability to set a map camera behavior for tracking the user
 * location. These different {@link CameraMode}s will track, stop tracking the location based on the
 * mode set with {@link LocationComponent#setCameraMode(int)}.
 * <p>
 * <strong>
 * To get the component object use {@link MapLibreMap#getLocationComponent()} and activate it with
 * {@link #activateLocationComponent(LocationComponentActivationOptions)}.
 * Then, manage its visibility with {@link #setLocationComponentEnabled(boolean)}.
 * The component will not process location updates right after activation, but only after being enabled.
 * </strong>
 * <p>
 * Using this component requires you to request permission beforehand manually or using
 * {@link PermissionsManager}. Either
 * {@code ACCESS_COARSE_LOCATION} or {@code ACCESS_FINE_LOCATION} permissions can be requested for
 * this component to work as expected.
 * <p>
 * This component offers a default, built-in {@link LocationEngine} called
 * {@link MapLibreFusedLocationEngineImpl}.
 * If you'd like to utilize the previously available Google Play Services for more precise location updates,
 * refer to the migration guide of 10.0.0 in the changelog.
 * After a custom engine is passed to the component, or the built-in is initialized,
 * the location updates are going to be requested with the {@link LocationEngineRequest}, either a default one,
 * or the one passed during the activation.
 * When using any engine, requesting/removing the location updates is going to be managed internally.
 * <p>
 * You can also push location updates to the component without any internal engine management.
 * To achieve that, set `useDefaultLocationEngine` in {@link LocationComponentActivationOptions} to false.
 * No engine is going to be initialized and you can push location updates with {@link #forceLocationUpdate(Location)}.
 * <p>
 * For location puck animation purposes, like navigation,
 * we recommend limiting the maximum zoom level of the map for the best user experience.
 * <p>
 * Location Component doesn't support state saving out-of-the-box.
 */
public final class LocationComponent {
  private static final String TAG = "Mbgl-LocationComponent";

  @NonNull
  private final MapLibreMap maplibreMap;
  @NonNull
  private final Transform transform;
  private Style style;
  private LocationComponentOptions options;
  @Nullable
  private LocationEngine locationEngine;
  @NonNull
  private LocationEngineRequest locationEngineRequest =
    new LocationEngineRequest.Builder(DEFAULT_INTERVAL_MILLIS)
      .setFastestInterval(DEFAULT_FASTEST_INTERVAL_MILLIS)
      .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY)
      .build();
  private LocationEngineCallback<LocationEngineResult> currentLocationEngineListener
    = new CurrentLocationEngineCallback(this);
  private LocationEngineCallback<LocationEngineResult> lastLocationEngineListener
    = new LastLocationEngineCallback(this);

  @Nullable
  private CompassEngine compassEngine;

  private LocationLayerController locationLayerController;
  private LocationCameraController locationCameraController;

  private LocationAnimatorCoordinator locationAnimatorCoordinator;

  /**
   * Holds last location which is being returned in the {@link #getLastKnownLocation()}
   * when there is no {@link #locationEngine} set or when the last location returned by the engine is null.
   */
  @Nullable
  private Location lastLocation;
  private CameraPosition lastCameraPosition;

  /**
   * Indicates whether the component has been initialized.
   */
  private boolean isComponentInitialized;

  /**
   * Indicates whether we're using the {@link LocationIndicatorLayer}
   * or the stack of {@link SymbolLayer}s.
   */
  private boolean useSpecializedLocationLayer;

  /**
   * Indicates that the component is enabled and should be displaying location if MapLibre components are available and
   * the lifecycle is in a started state.
   */
  private boolean isEnabled;

  /**
   * Indicated that component's lifecycle {@link #onStart()} method has been called.
   * This allows MapLibre components enter started state and display data, and adds state safety for methods like
   * {@link #setLocationComponentEnabled(boolean)}
   */
  private boolean isComponentStarted;

  /**
   * Indicates if MapLibre components are ready to be interacted with. This can differ from {@link #isComponentStarted}
   * if the MapLibre style is being reloaded.
   */
  private boolean isLayerReady;

  /**
   * Indicates whether we are listening for compass updates.
   */
  private boolean isListeningToCompass;

  private StaleStateManager staleStateManager;
  private final CopyOnWriteArrayList<OnLocationStaleListener> onLocationStaleListeners
    = new CopyOnWriteArrayList<>();
  private final CopyOnWriteArrayList<OnLocationClickListener> onLocationClickListeners
    = new CopyOnWriteArrayList<>();
  private final CopyOnWriteArrayList<OnLocationLongClickListener> onLocationLongClickListeners
    = new CopyOnWriteArrayList<>();
  private final CopyOnWriteArrayList<OnCameraTrackingChangedListener> onCameraTrackingChangedListeners
    = new CopyOnWriteArrayList<>();
  private final CopyOnWriteArrayList<OnRenderModeChangedListener> onRenderModeChangedListeners
    = new CopyOnWriteArrayList<>();

  // Workaround for too frequent updates, see https://github.com/mapbox/mapbox-gl-native/issues/13587
  private long fastestInterval;
  private long lastUpdateTime;

  /**
   * Internal use.
   * <p>
   * To get the component object use {@link MapLibreMap#getLocationComponent()}.
   */
  public LocationComponent(@NonNull MapLibreMap maplibreMap,
                           @NonNull Transform transform,
                           @NonNull List<MapLibreMap.OnDeveloperAnimationListener> developerAnimationListeners) {
    this.maplibreMap = maplibreMap;
    this.transform = transform;
    developerAnimationListeners.add(developerAnimationListener);
  }

  // used for creating a spy
  LocationComponent() {
    //noinspection ConstantConditions
    maplibreMap = null;
    transform = null;
  }

  @VisibleForTesting
  LocationComponent(@NonNull MapLibreMap maplibreMap,
                    @NonNull Transform transform,
                    @NonNull List<MapLibreMap.OnDeveloperAnimationListener> developerAnimationListeners,
                    @NonNull LocationEngineCallback<LocationEngineResult> currentListener,
                    @NonNull LocationEngineCallback<LocationEngineResult> lastListener,
                    @NonNull LocationLayerController locationLayerController,
                    @NonNull LocationCameraController locationCameraController,
                    @NonNull LocationAnimatorCoordinator locationAnimatorCoordinator,
                    @NonNull StaleStateManager staleStateManager,
                    @NonNull CompassEngine compassEngine,
                    boolean useSpecializedLocationLayer) {
    this.maplibreMap = maplibreMap;
    this.transform = transform;
    developerAnimationListeners.add(developerAnimationListener);
    this.currentLocationEngineListener = currentListener;
    this.lastLocationEngineListener = lastListener;
    this.locationLayerController = locationLayerController;
    this.locationCameraController = locationCameraController;
    this.locationAnimatorCoordinator = locationAnimatorCoordinator;
    this.staleStateManager = staleStateManager;
    this.compassEngine = compassEngine;
    this.useSpecializedLocationLayer = useSpecializedLocationLayer;
    isComponentInitialized = true;
  }

  /**
   * This method initializes the component and needs to be called before any other operations are performed.
   * Afterwards, you can manage component's visibility by {@link #setLocationComponentEnabled(boolean)}.
   *
   * @param activationOptions a fully built {@link LocationComponentActivationOptions} object
   */
  public void activateLocationComponent(@NonNull LocationComponentActivationOptions activationOptions) {
    LocationComponentOptions options = activationOptions.locationComponentOptions();
    if (options == null) {
      int styleRes = activationOptions.styleRes();
      if (styleRes == 0) {
        styleRes = R.style.maplibre_LocationComponent;
      }
      options = LocationComponentOptions.createFromAttributes(activationOptions.context(), styleRes);
    }

    // Initialize the LocationComponent with Context, the map's `Style`, and either custom LocationComponentOptions
    // or backup options created from default/custom attributes
    initialize(activationOptions.context(), activationOptions.style(),
      activationOptions.useSpecializedLocationLayer(), options);

    // Apply the LocationComponent styling
    // TODO avoid doubling style initialization
    applyStyle(options);

    // Set the LocationEngine request if one was given to LocationComponentActivationOptions
    LocationEngineRequest locationEngineRequest = activationOptions.locationEngineRequest();
    if (locationEngineRequest != null) {
      setLocationEngineRequest(locationEngineRequest);
    }

    // Set the LocationEngine if one was given to LocationComponentActivationOptions
    LocationEngine locationEngine = activationOptions.locationEngine();
    if (locationEngine != null) {
      setLocationEngine(locationEngine);
    } else {
      if (activationOptions.useDefaultLocationEngine()) {
        setLocationEngine(LocationEngineDefault.INSTANCE.getDefaultLocationEngine(activationOptions.context()));
      } else {
        setLocationEngine(null);
      }
    }
  }

  /**
   * Manage component's visibility after activation.
   *
   * @param isEnabled true if the plugin should be visible and listen for location updates, false otherwise.
   */
  @RequiresPermission(anyOf = {ACCESS_FINE_LOCATION, ACCESS_COARSE_LOCATION})
  public void setLocationComponentEnabled(boolean isEnabled) {
    checkActivationState();
    if (isEnabled) {
      enableLocationComponent();
    } else {
      disableLocationComponent();
    }
    locationCameraController.setEnabled(isEnabled);
  }

  /**
   * Returns whether the plugin is enabled, meaning that location can be displayed and camera modes can be used.
   *
   * @return true if the plugin is enabled, false otherwise
   */
  public boolean isLocationComponentEnabled() {
    checkActivationState();
    return isEnabled;
  }

  /**
   * Sets the camera mode, which determines how the map camera will track the rendered location.
   * <p>
   * When camera is transitioning to a new mode, it will reject inputs like {@link #zoomWhileTracking(double)} or
   * {@link #tiltWhileTracking(double)}.
   * Use {@link OnLocationCameraTransitionListener} to listen for the transition state.
   * <p>
   * <ul>
   * <li>{@link CameraMode#NONE}: No camera tracking</li>
   * <li>{@link CameraMode#NONE_COMPASS}: Camera does not track location, but does track compass bearing</li>
   * <li>{@link CameraMode#NONE_GPS}: Camera does not track location, but does track GPS bearing</li>
   * <li>{@link CameraMode#TRACKING}: Camera tracks the user location</li>
   * <li>{@link CameraMode#TRACKING_COMPASS}: Camera tracks the user location, with bearing provided by a compass</li>
   * <li>{@link CameraMode#TRACKING_GPS}: Camera tracks the user location, with normalized bearing</li>
   * <li>{@link CameraMode#TRACKING_GPS_NORTH}: Camera tracks the user location, with bearing always set to north</li>
   * </ul>
   *
   * @param cameraMode one of the modes found in {@link CameraMode}
   */
  public void setCameraMode(@CameraMode.Mode int cameraMode) {
    setCameraMode(cameraMode, null);
  }

  /**
   * Sets the camera mode, which determines how the map camera will track the rendered location.
   * <p>
   * When camera is transitioning to a new mode, it will reject inputs like {@link #zoomWhileTracking(double)} or
   * {@link #tiltWhileTracking(double)}.
   * Use {@link OnLocationCameraTransitionListener} to listen for the transition state.
   * <p>
   * <ul>
   * <li>{@link CameraMode#NONE}: No camera tracking</li>
   * <li>{@link CameraMode#NONE_COMPASS}: Camera does not track location, but does track compass bearing</li>
   * <li>{@link CameraMode#NONE_GPS}: Camera does not track location, but does track GPS bearing</li>
   * <li>{@link CameraMode#TRACKING}: Camera tracks the user location</li>
   * <li>{@link CameraMode#TRACKING_COMPASS}: Camera tracks the user location, with bearing provided by a compass</li>
   * <li>{@link CameraMode#TRACKING_GPS}: Camera tracks the user location, with normalized bearing</li>
   * <li>{@link CameraMode#TRACKING_GPS_NORTH}: Camera tracks the user location, with bearing always set to north</li>
   * </ul>
   *
   * @param cameraMode         one of the modes found in {@link CameraMode}
   * @param transitionListener callback that's going to be invoked when the transition animation finishes
   */
  public void setCameraMode(@CameraMode.Mode int cameraMode,
                            @Nullable OnLocationCameraTransitionListener transitionListener) {
    setCameraMode(cameraMode, TRANSITION_ANIMATION_DURATION_MS, null, null, null, transitionListener);
  }

  /**
   * Sets the camera mode, which determines how the map camera will track the rendered location.
   * <p>
   * When camera is transitioning to a new mode, it will reject inputs like {@link #zoomWhileTracking(double)} or
   * {@link #tiltWhileTracking(double)}.
   * Use {@link OnLocationCameraTransitionListener} to listen for the transition state.
   * <p>
   * Set values of zoom, bearing and tilt that the camera will transition to. If null is passed to any of those,
   * current value will be used for that parameter instead.
   * If the camera is already tracking, provided values are ignored.
   * <p>
   * <ul>
   * <li>{@link CameraMode#NONE}: No camera tracking</li>
   * <li>{@link CameraMode#NONE_COMPASS}: Camera does not track location, but does track compass bearing</li>
   * <li>{@link CameraMode#NONE_GPS}: Camera does not track location, but does track GPS bearing</li>
   * <li>{@link CameraMode#TRACKING}: Camera tracks the user location</li>
   * <li>{@link CameraMode#TRACKING_COMPASS}: Camera tracks the user location, with bearing provided by a compass</li>
   * <li>{@link CameraMode#TRACKING_GPS}: Camera tracks the user location, with normalized bearing</li>
   * <li>{@link CameraMode#TRACKING_GPS_NORTH}: Camera tracks the user location, with bearing always set to north</li>
   * </ul>
   *
   * @param cameraMode         one of the modes found in {@link CameraMode}
   * @param transitionDuration duration of the transition in milliseconds
   * @param zoom               target zoom, set to null to use current camera position
   * @param bearing            target bearing, set to null to use current camera position
   * @param tilt               target tilt, set to null to use current camera position
   * @param transitionListener callback that's going to be invoked when the transition animation finishes
   */
  public void setCameraMode(@CameraMode.Mode int cameraMode,
                            long transitionDuration,
                            @Nullable Double zoom, @Nullable Double bearing, @Nullable Double tilt,
                            @Nullable OnLocationCameraTransitionListener transitionListener) {
    checkActivationState();
    locationCameraController.setCameraMode(
      cameraMode, lastLocation, transitionDuration, zoom, bearing, tilt,
      new CameraTransitionListener(transitionListener));
    updateCompassListenerState(true);
  }

  /**
   * Used to reset camera animators and notify listeners when the transition finishes.
   */
  private class CameraTransitionListener implements OnLocationCameraTransitionListener {

    private final OnLocationCameraTransitionListener externalListener;

    private CameraTransitionListener(OnLocationCameraTransitionListener externalListener) {
      this.externalListener = externalListener;
    }

    @Override
    public void onLocationCameraTransitionFinished(int cameraMode) {
      if (externalListener != null) {
        externalListener.onLocationCameraTransitionFinished(cameraMode);
      }
      reset(cameraMode);
    }

    @Override
    public void onLocationCameraTransitionCanceled(int cameraMode) {
      if (externalListener != null) {
        externalListener.onLocationCameraTransitionCanceled(cameraMode);
      }
      reset(cameraMode);
    }

    private void reset(@CameraMode.Mode int cameraMode) {
      locationAnimatorCoordinator.resetAllCameraAnimations(maplibreMap.getCameraPosition(),
        cameraMode == CameraMode.TRACKING_GPS_NORTH);
    }
  }

  /**
   * Provides the current camera mode being used to track the location or compass updates.
   *
   * @return the current camera mode
   */
  @CameraMode.Mode
  public int getCameraMode() {
    checkActivationState();
    return locationCameraController.getCameraMode();
  }

  /**
   * Sets the render mode, which determines how the location updates will be rendered on the map.
   * <p>
   * <ul>
   * <li>{@link RenderMode#NORMAL}: Shows user location, bearing ignored</li>
   * <li>{@link RenderMode#COMPASS}: Shows user location with bearing considered from compass</li>
   * <li>{@link RenderMode#GPS}: Shows user location with bearing considered from location</li>
   * </ul>
   *
   * @param renderMode one of the modes found in {@link RenderMode}
   */
  public void setRenderMode(@RenderMode.Mode int renderMode) {
    checkActivationState();
    if (lastLocation != null && renderMode == GPS) {
      locationAnimatorCoordinator.cancelAndRemoveGpsBearingAnimation();
      locationLayerController.setGpsBearing(lastLocation.getBearing());
    }
    locationLayerController.setRenderMode(renderMode);
    updateLayerOffsets(true);
    updateCompassListenerState(true);
  }

  /**
   * Provides the current render mode being used to show
   * the location and/or compass updates on the map.
   *
   * @return the current render mode
   */
  @RenderMode.Mode
  public int getRenderMode() {
    checkActivationState();
    return locationLayerController.getRenderMode();
  }

  /**
   * Returns the current location options being used.
   *
   * @return the current {@link LocationComponentOptions}
   */
  public LocationComponentOptions getLocationComponentOptions() {
    checkActivationState();
    return options;
  }

  /**
   * Apply a new component style with a style resource.
   *
   * @param styleRes a XML style overriding some or all the options
   */
  public void applyStyle(@NonNull Context context, @StyleRes int styleRes) {
    checkActivationState();
    applyStyle(LocationComponentOptions.createFromAttributes(context, styleRes));
  }

  /**
   * Apply a new component style with location component options.
   *
   * @param options to update the current style
   */
  public void applyStyle(@NonNull final LocationComponentOptions options) {
    checkActivationState();
    LocationComponent.this.options = options;
    if (maplibreMap.getStyle() != null) {
      locationLayerController.applyStyle(options);
      locationCameraController.initializeOptions(options);
      staleStateManager.setEnabled(options.enableStaleState());
      staleStateManager.setDelayTime(options.staleStateTimeout());
      locationAnimatorCoordinator.setTrackingAnimationDurationMultiplier(options.trackingAnimationDurationMultiplier());
      locationAnimatorCoordinator.setCompassAnimationEnabled(options.compassAnimationEnabled());
      locationAnimatorCoordinator.setAccuracyAnimationEnabled(options.accuracyAnimationEnabled());
      if (options.pulseEnabled()) {
        startPulsingLocationCircle();
      } else {
        stopPulsingLocationCircle();
      }
      updateMapWithOptions(options);
    }
  }

  /**
   * Starts the LocationComponent's pulsing circle UI.
   */
  private void startPulsingLocationCircle() {
    if (isEnabled && isLayerReady) {
      locationAnimatorCoordinator.startLocationComponentCirclePulsing(options);
      locationLayerController.adjustPulsingCircleLayerVisibility(true);
    }
  }

  /**
   * Zooms to the desired zoom level.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the zoom change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the zoom as a camera change argument.
   * </p>
   *
   * @param zoomLevel         The desired zoom level.
   * @param animationDuration The zoom animation duration.
   * @param callback          The callback with finish/cancel information
   */
  public void zoomWhileTracking(double zoomLevel, long animationDuration,
                                @Nullable MapLibreMap.CancelableCallback callback) {
    checkActivationState();
    if (!isLayerReady) {
      notifyUnsuccessfulCameraOperation(callback, null);
      return;
    } else if (getCameraMode() == CameraMode.NONE) {
      notifyUnsuccessfulCameraOperation(callback, String.format("%s%s",
        "LocationComponent#zoomWhileTracking method can only be used",
        " when a camera mode other than CameraMode#NONE is engaged."));
      return;
    } else if (locationCameraController.isTransitioning()) {
      notifyUnsuccessfulCameraOperation(callback,
        "LocationComponent#zoomWhileTracking method call is ignored because the camera mode is transitioning");
      return;
    }
    locationAnimatorCoordinator.feedNewZoomLevel(
      zoomLevel, maplibreMap.getCameraPosition(), animationDuration, callback
    );
  }

  /**
   * Zooms to the desired zoom level.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the zoom change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the zoom as a camera change argument.
   * </p>
   *
   * @param zoomLevel         The desired zoom level.
   * @param animationDuration The zoom animation duration.
   */
  public void zoomWhileTracking(double zoomLevel, long animationDuration) {
    checkActivationState();
    zoomWhileTracking(zoomLevel, animationDuration, null);
  }

  /**
   * Zooms to the desired zoom level.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the zoom change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the zoom as a camera change argument.
   * </p>
   *
   * @param zoomLevel The desired zoom level.
   */
  public void zoomWhileTracking(double zoomLevel) {
    checkActivationState();
    zoomWhileTracking(zoomLevel, DEFAULT_TRACKING_ZOOM_ANIM_DURATION, null);
  }

  /**
   * Cancels animation started by {@link #zoomWhileTracking(double, long, MapLibreMap.CancelableCallback)}.
   */
  public void cancelZoomWhileTrackingAnimation() {
    checkActivationState();
    locationAnimatorCoordinator.cancelZoomAnimation();
  }

  /**
   * Sets the padding.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the padding change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the padding as a camera change argument.
   * </p>
   *
   * @param padding The desired padding.
   */
  public void paddingWhileTracking(double[] padding) {
    paddingWhileTracking(padding, DEFAULT_TRACKING_PADDING_ANIM_DURATION, null);
  }

  /**
   * Sets the padding.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the padding change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the padding as a camera change argument.
   * </p>
   *
   * @param padding           The desired padding.
   * @param animationDuration The padding animation duration.
   */
  public void paddingWhileTracking(double[] padding, long animationDuration) {
    paddingWhileTracking(padding, animationDuration, null);
  }

  /**
   * Sets the padding.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the padding change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the padding as a camera change argument.
   * </p>
   *
   * @param padding           The desired padding.
   * @param animationDuration The padding animation duration.
   * @param callback          The callback with finish/cancel information
   */
  public void paddingWhileTracking(double[] padding, long animationDuration,
                                   @Nullable MapLibreMap.CancelableCallback callback) {
    checkActivationState();
    if (!isLayerReady) {
      notifyUnsuccessfulCameraOperation(callback, null);
      return;
    } else if (getCameraMode() == CameraMode.NONE) {
      notifyUnsuccessfulCameraOperation(callback, String.format("%s%s",
              "LocationComponent#paddingWhileTracking method can only be used",
              " when a camera mode other than CameraMode#NONE is engaged."));
      return;
    } else if (locationCameraController.isTransitioning()) {
      notifyUnsuccessfulCameraOperation(callback,
              "LocationComponent#paddingWhileTracking method call is ignored because the camera mode is transitioning");
      return;
    }

    locationAnimatorCoordinator.feedNewPadding(padding, maplibreMap.getCameraPosition(), animationDuration, callback);
  }

  /**
   * Cancels animation started by {@link #paddingWhileTracking(double[], long, MapLibreMap.CancelableCallback)}.
   */
  public void cancelPaddingWhileTrackingAnimation() {
    checkActivationState();
    locationAnimatorCoordinator.cancelPaddingAnimation();
  }

  /**
   * Tilts the camera.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the tilt change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the tilt as a camera change argument.
   * </p>
   *
   * @param tilt              The desired camera tilt.
   * @param animationDuration The tilt animation duration.
   * @param callback          The callback with finish/cancel information
   */
  public void tiltWhileTracking(double tilt, long animationDuration,
                                @Nullable MapLibreMap.CancelableCallback callback) {
    checkActivationState();
    if (!isLayerReady) {
      notifyUnsuccessfulCameraOperation(callback, null);
      return;
    } else if (getCameraMode() == CameraMode.NONE) {
      notifyUnsuccessfulCameraOperation(callback, String.format("%s%s",
        "LocationComponent#tiltWhileTracking method can only be used",
        " when a camera mode other than CameraMode#NONE is engaged."));
      return;
    } else if (locationCameraController.isTransitioning()) {
      notifyUnsuccessfulCameraOperation(callback,
        "LocationComponent#tiltWhileTracking method call is ignored because the camera mode is transitioning");
      return;
    }
    locationAnimatorCoordinator.feedNewTilt(tilt, maplibreMap.getCameraPosition(), animationDuration, callback);
  }

  /**
   * Tilts the camera.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the tilt change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the tilt as a camera change argument.
   * </p>
   *
   * @param tilt              The desired camera tilt.
   * @param animationDuration The tilt animation duration.
   */
  public void tiltWhileTracking(double tilt, long animationDuration) {
    checkActivationState();
    tiltWhileTracking(tilt, animationDuration, null);
  }

  /**
   * Tilts the camera.
   * This API can only be used in pair with camera modes other than {@link CameraMode#NONE}.
   * If you are not using any of {@link CameraMode} modes,
   * use one of {@link MapLibreMap#moveCamera(CameraUpdate)},
   * {@link MapLibreMap#easeCamera(CameraUpdate)} or {@link MapLibreMap#animateCamera(CameraUpdate)} instead.
   * <p>
   * If the camera is transitioning when the tilt change is requested, the call is going to be ignored.
   * Use {@link CameraTransitionListener} to chain the animations, or provide the tilt as a camera change argument.
   * </p>
   *
   * @param tilt The desired camera tilt.
   */
  public void tiltWhileTracking(double tilt) {
    checkActivationState();
    tiltWhileTracking(tilt, DEFAULT_TRACKING_TILT_ANIM_DURATION, null);
  }

  /**
   * Cancels animation started by {@link #tiltWhileTracking(double, long, MapLibreMap.CancelableCallback)}.
   */
  public void cancelTiltWhileTrackingAnimation() {
    checkActivationState();
    locationAnimatorCoordinator.cancelTiltAnimation();
  }

  /**
   * Use to either force a location update or to manually control when the user location gets
   * updated.
   *
   * @param location where the location icon is placed on the map
   */
  public void forceLocationUpdate(@Nullable Location location) {
    checkActivationState();
    updateLocation(location, false);
  }

  /**
   * Use to either force a location update or to manually control when the user location gets
   * updated.
   * <p>
   * This method can be used to provide the list of locations where the last one is the target location
   * and the rest are intermediate points used as the animation path.
   * The puck and the camera will be animated between each of the points linearly until reaching the target.
   *
   * @param locations       where the location icon is placed on the map
   * @param lookAheadUpdate If set to true, the last location's timestamp has to be greater than current timestamp and
   *                        should represent the time at which the animation should actually reach this position,
   *                        cutting out the time interpolation delay.
   */
  public void forceLocationUpdate(@Nullable List<Location> locations, boolean lookAheadUpdate) {
    checkActivationState();
    if (locations != null && locations.size() >= 1) {
      updateLocation(
        locations.get(locations.size() - 1), // target location
        locations.subList(0, locations.size() - 1), // intermediate locations
        false,
        lookAheadUpdate);
    } else {
      updateLocation(null, false);
    }
  }

  /**
   * Set max FPS at which location animators can output updates. The throttling will only impact the location puck
   * and camera tracking smooth animations.
   * <p>
   * Setting this <b>will not impact</b> any other animations schedule with {@link MapLibreMap}, gesture animations or
   * {@link #zoomWhileTracking(double)}/{@link #tiltWhileTracking(double)}.
   * <p>
   * Use this setting to limit animation rate of the location puck on higher zoom levels to decrease the stress on
   * the device's CPU which can directly improve battery life, without sacrificing UX.
   * <p>
   * Example usage:
   * <pre>
   * {@code
   * MapLibreMap.addOnCameraIdleListener(new MapLibreMap.OnCameraIdleListener() {
   *   {@literal @}Override
   *   public void onCameraIdle() {
   *     double zoom = MapLibreMap.getCameraPosition().zoom;
   *     int maxAnimationFps;
   *     if (zoom < 5) {
   *       maxAnimationFps = 3;
   *     } else if (zoom < 10) {
   *       maxAnimationFps = 5;
   *     } else if (zoom < 15) {
   *       maxAnimationFps = 7;
   *     } else if (zoom < 18) {
   *       maxAnimationFps = 15;
   *     } else {
   *       maxAnimationFps = Integer.MAX_VALUE;
   *     }
   *     locationComponent.setMaxAnimationFps(maxAnimationFps);
   *   }
   * });
   * }
   * </pre>
   * <p>
   * If you're looking for a way to throttle the FPS of the whole map, including other animations and gestures, see
   * {@link MapView#setMaximumFps(int)}.
   *
   * @param maxAnimationFps max location animation FPS
   */
  public void setMaxAnimationFps(int maxAnimationFps) {
    checkActivationState();
    locationAnimatorCoordinator.setMaxAnimationFps(maxAnimationFps);
  }

  /**
   * Set the location engine to update the current user location.
   * <p>
   * If {@code null} is passed in, all updates will have to occur through the
   * {@link LocationComponent#forceLocationUpdate(Location)} method.
   *
   * @param locationEngine a {@link LocationEngine} this component should use to handle updates
   */
  @SuppressLint("MissingPermission")
  public void setLocationEngine(@Nullable LocationEngine locationEngine) {
    checkActivationState();
    if (this.locationEngine != null) {
      // If internal location engines being used, extra steps need to be taken to deconstruct the instance.
      this.locationEngine.removeLocationUpdates(currentLocationEngineListener);
      this.locationEngine = null;
    }

    if (locationEngine != null) {
      fastestInterval = locationEngineRequest.getFastestInterval();
      this.locationEngine = locationEngine;
      if (isLayerReady && isEnabled) {
        setLastLocation();
        locationEngine.requestLocationUpdates(
          locationEngineRequest, currentLocationEngineListener, Looper.getMainLooper());
      }
    } else {
      fastestInterval = 0;
    }
  }

  /**
   * Set the location request that's going to be used when requesting location updates.
   *
   * @param locationEngineRequest the location request
   */
  public void setLocationEngineRequest(@NonNull LocationEngineRequest locationEngineRequest) {
    checkActivationState();
    this.locationEngineRequest = locationEngineRequest;

    // reset internal LocationEngine ref to re-request location updates if needed
    setLocationEngine(locationEngine);
  }

  /**
   * Get the location request that's going to be used when requesting location updates.
   */
  @NonNull
  public LocationEngineRequest getLocationEngineRequest() {
    checkActivationState();
    return locationEngineRequest;
  }

  /**
   * Returns the current {@link LocationEngine} being used for updating the user location.
   *
   * @return the {@link LocationEngine} being used to update the user location
   */
  @Nullable
  public LocationEngine getLocationEngine() {
    checkActivationState();
    return locationEngine;
  }

  /**
   * Sets the compass engine used to provide compass heading values.
   *
   * @param compassEngine to be used
   */
  public void setCompassEngine(@Nullable CompassEngine compassEngine) {
    checkActivationState();
    if (this.compassEngine != null) {
      updateCompassListenerState(false);
    }
    this.compassEngine = compassEngine;
    updateCompassListenerState(true);
  }

  /**
   * Returns the compass engine used to provide compass heading values.
   *
   * @return compass engine currently being used
   */
  @Nullable
  public CompassEngine getCompassEngine() {
    checkActivationState();
    return compassEngine;
  }

  /**
   * Get the last know location of the location component.
   *
   * @return the last known location
   */
  @Nullable
  public Location getLastKnownLocation() {
    checkActivationState();
    return lastLocation;
  }

  /**
   * Adds a listener that gets invoked when the user clicks the displayed location.
   * <p>
   * If there are registered location click listeners and the location is clicked,
   * only {@link OnLocationClickListener#onLocationComponentClick()} is going to be delivered,
   * {@link MapLibreMap.OnMapClickListener#onMapClick(LatLng)} is going to be consumed
   * and not pushed to the listeners registered after the component's activation.
   *
   * @param listener The location click listener that is invoked when the
   *                 location is clicked
   */
  public void addOnLocationClickListener(@NonNull OnLocationClickListener listener) {
    onLocationClickListeners.add(listener);
  }

  /**
   * Removes the passed listener from the current list of location click listeners.
   *
   * @param listener to be removed
   */
  public void removeOnLocationClickListener(@NonNull OnLocationClickListener listener) {
    onLocationClickListeners.remove(listener);
  }

  /**
   * Adds a listener that gets invoked when the user long clicks the displayed location.
   * <p>
   * If there are registered location long click listeners and the location is long clicked,
   * only {@link OnLocationLongClickListener#onLocationComponentLongClick()} is going to be delivered,
   * {@link MapLibreMap.OnMapLongClickListener#onMapLongClick(LatLng)} is going to be consumed
   * and not pushed to the listeners registered after the component's activation.
   *
   * @param listener The location click listener that is invoked when the
   *                 location is clicked
   */
  public void addOnLocationLongClickListener(@NonNull OnLocationLongClickListener listener) {
    onLocationLongClickListeners.add(listener);
  }

  /**
   * Removes the passed listener from the current list of location long click listeners.
   *
   * @param listener to be removed
   */
  public void removeOnLocationLongClickListener(@NonNull OnLocationLongClickListener listener) {
    onLocationLongClickListeners.remove(listener);
  }

  /**
   * Adds a listener that gets invoked when camera tracking state changes.
   *
   * @param listener Listener that gets invoked when camera tracking state changes.
   */
  public void addOnCameraTrackingChangedListener(@NonNull OnCameraTrackingChangedListener listener) {
    onCameraTrackingChangedListeners.add(listener);
  }

  /**
   * Removes a listener that gets invoked when camera tracking state changes.
   *
   * @param listener Listener that gets invoked when camera tracking state changes.
   */
  public void removeOnCameraTrackingChangedListener(@NonNull OnCameraTrackingChangedListener listener) {
    onCameraTrackingChangedListeners.remove(listener);
  }

  /**
   * Adds a listener that gets invoked when render mode changes.
   *
   * @param listener Listener that gets invoked when render mode changes.
   */
  public void addOnRenderModeChangedListener(@NonNull OnRenderModeChangedListener listener) {
    onRenderModeChangedListeners.add(listener);
  }

  /**
   * Removes a listener that gets invoked when render mode changes.
   *
   * @param listener Listener that gets invoked when render mode changes.
   */
  public void removeRenderModeChangedListener(@NonNull OnRenderModeChangedListener listener) {
    onRenderModeChangedListeners.remove(listener);
  }

  /**
   * Adds the passed listener that gets invoked when user updates have stopped long enough for the last update
   * to be considered stale.
   * <p>
   * This timeout is set by {@link LocationComponentOptions#staleStateTimeout()}.
   *
   * @param listener invoked when last update is considered stale
   */
  public void addOnLocationStaleListener(@NonNull OnLocationStaleListener listener) {
    onLocationStaleListeners.add(listener);
  }

  /**
   * Removes the passed listener from the current list of stale listeners.
   *
   * @param listener to be removed from the list
   */
  public void removeOnLocationStaleListener(@NonNull OnLocationStaleListener listener) {
    onLocationStaleListeners.remove(listener);
  }

  /**
   * Internal use.
   */
  public void onStart() {
    isComponentStarted = true;
    onLocationLayerStart();
  }

  /**
   * Internal use.
   */
  public void onStop() {
    onLocationLayerStop();
    isComponentStarted = false;
  }

  /**
   * Internal use.
   */
  public void onDestroy() {
  }

  /**
   * Internal use.
   */
  public void onStartLoadingMap() {
    onLocationLayerStop();
  }

  /**
   * Internal use.
   */
  public void onFinishLoadingStyle() {
    if (isComponentInitialized) {
      style = maplibreMap.getStyle();
      locationLayerController.initializeComponents(style, options);
      locationCameraController.initializeOptions(options);
      onLocationLayerStart();
    }
  }

  /**
   * Stop the LocationComponent's pulsing circle animation.
   */
  private void stopPulsingLocationCircle() {
    locationAnimatorCoordinator.stopPulsingCircleAnimation();
    locationLayerController.adjustPulsingCircleLayerVisibility(false);
  }

  @SuppressLint("MissingPermission")
  private void onLocationLayerStart() {
    if (!isComponentInitialized || !isComponentStarted || maplibreMap.getStyle() == null) {
      return;
    }

    if (!isLayerReady) {
      isLayerReady = true;
      maplibreMap.addOnCameraMoveListener(onCameraMoveListener);
      maplibreMap.addOnCameraIdleListener(onCameraIdleListener);
      if (options.enableStaleState()) {
        staleStateManager.onStart();
      }
    }

    if (isEnabled) {
      if (locationEngine != null) {
        try {
          locationEngine.requestLocationUpdates(
            locationEngineRequest, currentLocationEngineListener, Looper.getMainLooper());
        } catch (SecurityException se) {
          Logger.e(TAG, "Unable to request location updates", se);
        }
      }
      setCameraMode(locationCameraController.getCameraMode());
      if (options.pulseEnabled()) {
        startPulsingLocationCircle();
      } else {
        stopPulsingLocationCircle();
      }
      setLastLocation();
      updateCompassListenerState(true);
      setLastCompassHeading();
    }
  }

  private void onLocationLayerStop() {
    if (!isComponentInitialized || !isLayerReady || !isComponentStarted) {
      return;
    }

    isLayerReady = false;
    staleStateManager.onStop();
    if (compassEngine != null) {
      updateCompassListenerState(false);
    }

    stopPulsingLocationCircle();
    locationAnimatorCoordinator.cancelAllAnimations();
    if (locationEngine != null) {
      locationEngine.removeLocationUpdates(currentLocationEngineListener);
    }
    maplibreMap.removeOnCameraMoveListener(onCameraMoveListener);
    maplibreMap.removeOnCameraIdleListener(onCameraIdleListener);
  }

  private void initialize(@NonNull final Context context, @NonNull Style style, boolean useSpecializedLocationLayer,
                          @NonNull final LocationComponentOptions options) {
    if (isComponentInitialized) {
      return;
    }
    isComponentInitialized = true;

    if (!style.isFullyLoaded()) {
      throw new IllegalStateException("Style is invalid, provide the most recently loaded one.");
    }

    this.style = style;
    this.options = options;
    this.useSpecializedLocationLayer = useSpecializedLocationLayer;

    maplibreMap.addOnMapClickListener(onMapClickListener);
    maplibreMap.addOnMapLongClickListener(onMapLongClickListener);

    LayerSourceProvider sourceProvider = new LayerSourceProvider();
    LayerFeatureProvider featureProvider = new LayerFeatureProvider();
    LayerBitmapProvider bitmapProvider = new LayerBitmapProvider(context);
    locationLayerController = new LocationLayerController(maplibreMap, style, sourceProvider, featureProvider,
      bitmapProvider, options, renderModeChangedListener, useSpecializedLocationLayer);
    locationCameraController = new LocationCameraController(
      context, maplibreMap, transform, cameraTrackingChangedListener, options, onCameraMoveInvalidateListener);

    locationAnimatorCoordinator = new LocationAnimatorCoordinator(
      maplibreMap.getProjection(),
      MapLibreAnimatorSetProvider.getInstance(),
      MapLibreAnimatorProvider.getInstance()
    );
    locationAnimatorCoordinator.setTrackingAnimationDurationMultiplier(options
      .trackingAnimationDurationMultiplier());

    WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
    SensorManager sensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
    if (windowManager != null && sensorManager != null) {
      compassEngine = new LocationComponentCompassEngine(windowManager, sensorManager);
    }
    staleStateManager = new StaleStateManager(onLocationStaleListener, options);

    updateMapWithOptions(options);

    setRenderMode(RenderMode.NORMAL);
    setCameraMode(CameraMode.NONE);

    onLocationLayerStart();
  }

  private void updateCompassListenerState(boolean canListen) {
    if (compassEngine != null) {
      if (!canListen) {
        // We shouldn't listen, simply unregistering
        removeCompassListener(compassEngine);
        return;
      }

      if (!isComponentInitialized || !isComponentStarted || !isEnabled || !isLayerReady) {
        return;
      }

      if (locationCameraController.isConsumingCompass() || locationLayerController.isConsumingCompass()) {
        // If we have a consumer, and not yet listening, then start listening
        if (!isListeningToCompass) {
          isListeningToCompass = true;
          compassEngine.addCompassListener(compassListener);
        }
      } else {
        // If we have no consumers, stop listening
        removeCompassListener(compassEngine);
      }
    }
  }

  private void removeCompassListener(@NonNull CompassEngine engine) {
    if (isListeningToCompass) {
      isListeningToCompass = false;
      engine.removeCompassListener(compassListener);
    }
  }

  private void enableLocationComponent() {
    isEnabled = true;
    onLocationLayerStart();
  }

  private void disableLocationComponent() {
    isEnabled = false;
    locationLayerController.hide();
    onLocationLayerStop();
  }

  private void updateMapWithOptions(@NonNull LocationComponentOptions options) {
    int[] padding = options.padding();
    if (padding != null) {
      maplibreMap.setPadding(
        padding[0], padding[1], padding[2], padding[3]
      );
    }
  }

  /**
   * Updates the user location icon.
   *
   * @param location the latest user location
   */
  private void updateLocation(@Nullable final Location location, boolean fromLastLocation) {
    updateLocation(location, null, fromLastLocation, false);
  }

  private void updateLocation(@Nullable final Location location, @Nullable List<Location> intermediatePoints,
                              boolean fromLastLocation, boolean lookAheadUpdate) {
    if (location == null) {
      return;
    } else if (!isLayerReady) {
      lastLocation = location;
      return;
    } else {
      long currentTime = SystemClock.elapsedRealtime();
      if (currentTime - lastUpdateTime < fastestInterval) {
        return;
      } else {
        lastUpdateTime = currentTime;
      }
    }

    showLocationLayerIfHidden();

    if (!fromLastLocation) {
      staleStateManager.updateLatestLocationTime();
    }
    CameraPosition currentCameraPosition = maplibreMap.getCameraPosition();
    boolean isGpsNorth = getCameraMode() == CameraMode.TRACKING_GPS_NORTH;
    if (intermediatePoints != null) {
      locationAnimatorCoordinator.feedNewLocation(
        getTargetLocationWithIntermediates(location, intermediatePoints),
        currentCameraPosition,
        isGpsNorth,
        lookAheadUpdate);
    } else {
      locationAnimatorCoordinator.feedNewLocation(location, currentCameraPosition, isGpsNorth);
    }
    updateAccuracyRadius(location, false);
    lastLocation = location;
  }

  private Location[] getTargetLocationWithIntermediates(Location location, List<Location> intermediatePoints) {
    Location[] locations = new Location[intermediatePoints.size() + 1];
    locations[locations.length - 1] = location;
    for (int i = 0; i < intermediatePoints.size(); i++) {
      locations[i] = intermediatePoints.get(i);
    }
    return locations;
  }

  private void showLocationLayerIfHidden() {
    boolean isLocationLayerHidden = locationLayerController.isHidden();
    if (isEnabled && isComponentStarted && isLocationLayerHidden) {
      locationLayerController.show();
      if (options.pulseEnabled()) {
        locationLayerController.adjustPulsingCircleLayerVisibility(true);
      }
    }
  }

  private void updateCompassHeading(float heading) {
    locationAnimatorCoordinator.feedNewCompassBearing(heading, maplibreMap.getCameraPosition());
  }

  /**
   * If the locationEngine contains a last location value, we use it for the initial location layer
   * position.
   */
  @SuppressLint("MissingPermission")
  private void setLastLocation() {
    if (locationEngine != null) {
      locationEngine.getLastLocation(lastLocationEngineListener);
    } else {
      updateLocation(getLastKnownLocation(), true);
    }
  }

  private void setLastCompassHeading() {
    updateCompassHeading(compassEngine != null ? compassEngine.getLastHeading() : 0);
  }

  @SuppressLint("MissingPermission")
  private void updateLayerOffsets(boolean forceUpdate) {
    if (useSpecializedLocationLayer) {
      return;
    }

    CameraPosition position = maplibreMap.getCameraPosition();
    if (lastCameraPosition == null || forceUpdate) {
      lastCameraPosition = position;
      locationLayerController.cameraBearingUpdated(position.bearing);
      locationLayerController.cameraTiltUpdated(position.tilt);
      updateAccuracyRadius(getLastKnownLocation(), true);
      return;
    }

    if (position.bearing != lastCameraPosition.bearing) {
      locationLayerController.cameraBearingUpdated(position.bearing);
    }
    if (position.tilt != lastCameraPosition.tilt) {
      locationLayerController.cameraTiltUpdated(position.tilt);
    }
    if (position.zoom != lastCameraPosition.zoom) {
      updateAccuracyRadius(getLastKnownLocation(), true);
    }
    lastCameraPosition = position;
  }

  private void updateAccuracyRadius(Location location, boolean noAnimation) {
    float radius;
    if (location == null) {
      radius = 0;
    } else if (useSpecializedLocationLayer) {
      radius = location.getAccuracy();
    } else {
      radius = Utils.calculateZoomLevelRadius(maplibreMap, location);
    }
    locationAnimatorCoordinator.feedNewAccuracyRadius(radius, noAnimation);
  }

  private void updateAnimatorListenerHolders() {
    Set<AnimatorListenerHolder> animationsValueChangeListeners = new HashSet<>();
    animationsValueChangeListeners.addAll(locationLayerController.getAnimationListeners());
    animationsValueChangeListeners.addAll(locationCameraController.getAnimationListeners());
    locationAnimatorCoordinator.updateAnimatorListenerHolders(animationsValueChangeListeners);
    locationAnimatorCoordinator.resetAllCameraAnimations(maplibreMap.getCameraPosition(),
      locationCameraController.getCameraMode() == CameraMode.TRACKING_GPS_NORTH);
    locationAnimatorCoordinator.resetAllLayerAnimations();
  }

  @NonNull
  private OnCameraMoveListener onCameraMoveListener = new OnCameraMoveListener() {
    @Override
    public void onCameraMove() {
      updateLayerOffsets(false);
    }
  };

  @NonNull
  private OnCameraIdleListener onCameraIdleListener = new OnCameraIdleListener() {
    @Override
    public void onCameraIdle() {
      updateLayerOffsets(false);
    }
  };

  @NonNull
  private OnMapClickListener onMapClickListener = new OnMapClickListener() {
    @Override
    public boolean onMapClick(@NonNull LatLng point) {
      if (!onLocationClickListeners.isEmpty() && locationLayerController.onMapClick(point)) {
        for (OnLocationClickListener listener : onLocationClickListeners) {
          listener.onLocationComponentClick();
        }
        return true;
      }
      return false;
    }
  };

  @NonNull
  private MapLibreMap.OnMapLongClickListener onMapLongClickListener = new MapLibreMap.OnMapLongClickListener() {
    @Override
    public boolean onMapLongClick(@NonNull LatLng point) {
      if (!onLocationLongClickListeners.isEmpty() && locationLayerController.onMapClick(point)) {
        for (OnLocationLongClickListener listener : onLocationLongClickListeners) {
          listener.onLocationComponentLongClick();
        }
        return true;
      }
      return false;
    }
  };

  @NonNull
  private OnLocationStaleListener onLocationStaleListener = new OnLocationStaleListener() {
    @Override
    public void onStaleStateChange(boolean isStale) {
      locationLayerController.setLocationsStale(isStale);

      for (OnLocationStaleListener listener : onLocationStaleListeners) {
        listener.onStaleStateChange(isStale);
      }
    }
  };

  @NonNull
  private OnCameraMoveInvalidateListener onCameraMoveInvalidateListener = new OnCameraMoveInvalidateListener() {
    @Override
    public void onInvalidateCameraMove() {
      onCameraMoveListener.onCameraMove();
    }
  };

  @NonNull
  private CompassListener compassListener = new CompassListener() {
    @Override
    public void onCompassChanged(float userHeading) {
      updateCompassHeading(userHeading);
    }

    @Override
    public void onCompassAccuracyChange(int compassStatus) {
      // Currently don't handle this inside SDK
    }
  };

  @VisibleForTesting
  static final class CurrentLocationEngineCallback implements LocationEngineCallback<LocationEngineResult> {
    private final WeakReference<LocationComponent> componentWeakReference;

    CurrentLocationEngineCallback(LocationComponent component) {
      this.componentWeakReference = new WeakReference<>(component);
    }

    @Override
    public void onSuccess(LocationEngineResult result) {
      LocationComponent component = componentWeakReference.get();
      if (component != null) {
        component.updateLocation(result.getLastLocation(), false);
      }
    }

    @Override
    public void onFailure(@NonNull Exception exception) {
      Logger.e(TAG, "Failed to obtain location update", exception);
    }
  }

  @VisibleForTesting
  static final class LastLocationEngineCallback implements LocationEngineCallback<LocationEngineResult> {
    private final WeakReference<LocationComponent> componentWeakReference;

    LastLocationEngineCallback(LocationComponent component) {
      this.componentWeakReference = new WeakReference<>(component);
    }

    @Override
    public void onSuccess(LocationEngineResult result) {
      LocationComponent component = componentWeakReference.get();
      if (component != null) {
        component.updateLocation(result.getLastLocation(), true);
      }
    }

    @Override
    public void onFailure(@NonNull Exception exception) {
      Logger.e(TAG, "Failed to obtain last location update", exception);
    }
  }

  @NonNull
  @VisibleForTesting
  OnCameraTrackingChangedListener cameraTrackingChangedListener = new OnCameraTrackingChangedListener() {
    @Override
    public void onCameraTrackingDismissed() {
      for (OnCameraTrackingChangedListener listener : onCameraTrackingChangedListeners) {
        listener.onCameraTrackingDismissed();
      }
    }

    @Override
    public void onCameraTrackingChanged(int currentMode) {
      locationAnimatorCoordinator.cancelZoomAnimation();
      locationAnimatorCoordinator.cancelTiltAnimation();
      updateAnimatorListenerHolders();
      for (OnCameraTrackingChangedListener listener : onCameraTrackingChangedListeners) {
        listener.onCameraTrackingChanged(currentMode);
      }
    }
  };

  @NonNull
  @VisibleForTesting
  OnRenderModeChangedListener renderModeChangedListener = new OnRenderModeChangedListener() {
    @Override
    public void onRenderModeChanged(int currentMode) {
      updateAnimatorListenerHolders();
      for (OnRenderModeChangedListener listener : onRenderModeChangedListeners) {
        listener.onRenderModeChanged(currentMode);
      }
    }
  };

  @NonNull
  private final MapLibreMap.OnDeveloperAnimationListener developerAnimationListener =
    new MapLibreMap.OnDeveloperAnimationListener() {
      @Override
      public void onDeveloperAnimationStarted() {
        if (isComponentInitialized && isEnabled) {
          setCameraMode(CameraMode.NONE);
        }
      }
    };

  private void checkActivationState() {
    if (!isComponentInitialized) {
      throw new LocationComponentNotInitializedException();
    }
  }

  private void notifyUnsuccessfulCameraOperation(@Nullable MapLibreMap.CancelableCallback callback,
                                                 @Nullable String msg) {
    if (msg != null) {
      Logger.e(TAG, msg);
    }

    if (callback != null) {
      callback.onCancel();
    }
  }

  /**
   * Returns whether the location component is activated.
   *
   * @return true if the component is activated, false otherwise
   */
  public boolean isLocationComponentActivated() {
    return isComponentInitialized;
  }
}
