package org.maplibre.android.location

import android.Manifest.permission.ACCESS_COARSE_LOCATION
import android.Manifest.permission.ACCESS_FINE_LOCATION
import android.annotation.SuppressLint
import android.content.Context
import android.hardware.SensorManager
import android.location.Location
import android.os.Looper
import android.os.SystemClock
import android.view.WindowManager
import androidx.annotation.RequiresPermission
import androidx.annotation.StyleRes
import androidx.annotation.VisibleForTesting
import org.maplibre.android.R
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_FASTEST_INTERVAL_MILLIS
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_INTERVAL_MILLIS
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_PADDING_ANIM_DURATION
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_TILT_ANIM_DURATION
import org.maplibre.android.location.LocationComponentConstants.DEFAULT_TRACKING_ZOOM_ANIM_DURATION
import org.maplibre.android.location.LocationComponentConstants.TRANSITION_ANIMATION_DURATION_MS
import org.maplibre.android.location.engine.LocationEngine
import org.maplibre.android.location.engine.LocationEngineCallback
import org.maplibre.android.location.engine.LocationEngineDefault
import org.maplibre.android.location.engine.LocationEngineRequest
import org.maplibre.android.location.engine.LocationEngineResult
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.maps.Transform
import java.lang.ref.WeakReference
import java.util.concurrent.CopyOnWriteArrayList

/**
 * The Location Component provides location awareness to your mobile application. Enabling this
 * component provides a contextual experience to your users by showing an icon representing the users
 * current location. A few different modes are offered to provide the right context to your users at
 * the correct time. [RenderMode.NORMAL] simply shows the users location on the map
 * represented as a dot. [RenderMode.COMPASS] mode allows you to display an arrow icon
 * (by default) that points in the direction the device is pointing in.
 * [RenderMode.GPS] can be used in conjunction with our Navigation SDK to
 * display a larger icon (customized with [LocationComponentOptions.gpsDrawable]) we call the user puck.
 *
 * This component also offers the ability to set a map camera behavior for tracking the user
 * location. These different [CameraMode]s will track, stop tracking the location based on the
 * mode set with [cameraMode].
 *
 * **
 * To get the component object use [MapLibreMap.getLocationComponent] and activate it with
 * [activateLocationComponent].
 * Then, manage its visibility with [isLocationComponentEnabled].
 * The component will not process location updates right after activation, but only after being enabled.
 * **
 *
 * Using this component requires you to request permission beforehand manually or using
 * [org.maplibre.android.location.permissions.PermissionsManager]. Either
 * `ACCESS_COARSE_LOCATION` or `ACCESS_FINE_LOCATION` permissions can be requested for
 * this component to work as expected.
 *
 * This component offers a default, built-in [LocationEngine] called
 * [org.maplibre.android.location.engine.MapLibreFusedLocationEngineImpl].
 * If you'd like to utilize the previously available Google Play Services for more precise location updates,
 * refer to the migration guide of 10.0.0 in the changelog.
 * After a custom engine is passed to the component, or the built-in is initialized,
 * the location updates are going to be requested with the [LocationEngineRequest], either a default one,
 * or the one passed during the activation.
 * When using any engine, requesting/removing the location updates is going to be managed internally.
 *
 * You can also push location updates to the component without any internal engine management.
 * To achieve that, set `useDefaultLocationEngine` in [LocationComponentActivationOptions] to false.
 * No engine is going to be initialized and you can push location updates with [forceLocationUpdate].
 *
 * For location puck animation purposes, like navigation,
 * we recommend limiting the maximum zoom level of the map for the best user experience.
 *
 * Location Component doesn't support state saving out-of-the-box.
 */
@Suppress("TooManyFunctions", "LargeClass")
class LocationComponent {
    // Assigned by every constructor but the no-arg one, which only exists to create a spy.
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var transform: Transform

    private var style: Style? = null
    private lateinit var options: LocationComponentOptions

    private var internalLocationEngine: LocationEngine? = null

    private var currentLocationEngineListener: LocationEngineCallback<LocationEngineResult> =
        CurrentLocationEngineCallback(this)
    private var lastLocationEngineListener: LocationEngineCallback<LocationEngineResult> =
        LastLocationEngineCallback(this)

    private var internalCompassEngine: CompassEngine? = null

    private lateinit var locationLayerController: LocationLayerController
    private lateinit var locationCameraController: LocationCameraController

    private lateinit var locationAnimatorCoordinator: LocationAnimatorCoordinator

    /**
     * Holds last location which is being returned in the [lastKnownLocation]
     * when there is no [locationEngine] set or when the last location returned by the engine is null.
     */
    private var lastLocation: Location? = null
    private var lastCameraPosition: CameraPosition? = null

    /**
     * Indicates whether the component has been initialized.
     */
    private var isComponentInitialized = false

    /**
     * Indicates whether we're using the [LocationIndicatorLayer]
     * or the stack of [org.maplibre.android.style.layers.SymbolLayer]s.
     */
    private var useSpecializedLocationLayer = false

    /**
     * Indicates that the component is enabled and should be displaying location if MapLibre components are available
     * and the lifecycle is in a started state.
     */
    private var isEnabled = false

    /**
     * Indicated that component's lifecycle [onStart] method has been called.
     * This allows MapLibre components enter started state and display data, and adds state safety for methods like
     * [isLocationComponentEnabled]
     */
    private var isComponentStarted = false

    /**
     * Indicates if MapLibre components are ready to be interacted with. This can differ from [isComponentStarted]
     * if the MapLibre style is being reloaded.
     */
    private var isLayerReady = false

    /**
     * Indicates whether we are listening for compass updates.
     */
    private var isListeningToCompass = false

    private lateinit var staleStateManager: StaleStateManager
    private val onLocationStaleListeners = CopyOnWriteArrayList<OnLocationStaleListener>()
    private val onLocationClickListeners = CopyOnWriteArrayList<OnLocationClickListener>()
    private val onLocationLongClickListeners = CopyOnWriteArrayList<OnLocationLongClickListener>()
    private val onCameraTrackingChangedListeners = CopyOnWriteArrayList<OnCameraTrackingChangedListener>()
    private val onRenderModeChangedListeners = CopyOnWriteArrayList<OnRenderModeChangedListener>()

    // Workaround for too frequent updates, see https://github.com/mapbox/mapbox-gl-native/issues/13587
    private var fastestInterval: Long = 0
    private var lastUpdateTime: Long = 0

    private val onCameraMoveListener = MapLibreMap.OnCameraMoveListener { updateLayerOffsets(false) }

    private val onCameraIdleListener = MapLibreMap.OnCameraIdleListener { updateLayerOffsets(false) }

    private val onMapClickListener =
        MapLibreMap.OnMapClickListener { point ->
            if (onLocationClickListeners.isNotEmpty() && locationLayerController.onMapClick(point)) {
                for (listener in onLocationClickListeners) {
                    listener.onLocationComponentClick()
                }
                true
            } else {
                false
            }
        }

    private val onMapLongClickListener =
        MapLibreMap.OnMapLongClickListener { point ->
            if (onLocationLongClickListeners.isNotEmpty() && locationLayerController.onMapClick(point)) {
                for (listener in onLocationLongClickListeners) {
                    listener.onLocationComponentLongClick()
                }
                true
            } else {
                false
            }
        }

    private val onLocationStaleListener =
        OnLocationStaleListener { isStale ->
            locationLayerController.setLocationsStale(isStale)

            for (listener in onLocationStaleListeners) {
                listener.onStaleStateChange(isStale)
            }
        }

    private val onCameraMoveInvalidateListener =
        OnCameraMoveInvalidateListener {
            onCameraMoveListener.onCameraMove()
        }

    private val compassListener =
        object : CompassListener {
            override fun onCompassChanged(userHeading: Float) {
                updateCompassHeading(userHeading)
            }

            override fun onCompassAccuracyChange(compassStatus: Int) {
                // Currently don't handle this inside SDK
            }
        }

    @VisibleForTesting
    internal val cameraTrackingChangedListener =
        object : OnCameraTrackingChangedListener {
            override fun onCameraTrackingDismissed() {
                for (listener in onCameraTrackingChangedListeners) {
                    listener.onCameraTrackingDismissed()
                }
            }

            override fun onCameraTrackingChanged(currentMode: Int) {
                locationAnimatorCoordinator.cancelZoomAnimation()
                locationAnimatorCoordinator.cancelTiltAnimation()
                updateAnimatorListenerHolders()
                for (listener in onCameraTrackingChangedListeners) {
                    listener.onCameraTrackingChanged(currentMode)
                }
            }
        }

    @VisibleForTesting
    internal val renderModeChangedListener =
        OnRenderModeChangedListener { currentMode ->
            updateAnimatorListenerHolders()
            for (listener in onRenderModeChangedListeners) {
                listener.onRenderModeChanged(currentMode)
            }
        }

    private val developerAnimationListener =
        MapLibreMap.OnDeveloperAnimationListener {
            if (isComponentInitialized && isEnabled) {
                cameraMode = CameraMode.NONE
            }
        }

    /**
     * Internal use.
     *
     * To get the component object use [MapLibreMap.getLocationComponent].
     */
    constructor(
        maplibreMap: MapLibreMap,
        transform: Transform,
        developerAnimationListeners: MutableList<MapLibreMap.OnDeveloperAnimationListener>,
    ) {
        this.maplibreMap = maplibreMap
        this.transform = transform
        developerAnimationListeners.add(developerAnimationListener)
    }

    // used for creating a spy
    internal constructor()

    @VisibleForTesting
    @Suppress("LongParameterList")
    internal constructor(
        maplibreMap: MapLibreMap,
        transform: Transform,
        developerAnimationListeners: MutableList<MapLibreMap.OnDeveloperAnimationListener>,
        currentListener: LocationEngineCallback<LocationEngineResult>,
        lastListener: LocationEngineCallback<LocationEngineResult>,
        locationLayerController: LocationLayerController,
        locationCameraController: LocationCameraController,
        locationAnimatorCoordinator: LocationAnimatorCoordinator,
        staleStateManager: StaleStateManager,
        compassEngine: CompassEngine,
        useSpecializedLocationLayer: Boolean,
    ) {
        this.maplibreMap = maplibreMap
        this.transform = transform
        developerAnimationListeners.add(developerAnimationListener)
        this.currentLocationEngineListener = currentListener
        this.lastLocationEngineListener = lastListener
        this.locationLayerController = locationLayerController
        this.locationCameraController = locationCameraController
        this.locationAnimatorCoordinator = locationAnimatorCoordinator
        this.staleStateManager = staleStateManager
        this.internalCompassEngine = compassEngine
        this.useSpecializedLocationLayer = useSpecializedLocationLayer
        isComponentInitialized = true
    }

    /**
     * This method initializes the component and needs to be called before any other operations are performed.
     * Afterwards, you can manage component's visibility by [isLocationComponentEnabled].
     *
     * @param activationOptions a fully built [LocationComponentActivationOptions] object
     */
    fun activateLocationComponent(activationOptions: LocationComponentActivationOptions) {
        var options = activationOptions.locationComponentOptions()
        if (options == null) {
            var styleRes = activationOptions.styleRes()
            if (styleRes == 0) {
                styleRes = R.style.maplibre_LocationComponent
            }
            options = LocationComponentOptions.createFromAttributes(activationOptions.context(), styleRes)
        }

        // Initialize the LocationComponent with Context, the map's `Style`, and either custom LocationComponentOptions
        // or backup options created from default/custom attributes
        initialize(
            activationOptions.context(),
            activationOptions.style(),
            activationOptions.useSpecializedLocationLayer(),
            options,
        )

        // Apply the LocationComponent styling
        // TODO avoid doubling style initialization
        applyStyle(options)

        // Set the LocationEngine request if one was given to LocationComponentActivationOptions
        val locationEngineRequest = activationOptions.locationEngineRequest()
        if (locationEngineRequest != null) {
            this.locationEngineRequest = locationEngineRequest
        }

        // Set the LocationEngine if one was given to LocationComponentActivationOptions
        val locationEngine = activationOptions.locationEngine()
        if (locationEngine != null) {
            this.locationEngine = locationEngine
        } else {
            if (activationOptions.useDefaultLocationEngine()) {
                this.locationEngine = LocationEngineDefault.getDefaultLocationEngine(activationOptions.context())
            } else {
                this.locationEngine = null
            }
        }
    }

    /**
     * Manage component's visibility after activation.
     *
     * Returns whether the plugin is enabled, meaning that location can be displayed and camera modes can be used.
     */
    @set:RequiresPermission(anyOf = [ACCESS_FINE_LOCATION, ACCESS_COARSE_LOCATION])
    var isLocationComponentEnabled: Boolean
        get() {
            checkActivationState()
            return isEnabled
        }
        set(value) {
            checkActivationState()
            if (value) {
                enableLocationComponent()
            } else {
                disableLocationComponent()
            }
            locationCameraController.setEnabled(value)
        }

    /**
     * The camera mode, which determines how the map camera will track the rendered location.
     *
     * When camera is transitioning to a new mode, it will reject inputs like [zoomWhileTracking] or
     * [tiltWhileTracking].
     * Use [OnLocationCameraTransitionListener] to listen for the transition state.
     *
     *  - [CameraMode.NONE]: No camera tracking
     *  - [CameraMode.NONE_COMPASS]: Camera does not track location, but does track compass bearing
     *  - [CameraMode.NONE_GPS]: Camera does not track location, but does track GPS bearing
     *  - [CameraMode.TRACKING]: Camera tracks the user location
     *  - [CameraMode.TRACKING_COMPASS]: Camera tracks the user location, with bearing provided by a compass
     *  - [CameraMode.TRACKING_GPS]: Camera tracks the user location, with normalized bearing
     *  - [CameraMode.TRACKING_GPS_NORTH]: Camera tracks the user location, with bearing always set to north
     */
    @get:CameraMode.Mode
    @setparam:CameraMode.Mode
    var cameraMode: Int
        get() {
            checkActivationState()
            return locationCameraController.cameraMode
        }
        set(value) = setCameraMode(value, null)

    /**
     * Sets the camera mode, which determines how the map camera will track the rendered location.
     *
     * When camera is transitioning to a new mode, it will reject inputs like [zoomWhileTracking] or
     * [tiltWhileTracking].
     * Use [OnLocationCameraTransitionListener] to listen for the transition state.
     *
     *  - [CameraMode.NONE]: No camera tracking
     *  - [CameraMode.NONE_COMPASS]: Camera does not track location, but does track compass bearing
     *  - [CameraMode.NONE_GPS]: Camera does not track location, but does track GPS bearing
     *  - [CameraMode.TRACKING]: Camera tracks the user location
     *  - [CameraMode.TRACKING_COMPASS]: Camera tracks the user location, with bearing provided by a compass
     *  - [CameraMode.TRACKING_GPS]: Camera tracks the user location, with normalized bearing
     *  - [CameraMode.TRACKING_GPS_NORTH]: Camera tracks the user location, with bearing always set to north
     *
     * @param cameraMode         one of the modes found in [CameraMode]
     * @param transitionListener callback that's going to be invoked when the transition animation finishes
     */
    fun setCameraMode(
        @CameraMode.Mode cameraMode: Int,
        transitionListener: OnLocationCameraTransitionListener?,
    ) {
        setCameraMode(cameraMode, TRANSITION_ANIMATION_DURATION_MS, null, null, null, transitionListener)
    }

    /**
     * Sets the camera mode, which determines how the map camera will track the rendered location.
     *
     * When camera is transitioning to a new mode, it will reject inputs like [zoomWhileTracking] or
     * [tiltWhileTracking].
     * Use [OnLocationCameraTransitionListener] to listen for the transition state.
     *
     * Set values of zoom, bearing and tilt that the camera will transition to. If null is passed to any of those,
     * current value will be used for that parameter instead.
     * If the camera is already tracking, provided values are ignored.
     *
     *  - [CameraMode.NONE]: No camera tracking
     *  - [CameraMode.NONE_COMPASS]: Camera does not track location, but does track compass bearing
     *  - [CameraMode.NONE_GPS]: Camera does not track location, but does track GPS bearing
     *  - [CameraMode.TRACKING]: Camera tracks the user location
     *  - [CameraMode.TRACKING_COMPASS]: Camera tracks the user location, with bearing provided by a compass
     *  - [CameraMode.TRACKING_GPS]: Camera tracks the user location, with normalized bearing
     *  - [CameraMode.TRACKING_GPS_NORTH]: Camera tracks the user location, with bearing always set to north
     *
     * @param cameraMode         one of the modes found in [CameraMode]
     * @param transitionDuration duration of the transition in milliseconds
     * @param zoom               target zoom, set to null to use current camera position
     * @param bearing            target bearing, set to null to use current camera position
     * @param tilt               target tilt, set to null to use current camera position
     * @param transitionListener callback that's going to be invoked when the transition animation finishes
     */
    fun setCameraMode(
        @CameraMode.Mode cameraMode: Int,
        transitionDuration: Long,
        zoom: Double?,
        bearing: Double?,
        tilt: Double?,
        transitionListener: OnLocationCameraTransitionListener?,
    ) {
        checkActivationState()
        locationCameraController.setCameraMode(
            cameraMode,
            lastLocation,
            transitionDuration,
            zoom,
            bearing,
            tilt,
            CameraTransitionListener(transitionListener),
        )
        updateCompassListenerState(true)
    }

    /**
     * Used to reset camera animators and notify listeners when the transition finishes.
     */
    private inner class CameraTransitionListener(
        private val externalListener: OnLocationCameraTransitionListener?,
    ) : OnLocationCameraTransitionListener {
        override fun onLocationCameraTransitionFinished(cameraMode: Int) {
            externalListener?.onLocationCameraTransitionFinished(cameraMode)
            reset(cameraMode)
        }

        override fun onLocationCameraTransitionCanceled(cameraMode: Int) {
            externalListener?.onLocationCameraTransitionCanceled(cameraMode)
            reset(cameraMode)
        }

        private fun reset(
            @CameraMode.Mode cameraMode: Int,
        ) {
            locationAnimatorCoordinator.resetAllCameraAnimations(
                maplibreMap.cameraPosition,
                cameraMode == CameraMode.TRACKING_GPS_NORTH,
            )
        }
    }

    /**
     * The render mode, which determines how the location updates will be rendered on the map.
     *
     *  - [RenderMode.NORMAL]: Shows user location, bearing ignored
     *  - [RenderMode.COMPASS]: Shows user location with bearing considered from compass
     *  - [RenderMode.GPS]: Shows user location with bearing considered from location
     */
    @get:RenderMode.Mode
    @setparam:RenderMode.Mode
    var renderMode: Int
        get() {
            checkActivationState()
            return locationLayerController.renderMode
        }
        set(value) {
            checkActivationState()
            val currentLastLocation = lastLocation
            if (currentLastLocation != null && value == RenderMode.GPS) {
                locationAnimatorCoordinator.cancelAndRemoveGpsBearingAnimation()
                locationLayerController.setGpsBearing(currentLastLocation.bearing)
            }
            locationLayerController.renderMode = value
            updateLayerOffsets(true)
            updateCompassListenerState(true)
        }

    /**
     * The current location options being used.
     */
    val locationComponentOptions: LocationComponentOptions
        get() {
            checkActivationState()
            return options
        }

    /**
     * Apply a new component style with a style resource.
     *
     * @param styleRes a XML style overriding some or all the options
     */
    fun applyStyle(
        context: Context,
        @StyleRes styleRes: Int,
    ) {
        checkActivationState()
        applyStyle(LocationComponentOptions.createFromAttributes(context, styleRes))
    }

    /**
     * Apply a new component style with location component options.
     *
     * @param options to update the current style
     */
    fun applyStyle(options: LocationComponentOptions) {
        checkActivationState()
        this.options = options
        if (maplibreMap.style != null) {
            locationLayerController.applyStyle(options)
            locationCameraController.initializeOptions(options)
            staleStateManager.setEnabled(options.enableStaleState())
            staleStateManager.setDelayTime(options.staleStateTimeout())
            locationAnimatorCoordinator.setTrackingAnimationDurationMultiplier(
                options.trackingAnimationDurationMultiplier(),
            )
            locationAnimatorCoordinator.setCompassAnimationEnabled(options.compassAnimationEnabled())
            locationAnimatorCoordinator.setAccuracyAnimationEnabled(options.accuracyAnimationEnabled())
            if (options.pulseEnabled() == true) {
                startPulsingLocationCircle()
            } else {
                stopPulsingLocationCircle()
            }
            updateMapWithOptions(options)
        }
    }

    /**
     * Starts the LocationComponent's pulsing circle UI.
     */
    private fun startPulsingLocationCircle() {
        if (isEnabled && isLayerReady) {
            locationAnimatorCoordinator.startLocationComponentCirclePulsing(options)
            locationLayerController.adjustPulsingCircleLayerVisibility(true)
        }
    }

    /**
     * Zooms to the desired zoom level.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the zoom change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the zoom as a camera change argument.
     *
     * @param zoomLevel         The desired zoom level.
     * @param animationDuration The zoom animation duration.
     * @param callback          The callback with finish/cancel information
     */
    fun zoomWhileTracking(
        zoomLevel: Double,
        animationDuration: Long,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        checkActivationState()
        if (!isLayerReady) {
            notifyUnsuccessfulCameraOperation(callback, null)
            return
        } else if (cameraMode == CameraMode.NONE) {
            notifyUnsuccessfulCameraOperation(
                callback,
                "LocationComponent#zoomWhileTracking method can only be used" +
                    " when a camera mode other than CameraMode#NONE is engaged.",
            )
            return
        } else if (locationCameraController.isTransitioning) {
            notifyUnsuccessfulCameraOperation(
                callback,
                "LocationComponent#zoomWhileTracking method call is ignored because the camera mode is transitioning",
            )
            return
        }
        locationAnimatorCoordinator.feedNewZoomLevel(
            zoomLevel,
            maplibreMap.cameraPosition,
            animationDuration,
            callback,
        )
    }

    /**
     * Zooms to the desired zoom level.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the zoom change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the zoom as a camera change argument.
     *
     * @param zoomLevel         The desired zoom level.
     * @param animationDuration The zoom animation duration.
     */
    fun zoomWhileTracking(
        zoomLevel: Double,
        animationDuration: Long,
    ) {
        checkActivationState()
        zoomWhileTracking(zoomLevel, animationDuration, null)
    }

    /**
     * Zooms to the desired zoom level.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the zoom change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the zoom as a camera change argument.
     *
     * @param zoomLevel The desired zoom level.
     */
    fun zoomWhileTracking(zoomLevel: Double) {
        checkActivationState()
        zoomWhileTracking(zoomLevel, DEFAULT_TRACKING_ZOOM_ANIM_DURATION, null)
    }

    /**
     * Cancels animation started by [zoomWhileTracking].
     */
    fun cancelZoomWhileTrackingAnimation() {
        checkActivationState()
        locationAnimatorCoordinator.cancelZoomAnimation()
    }

    /**
     * Sets the padding.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the padding change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the padding as a camera change argument.
     *
     * @param padding The desired padding.
     */
    fun paddingWhileTracking(padding: DoubleArray) {
        paddingWhileTracking(padding, DEFAULT_TRACKING_PADDING_ANIM_DURATION, null)
    }

    /**
     * Sets the padding.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the padding change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the padding as a camera change argument.
     *
     * @param padding           The desired padding.
     * @param animationDuration The padding animation duration.
     */
    fun paddingWhileTracking(
        padding: DoubleArray,
        animationDuration: Long,
    ) {
        paddingWhileTracking(padding, animationDuration, null)
    }

    /**
     * Sets the padding.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the padding change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the padding as a camera change argument.
     *
     * @param padding           The desired padding.
     * @param animationDuration The padding animation duration.
     * @param callback          The callback with finish/cancel information
     */
    fun paddingWhileTracking(
        padding: DoubleArray,
        animationDuration: Long,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        checkActivationState()
        if (!isLayerReady) {
            notifyUnsuccessfulCameraOperation(callback, null)
            return
        } else if (cameraMode == CameraMode.NONE) {
            notifyUnsuccessfulCameraOperation(
                callback,
                "LocationComponent#paddingWhileTracking method can only be used" +
                    " when a camera mode other than CameraMode#NONE is engaged.",
            )
            return
        } else if (locationCameraController.isTransitioning) {
            notifyUnsuccessfulCameraOperation(
                callback,
                "LocationComponent#paddingWhileTracking method call is ignored because the camera mode is transitioning",
            )
            return
        }

        locationAnimatorCoordinator.feedNewPadding(padding, maplibreMap.cameraPosition, animationDuration, callback)
    }

    /**
     * Cancels animation started by [paddingWhileTracking].
     */
    fun cancelPaddingWhileTrackingAnimation() {
        checkActivationState()
        locationAnimatorCoordinator.cancelPaddingAnimation()
    }

    /**
     * Tilts the camera.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the tilt change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the tilt as a camera change argument.
     *
     * @param tilt              The desired camera tilt.
     * @param animationDuration The tilt animation duration.
     * @param callback          The callback with finish/cancel information
     */
    fun tiltWhileTracking(
        tilt: Double,
        animationDuration: Long,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        checkActivationState()
        if (!isLayerReady) {
            notifyUnsuccessfulCameraOperation(callback, null)
            return
        } else if (cameraMode == CameraMode.NONE) {
            notifyUnsuccessfulCameraOperation(
                callback,
                "LocationComponent#tiltWhileTracking method can only be used" +
                    " when a camera mode other than CameraMode#NONE is engaged.",
            )
            return
        } else if (locationCameraController.isTransitioning) {
            notifyUnsuccessfulCameraOperation(
                callback,
                "LocationComponent#tiltWhileTracking method call is ignored because the camera mode is transitioning",
            )
            return
        }
        locationAnimatorCoordinator.feedNewTilt(tilt, maplibreMap.cameraPosition, animationDuration, callback)
    }

    /**
     * Tilts the camera.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the tilt change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the tilt as a camera change argument.
     *
     * @param tilt              The desired camera tilt.
     * @param animationDuration The tilt animation duration.
     */
    fun tiltWhileTracking(
        tilt: Double,
        animationDuration: Long,
    ) {
        checkActivationState()
        tiltWhileTracking(tilt, animationDuration, null)
    }

    /**
     * Tilts the camera.
     * This API can only be used in pair with camera modes other than [CameraMode.NONE].
     * If you are not using any of [CameraMode] modes,
     * use one of [MapLibreMap.moveCamera],
     * [MapLibreMap.easeCamera] or [MapLibreMap.animateCamera] instead.
     *
     * If the camera is transitioning when the tilt change is requested, the call is going to be ignored.
     * Use [CameraTransitionListener] to chain the animations, or provide the tilt as a camera change argument.
     *
     * @param tilt The desired camera tilt.
     */
    fun tiltWhileTracking(tilt: Double) {
        checkActivationState()
        tiltWhileTracking(tilt, DEFAULT_TRACKING_TILT_ANIM_DURATION, null)
    }

    /**
     * Cancels animation started by [tiltWhileTracking].
     */
    fun cancelTiltWhileTrackingAnimation() {
        checkActivationState()
        locationAnimatorCoordinator.cancelTiltAnimation()
    }

    /**
     * Use to either force a location update or to manually control when the user location gets
     * updated.
     *
     * @param location where the location icon is placed on the map
     */
    fun forceLocationUpdate(location: Location?) {
        checkActivationState()
        updateLocation(location, false)
    }

    /**
     * Use to either force a location update or to manually control when the user location gets
     * updated.
     *
     * This method can be used to provide the list of locations where the last one is the target location
     * and the rest are intermediate points used as the animation path.
     * The puck and the camera will be animated between each of the points linearly until reaching the target.
     *
     * @param locations       where the location icon is placed on the map
     * @param lookAheadUpdate If set to true, the last location's timestamp has to be greater than current timestamp and
     *                        should represent the time at which the animation should actually reach this position,
     *                        cutting out the time interpolation delay.
     */
    fun forceLocationUpdate(
        locations: List<Location>?,
        lookAheadUpdate: Boolean,
    ) {
        checkActivationState()
        if (locations != null && locations.isNotEmpty()) {
            updateLocation(
                locations[locations.size - 1], // target location
                locations.subList(0, locations.size - 1), // intermediate locations
                false,
                lookAheadUpdate,
            )
        } else {
            updateLocation(null, false)
        }
    }

    /**
     * Set max FPS at which location animators can output updates. The throttling will only impact the location puck
     * and camera tracking smooth animations.
     *
     * Setting this **will not impact** any other animations schedule with [MapLibreMap], gesture animations or
     * [zoomWhileTracking]/[tiltWhileTracking].
     *
     * Use this setting to limit animation rate of the location puck on higher zoom levels to decrease the stress on
     * the device's CPU which can directly improve battery life, without sacrificing UX.
     *
     * Example usage:
     * ```
     * maplibreMap.addOnCameraIdleListener {
     *     val zoom = maplibreMap.cameraPosition.zoom
     *     val maxAnimationFps = when {
     *         zoom < 5 -> 3
     *         zoom < 10 -> 5
     *         zoom < 15 -> 7
     *         zoom < 18 -> 15
     *         else -> Int.MAX_VALUE
     *     }
     *     locationComponent.setMaxAnimationFps(maxAnimationFps)
     * }
     * ```
     *
     * If you're looking for a way to throttle the FPS of the whole map, including other animations and gestures, see
     * [org.maplibre.android.maps.MapView.setMaximumFps].
     *
     * @param maxAnimationFps max location animation FPS
     */
    fun setMaxAnimationFps(maxAnimationFps: Int) {
        checkActivationState()
        locationAnimatorCoordinator.maxAnimationFps = maxAnimationFps
    }

    /**
     * The location engine used to update the current user location.
     *
     * If `null` is passed in, all updates will have to occur through the
     * [forceLocationUpdate] method.
     */
    var locationEngine: LocationEngine?
        get() {
            checkActivationState()
            return internalLocationEngine
        }

        @SuppressLint("MissingPermission")
        set(value) {
            checkActivationState()
            val currentEngine = internalLocationEngine
            if (currentEngine != null) {
                // If internal location engines being used, extra steps need to be taken to deconstruct the instance.
                currentEngine.removeLocationUpdates(currentLocationEngineListener)
                internalLocationEngine = null
            }

            if (value != null) {
                fastestInterval = locationEngineRequest.fastestInterval
                internalLocationEngine = value
                if (isLayerReady && isEnabled) {
                    setLastLocation()
                    value.requestLocationUpdates(
                        locationEngineRequest,
                        currentLocationEngineListener,
                        Looper.getMainLooper(),
                    )
                }
            } else {
                fastestInterval = 0
            }
        }

    /**
     * The location request that's going to be used when requesting location updates.
     */
    var locationEngineRequest: LocationEngineRequest =
        LocationEngineRequest
            .Builder(DEFAULT_INTERVAL_MILLIS)
            .setFastestInterval(DEFAULT_FASTEST_INTERVAL_MILLIS)
            .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY)
            .build()
        get() {
            checkActivationState()
            return field
        }
        set(value) {
            checkActivationState()
            field = value

            // reset internal LocationEngine ref to re-request location updates if needed
            locationEngine = internalLocationEngine
        }

    /**
     * The compass engine used to provide compass heading values.
     */
    var compassEngine: CompassEngine?
        get() {
            checkActivationState()
            return internalCompassEngine
        }
        set(value) {
            checkActivationState()
            if (internalCompassEngine != null) {
                updateCompassListenerState(false)
            }
            internalCompassEngine = value
            updateCompassListenerState(true)
        }

    /**
     * The last known location of the location component.
     */
    val lastKnownLocation: Location?
        get() {
            checkActivationState()
            return lastLocation
        }

    /**
     * Adds a listener that gets invoked when the user clicks the displayed location.
     *
     * If there are registered location click listeners and the location is clicked,
     * only [OnLocationClickListener.onLocationComponentClick] is going to be delivered,
     * [MapLibreMap.OnMapClickListener.onMapClick] is going to be consumed
     * and not pushed to the listeners registered after the component's activation.
     *
     * @param listener The location click listener that is invoked when the
     *                 location is clicked
     */
    fun addOnLocationClickListener(listener: OnLocationClickListener) {
        onLocationClickListeners.add(listener)
    }

    /**
     * Removes the passed listener from the current list of location click listeners.
     *
     * @param listener to be removed
     */
    fun removeOnLocationClickListener(listener: OnLocationClickListener) {
        onLocationClickListeners.remove(listener)
    }

    /**
     * Adds a listener that gets invoked when the user long clicks the displayed location.
     *
     * If there are registered location long click listeners and the location is long clicked,
     * only [OnLocationLongClickListener.onLocationComponentLongClick] is going to be delivered,
     * [MapLibreMap.OnMapLongClickListener.onMapLongClick] is going to be consumed
     * and not pushed to the listeners registered after the component's activation.
     *
     * @param listener The location click listener that is invoked when the
     *                 location is clicked
     */
    fun addOnLocationLongClickListener(listener: OnLocationLongClickListener) {
        onLocationLongClickListeners.add(listener)
    }

    /**
     * Removes the passed listener from the current list of location long click listeners.
     *
     * @param listener to be removed
     */
    fun removeOnLocationLongClickListener(listener: OnLocationLongClickListener) {
        onLocationLongClickListeners.remove(listener)
    }

    /**
     * Adds a listener that gets invoked when camera tracking state changes.
     *
     * @param listener Listener that gets invoked when camera tracking state changes.
     */
    fun addOnCameraTrackingChangedListener(listener: OnCameraTrackingChangedListener) {
        onCameraTrackingChangedListeners.add(listener)
    }

    /**
     * Removes a listener that gets invoked when camera tracking state changes.
     *
     * @param listener Listener that gets invoked when camera tracking state changes.
     */
    fun removeOnCameraTrackingChangedListener(listener: OnCameraTrackingChangedListener) {
        onCameraTrackingChangedListeners.remove(listener)
    }

    /**
     * Adds a listener that gets invoked when render mode changes.
     *
     * @param listener Listener that gets invoked when render mode changes.
     */
    fun addOnRenderModeChangedListener(listener: OnRenderModeChangedListener) {
        onRenderModeChangedListeners.add(listener)
    }

    /**
     * Removes a listener that gets invoked when render mode changes.
     *
     * @param listener Listener that gets invoked when render mode changes.
     */
    fun removeRenderModeChangedListener(listener: OnRenderModeChangedListener) {
        onRenderModeChangedListeners.remove(listener)
    }

    /**
     * Adds the passed listener that gets invoked when user updates have stopped long enough for the last update
     * to be considered stale.
     *
     * This timeout is set by [LocationComponentOptions.staleStateTimeout].
     *
     * @param listener invoked when last update is considered stale
     */
    fun addOnLocationStaleListener(listener: OnLocationStaleListener) {
        onLocationStaleListeners.add(listener)
    }

    /**
     * Removes the passed listener from the current list of stale listeners.
     *
     * @param listener to be removed from the list
     */
    fun removeOnLocationStaleListener(listener: OnLocationStaleListener) {
        onLocationStaleListeners.remove(listener)
    }

    /**
     * Internal use.
     */
    fun onStart() {
        isComponentStarted = true
        onLocationLayerStart()
    }

    /**
     * Internal use.
     */
    fun onStop() {
        onLocationLayerStop()
        isComponentStarted = false
    }

    /**
     * Internal use.
     */
    fun onDestroy() = Unit

    /**
     * Internal use.
     */
    fun onStartLoadingMap() {
        onLocationLayerStop()
    }

    /**
     * Internal use.
     */
    fun onFinishLoadingStyle() {
        if (isComponentInitialized) {
            val loadedStyle = maplibreMap.style
            style = loadedStyle
            locationLayerController.initializeComponents(loadedStyle!!, options)
            locationCameraController.initializeOptions(options)
            onLocationLayerStart()
        }
    }

    /**
     * Stop the LocationComponent's pulsing circle animation.
     */
    private fun stopPulsingLocationCircle() {
        locationAnimatorCoordinator.stopPulsingCircleAnimation()
        locationLayerController.adjustPulsingCircleLayerVisibility(false)
    }

    @SuppressLint("MissingPermission")
    private fun onLocationLayerStart() {
        if (!isComponentInitialized || !isComponentStarted || maplibreMap.style == null) {
            return
        }

        if (!isLayerReady) {
            isLayerReady = true
            maplibreMap.addOnCameraMoveListener(onCameraMoveListener)
            maplibreMap.addOnCameraIdleListener(onCameraIdleListener)
            if (options.enableStaleState()) {
                staleStateManager.onStart()
            }
        }

        if (isEnabled) {
            internalLocationEngine?.let { engine ->
                try {
                    engine.requestLocationUpdates(
                        locationEngineRequest,
                        currentLocationEngineListener,
                        Looper.getMainLooper(),
                    )
                } catch (se: SecurityException) {
                    Logger.e(TAG, "Unable to request location updates", se)
                }
            }
            cameraMode = locationCameraController.cameraMode
            if (options.pulseEnabled() == true) {
                startPulsingLocationCircle()
            } else {
                stopPulsingLocationCircle()
            }
            setLastLocation()
            updateCompassListenerState(true)
            setLastCompassHeading()
        }
    }

    private fun onLocationLayerStop() {
        if (!isComponentInitialized || !isLayerReady || !isComponentStarted) {
            return
        }

        isLayerReady = false
        staleStateManager.onStop()
        if (internalCompassEngine != null) {
            updateCompassListenerState(false)
        }

        stopPulsingLocationCircle()
        locationAnimatorCoordinator.cancelAllAnimations()
        internalLocationEngine?.removeLocationUpdates(currentLocationEngineListener)
        maplibreMap.removeOnCameraMoveListener(onCameraMoveListener)
        maplibreMap.removeOnCameraIdleListener(onCameraIdleListener)
    }

    private fun initialize(
        context: Context,
        style: Style,
        useSpecializedLocationLayer: Boolean,
        options: LocationComponentOptions,
    ) {
        if (isComponentInitialized) {
            return
        }
        isComponentInitialized = true

        check(style.isFullyLoaded) { "Style is invalid, provide the most recently loaded one." }

        this.style = style
        this.options = options
        this.useSpecializedLocationLayer = useSpecializedLocationLayer

        maplibreMap.addOnMapClickListener(onMapClickListener)
        maplibreMap.addOnMapLongClickListener(onMapLongClickListener)

        val sourceProvider = LayerSourceProvider()
        val featureProvider = LayerFeatureProvider()
        val bitmapProvider = LayerBitmapProvider(context)
        locationLayerController =
            LocationLayerController(
                maplibreMap,
                style,
                sourceProvider,
                featureProvider,
                bitmapProvider,
                options,
                renderModeChangedListener,
                useSpecializedLocationLayer,
            )
        locationCameraController =
            LocationCameraController(
                context,
                maplibreMap,
                transform,
                cameraTrackingChangedListener,
                options,
                onCameraMoveInvalidateListener,
            )

        locationAnimatorCoordinator =
            LocationAnimatorCoordinator(
                maplibreMap.projection,
                MapLibreAnimatorSetProvider.getInstance(),
                MapLibreAnimatorProvider.getInstance(),
            )
        locationAnimatorCoordinator.setTrackingAnimationDurationMultiplier(
            options.trackingAnimationDurationMultiplier(),
        )

        val windowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager?
        val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager?
        if (windowManager != null && sensorManager != null) {
            internalCompassEngine = LocationComponentCompassEngine(windowManager, sensorManager)
        }
        staleStateManager = StaleStateManager(onLocationStaleListener, options)

        updateMapWithOptions(options)

        renderMode = RenderMode.NORMAL
        cameraMode = CameraMode.NONE

        onLocationLayerStart()
    }

    private fun updateCompassListenerState(canListen: Boolean) {
        val engine = internalCompassEngine ?: return

        if (!canListen) {
            // We shouldn't listen, simply unregistering
            removeCompassListener(engine)
            return
        }

        if (!isComponentInitialized || !isComponentStarted || !isEnabled || !isLayerReady) {
            return
        }

        if (locationCameraController.isConsumingCompass || locationLayerController.isConsumingCompass) {
            // If we have a consumer, and not yet listening, then start listening
            if (!isListeningToCompass) {
                isListeningToCompass = true
                engine.addCompassListener(compassListener)
            }
        } else {
            // If we have no consumers, stop listening
            removeCompassListener(engine)
        }
    }

    private fun removeCompassListener(engine: CompassEngine) {
        if (isListeningToCompass) {
            isListeningToCompass = false
            engine.removeCompassListener(compassListener)
        }
    }

    private fun enableLocationComponent() {
        isEnabled = true
        onLocationLayerStart()
    }

    private fun disableLocationComponent() {
        isEnabled = false
        locationLayerController.hide()
        onLocationLayerStop()
    }

    private fun updateMapWithOptions(options: LocationComponentOptions) {
        val padding = options.padding()
        if (padding != null) {
            maplibreMap.setPadding(padding[0], padding[1], padding[2], padding[3])
        }
    }

    /**
     * Updates the user location icon.
     *
     * @param location the latest user location
     */
    private fun updateLocation(
        location: Location?,
        fromLastLocation: Boolean,
    ) {
        updateLocation(location, null, fromLastLocation, false)
    }

    private fun updateLocation(
        location: Location?,
        intermediatePoints: List<Location>?,
        fromLastLocation: Boolean,
        lookAheadUpdate: Boolean,
    ) {
        if (location == null) {
            return
        } else if (!isLayerReady) {
            lastLocation = location
            return
        } else {
            val currentTime = SystemClock.elapsedRealtime()
            if (currentTime - lastUpdateTime < fastestInterval) {
                return
            } else {
                lastUpdateTime = currentTime
            }
        }

        showLocationLayerIfHidden()

        if (!fromLastLocation) {
            staleStateManager.updateLatestLocationTime()
        }
        val currentCameraPosition = maplibreMap.cameraPosition
        val isGpsNorth = cameraMode == CameraMode.TRACKING_GPS_NORTH
        if (intermediatePoints != null) {
            locationAnimatorCoordinator.feedNewLocation(
                getTargetLocationWithIntermediates(location, intermediatePoints),
                currentCameraPosition,
                isGpsNorth,
                lookAheadUpdate,
            )
        } else {
            locationAnimatorCoordinator.feedNewLocation(location, currentCameraPosition, isGpsNorth)
        }
        updateAccuracyRadius(location, false)
        lastLocation = location
    }

    private fun getTargetLocationWithIntermediates(
        location: Location,
        intermediatePoints: List<Location>,
    ): Array<Location> = (intermediatePoints + location).toTypedArray()

    private fun showLocationLayerIfHidden() {
        val isLocationLayerHidden = locationLayerController.isHidden
        if (isEnabled && isComponentStarted && isLocationLayerHidden) {
            locationLayerController.show()
            if (options.pulseEnabled() == true) {
                locationLayerController.adjustPulsingCircleLayerVisibility(true)
            }
        }
    }

    private fun updateCompassHeading(heading: Float) {
        locationAnimatorCoordinator.feedNewCompassBearing(heading, maplibreMap.cameraPosition)
    }

    /**
     * If the locationEngine contains a last location value, we use it for the initial location layer
     * position.
     */
    @SuppressLint("MissingPermission")
    private fun setLastLocation() {
        val engine = internalLocationEngine
        if (engine != null) {
            engine.getLastLocation(lastLocationEngineListener)
        } else {
            updateLocation(lastLocation, true)
        }
    }

    private fun setLastCompassHeading() {
        updateCompassHeading(internalCompassEngine?.lastHeading ?: 0f)
    }

    @SuppressLint("MissingPermission")
    private fun updateLayerOffsets(forceUpdate: Boolean) {
        if (useSpecializedLocationLayer) {
            return
        }

        val position = maplibreMap.cameraPosition
        val previousPosition = lastCameraPosition
        if (previousPosition == null || forceUpdate) {
            lastCameraPosition = position
            locationLayerController.cameraBearingUpdated(position.bearing)
            locationLayerController.cameraTiltUpdated(position.tilt)
            updateAccuracyRadius(lastLocation, true)
            return
        }

        if (position.bearing != previousPosition.bearing) {
            locationLayerController.cameraBearingUpdated(position.bearing)
        }
        if (position.tilt != previousPosition.tilt) {
            locationLayerController.cameraTiltUpdated(position.tilt)
        }
        if (position.zoom != previousPosition.zoom) {
            updateAccuracyRadius(lastLocation, true)
        }
        lastCameraPosition = position
    }

    private fun updateAccuracyRadius(
        location: Location?,
        noAnimation: Boolean,
    ) {
        val radius =
            when {
                location == null -> 0f
                useSpecializedLocationLayer -> location.accuracy
                else -> Utils.calculateZoomLevelRadius(maplibreMap, location)
            }
        locationAnimatorCoordinator.feedNewAccuracyRadius(radius, noAnimation)
    }

    private fun updateAnimatorListenerHolders() {
        val animationsValueChangeListeners = HashSet<AnimatorListenerHolder>()
        animationsValueChangeListeners.addAll(locationLayerController.animationListeners)
        animationsValueChangeListeners.addAll(locationCameraController.animationListeners)
        locationAnimatorCoordinator.updateAnimatorListenerHolders(animationsValueChangeListeners)
        locationAnimatorCoordinator.resetAllCameraAnimations(
            maplibreMap.cameraPosition,
            locationCameraController.cameraMode == CameraMode.TRACKING_GPS_NORTH,
        )
        locationAnimatorCoordinator.resetAllLayerAnimations()
    }

    private fun checkActivationState() {
        if (!isComponentInitialized) {
            throw LocationComponentNotInitializedException()
        }
    }

    private fun notifyUnsuccessfulCameraOperation(
        callback: MapLibreMap.CancelableCallback?,
        msg: String?,
    ) {
        if (msg != null) {
            Logger.e(TAG, msg)
        }

        callback?.onCancel()
    }

    /**
     * Returns whether the location component is activated.
     */
    val isLocationComponentActivated: Boolean
        get() = isComponentInitialized

    @VisibleForTesting
    internal class CurrentLocationEngineCallback(
        component: LocationComponent,
    ) : LocationEngineCallback<LocationEngineResult> {
        private val componentWeakReference = WeakReference(component)

        override fun onSuccess(result: LocationEngineResult) {
            componentWeakReference.get()?.updateLocation(result.lastLocation, false)
        }

        override fun onFailure(exception: Exception) {
            Logger.e(TAG, "Failed to obtain location update", exception)
        }
    }

    @VisibleForTesting
    internal class LastLocationEngineCallback(
        component: LocationComponent,
    ) : LocationEngineCallback<LocationEngineResult> {
        private val componentWeakReference = WeakReference(component)

        override fun onSuccess(result: LocationEngineResult) {
            componentWeakReference.get()?.updateLocation(result.lastLocation, true)
        }

        override fun onFailure(exception: Exception) {
            Logger.e(TAG, "Failed to obtain last location update", exception)
        }
    }

    private companion object {
        const val TAG = "Mbgl-LocationComponent"
    }
}
