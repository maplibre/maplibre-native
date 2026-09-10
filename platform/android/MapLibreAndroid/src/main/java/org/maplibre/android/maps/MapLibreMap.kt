package org.maplibre.android.maps

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import android.os.Bundle
import android.text.TextUtils
import android.view.View
import androidx.annotation.FloatRange
import androidx.annotation.IntRange
import androidx.annotation.Size
import androidx.annotation.UiThread
import org.maplibre.android.MapStrictMode
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.annotations.Polygon
import org.maplibre.android.annotations.PolygonOptions
import org.maplibre.android.annotations.Polyline
import org.maplibre.android.annotations.PolylineOptions
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdate
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.gestures.AndroidGesturesManager
import org.maplibre.android.gestures.MoveGestureDetector
import org.maplibre.android.gestures.RotateGestureDetector
import org.maplibre.android.gestures.ShoveGestureDetector
import org.maplibre.android.gestures.StandardScaleGestureDetector
import org.maplibre.android.location.LocationComponent
import org.maplibre.android.location.LocationComponentActivationOptions
import org.maplibre.android.offline.OfflineRegionDefinition
import org.maplibre.android.style.expressions.Expression
import org.maplibre.geojson.Feature
import org.maplibre.geojson.Geometry
import kotlin.math.PI

/**
 * The general class to interact with in the Android MapLibre SDK. It exposes the entry point for all
 * methods related to the MapView. You cannot instantiate [MapLibreMap] object directly, rather,
 * you must obtain one from the getMapAsync() method on a MapFragment or MapView that you have
 * added to your application.
 *
 * Note: Similar to a View object, a MapLibreMap should only be read and modified from the main thread.
 */
@UiThread
@Suppress("TooManyFunctions", "LargeClass", "LongParameterList")
class MapLibreMap internal constructor(
    private val nativeMapView: NativeMap,
    private val transform: Transform,
    /**
     * The user interface settings for the map.
     */
    val uiSettings: UiSettings,
    /**
     * The Projection object that you can use to convert between screen coordinates and
     * latitude/longitude coordinates.
     */
    val projection: Projection,
    private val onGesturesManagerInteractionListener: OnGesturesManagerInteractionListener,
    private val cameraChangeDispatcher: CameraChangeDispatcher,
    private val developerAnimationStartedListeners: List<OnDeveloperAnimationListener>,
) {
    private val awaitingStyleGetters = mutableListOf<Style.OnStyleLoaded>()

    private var styleLoadedCallback: Style.OnStyleLoaded? = null
    private var loadedStyle: Style? = null
    private var fpsChangedListener: OnFpsChangedListener? = null
    private var started = false

    private lateinit var annotationManager: AnnotationManager

    /**
     * The [LocationComponent] that can be used to display user's location on the map.
     *
     * Use [LocationComponent.activateLocationComponent] or any overload to activate the component,
     * then, enable it with [LocationComponent.setLocationComponentEnabled].
     *
     * You can customize the location icon and more with
     * [org.maplibre.android.location.LocationComponentOptions].
     */
    lateinit var locationComponent: LocationComponent
        private set

    /**
     * Whether the map debug information is shown.
     *
     * The default value is false.
     */
    var isDebugActive: Boolean = false
        set(value) {
            field = value
            nativeMapView.setDebug(value)
        }

    /**
     * Trigger the mapview to repaint.
     */
    fun triggerRepaint() {
        nativeMapView.triggerRepaint()
    }

    /**
     * Query rendering statistics overlay status.
     */
    fun isRenderingStatsViewEnabled(): Boolean = nativeMapView.isRenderingStatsViewEnabled()

    /**
     * Enable rendering statistics overlay with [RenderingStats] values.
     */
    fun enableRenderingStatsView(value: Boolean) {
        nativeMapView.enableRenderingStatsView(value)
    }

    /**
     * Frustum offset used to disable rendering of elements at the edge of the screen
     *
     * Offset applied to camera frustum and scissor rectangle. The camrea frustum is modified
     * to avoid loading geometry that's behind UI elements at the top of the screen. The scissor
     * rectangle is used to avoid shading fragments that are behind UI elements at the edges of
     * the screen. All values are in logical pixels.
     */
    fun setFrustumOffset(offset: RectF) {
        nativeMapView.setFrustumOffset(offset)
    }

    fun setSwapBehaviorFlush(flush: Boolean) {
        nativeMapView.setSwapBehaviorFlush(flush)
    }

    internal fun initialise(
        context: Context,
        options: MapLibreMapOptions,
    ) {
        transform.initialise(this, options)
        uiSettings.initialise(context, options)

        // Map configuration
        isDebugActive = options.debugActive
        setApiBaseUrl(options)
        setPrefetchesTiles(options)
    }

    /**
     * Get the Style of the map asynchronously.
     */
    fun getStyle(onStyleLoaded: Style.OnStyleLoaded) {
        val style = loadedStyle
        if (style != null && style.isFullyLoaded) {
            onStyleLoaded.onStyleLoaded(style)
        } else {
            awaitingStyleGetters.add(onStyleLoaded)
        }
    }

    /**
     * The style of the map.
     *
     * Returns null when style is being loaded.
     */
    val style: Style?
        get() = loadedStyle?.takeIf { it.isFullyLoaded }

    /**
     * Called when the hosting Activity/Fragment onStart() method is called.
     */
    internal fun onStart() {
        started = true
        locationComponent.onStart()
    }

    /**
     * Called when the hosting Activity/Fragment onStop() method is called.
     */
    internal fun onStop() {
        started = false
        locationComponent.onStop()
    }

    /**
     * Called when the hosting Activity/Fragment is going to be destroyed and map state needs to be saved.
     *
     * @param outState the bundle to save the state to.
     */
    internal fun onSaveInstanceState(outState: Bundle) {
        outState.putParcelable(MapLibreConstants.STATE_CAMERA_POSITION, transform.cameraPosition)
        outState.putBoolean(MapLibreConstants.STATE_DEBUG_ACTIVE, isDebugActive)
        uiSettings.onSaveInstanceState(outState)
    }

    /**
     * Called when the hosting Activity/Fragment is recreated and map state needs to be restored.
     *
     * @param savedInstanceState the bundle containing the saved state
     */
    @Suppress("DEPRECATION")
    internal fun onRestoreInstanceState(savedInstanceState: Bundle) {
        val cameraPosition: CameraPosition? =
            savedInstanceState.getParcelable(MapLibreConstants.STATE_CAMERA_POSITION)

        uiSettings.onRestoreInstanceState(savedInstanceState)

        if (cameraPosition != null) {
            moveCamera(
                CameraUpdateFactory.newCameraPosition(CameraPosition.Builder(cameraPosition).build()),
            )
        }

        nativeMapView.setDebug(savedInstanceState.getBoolean(MapLibreConstants.STATE_DEBUG_ACTIVE))
    }

    /**
     * Called when the hosting Activity/Fragment onDestroy()/onDestroyView() method is called.
     */
    internal fun onDestroy() {
        locationComponent.onDestroy()
        loadedStyle?.clear()
        cameraChangeDispatcher.onDestroy()
    }

    /**
     * Called before the OnMapReadyCallback is invoked.
     */
    internal fun onPreMapReady() {
        transform.invalidateCameraPosition()
        annotationManager.reloadMarkers()
        annotationManager.adjustTopOffsetPixels(this)
    }

    /**
     * Called when the OnMapReadyCallback has finished executing.
     *
     * Invalidation of the camera position is required to update the added components in
     * OnMapReadyCallback with the correct transformation.
     */
    internal fun onPostMapReady() {
        transform.invalidateCameraPosition()
    }

    /**
     * Called when the map finished loading a style.
     */
    internal fun onFinishLoadingStyle() {
        notifyStyleLoaded()
    }

    /**
     * Called when the map failed loading a style.
     */
    internal fun onFailLoadingStyle() {
        styleLoadedCallback = null
    }

    /**
     * Called when the region is changing or has changed.
     */
    internal fun onUpdateRegionChange() {
        annotationManager.update()
    }

    /**
     * Called when the map frame is fully rendered.
     */
    internal fun onUpdateFullyRendered() {
        transform.invalidateCameraPosition()?.let { uiSettings.update(it) }
    }

    /**
     * Experimental feature. Do not use.
     */
    internal val nativeMapPtr: Long
        get() = nativeMapView.nativePtr

    // Style

    /**
     * Sets tile pre-fetching zoom delta from MapboxOptions.
     *
     * @param options the options object
     */
    @Suppress("DEPRECATION")
    private fun setPrefetchesTiles(options: MapLibreMapOptions) {
        prefetchZoomDelta = if (!options.prefetchesTiles) 0 else options.prefetchZoomDelta
    }

    /**
     * Enable or disable tile pre-fetching. Pre-fetching makes sure that a low-resolution
     * tile is rendered as soon as possible at the expense of a little bandwidth.
     */
    @Deprecated("Use setPrefetchZoomDelta instead.", ReplaceWith("prefetchZoomDelta"))
    var prefetchesTiles: Boolean
        get() = nativeMapView.prefetchTiles
        set(value) {
            nativeMapView.prefetchTiles = value
        }

    /**
     * The tile pre-fetching zoom delta. Pre-fetching makes sure that a low-resolution
     * tile at the (current_zoom_level - delta) is rendered as soon as possible at the
     * expense of a little bandwidth.
     *
     * Note: This operation will override the [MapLibreMapOptions.setPrefetchesTiles]
     *       Setting zoom delta to 0 will disable pre-fetching.
     * Default zoom delta is 4.
     */
    @get:IntRange(from = 0)
    var prefetchZoomDelta: Int
        get() = nativeMapView.prefetchZoomDelta
        set(
            @IntRange(from = 0) value,
        ) {
            nativeMapView.prefetchZoomDelta = value
        }

    /**
     * Indicating whether the map may cache tiles for different zoom levels or not.
     *
     * true causes the map view to consume more memory and have a smoother user
     * experience when zoom in/out. The default value of this property is `true`.
     */
    var tileCacheEnabled: Boolean
        get() = nativeMapView.getTileCacheEnabled()
        set(value) {
            nativeMapView.setTileCacheEnabled(value)
        }

    /**
     * Camera based tile level of detail controls
     *
     * Minimum radius around the view point in unit of tiles in which the fine grained zoom level
     * tiles are always used when performing LOD.
     * The radius must be greater than 1 (At least 1 fine detailed tile is present).
     * A smaller radius value may improve performance at the cost of quality (tiles away from
     * camera use lower Zoom levels)
     */
    var tileLodMinRadius: Double
        get() = nativeMapView.getTileLodMinRadius()
        set(
            @FloatRange(from = 1.0, fromInclusive = true) value,
        ) {
            nativeMapView.setTileLodMinRadius(value)
        }

    /**
     * Camera based tile level of detail controls
     *
     * Scale factor for the distance to the camera view point.
     * A value larger than 1 increases the distance to the camera view point reducing LOD.
     * Larger values may improve performance at the cost of quality (tiles away from camera
     * use lower Zoom levels)
     */
    var tileLodScale: Double
        get() = nativeMapView.getTileLodScale()
        set(
            @FloatRange(from = 0.0, fromInclusive = false) value,
        ) {
            nativeMapView.setTileLodScale(value)
        }

    /**
     * Camera based tile level of detail controls
     *
     * Pitch angle in radians above which LOD calculation is performed.
     * A smaller radius value may improve performance at the cost of quality.
     */
    var tileLodPitchThreshold: Double
        get() = nativeMapView.getTileLodPitchThreshold()
        set(
            @FloatRange(from = 0.0, to = PI) value,
        ) {
            nativeMapView.setTileLodPitchThreshold(value)
        }

    /**
     * Camera based tile level of detail controls
     *
     * Shift applied to the Zoom level during LOD calculation.
     * A negative value shifts the Zoom level to a coarser level reducing quality but
     * improving performance.
     * A positive value shifts the Zoom level to a finer level increasing details but
     * negatively affecting performance.
     * A value of zero (default) does not apply any shift to the Zoom level.
     * It is not recommended to change the default value unless performance is critical
     * and the loss of quality is acceptable. A value of -1 reduces the number of
     * displayed tiles by a factor of 4 on average.
     * It is recommended to first configure the pixelRatio before adjusting
     * TileLodZoomShift. See [MapLibreMapOptions.pixelRatio].
     */
    var tileLodZoomShift: Double
        get() = nativeMapView.getTileLodZoomShift()
        set(value) {
            nativeMapView.setTileLodZoomShift(value)
        }

    //
    // MinZoom
    //

    /**
     * Sets the minimum zoom level the map can be displayed at.
     *
     * @param minZoom The new minimum zoom level.
     */
    fun setMinZoomPreference(
        @FloatRange(
            from = MapLibreConstants.MINIMUM_ZOOM.toDouble(),
            to = MapLibreConstants.MAXIMUM_ZOOM.toDouble(),
        ) minZoom: Double,
    ) {
        transform.minZoom = minZoom
    }

    /**
     * The minimum zoom level the map can be displayed at.
     */
    val minZoomLevel: Double
        get() = transform.minZoom

    //
    // MaxZoom
    //

    /**
     * Sets the maximum zoom level the map can be displayed at.
     *
     * The default maximum zoomn level is 22. The upper bound for this value is 25.5.
     *
     * @param maxZoom The new maximum zoom level.
     */
    fun setMaxZoomPreference(
        @FloatRange(
            from = MapLibreConstants.MINIMUM_ZOOM.toDouble(),
            to = MapLibreConstants.MAXIMUM_ZOOM.toDouble(),
        ) maxZoom: Double,
    ) {
        transform.maxZoom = maxZoom
    }

    /**
     * The maximum zoom level the map can be displayed at.
     */
    val maxZoomLevel: Double
        get() = transform.maxZoom

    //
    // MinPitch
    //

    /**
     * Sets the minimum Pitch the map can be displayed at.
     *
     * The default and lower bound for minPitch Pitch is 0.
     *
     * @param minPitch The new minimum Pitch.
     */
    fun setMinPitchPreference(
        @FloatRange(
            from = MapLibreConstants.MINIMUM_PITCH.toDouble(),
            to = MapLibreConstants.MAXIMUM_PITCH.toDouble(),
        ) minPitch: Double,
    ) {
        transform.minPitch = minPitch
    }

    /**
     * The minimum Pitch the map can be displayed at.
     */
    val minPitch: Double
        get() = transform.minPitch

    //
    // MaxPitch
    //

    /**
     * Sets the maximum Pitch the map can be displayed at.
     *
     * The default and upper bound for maximum Pitch is 60.
     *
     * @param maxPitch The new maximum Pitch.
     */
    fun setMaxPitchPreference(
        @FloatRange(
            from = MapLibreConstants.MINIMUM_PITCH.toDouble(),
            to = MapLibreConstants.MAXIMUM_PITCH.toDouble(),
        ) maxPitch: Double,
    ) {
        transform.maxPitch = maxPitch
    }

    /**
     * The maximum Pitch the map can be displayed at.
     */
    val maxPitch: Double
        get() = transform.maxPitch

    //
    // Camera API
    //

    /**
     * Cancels ongoing animations.
     *
     * This invokes the [CancelableCallback] for ongoing camera updates.
     */
    fun cancelTransitions() {
        transform.cancelTransitions()
    }

    /**
     * The current position of the camera.
     *
     * The CameraPosition returned by the getter is a snapshot of the current position, and will not
     * automatically update when the camera moves.
     *
     * Setting a camera position repositions the camera; the move is instantaneous, and a subsequent
     * read will reflect the new position. See CameraUpdateFactory for a set of updates.
     */
    var cameraPosition: CameraPosition
        get() = transform.cameraPosition!!
        set(value) {
            moveCamera(CameraUpdateFactory.newCameraPosition(value), null)
        }

    /**
     * Repositions the camera according to the instructions defined in the update.
     * The move is instantaneous, and a subsequent [cameraPosition] read will reflect the new position.
     * See CameraUpdateFactory for a set of updates.
     *
     * @param update   The change that should be applied to the camera
     * @param callback the callback to be invoked when an animation finishes or is canceled
     */
    @JvmOverloads
    fun moveCamera(
        update: CameraUpdate,
        callback: CancelableCallback? = null,
    ) {
        notifyDeveloperAnimationListeners()
        transform.moveCamera(this, update, callback)
    }

    /**
     * Gradually move the camera by the default duration, zoom will not be affected unless specified
     * within [CameraUpdate]. If [cameraPosition] is read during the animation,
     * it will return the current location of the camera in flight.
     *
     * @param update   The change that should be applied to the camera.
     * @param callback An optional callback to be notified from the main thread when the animation
     *                 stops. If the animation stops due to its natural completion, the callback
     *                 will be notified with onFinish(). If the animation stops due to interruption
     *                 by a later camera movement or a user gesture, onCancel() will be called.
     *                 Do not update or ease the camera from within onCancel().
     * @see CameraUpdateFactory for a set of updates.
     */
    @JvmOverloads
    fun easeCamera(
        update: CameraUpdate,
        callback: CancelableCallback? = null,
    ) {
        easeCamera(update, MapLibreConstants.ANIMATION_DURATION, callback)
    }

    /**
     * Gradually move the camera by a specified duration in milliseconds, zoom will not be affected
     * unless specified within [CameraUpdate]. A callback can be used to be notified when
     * easing the camera stops. If [cameraPosition] is read during the animation, it
     * will return the current location of the camera in flight.
     *
     * Note that this will cancel location tracking mode if enabled.
     *
     * @param update     The change that should be applied to the camera.
     * @param durationMs The duration of the animation in milliseconds. This must be strictly
     *                   positive, otherwise an IllegalArgumentException will be thrown.
     * @param callback   An optional callback to be notified from the main thread when the animation
     *                   stops. If the animation stops due to its natural completion, the callback
     *                   will be notified with onFinish(). If the animation stops due to interruption
     *                   by a later camera movement or a user gesture, onCancel() will be called.
     *                   Do not update or ease the camera from within onCancel().
     * @see CameraUpdateFactory for a set of updates.
     */
    @JvmOverloads
    fun easeCamera(
        update: CameraUpdate,
        durationMs: Int,
        callback: CancelableCallback? = null,
    ) {
        easeCamera(update, durationMs, true, callback)
    }

    /**
     * Gradually move the camera by a specified duration in milliseconds, zoom will not be affected
     * unless specified within [CameraUpdate]. A callback can be used to be notified when
     * easing the camera stops. If [cameraPosition] is read during the animation, it
     * will return the current location of the camera in flight.
     *
     * @param update             The change that should be applied to the camera.
     * @param durationMs         The duration of the animation in milliseconds. This must be strictly
     *                           positive, otherwise an IllegalArgumentException will be thrown.
     * @param easingInterpolator True for easing interpolator, false for linear.
     * @param callback           An optional callback to be notified from the main thread when the animation
     *                           stops. If the animation stops due to its natural completion, the callback
     *                           will be notified with onFinish(). If the animation stops due to interruption
     *                           by a later camera movement or a user gesture, onCancel() will be called.
     *                           Do not update or ease the camera from within onCancel().
     */
    @JvmOverloads
    fun easeCamera(
        update: CameraUpdate,
        durationMs: Int,
        easingInterpolator: Boolean,
        callback: CancelableCallback? = null,
    ) {
        require(durationMs > 0) { "Null duration passed into easeCamera" }
        notifyDeveloperAnimationListeners()
        transform.easeCamera(this, update, durationMs, easingInterpolator, callback)
    }

    /**
     * Animate the camera to a new location defined within [CameraUpdate] using a transition
     * animation that evokes powered flight. The animation will last the default amount of time. A
     * callback can be used to be notified when animating the camera stops. During the animation, a
     * read of [cameraPosition] returns an intermediate location of the camera in flight.
     *
     * @param update   The change that should be applied to the camera.
     * @param callback The callback to invoke from the main thread when the animation stops. If the
     *                 animation completes normally, onFinish() is called; otherwise, onCancel() is
     *                 called. Do not update or animate the camera from within onCancel().
     * @see CameraUpdateFactory for a set of updates.
     */
    @JvmOverloads
    fun animateCamera(
        update: CameraUpdate,
        callback: CancelableCallback? = null,
    ) {
        animateCamera(update, MapLibreConstants.ANIMATION_DURATION, callback)
    }

    /**
     * Animate the camera to a new location defined within [CameraUpdate] using a transition
     * animation that evokes powered flight. The animation will last a specified amount of time
     * given in milliseconds. A callback can be used to be notified when animating the camera stops.
     * During the animation, a read of [cameraPosition] returns an intermediate location
     * of the camera in flight.
     *
     * @param update     The change that should be applied to the camera.
     * @param durationMs The duration of the animation in milliseconds. This must be strictly
     *                   positive, otherwise an IllegalArgumentException will be thrown.
     * @param callback   An optional callback to be notified from the main thread when the animation
     *                   stops. If the animation stops due to its natural completion, the callback
     *                   will be notified with onFinish(). If the animation stops due to interruption
     *                   by a later camera movement or a user gesture, onCancel() will be called.
     *                   Do not update or animate the camera from within onCancel(). If a callback
     *                   isn't required, leave it as null.
     * @see CameraUpdateFactory for a set of updates.
     */
    @JvmOverloads
    fun animateCamera(
        update: CameraUpdate,
        durationMs: Int,
        callback: CancelableCallback? = null,
    ) {
        require(durationMs > 0) { "Null duration passed into animateCamera" }
        notifyDeveloperAnimationListeners()
        transform.animateCamera(this, update, durationMs, callback)
    }

    /**
     * Scrolls the camera over the map, shifting the center of view by the specified number of pixels in the x and y
     * directions.
     *
     * @param x        Amount of pixels to scroll to in x direction
     * @param y        Amount of pixels to scroll to in y direction
     * @param duration Amount of time the scrolling should take
     */
    @JvmOverloads
    fun scrollBy(
        x: Float,
        y: Float,
        duration: Long = 0,
    ) {
        notifyDeveloperAnimationListeners()
        nativeMapView.moveBy(x.toDouble(), y.toDouble(), duration)
    }

    /**
     * The current zoom level.
     */
    val zoom: Double
        get() = nativeMapView.zoom

    /**
     * Zooms the camera to the specified level.
     *
     * @param zoom       The zoom level to which the camera should move.
     * @param focalPoint The point around which to zoom.
     * @param duration   The duration for the zoom animation
     */
    fun setZoom(
        zoom: Double,
        focalPoint: PointF,
        duration: Long,
    ) {
        notifyDeveloperAnimationListeners()
        nativeMapView.setZoom(zoom, focalPoint, duration)
    }

    //
    //  Reset North
    //

    /**
     * Resets the map view to face north.
     */
    fun resetNorth() {
        notifyDeveloperAnimationListeners()
        transform.resetNorth()
    }

    /**
     * Transform the map bearing given a bearing, focal point coordinates, and a duration.
     *
     * @param bearing  The bearing of the Map to be transformed to
     * @param focalX   The x coordinate of the focal point
     * @param focalY   The y coordinate of the focal point
     * @param duration The duration of the transformation
     */
    fun setFocalBearing(
        bearing: Double,
        focalX: Float,
        focalY: Float,
        duration: Long,
    ) {
        notifyDeveloperAnimationListeners()
        transform.setBearing(bearing, focalX, focalY, duration)
    }

    /**
     * The measured height of the Map.
     */
    val height: Float
        get() = projection.getHeight()

    /**
     * The measured width of the Map.
     */
    val width: Float
        get() = projection.getWidth()

    //
    // Offline
    //

    /**
     * Loads a new style from the specified offline region definition and moves the map camera to that region.
     *
     * @param definition the offline region definition
     * @param callback   the callback to be invoked when the style has loaded
     * @see OfflineRegionDefinition
     */
    @JvmOverloads
    fun setOfflineRegionDefinition(
        definition: OfflineRegionDefinition,
        callback: Style.OnStyleLoaded? = null,
    ) {
        val minZoom = definition.minZoom
        val maxZoom = definition.maxZoom
        val cameraPosition =
            CameraPosition
                .Builder()
                .target(definition.bounds!!.center)
                .zoom(minZoom)
                .build()
        moveCamera(CameraUpdateFactory.newCameraPosition(cameraPosition))
        setMinZoomPreference(minZoom)
        setMaxZoomPreference(maxZoom)
        setStyle(Style.Builder().fromUri(definition.styleURL!!), callback)
    }

    //
    // Debug
    //

    /**
     * Cycles through the map debug options.
     *
     * The value of [isDebugActive] reflects whether there are
     * any map debug options enabled or disabled.
     */
    @Deprecated("Use isDebugActive instead", ReplaceWith("isDebugActive"))
    fun cycleDebugOptions() {
        isDebugActive = !nativeMapView.getDebug()
    }

    /**
     * The list of action journal log files from oldest to newest.
     */
    val actionJournalLogFiles: Array<String>
        get() = nativeMapView.getActionJournalLogFiles()

    /**
     * The action journal events from oldest to newest.
     *
     * Each element contains a serialized json object with the event data.
     * Example
     * ```
     * {
     *   "name" : "onTileAction",
     *   "time" : "2025-04-17T13:13:13.974Z",
     *   "styleName" : "Streets",
     *   "styleURL" : "maptiler://maps/streets",
     *   "event" : {
     *     "action" : "RequestedFromNetwork",
     *     "tileX" : 0,
     *     "tileY" : 0,
     *     "tileZ" : 0,
     *     "overscaledZ" : 0,
     *     "sourceID" : "openmaptiles"
     *   }
     * }
     * ```
     */
    val actionJournalLog: Array<String>
        get() = nativeMapView.getActionJournalLog()

    /**
     * Clear stored action journal events.
     */
    fun clearActionJournalLog() {
        nativeMapView.clearActionJournalLog()
    }

    //
    // API endpoint config
    //

    @Suppress("DEPRECATION")
    private fun setApiBaseUrl(options: MapLibreMapOptions) {
        val apiBaseUrl = options.apiBaseUrl
        if (!TextUtils.isEmpty(apiBaseUrl)) {
            nativeMapView.setApiBaseUrl(apiBaseUrl!!)
        }
    }

    //
    // Styling
    //

    /**
     * Loads a new map style from the specified bundled style.
     *
     * If the style fails to load or an invalid style URL is set, the map view will become blank.
     * An error message will be logged in the Android logcat and
     * [MapView.OnDidFailLoadingMapListener] callback will be triggered.
     *
     * @param style    The bundled style
     * @param callback The callback to be invoked when the style has loaded
     * @see Style
     */
    @JvmOverloads
    fun setStyle(
        style: String,
        callback: Style.OnStyleLoaded? = null,
    ) {
        setStyle(Style.Builder().fromUri(style), callback)
    }

    /**
     * Loads a new map style from the specified builder.
     *
     * If the builder fails to load, the map view will become blank. An error message will be logged
     * in the Android logcat and [MapView.OnDidFailLoadingMapListener] callback will be triggered.
     *
     * @param builder  The style builder
     * @param callback The callback to be invoked when the style has loaded
     * @see Style
     */
    @JvmOverloads
    fun setStyle(
        builder: Style.Builder,
        callback: Style.OnStyleLoaded? = null,
    ) {
        styleLoadedCallback = callback
        locationComponent.onStartLoadingMap()
        loadedStyle?.clear()

        loadedStyle = builder.build(nativeMapView)
        val uri = builder.uri
        val json = builder.json
        if (!TextUtils.isEmpty(uri)) {
            nativeMapView.styleUri = uri!!
        } else if (!TextUtils.isEmpty(json)) {
            nativeMapView.styleJson = json!!
        } else {
            // user didn't provide a `from` component, load a blank style instead
            nativeMapView.styleJson = Style.EMPTY_JSON
        }
    }

    internal fun notifyStyleLoaded() {
        if (nativeMapView.isDestroyed) {
            return
        }

        val style = loadedStyle
        if (style != null) {
            style.onDidFinishLoadingStyle()
            locationComponent.onFinishLoadingStyle()

            // notify the listener provided with the style setter
            styleLoadedCallback?.onStyleLoaded(style)

            // notify style getters
            for (styleGetter in awaitingStyleGetters) {
                styleGetter.onStyleLoaded(style)
            }
        } else {
            MapStrictMode.strictModeViolation("No style to provide.")
        }
        styleLoadedCallback = null
        awaitingStyleGetters.clear()
    }

    //
    // Annotations
    //

    /**
     * Adds a marker to this map.
     *
     * The marker's icon is rendered on the map at the location `Marker.position`.
     * If `Marker.title` is defined, the map shows an info box with the marker's title and snippet.
     *
     * @param markerOptions A marker options object that defines how to render the marker
     * @return The `Marker` that was added to the map
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addMarker(markerOptions: MarkerOptions): Marker = annotationManager.addMarker(markerOptions, this)

    /**
     * Adds a marker to this map.
     *
     * The marker's icon is rendered on the map at the location `Marker.position`.
     * If `Marker.title` is defined, the map shows an info box with the marker's title and snippet.
     *
     * @param markerOptions A marker options object that defines how to render the marker
     * @return The `Marker` that was added to the map
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addMarker(markerOptions: BaseMarkerOptions<*, *>): Marker = annotationManager.addMarker(markerOptions, this)

    /**
     * Adds multiple markers to this map.
     *
     * The marker's icon is rendered on the map at the location `Marker.position`.
     * If `Marker.title` is defined, the map shows an info box with the marker's title and snippet.
     *
     * @param markerOptionsList A list of marker options objects that defines how to render the markers
     * @return A list of the `Marker`s that were added to the map
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addMarkers(markerOptionsList: List<BaseMarkerOptions<*, *>>): List<Marker> = annotationManager.addMarkers(markerOptionsList, this)

    /**
     * Updates a marker on this map. Does nothing if the marker isn't already added.
     *
     * @param updatedMarker An updated marker object
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun updateMarker(updatedMarker: Marker) {
        annotationManager.updateMarker(updatedMarker, this)
    }

    /**
     * Adds a polyline to this map.
     *
     * @param polylineOptions A polyline options object that defines how to render the polyline
     * @return The `Polyline` that was added to the map
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addPolyline(polylineOptions: PolylineOptions): Polyline = annotationManager.addPolyline(polylineOptions, this)

    /**
     * Adds multiple polylines to this map.
     *
     * @param polylineOptionsList A list of polyline options objects that defines how to render the polylines.
     * @return A list of the `Polyline`s that were added to the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addPolylines(polylineOptionsList: List<PolylineOptions>): List<Polyline> = annotationManager.addPolylines(polylineOptionsList, this)

    /**
     * Update a polyline on this map.
     *
     * @param polyline An updated polyline object.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun updatePolyline(polyline: Polyline) {
        annotationManager.updatePolyline(polyline)
    }

    /**
     * Adds a polygon to this map.
     *
     * @param polygonOptions A polygon options object that defines how to render the polygon.
     * @return The `Polygon` that was added to the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addPolygon(polygonOptions: PolygonOptions): Polygon = annotationManager.addPolygon(polygonOptions, this)

    /**
     * Adds multiple polygons to this map.
     *
     * @param polygonOptionsList A list of polygon options objects that defines how to render the polygons
     * @return A list of the `Polygon`s that were added to the map
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun addPolygons(polygonOptionsList: List<PolygonOptions>): List<Polygon> = annotationManager.addPolygons(polygonOptionsList, this)

    /**
     * Update a polygon on this map.
     *
     * @param polygon An updated polygon object
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun updatePolygon(polygon: Polygon) {
        annotationManager.updatePolygon(polygon)
    }

    /**
     * Convenience method for removing a Marker from the map.
     *
     * Calls removeAnnotation() internally.
     *
     * @param marker Marker to remove
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removeMarker(marker: Marker) {
        annotationManager.removeAnnotation(marker)
    }

    /**
     * Convenience method for removing a Polyline from the map.
     *
     * Calls removeAnnotation() internally.
     *
     * @param polyline Polyline to remove
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removePolyline(polyline: Polyline) {
        annotationManager.removeAnnotation(polyline)
    }

    /**
     * Convenience method for removing a Polygon from the map.
     *
     * Calls removeAnnotation() internally.
     *
     * @param polygon Polygon to remove
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removePolygon(polygon: Polygon) {
        annotationManager.removeAnnotation(polygon)
    }

    /**
     * Removes an annotation from the map.
     *
     * @param annotation The annotation object to remove.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removeAnnotation(annotation: Annotation) {
        annotationManager.removeAnnotation(annotation)
    }

    /**
     * Removes an annotation from the map
     *
     * @param id The identifier associated to the annotation to be removed
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removeAnnotation(id: Long) {
        annotationManager.removeAnnotation(id)
    }

    /**
     * Removes multiple annotations from the map.
     *
     * @param annotationList A list of annotation objects to remove.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removeAnnotations(annotationList: List<Annotation>) {
        annotationManager.removeAnnotations(annotationList)
    }

    /**
     * Removes all annotations from the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun removeAnnotations() {
        annotationManager.removeAnnotations()
    }

    /**
     * Removes all markers, polylines, polygons, overlays, etc from the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun clear() {
        annotationManager.removeAnnotations()
    }

    /**
     * Return a annotation based on its id.
     *
     * @param id the id used to look up an annotation
     * @return An annotation with a matched id, null is returned if no match was found
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun getAnnotation(id: Long): Annotation? = annotationManager.getAnnotation(id)

    /**
     * A list of all the annotations on the map. The returned object is a copy so modifying this
     * list will not update the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    val annotations: List<Annotation>
        get() = annotationManager.annotations

    /**
     * A list of all the markers on the map. The returned object is a copy so modifying this
     * list will not update the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    val markers: List<Marker>
        get() = annotationManager.getMarkers()

    /**
     * A list of all the polygons on the map. The returned object is a copy so modifying this
     * list will not update the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    val polygons: List<Polygon>
        get() = annotationManager.getPolygons()

    /**
     * A list of all the polylines on the map. The returned object is a copy so modifying this
     * list will not update the map.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    val polylines: List<Polyline>
        get() = annotationManager.getPolylines()

    /**
     * Sets a callback that's invoked when the user clicks on a marker.
     *
     * @param listener The callback that's invoked when the user clicks on a marker.
     *                 To unset the callback, use null.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun setOnMarkerClickListener(listener: OnMarkerClickListener?) {
        annotationManager.setOnMarkerClickListener(listener)
    }

    /**
     * Sets a callback that's invoked when the user clicks on a polygon.
     *
     * @param listener The callback that's invoked when the user clicks on a polygon.
     *                 To unset the callback, use null.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun setOnPolygonClickListener(listener: OnPolygonClickListener?) {
        annotationManager.setOnPolygonClickListener(listener)
    }

    /**
     * Sets a callback that's invoked when the user clicks on a polyline.
     *
     * @param listener The callback that's invoked when the user clicks on a polyline.
     *                 To unset the callback, use null.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun setOnPolylineClickListener(listener: OnPolylineClickListener?) {
        annotationManager.setOnPolylineClickListener(listener)
    }

    /**
     * Selects a marker. The selected marker will have it's info window opened.
     * Any other open info windows will be closed unless [isAllowConcurrentMultipleOpenInfoWindows]
     * is true.
     *
     * Selecting an already selected marker will have no effect.
     *
     * @param marker The marker to select.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun selectMarker(marker: Marker) {
        annotationManager.selectMarker(marker)
    }

    /**
     * Deselects any currently selected marker. All markers will have it's info window closed.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun deselectMarkers() {
        annotationManager.deselectMarkers()
    }

    /**
     * Deselects a currently selected marker. The selected marker will have it's info window closed.
     *
     * @param marker the marker to deselect
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun deselectMarker(marker: Marker) {
        annotationManager.deselectMarker(marker)
    }

    /**
     * The currently selected markers.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    val selectedMarkers: List<Marker>
        get() = annotationManager.getSelectedMarkers()

    //
    // InfoWindow
    //

    /**
     * A custom renderer for the contents of info window.
     *
     * When set your callback is invoked when an info window is about to be shown. By returning
     * a custom [View], the default info window will be replaced. To unset the callback, use null.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    var infoWindowAdapter: InfoWindowAdapter?
        get() = annotationManager.getInfoWindowManager().getInfoWindowAdapter()
        set(value) {
            annotationManager.getInfoWindowManager().setInfoWindowAdapter(value)
        }

    /**
     * Whether the map allows concurrent multiple infowindows to be shown.
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    var isAllowConcurrentMultipleOpenInfoWindows: Boolean
        get() = annotationManager.getInfoWindowManager().isAllowConcurrentMultipleOpenInfoWindows()
        set(value) {
            annotationManager.getInfoWindowManager().setAllowConcurrentMultipleOpenInfoWindows(value)
        }

    //
    // LatLngBounds
    //

    /**
     * Sets a LatLngBounds that constraints map transformations to this bounds.
     *
     * Set to null to clear current bounds, newly set bounds will override previously set bounds.
     *
     * @param latLngBounds the bounds to constrain the map with
     */
    fun setLatLngBoundsForCameraTarget(latLngBounds: LatLngBounds?) {
        nativeMapView.setLatLngBounds(latLngBounds)
    }

    /**
     * Get a camera position that fits a provided bounds and padding and the current camera tilt and bearing.
     *
     * @param latLngBounds the bounds to set the map with
     * @param padding      the padding to apply to the bounds (in left, top, right, bottom order)
     * @return the camera position that fits the bounds and padding
     */
    @JvmOverloads
    fun getCameraForLatLngBounds(
        latLngBounds: LatLngBounds,
        @Size(value = 4) padding: IntArray = intArrayOf(0, 0, 0, 0),
    ): CameraPosition? =
        // we use current camera tilt/bearing value to provide expected transformations as #11993
        getCameraForLatLngBounds(latLngBounds, padding, transform.getRawBearing(), transform.getTilt())

    /**
     * Get a camera position that fits a provided bounds, bearing and tilt.
     *
     * @param latLngBounds the bounds to set the map with
     * @param bearing      the bearing to transform the camera position with
     * @param tilt         to transform the camera position with
     * @return the camera position that fits the bounds and given bearing and tilt
     */
    fun getCameraForLatLngBounds(
        latLngBounds: LatLngBounds,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_DIRECTION,
            to = MapLibreConstants.MAXIMUM_DIRECTION,
        ) bearing: Double,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_TILT,
            to = MapLibreConstants.MAXIMUM_TILT,
        ) tilt: Double,
    ): CameraPosition? = getCameraForLatLngBounds(latLngBounds, intArrayOf(0, 0, 0, 0), bearing, tilt)

    /**
     * Get a camera position that fits a provided bounds, padding, bearing and tilt.
     *
     * @param latLngBounds the bounds to set the map with
     * @param padding      the padding to apply to the bounds (in left, top, right, bottom order)
     * @param bearing      the bearing to transform the camera position with
     * @param tilt         to transform the camera position with
     * @return the camera position that fits the bounds, bearing and tilt
     */
    fun getCameraForLatLngBounds(
        latLngBounds: LatLngBounds,
        @Size(value = 4) padding: IntArray,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_DIRECTION,
            to = MapLibreConstants.MAXIMUM_DIRECTION,
        ) bearing: Double,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_TILT,
            to = MapLibreConstants.MAXIMUM_TILT,
        ) tilt: Double,
    ): CameraPosition? = nativeMapView.getCameraForLatLngBounds(latLngBounds, padding, bearing, tilt)

    /**
     * Get a camera position that fits a provided shape and padding.
     *
     * @param geometry the geometry to wraps the map with
     * @param padding  the padding to apply to the bounds
     * @return the camera position that fits the geometry inside and padding
     */
    @JvmOverloads
    fun getCameraForGeometry(
        geometry: Geometry,
        @Size(value = 4) padding: IntArray = intArrayOf(0, 0, 0, 0),
    ): CameraPosition? =
        // we use current camera tilt/bearing value to provide expected transformations as #11993
        getCameraForGeometry(geometry, padding, transform.getBearing(), transform.getTilt())

    /**
     * Get a camera position that fits a provided shape with a given bearing and tilt.
     *
     * @param geometry the geometry to wraps the map with
     * @param bearing  the bearing at which to compute the geometry's bounds
     * @param tilt     the tilt at which to compute the geometry's bounds
     * @return the camera position that the geometry inside with bearing and tilt
     */
    fun getCameraForGeometry(
        geometry: Geometry,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_DIRECTION,
            to = MapLibreConstants.MAXIMUM_DIRECTION,
        ) bearing: Double,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_TILT,
            to = MapLibreConstants.MAXIMUM_TILT,
        ) tilt: Double,
    ): CameraPosition? = getCameraForGeometry(geometry, intArrayOf(0, 0, 0, 0), bearing, tilt)

    /**
     * Get a camera position that fits a provided shape with a given padding, bearing and tilt.
     *
     * @param geometry the geometry to wraps the map with
     * @param padding  the padding to apply to the bounds
     * @param bearing  the bearing at which to compute the geometry's bounds
     * @param tilt     the tilt at which to compute the geometry's bounds
     * @return the camera position that fits the geometry inside with padding, bearing and tilt
     */
    fun getCameraForGeometry(
        geometry: Geometry,
        @Size(value = 4) padding: IntArray,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_DIRECTION,
            to = MapLibreConstants.MAXIMUM_DIRECTION,
        ) bearing: Double,
        @FloatRange(
            from = MapLibreConstants.MINIMUM_TILT,
            to = MapLibreConstants.MAXIMUM_TILT,
        ) tilt: Double,
    ): CameraPosition? = nativeMapView.getCameraForGeometry(geometry, padding, bearing, tilt)

    //
    // Padding
    //

    /**
     * Sets the distance from the edges of the map view's frame to the edges of the map
     * view's logical viewport.
     *
     * When the value of this property is equal to {0,0,0,0}, viewport
     * properties such as 'centerCoordinate' assume a viewport that matches the map
     * view's frame. Otherwise, those properties are inset, excluding part of the
     * frame from the viewport. For instance, if the only the top edge is inset, the
     * map center is effectively shifted downward.
     *
     * This method sets the padding "lazily".
     * This means that the **padding is going to be applied with the next camera transformation.**
     * To apply the padding immediately use [CameraPosition.Builder.padding]
     * or [CameraUpdateFactory.paddingTo].
     *
     * @param left   The left margin in pixels.
     * @param top    The top margin in pixels.
     * @param right  The right margin in pixels.
     * @param bottom The bottom margin in pixels.
     */
    @Deprecated("Use CameraPosition.Builder#padding or CameraUpdateFactory#paddingTo instead.")
    fun setPadding(
        left: Int,
        top: Int,
        right: Int,
        bottom: Int,
    ) {
        // TODO padding should be passed as doubles
        projection.contentPadding = intArrayOf(left, top, right, bottom)
        uiSettings.invalidate()
    }

    /**
     * The current configured content padding on map view. This might return the currently visible
     * padding or the padding cached but not yet applied by [setPadding].
     *
     * An array with length 4 in the LTRB order.
     */
    @Deprecated("Use CameraPosition.padding instead.")
    val padding: IntArray
        // TODO this should return double[] (semver major change)
        get() = projection.contentPadding

    //
    // Map events
    //

    /**
     * Adds a callback that is invoked when camera movement has ended.
     *
     * @param listener the listener to notify
     */
    fun addOnCameraIdleListener(listener: OnCameraIdleListener) {
        cameraChangeDispatcher.addOnCameraIdleListener(listener)
    }

    /**
     * Removes a callback that is invoked when camera movement has ended.
     *
     * @param listener the listener to remove
     */
    fun removeOnCameraIdleListener(listener: OnCameraIdleListener) {
        cameraChangeDispatcher.removeOnCameraIdleListener(listener)
    }

    /**
     * Adds a callback that is invoked when camera movement was cancelled.
     *
     * @param listener the listener to notify
     */
    fun addOnCameraMoveCancelListener(listener: OnCameraMoveCanceledListener) {
        cameraChangeDispatcher.addOnCameraMoveCancelListener(listener)
    }

    /**
     * Removes a callback that is invoked when camera movement was cancelled.
     *
     * @param listener the listener to remove
     */
    fun removeOnCameraMoveCancelListener(listener: OnCameraMoveCanceledListener) {
        cameraChangeDispatcher.removeOnCameraMoveCancelListener(listener)
    }

    /**
     * Adds a callback that is invoked when camera movement has started.
     *
     * @param listener the listener to notify
     */
    fun addOnCameraMoveStartedListener(listener: OnCameraMoveStartedListener) {
        cameraChangeDispatcher.addOnCameraMoveStartedListener(listener)
    }

    /**
     * Removes a callback that is invoked when camera movement has started.
     *
     * @param listener the listener to remove
     */
    fun removeOnCameraMoveStartedListener(listener: OnCameraMoveStartedListener) {
        cameraChangeDispatcher.removeOnCameraMoveStartedListener(listener)
    }

    /**
     * Adds a callback that is invoked when camera position changes.
     *
     * @param listener the listener to notify
     */
    fun addOnCameraMoveListener(listener: OnCameraMoveListener) {
        cameraChangeDispatcher.addOnCameraMoveListener(listener)
    }

    /**
     * Removes a callback that is invoked when camera position changes.
     *
     * @param listener the listener to remove
     */
    fun removeOnCameraMoveListener(listener: OnCameraMoveListener) {
        cameraChangeDispatcher.removeOnCameraMoveListener(listener)
    }

    /**
     * A callback that's invoked on every frame rendered to the map view.
     *
     * Set to null to unset the callback.
     */
    var onFpsChangedListener: OnFpsChangedListener?
        get() = fpsChangedListener
        set(value) {
            fpsChangedListener = value
            nativeMapView.setOnFpsChangedListener(value)
        }

    /**
     * Adds a callback that's invoked when the map is flinged.
     *
     * @param listener The callback that's invoked when the map is flinged.
     */
    fun addOnFlingListener(listener: OnFlingListener) {
        onGesturesManagerInteractionListener.onAddFlingListener(listener)
    }

    /**
     * Removes a callback that's invoked when the map is flinged.
     *
     * @param listener The callback that's invoked when the map is flinged.
     */
    fun removeOnFlingListener(listener: OnFlingListener) {
        onGesturesManagerInteractionListener.onRemoveFlingListener(listener)
    }

    /**
     * Adds a callback that's invoked when the map is moved.
     *
     * @param listener The callback that's invoked when the map is moved.
     */
    fun addOnMoveListener(listener: OnMoveListener) {
        onGesturesManagerInteractionListener.onAddMoveListener(listener)
    }

    /**
     * Removes a callback that's invoked when the map is moved.
     *
     * @param listener The callback that's invoked when the map is moved.
     */
    fun removeOnMoveListener(listener: OnMoveListener) {
        onGesturesManagerInteractionListener.onRemoveMoveListener(listener)
    }

    /**
     * Adds a callback that's invoked when the map is rotated.
     *
     * @param listener The callback that's invoked when the map is rotated.
     */
    fun addOnRotateListener(listener: OnRotateListener) {
        onGesturesManagerInteractionListener.onAddRotateListener(listener)
    }

    /**
     * Removes a callback that's invoked when the map is rotated.
     *
     * @param listener The callback that's invoked when the map is rotated.
     */
    fun removeOnRotateListener(listener: OnRotateListener) {
        onGesturesManagerInteractionListener.onRemoveRotateListener(listener)
    }

    /**
     * Adds a callback that's invoked when the map is scaled.
     *
     * @param listener The callback that's invoked when the map is scaled.
     */
    fun addOnScaleListener(listener: OnScaleListener) {
        onGesturesManagerInteractionListener.onAddScaleListener(listener)
    }

    /**
     * Removes a callback that's invoked when the map is scaled.
     *
     * @param listener The callback that's invoked when the map is scaled.
     */
    fun removeOnScaleListener(listener: OnScaleListener) {
        onGesturesManagerInteractionListener.onRemoveScaleListener(listener)
    }

    /**
     * Adds a callback that's invoked when the map is tilted.
     *
     * @param listener The callback that's invoked when the map is tilted.
     */
    fun addOnShoveListener(listener: OnShoveListener) {
        onGesturesManagerInteractionListener.onAddShoveListener(listener)
    }

    /**
     * Remove a callback that's invoked when the map is tilted.
     *
     * @param listener The callback that's invoked when the map is tilted.
     */
    fun removeOnShoveListener(listener: OnShoveListener) {
        onGesturesManagerInteractionListener.onRemoveShoveListener(listener)
    }

    /**
     * Sets a custom [AndroidGesturesManager] to handle [android.view.MotionEvent]s
     * registered by the [MapView].
     *
     * @param androidGesturesManager       Gestures manager that interprets gestures based on the motion events.
     * @param attachDefaultListeners       If true, pre-defined listeners will be attach
     *                                     to change map based on [AndroidGesturesManager] callbacks.
     * @param setDefaultMutuallyExclusives If true, pre-defined mutually exclusive gesture sets
     *                                     will be added to the passed gestures manager.
     * @see [mapbox-gestures-android library](https://github.com/mapbox/mapbox-gestures-android)
     */
    fun setGesturesManager(
        androidGesturesManager: AndroidGesturesManager,
        attachDefaultListeners: Boolean,
        setDefaultMutuallyExclusives: Boolean,
    ) {
        onGesturesManagerInteractionListener.setGesturesManager(
            androidGesturesManager,
            attachDefaultListeners,
            setDefaultMutuallyExclusives,
        )
    }

    /**
     * Current [AndroidGesturesManager] that handles [android.view.MotionEvent]s
     * registered by the [MapView].
     */
    val gesturesManager: AndroidGesturesManager
        get() = onGesturesManagerInteractionListener.getGesturesManager()

    /**
     * Interrupts any ongoing gesture velocity animations.
     */
    fun cancelAllVelocityAnimations() {
        onGesturesManagerInteractionListener.cancelAllVelocityAnimations()
    }

    /**
     * Adds a callback that's invoked when the user clicks on the map view.
     *
     * @param listener The callback that's invoked when the user clicks on the map view.
     */
    fun addOnMapClickListener(listener: OnMapClickListener) {
        onGesturesManagerInteractionListener.onAddMapClickListener(listener)
    }

    /**
     * Removes a callback that's invoked when the user clicks on the map view.
     *
     * @param listener The callback that's invoked when the user clicks on the map view.
     */
    fun removeOnMapClickListener(listener: OnMapClickListener) {
        onGesturesManagerInteractionListener.onRemoveMapClickListener(listener)
    }

    /**
     * Adds a callback that's invoked when the user long clicks on the map view.
     *
     * @param listener The callback that's invoked when the user long clicks on the map view.
     */
    fun addOnMapLongClickListener(listener: OnMapLongClickListener) {
        onGesturesManagerInteractionListener.onAddMapLongClickListener(listener)
    }

    /**
     * Removes a callback that's invoked when the user long clicks on the map view.
     *
     * @param listener The callback that's invoked when the user long clicks on the map view.
     */
    fun removeOnMapLongClickListener(listener: OnMapLongClickListener) {
        onGesturesManagerInteractionListener.onRemoveMapLongClickListener(listener)
    }

    /**
     * The callback that's invoked when the user clicks on an info window.
     *
     * To unset the callback, use null.
     */
    var onInfoWindowClickListener: OnInfoWindowClickListener?
        get() = annotationManager.getInfoWindowManager().getOnInfoWindowClickListener()
        set(value) {
            annotationManager.getInfoWindowManager().setOnInfoWindowClickListener(value)
        }

    /**
     * The callback that's invoked when a marker's info window is long pressed.
     *
     * To unset the callback, use null.
     */
    var onInfoWindowLongClickListener: OnInfoWindowLongClickListener?
        get() = annotationManager.getInfoWindowManager().getOnInfoWindowLongClickListener()
        set(value) {
            annotationManager.getInfoWindowManager().setOnInfoWindowLongClickListener(value)
        }

    /**
     * The callback that's invoked when an InfoWindow closes.
     *
     * To unset the callback, use null.
     */
    var onInfoWindowCloseListener: OnInfoWindowCloseListener?
        get() = annotationManager.getInfoWindowManager().getOnInfoWindowCloseListener()
        set(value) {
            annotationManager.getInfoWindowManager().setOnInfoWindowCloseListener(value)
        }

    //
    // Invalidate
    //

    /**
     * Takes a snapshot of the map.
     *
     * @param callback Callback method invoked when the snapshot is taken.
     */
    fun snapshot(callback: SnapshotReadyCallback) {
        if (!started) {
            return
        }
        nativeMapView.addSnapshotCallback(callback)
    }

    /**
     * Queries the map for rendered features.
     *
     * Returns an empty list if either the map or underlying render surface has been destroyed.
     *
     * @param coordinates the point to query
     * @param layerIds    optionally - only query these layers
     * @return the list of feature
     */
    fun queryRenderedFeatures(
        coordinates: PointF,
        vararg layerIds: String,
    ): List<Feature> = nativeMapView.queryRenderedFeatures(coordinates, layerIds, null)

    /**
     * Queries the map for rendered features
     *
     * Returns an empty list if either the map or underlying render surface has been destroyed.
     *
     * @param coordinates the point to query
     * @param filter      filters the returned features with an expression
     * @param layerIds    optionally - only query these layers
     * @return the list of feature
     */
    fun queryRenderedFeatures(
        coordinates: PointF,
        filter: Expression?,
        vararg layerIds: String,
    ): List<Feature> = nativeMapView.queryRenderedFeatures(coordinates, layerIds, filter)

    /**
     * Queries the map for rendered features
     *
     * Returns an empty list if either the map or underlying render surface has been destroyed.
     *
     * @param coordinates the box to query
     * @param layerIds    optionally - only query these layers
     * @return the list of feature
     */
    fun queryRenderedFeatures(
        coordinates: RectF,
        vararg layerIds: String,
    ): List<Feature> = nativeMapView.queryRenderedFeatures(coordinates, layerIds, null)

    /**
     * Queries the map for rendered features
     *
     * Returns an empty list if either the map or underlying render surface has been destroyed.
     *
     * @param coordinates the box to query
     * @param filter      filters the returned features with an expression
     * @param layerIds    optionally - only query these layers
     * @return the list of feature
     */
    fun queryRenderedFeatures(
        coordinates: RectF,
        filter: Expression?,
        vararg layerIds: String,
    ): List<Feature> = nativeMapView.queryRenderedFeatures(coordinates, layerIds, filter)

    //
    // LocationComponent
    //

    internal fun injectLocationComponent(locationComponent: LocationComponent) {
        this.locationComponent = locationComponent
    }

    internal fun injectAnnotationManager(annotationManager: AnnotationManager) {
        this.annotationManager = annotationManager.bind(this)
    }

    //
    // Interfaces
    //

    /**
     * Interface definition for a callback to be invoked when the map is flinged.
     *
     * @see MapLibreMap.addOnFlingListener
     */
    fun interface OnFlingListener {
        /**
         * Called when the map is flinged.
         */
        fun onFling()
    }

    /**
     * Interface definition for a callback to be invoked when the map is moved.
     *
     * @see MapLibreMap.addOnMoveListener
     */
    interface OnMoveListener {
        fun onMoveBegin(detector: MoveGestureDetector)

        fun onMove(detector: MoveGestureDetector)

        fun onMoveEnd(detector: MoveGestureDetector)
    }

    /**
     * Interface definition for a callback to be invoked when the map is rotated.
     *
     * @see MapLibreMap.addOnRotateListener
     */
    interface OnRotateListener {
        fun onRotateBegin(detector: RotateGestureDetector)

        fun onRotate(detector: RotateGestureDetector)

        fun onRotateEnd(detector: RotateGestureDetector)
    }

    /**
     * Interface definition for a callback to be invoked when the map is scaled.
     *
     * @see MapLibreMap.addOnScaleListener
     */
    interface OnScaleListener {
        fun onScaleBegin(detector: StandardScaleGestureDetector)

        fun onScale(detector: StandardScaleGestureDetector)

        fun onScaleEnd(detector: StandardScaleGestureDetector)
    }

    /**
     * Interface definition for a callback to be invoked when the map is tilted.
     *
     * @see MapLibreMap.addOnShoveListener
     */
    interface OnShoveListener {
        fun onShoveBegin(detector: ShoveGestureDetector)

        fun onShove(detector: ShoveGestureDetector)

        fun onShoveEnd(detector: ShoveGestureDetector)
    }

    /**
     * Interface definition for a callback to be invoked for when the camera motion starts.
     */
    fun interface OnCameraMoveStartedListener {
        /**
         * Called when the camera starts moving after it has been idle or when the reason for camera motion has changed.
         *
         * @param reason the reason for the camera change
         */
        fun onCameraMoveStarted(reason: Int)

        companion object {
            const val REASON_API_GESTURE = 1
            const val REASON_DEVELOPER_ANIMATION = 2
            const val REASON_API_ANIMATION = 3
        }
    }

    /**
     * Interface definition for a callback to be invoked for when the camera changes position.
     */
    fun interface OnCameraMoveListener {
        /**
         * Called repeatedly as the camera continues to move after an onCameraMoveStarted call.
         * This may be called as often as once every frame and should not perform expensive operations.
         */
        fun onCameraMove()
    }

    /**
     * Interface definition for a callback to be invoked for when the camera's motion has been stopped or when the camera
     * starts moving for a new reason.
     */
    fun interface OnCameraMoveCanceledListener {
        /**
         * Called when the developer explicitly calls the cancelTransitions() method or if the reason for camera motion
         * has changed before the onCameraIdle had a chance to fire after the previous animation.
         * Do not update or animate the camera from within this method.
         */
        fun onCameraMoveCanceled()
    }

    /**
     * Interface definition for a callback to be invoked for when camera movement has ended.
     */
    fun interface OnCameraIdleListener {
        /**
         * Called when camera movement has ended.
         */
        fun onCameraIdle()
    }

    /**
     * Interface definition for a callback to be invoked for when the compass is animating.
     */
    interface OnCompassAnimationListener {
        /**
         * Called repeatedly as the compass continues to move after clicking on it.
         */
        fun onCompassAnimation()

        /**
         * Called when compass animation has ended.
         */
        fun onCompassAnimationFinished()
    }

    /**
     * Interface definition for a callback to be invoked when a frame is rendered to the map view.
     *
     * @see MapLibreMap.onFpsChangedListener
     */
    fun interface OnFpsChangedListener {
        /**
         * Called for every frame rendered to the map view.
         *
         * @param fps The average number of frames rendered over the last second.
         */
        fun onFpsChanged(fps: Double)
    }

    /**
     * Interface definition for a callback to be invoked when a user registers an listener that is
     * related to touch and click events.
     */
    @Suppress("TooManyFunctions")
    internal interface OnGesturesManagerInteractionListener {
        fun onAddMapClickListener(listener: OnMapClickListener)

        fun onRemoveMapClickListener(listener: OnMapClickListener)

        fun onAddMapLongClickListener(listener: OnMapLongClickListener)

        fun onRemoveMapLongClickListener(listener: OnMapLongClickListener)

        fun onAddFlingListener(listener: OnFlingListener)

        fun onRemoveFlingListener(listener: OnFlingListener)

        fun onAddMoveListener(listener: OnMoveListener)

        fun onRemoveMoveListener(listener: OnMoveListener)

        fun onAddRotateListener(listener: OnRotateListener)

        fun onRemoveRotateListener(listener: OnRotateListener)

        fun onAddScaleListener(listener: OnScaleListener)

        fun onRemoveScaleListener(listener: OnScaleListener)

        fun onAddShoveListener(listener: OnShoveListener)

        fun onRemoveShoveListener(listener: OnShoveListener)

        fun getGesturesManager(): AndroidGesturesManager

        fun setGesturesManager(
            gesturesManager: AndroidGesturesManager,
            attachDefaultListeners: Boolean,
            setDefaultMutuallyExclusives: Boolean,
        )

        fun cancelAllVelocityAnimations()
    }

    /**
     * Interface definition for a callback to be invoked when the user clicks on the map view.
     *
     * @see MapLibreMap.addOnMapClickListener
     */
    fun interface OnMapClickListener {
        /**
         * Called when the user clicks on the map view.
         *
         * @param point The projected map coordinate the user clicked on.
         * @return True if this click should be consumed and not passed further to other listeners registered afterwards,
         * false otherwise.
         */
        fun onMapClick(point: LatLng): Boolean
    }

    /**
     * Interface definition for a callback to be invoked when the user long clicks on the map view.
     *
     * @see MapLibreMap.addOnMapLongClickListener
     */
    fun interface OnMapLongClickListener {
        /**
         * Called when the user long clicks on the map view.
         *
         * @param point The projected map coordinate the user long clicked on.
         * @return True if this click should be consumed and not passed further to other listeners registered afterwards,
         * false otherwise.
         */
        fun onMapLongClick(point: LatLng): Boolean
    }

    /**
     * Interface definition for a callback to be invoked when the user clicks on a marker.
     *
     * @see MapLibreMap.setOnMarkerClickListener
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun interface OnMarkerClickListener {
        /**
         * Called when the user clicks on a marker.
         *
         * @param marker The marker the user clicked on.
         * @return If true the listener has consumed the event and the info window will not be shown.
         */
        fun onMarkerClick(marker: Marker): Boolean
    }

    /**
     * Interface definition for a callback to be invoked when the user clicks on a polygon.
     *
     * @see MapLibreMap.setOnPolygonClickListener
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun interface OnPolygonClickListener {
        /**
         * Called when the user clicks on a polygon.
         *
         * @param polygon The polygon the user clicked on.
         */
        fun onPolygonClick(polygon: Polygon)
    }

    /**
     * Interface definition for a callback to be invoked when the user clicks on a polyline.
     *
     * @see MapLibreMap.setOnPolylineClickListener
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun interface OnPolylineClickListener {
        /**
         * Called when the user clicks on a polyline.
         *
         * @param polyline The polyline the user clicked on.
         */
        fun onPolylineClick(polyline: Polyline)
    }

    /**
     * Interface definition for a callback to be invoked when the user clicks on an info window.
     *
     * @see MapLibreMap.onInfoWindowClickListener
     */
    fun interface OnInfoWindowClickListener {
        /**
         * Called when the user clicks on an info window.
         *
         * @param marker The marker of the info window the user clicked on.
         * @return If true the listener has consumed the event and the info window will not be closed.
         */
        fun onInfoWindowClick(marker: Marker): Boolean
    }

    /**
     * Interface definition for a callback to be invoked when the user long presses on a marker's info window.
     *
     * @see MapLibreMap.onInfoWindowLongClickListener
     */
    fun interface OnInfoWindowLongClickListener {
        /**
         * Called when the user makes a long-press gesture on the marker's info window.
         *
         * @param marker The marker were the info window is attached to
         */
        fun onInfoWindowLongClick(marker: Marker)
    }

    /**
     * Interface definition for a callback to be invoked when a marker's info window is closed.
     *
     * @see MapLibreMap.onInfoWindowCloseListener
     */
    fun interface OnInfoWindowCloseListener {
        /**
         * Called when the marker's info window is closed.
         *
         * @param marker The marker of the info window that was closed.
         */
        fun onInfoWindowClose(marker: Marker)
    }

    /**
     * Interface definition for a callback to be invoked when an info window will be shown.
     *
     * @see MapLibreMap.infoWindowAdapter
     */
    @Deprecated(
        "As of 7.0.0, use " +
            "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
    )
    fun interface InfoWindowAdapter {
        /**
         * Called when an info window will be shown as a result of a marker click.
         *
         * @param marker The marker the user clicked on.
         * @return View to be shown as a info window. If null is returned the default
         * info window will be shown.
         */
        fun getInfoWindow(marker: Marker): View?
    }

    /**
     * Interface definition for a callback to be invoked when a task is complete or cancelled.
     */
    interface CancelableCallback {
        /**
         * Invoked when a task is cancelled.
         */
        fun onCancel()

        /**
         * Invoked when a task is complete.
         */
        fun onFinish()
    }

    /**
     * Interface definition for a callback to be invoked when the snapshot has been taken.
     */
    fun interface SnapshotReadyCallback {
        /**
         * Invoked when the snapshot has been taken.
         *
         * @param snapshot the snapshot bitmap
         */
        fun onSnapshotReady(snapshot: Bitmap)
    }

    /**
     * Internal use.
     */
    fun interface OnDeveloperAnimationListener {
        /**
         * Notifies listener when a developer invoked animation is about to start.
         */
        fun onDeveloperAnimationStarted()
    }

    //
    // Used for instrumentation testing
    //
    internal fun getTransform(): Transform = transform

    private fun notifyDeveloperAnimationListeners() {
        for (listener in developerAnimationStartedListeners) {
            listener.onDeveloperAnimationStarted()
        }
    }
}
