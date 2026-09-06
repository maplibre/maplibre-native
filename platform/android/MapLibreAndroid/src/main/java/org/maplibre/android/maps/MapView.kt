package org.maplibre.android.maps

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.util.AttributeSet
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageView
import androidx.annotation.CallSuper
import androidx.annotation.UiThread
import androidx.collection.LongSparseArray
import org.maplibre.android.MapLibre
import org.maplibre.android.MapStrictMode
import org.maplibre.android.R
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.exceptions.MapLibreConfigurationException
import org.maplibre.android.gestures.AndroidGesturesManager
import org.maplibre.android.location.LocationComponent
import org.maplibre.android.maps.renderer.MapRenderer
import org.maplibre.android.maps.widgets.CompassView
import org.maplibre.android.net.ConnectivityReceiver
import org.maplibre.android.storage.FileSource
import org.maplibre.android.tile.TileOperation
import org.maplibre.android.utils.BitmapUtils
import timber.log.Timber

/**
 * A `MapView` provides an embeddable map interface.
 * You use this class to display map information and to manipulate the map contents from your application.
 * You can center the map on a given coordinate, specify the size of the area you want to display,
 * and style the features of the map to fit your application's use case.
 *
 * Use of `MapView` requires a MapLibre API access token.
 * Obtain an access token on the [MapLibre account page](https://www.mapbox.com/studio/account/tokens/).
 *
 * **Warning:** Please note that you are responsible for getting permission to use the map data,
 * and for ensuring your use adheres to the relevant terms of use.
 */
@Suppress("TooManyFunctions", "LargeClass")
open class MapView :
    FrameLayout,
    NativeMapView.ViewCallback {
    private val mapChangeReceiver = MapChangeReceiver()
    private val mapCallback = MapCallback()
    private val initialRenderCallback = InitialRenderCallback()

    private var nativeMapView: NativeMap? = null
    private var maplibreMap: MapLibreMap? = null

    /**
     * The View used for rendering.
     *
     * The type of the returned view is either a SurfaceView or a TextureView.
     */
    lateinit var renderView: View
        private set

    private var attributionClickListener: AttributionClickListener? = null

    internal lateinit var maplibreMapOptions: MapLibreMapOptions

    private var mapRenderer: MapRenderer? = null
    private var destroyed = false

    private var compassView: CompassView? = null
    private var focalPoint: PointF? = null

    // callback for focal point invalidation
    private val focalInvalidator = FocalPointInvalidator()

    // callback for registering touch listeners
    private val registerTouchListener = GesturesManagerInteractionListener()

    // callback for camera change events
    private val cameraDispatcher = CameraChangeDispatcher()

    private var mapGestureDetector: MapGestureDetector? = null
    private var mapKeyListener: MapKeyListener? = null
    private var savedInstanceState: Bundle? = null
    private var isStarted = false

    @UiThread
    constructor(context: Context) : super(context) {
        Timber.d("MapView constructed with context")
        initialize(context, MapLibreMapOptions.createFromAttributes(context))
    }

    @UiThread
    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        Timber.d("MapView constructed with context and attribute set")
        initialize(context, MapLibreMapOptions.createFromAttributes(context, attrs))
    }

    @UiThread
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        Timber.d("MapView constructed with context, attributeSet and defStyleAttr")
        initialize(context, MapLibreMapOptions.createFromAttributes(context, attrs))
    }

    @UiThread
    constructor(context: Context, options: MapLibreMapOptions?) : super(context) {
        Timber.d("MapView constructed with context and MapLibreMapOptions")
        initialize(context, options ?: MapLibreMapOptions.createFromAttributes(context))
    }

    @CallSuper
    @UiThread
    protected open fun initialize(
        context: Context,
        options: MapLibreMapOptions,
    ) {
        if (isInEditMode) {
            // in IDE layout editor, just return
            return
        }

        if (!MapLibre.hasInstance()) {
            throw MapLibreConfigurationException()
        }

        // hide surface until map is fully loaded #10990
        foreground = ColorDrawable(options.foregroundLoadColor)

        maplibreMapOptions = options

        // add accessibility support
        contentDescription = context.getString(R.string.maplibre_mapActionDescription)
        setWillNotDraw(false)
        initializeDrawingSurface(options)
    }

    private fun initializeMap() {
        val context = context
        val nativeMap = nativeMapView!!

        // callback for focal point invalidation
        focalInvalidator.addListener(createFocalPointChangeListener())

        // setup components for MapLibreMap creation
        val proj = Projection(nativeMap, this)
        val uiSettings = UiSettings(proj, focalInvalidator, getPixelRatio(), this)
        val annotationsArray = LongSparseArray<Annotation>()
        val iconManager = IconManager(nativeMap)
        val annotations: Annotations = AnnotationContainer(nativeMap, annotationsArray)
        val markers: Markers = MarkerContainer(nativeMap, annotationsArray, iconManager)
        val polygons: Polygons = PolygonContainer(nativeMap, annotationsArray)
        val polylines: Polylines = PolylineContainer(nativeMap, annotationsArray)
        val shapeAnnotations: ShapeAnnotations = ShapeAnnotationContainer(nativeMap, annotationsArray)
        val annotationManager =
            AnnotationManager(
                this,
                annotationsArray,
                iconManager,
                annotations,
                markers,
                polygons,
                polylines,
                shapeAnnotations,
            )
        val transform = Transform(this, nativeMap, cameraDispatcher)

        // MapLibreMap
        val developerAnimationListeners = mutableListOf<MapLibreMap.OnDeveloperAnimationListener>()
        val map =
            MapLibreMap(
                nativeMap,
                transform,
                uiSettings,
                proj,
                registerTouchListener,
                cameraDispatcher,
                developerAnimationListeners,
            )
        maplibreMap = map
        map.injectAnnotationManager(annotationManager)

        // user input
        val gestureDetector = MapGestureDetector(context, transform, proj, uiSettings, annotationManager, cameraDispatcher)
        mapGestureDetector = gestureDetector
        mapKeyListener = MapKeyListener(transform, uiSettings, gestureDetector)

        // LocationComponent
        map.injectLocationComponent(LocationComponent(map, transform, developerAnimationListeners))

        // Ensure this view is interactable
        isClickable = true
        isLongClickable = true
        isFocusable = true
        isFocusableInTouchMode = true
        requestDisallowInterceptTouchEvent(true)

        // notify Map object about current connectivity state
        nativeMap.setReachability(MapLibre.isConnected())

        // initialise MapLibreMap
        val savedInstanceState = this.savedInstanceState
        if (savedInstanceState == null) {
            map.initialise(context, maplibreMapOptions)
        } else {
            map.onRestoreInstanceState(savedInstanceState)
        }

        mapCallback.initialised()
    }

    internal open fun initialiseCompassView(): CompassView {
        val view = CompassView(this.context)
        compassView = view
        addView(view)
        view.tag = "compassView"
        view.layoutParams.width = ViewGroup.LayoutParams.WRAP_CONTENT
        view.layoutParams.height = ViewGroup.LayoutParams.WRAP_CONTENT
        view.contentDescription = resources.getString(R.string.maplibre_compassContentDescription)
        view.injectCompassAnimationListener(createCompassAnimationListener(cameraDispatcher))
        view.setOnClickListener(createCompassClickListener(cameraDispatcher))
        return view
    }

    internal open fun initialiseAttributionView(): ImageView {
        val attrView = ImageView(this.context)
        addView(attrView)
        attrView.tag = "attrView"
        attrView.layoutParams.width = ViewGroup.LayoutParams.WRAP_CONTENT
        attrView.layoutParams.height = ViewGroup.LayoutParams.WRAP_CONTENT
        attrView.adjustViewBounds = true
        attrView.isClickable = true
        attrView.isFocusable = true
        attrView.contentDescription = resources.getString(R.string.maplibre_attributionsIconContentDescription)
        attrView.setImageDrawable(BitmapUtils.getDrawableFromRes(context, R.drawable.maplibre_info_bg_selector))
        // inject widgets with MapLibreMap
        val listener = AttributionClickListener(context, maplibreMap!!)
        attributionClickListener = listener
        attrView.setOnClickListener(listener)
        return attrView
    }

    internal open fun initialiseLogoView(): ImageView {
        val logoView = ImageView(this.context)
        addView(logoView)
        logoView.tag = "logoView"
        logoView.layoutParams.width = ViewGroup.LayoutParams.WRAP_CONTENT
        logoView.layoutParams.height = ViewGroup.LayoutParams.WRAP_CONTENT
        logoView.setImageDrawable(BitmapUtils.getDrawableFromRes(context, R.drawable.maplibre_logo_icon))
        return logoView
    }

    private fun createFocalPointChangeListener() = FocalPointChangeListener { pointF -> focalPoint = pointF }

    private fun createCompassAnimationListener(cameraChangeDispatcher: CameraChangeDispatcher): MapLibreMap.OnCompassAnimationListener =
        object : MapLibreMap.OnCompassAnimationListener {
            override fun onCompassAnimation() {
                cameraChangeDispatcher.onCameraMove()
            }

            override fun onCompassAnimationFinished() {
                compassView?.isAnimating(false)
                cameraChangeDispatcher.onCameraIdle()
            }
        }

    private fun createCompassClickListener(cameraChangeDispatcher: CameraChangeDispatcher) =
        OnClickListener {
            val map = maplibreMap
            val view = compassView
            if (map != null && view != null) {
                val point = focalPoint
                if (point != null) {
                    map.setFocalBearing(0.0, point.x, point.y, CompassView.TIME_MAP_NORTH_ANIMATION)
                } else {
                    map.setFocalBearing(
                        0.0,
                        map.width / 2,
                        map.height / 2,
                        CompassView.TIME_MAP_NORTH_ANIMATION,
                    )
                }
                cameraChangeDispatcher.onCameraMoveStarted(
                    MapLibreMap.OnCameraMoveStartedListener.REASON_API_ANIMATION,
                )
                view.isAnimating(true)
                view.postDelayed(view, CompassView.TIME_WAIT_IDLE + CompassView.TIME_MAP_NORTH_ANIMATION)
            }
        }

    //
    // Lifecycle events
    //

    /**
     * You must call this method from the parent's Activity#onCreate(Bundle) or
     * Fragment#onViewCreated(View, Bundle).
     *
     * You must set a valid access token with
     * [MapLibre.getInstance] before you call this method or an exception will be thrown.
     *
     * @param savedInstanceState Pass in the parent's savedInstanceState.
     * @see MapLibre.getInstance
     */
    @UiThread
    fun onCreate(savedInstanceState: Bundle?) {
        if (savedInstanceState != null && savedInstanceState.getBoolean(MapLibreConstants.STATE_HAS_SAVED_STATE)) {
            this.savedInstanceState = savedInstanceState
        }
    }

    private fun initializeDrawingSurface(options: MapLibreMapOptions) {
        val renderer = MapRenderer.create(options, context) { onSurfaceCreated() }
        mapRenderer = renderer
        renderView = renderer.view

        addView(renderView, 0)

        options.pixelRatio(getPixelRatio())
        nativeMapView = NativeMapView(context, options, this, mapChangeReceiver, renderer)
    }

    private fun onSurfaceCreated() {
        post {
            // Initialize only when not destroyed and only once
            if (!destroyed && maplibreMap == null) {
                initializeMap()
                maplibreMap?.onStart()
            }
        }
    }

    /**
     * You must call this method from the parent's Activity#onSaveInstanceState(Bundle)
     * or Fragment#onSaveInstanceState(Bundle).
     *
     * @param outState Pass in the parent's outState.
     */
    @UiThread
    fun onSaveInstanceState(outState: Bundle) {
        maplibreMap?.let {
            outState.putBoolean(MapLibreConstants.STATE_HAS_SAVED_STATE, true)
            it.onSaveInstanceState(outState)
        }
    }

    /**
     * You must call this method from the parent's Activity#onStart() or Fragment#onStart()
     */
    @UiThread
    fun onStart() {
        if (!isStarted) {
            ConnectivityReceiver.instance(context).activate()
            FileSource.getInstance(context).activate()
            isStarted = true
        }
        maplibreMap?.onStart()
        mapRenderer?.onStart()
    }

    /**
     * You must call this method from the parent's Activity#onResume() or Fragment#onResume().
     */
    @UiThread
    fun onResume() {
        mapRenderer?.onResume()
    }

    /**
     * You must call this method from the parent's Activity#onPause() or Fragment#onPause().
     */
    @UiThread
    fun onPause() {
        mapRenderer?.onPause()
    }

    /**
     * You must call this method from the parent's Activity#onStop() or Fragment#onStop().
     */
    @UiThread
    fun onStop() {
        attributionClickListener?.onStop()

        maplibreMap?.let {
            // map was destroyed before it was started
            mapGestureDetector?.cancelAnimators()
            it.onStop()
        }

        mapRenderer?.onStop()

        if (isStarted) {
            ConnectivityReceiver.instance(context).deactivate()
            FileSource.getInstance(context).deactivate()
            isStarted = false
        }
    }

    /**
     * You must call this method from the parent's Activity#onDestroy() or Fragment#onDestroyView().
     */
    @UiThread
    fun onDestroy() {
        destroyed = true
        mapChangeReceiver.clear()
        mapCallback.onDestroy()
        initialRenderCallback.onDestroy()

        // avoid leaking context through animator #13742
        compassView?.resetAnimation()

        maplibreMap?.onDestroy()

        // null when destroying an activity programmatically mapbox-navigation-android/issues/503
        nativeMapView?.destroy()
        nativeMapView = null

        mapRenderer?.onDestroy()
    }

    /**
     * Queue a runnable to be executed on the map renderer thread.
     *
     * @param runnable the runnable to queue
     */
    fun queueEvent(runnable: Runnable) {
        val renderer =
            mapRenderer ?: throw IllegalStateException(
                "Calling MapView#queueEvent before mapRenderer is created.",
            )
        renderer.queueEvent(runnable)
    }

    /**
     * The maximum frame rate at which the map view is rendered,
     * but it can't excess the ability of device hardware.
     *
     * @param maximumFps Can be set to arbitrary integer values.
     */
    fun setMaximumFps(maximumFps: Int) {
        val renderer =
            mapRenderer ?: throw IllegalStateException(
                "Calling MapView#setMaximumFps before mapRenderer is created.",
            )
        renderer.setMaximumFps(maximumFps)
    }

    /**
     * Set the rendering refresh mode and wake up the render thread if it is sleeping.
     *
     * @param mode can be:
     * [MapRenderer.RenderingRefreshMode.CONTINUOUS] or [MapRenderer.RenderingRefreshMode.WHEN_DIRTY]
     * default is [MapRenderer.RenderingRefreshMode.WHEN_DIRTY]
     */
    fun setRenderingRefreshMode(mode: MapRenderer.RenderingRefreshMode) {
        val renderer =
            mapRenderer ?: throw IllegalStateException(
                "Calling MapView#setRenderingRefreshMode before mapRenderer is created.",
            )
        renderer.setRenderingRefreshMode(mode)
    }

    /**
     * Get the rendering refresh mode
     *
     * @return one of the MapRenderer.RenderingRefreshMode modes
     * @see setRenderingRefreshMode
     */
    fun getRenderingRefreshMode(): MapRenderer.RenderingRefreshMode {
        val renderer =
            mapRenderer ?: throw IllegalStateException(
                "Calling MapView#getRenderingRefreshMode before mapRenderer is created.",
            )
        return renderer.getRenderingRefreshMode()
    }

    /**
     * Returns if the map has been destroyed.
     *
     * This method can be used to determine if the result of an asynchronous operation should be set.
     *
     * @return true, if the map has been destroyed
     */
    @get:UiThread
    val isDestroyed: Boolean
        get() = destroyed

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val gestureDetector = mapGestureDetector ?: return super.onTouchEvent(event)
        return gestureDetector.onTouchEvent(event) || super.onTouchEvent(event)
    }

    override fun onKeyDown(
        keyCode: Int,
        event: KeyEvent,
    ): Boolean {
        val keyListener = mapKeyListener ?: return super.onKeyDown(keyCode, event)
        return keyListener.onKeyDown(keyCode, event) || super.onKeyDown(keyCode, event)
    }

    override fun onKeyLongPress(
        keyCode: Int,
        event: KeyEvent,
    ): Boolean {
        val keyListener = mapKeyListener ?: return super.onKeyLongPress(keyCode, event)
        return keyListener.onKeyLongPress(keyCode, event) || super.onKeyLongPress(keyCode, event)
    }

    override fun onKeyUp(
        keyCode: Int,
        event: KeyEvent,
    ): Boolean {
        val keyListener = mapKeyListener ?: return super.onKeyUp(keyCode, event)
        return keyListener.onKeyUp(keyCode, event) || super.onKeyUp(keyCode, event)
    }

    override fun onTrackballEvent(event: MotionEvent): Boolean {
        val keyListener = mapKeyListener ?: return super.onTrackballEvent(event)
        return keyListener.onTrackballEvent(event) || super.onTrackballEvent(event)
    }

    override fun onGenericMotionEvent(event: MotionEvent): Boolean {
        val gestureDetector = mapGestureDetector ?: return super.onGenericMotionEvent(event)
        return gestureDetector.onGenericMotionEvent(event) || super.onGenericMotionEvent(event)
    }

    /**
     * You must call this method from the parent's Activity#onLowMemory() or Fragment#onLowMemory().
     */
    @UiThread
    fun onLowMemory() {
        val nativeMap = nativeMapView
        if (nativeMap != null && maplibreMap != null && !destroyed) {
            nativeMap.onLowMemory()
        }
    }

    //
    // Rendering
    //

    override fun onSizeChanged(
        width: Int,
        height: Int,
        oldw: Int,
        oldh: Int,
    ) {
        if (!isInEditMode) {
            // null-checking the nativeMapView, see #13277
            nativeMapView?.resizeView(width, height)
        }
    }

    /**
     * Returns the map pixel ratio, by default it returns the device pixel ratio.
     * Can be overwritten using [MapLibreMapOptions.pixelRatio].
     *
     * @return the current map pixel ratio
     */
    fun getPixelRatio(): Float {
        // check is user defined his own pixel ratio value
        val pixelRatio = maplibreMapOptions.pixelRatio
        // if not, get the one defined by the system
        return if (pixelRatio == 0f) resources.displayMetrics.density else pixelRatio
    }

    //
    // ViewCallback
    //

    override fun getViewContent(): Bitmap? = BitmapUtils.createBitmapFromView(this)

    //
    // Map events
    //

    /**
     * Set a callback that's invoked when the camera region will change.
     *
     * @param listener The callback that's invoked when the camera region will change
     */
    fun addOnCameraWillChangeListener(listener: OnCameraWillChangeListener) {
        mapChangeReceiver.addOnCameraWillChangeListener(listener)
    }

    /**
     * Remove a callback that's invoked when the camera region will change.
     *
     * @param listener The callback that's invoked when the camera region will change
     */
    fun removeOnCameraWillChangeListener(listener: OnCameraWillChangeListener) {
        mapChangeReceiver.removeOnCameraWillChangeListener(listener)
    }

    /**
     * Set a callback that's invoked when the camera is changing.
     *
     * @param listener The callback that's invoked when the camera is changing
     */
    fun addOnCameraIsChangingListener(listener: OnCameraIsChangingListener) {
        mapChangeReceiver.addOnCameraIsChangingListener(listener)
    }

    /**
     * Remove a callback that's invoked when the camera is changing.
     *
     * @param listener The callback that's invoked when the camera is changing
     */
    fun removeOnCameraIsChangingListener(listener: OnCameraIsChangingListener) {
        mapChangeReceiver.removeOnCameraIsChangingListener(listener)
    }

    /**
     * Set a callback that's invoked when the camera region did change.
     *
     * @param listener The callback that's invoked when the camera region did change
     */
    fun addOnCameraDidChangeListener(listener: OnCameraDidChangeListener) {
        mapChangeReceiver.addOnCameraDidChangeListener(listener)
    }

    /**
     * Set a callback that's invoked when the camera region did change.
     *
     * @param listener The callback that's invoked when the camera region did change
     */
    fun removeOnCameraDidChangeListener(listener: OnCameraDidChangeListener) {
        mapChangeReceiver.removeOnCameraDidChangeListener(listener)
    }

    /**
     * Set a callback that's invoked when the map will start loading.
     *
     * @param listener The callback that's invoked when the map will start loading
     */
    fun addOnWillStartLoadingMapListener(listener: OnWillStartLoadingMapListener) {
        mapChangeReceiver.addOnWillStartLoadingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map will start loading.
     *
     * @param listener The callback that's invoked when the map will start loading
     */
    fun removeOnWillStartLoadingMapListener(listener: OnWillStartLoadingMapListener) {
        mapChangeReceiver.removeOnWillStartLoadingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished loading.
     *
     * @param listener The callback that's invoked when the map has finished loading
     */
    fun addOnDidFinishLoadingMapListener(listener: OnDidFinishLoadingMapListener) {
        mapChangeReceiver.addOnDidFinishLoadingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished loading.
     *
     * @param listener The callback that's invoked when the map has finished loading
     */
    fun removeOnDidFinishLoadingMapListener(listener: OnDidFinishLoadingMapListener) {
        mapChangeReceiver.removeOnDidFinishLoadingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map failed to load.
     *
     * @param listener The callback that's invoked when the map failed to load
     */
    fun addOnDidFailLoadingMapListener(listener: OnDidFailLoadingMapListener) {
        mapChangeReceiver.addOnDidFailLoadingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map failed to load.
     *
     * @param listener The callback that's invoked when the map failed to load
     */
    fun removeOnDidFailLoadingMapListener(listener: OnDidFailLoadingMapListener) {
        mapChangeReceiver.removeOnDidFailLoadingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map will start rendering a frame.
     *
     * @param listener The callback that's invoked when the camera will start rendering a frame
     */
    fun addOnWillStartRenderingFrameListener(listener: OnWillStartRenderingFrameListener) {
        mapChangeReceiver.addOnWillStartRenderingFrameListener(listener)
    }

    /**
     * Set a callback that's invoked when the map will start rendering a frame.
     *
     * @param listener The callback that's invoked when the camera will start rendering a frame
     */
    fun removeOnWillStartRenderingFrameListener(listener: OnWillStartRenderingFrameListener) {
        mapChangeReceiver.removeOnWillStartRenderingFrameListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished rendering a frame.
     *
     * @param listener The callback that's invoked when the map has finished rendering a frame
     */
    fun addOnDidFinishRenderingFrameListener(listener: OnDidFinishRenderingFrameListener) {
        mapChangeReceiver.addOnDidFinishRenderingFrameListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished rendering a frame.
     *
     * @param listener The callback that's invoked when the map has finished rendering a frame
     */
    fun removeOnDidFinishRenderingFrameListener(listener: OnDidFinishRenderingFrameListener) {
        mapChangeReceiver.removeOnDidFinishRenderingFrameListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished rendering a frame.
     *
     * @param listener The callback that's invoked when the map has finished rendering a frame
     */
    fun addOnDidFinishRenderingFrameListener(listener: OnDidFinishRenderingFrameWithStatsListener) {
        mapChangeReceiver.addOnDidFinishRenderingFrameListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished rendering a frame.
     *
     * @param listener The callback that's invoked when the map has finished rendering a frame
     */
    fun removeOnDidFinishRenderingFrameListener(listener: OnDidFinishRenderingFrameWithStatsListener) {
        mapChangeReceiver.removeOnDidFinishRenderingFrameListener(listener)
    }

    /**
     * Set a callback that's invoked when the map will start rendering.
     *
     * @param listener The callback that's invoked when the map will start rendering
     */
    fun addOnWillStartRenderingMapListener(listener: OnWillStartRenderingMapListener) {
        mapChangeReceiver.addOnWillStartRenderingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map will start rendering.
     *
     * @param listener The callback that's invoked when the map will start rendering
     */
    fun removeOnWillStartRenderingMapListener(listener: OnWillStartRenderingMapListener) {
        mapChangeReceiver.removeOnWillStartRenderingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has finished rendering.
     *
     * @param listener The callback that's invoked when the map has finished rendering
     */
    fun addOnDidFinishRenderingMapListener(listener: OnDidFinishRenderingMapListener) {
        mapChangeReceiver.addOnDidFinishRenderingMapListener(listener)
    }

    /**
     * Remove a callback that's invoked when the map has finished rendering.
     *
     * @param listener The callback that's invoked when the map has has finished rendering.
     */
    fun removeOnDidFinishRenderingMapListener(listener: OnDidFinishRenderingMapListener) {
        mapChangeReceiver.removeOnDidFinishRenderingMapListener(listener)
    }

    /**
     * Set a callback that's invoked when the map has entered the idle state.
     *
     * @param listener The callback that's invoked when the map has entered the idle state.
     */
    fun addOnDidBecomeIdleListener(listener: OnDidBecomeIdleListener) {
        mapChangeReceiver.addOnDidBecomeIdleListener(listener)
    }

    /**
     * Remove a callback that's invoked when the map has entered the idle state.
     *
     * @param listener The callback that's invoked when the map has entered the idle state.
     */
    fun removeOnDidBecomeIdleListener(listener: OnDidBecomeIdleListener) {
        mapChangeReceiver.removeOnDidBecomeIdleListener(listener)
    }

    /**
     * Set a callback that's invoked when the style has finished loading.
     *
     * @param listener The callback that's invoked when the style has finished loading
     */
    fun addOnDidFinishLoadingStyleListener(listener: OnDidFinishLoadingStyleListener) {
        mapChangeReceiver.addOnDidFinishLoadingStyleListener(listener)
    }

    /**
     * Set a callback that's invoked when the style has finished loading.
     *
     * @param listener The callback that's invoked when the style has finished loading
     */
    fun removeOnDidFinishLoadingStyleListener(listener: OnDidFinishLoadingStyleListener) {
        mapChangeReceiver.removeOnDidFinishLoadingStyleListener(listener)
    }

    /**
     * Set a callback that's invoked when a map source has changed.
     *
     * @param listener The callback that's invoked when the source has changed
     */
    fun addOnSourceChangedListener(listener: OnSourceChangedListener) {
        mapChangeReceiver.addOnSourceChangedListener(listener)
    }

    /**
     * Set a callback that's invoked when a map source has changed.
     *
     * @param listener The callback that's invoked when the source has changed
     */
    fun removeOnSourceChangedListener(listener: OnSourceChangedListener) {
        mapChangeReceiver.removeOnSourceChangedListener(listener)
    }

    /**
     * Set a callback that's invoked when the id of an icon is missing.
     *
     * @param listener The callback that's invoked when the id of an icon is missing
     */
    fun addOnStyleImageMissingListener(listener: OnStyleImageMissingListener) {
        mapChangeReceiver.addOnStyleImageMissingListener(listener)
    }

    /**
     * Set a callback that's invoked when a map source has changed.
     *
     * @param listener The callback that's invoked when the source has changed
     */
    fun removeOnStyleImageMissingListener(listener: OnStyleImageMissingListener) {
        mapChangeReceiver.removeOnStyleImageMissingListener(listener)
    }

    /**
     * Set a callback that's invoked when map needs to release unused image resources.
     *
     * A callback will be called only for unused images that were provided by the client via
     * [OnStyleImageMissingListener.onStyleImageMissing] listener interface.
     *
     * By default, platform will remove unused images from the style. By adding listener, default
     * behavior can be overridden and client can control whether to release unused resources.
     *
     * @param listener The callback that's invoked when map needs to release unused image resources
     */
    fun addOnCanRemoveUnusedStyleImageListener(listener: OnCanRemoveUnusedStyleImageListener) {
        mapChangeReceiver.addOnCanRemoveUnusedStyleImageListener(listener)
    }

    /**
     * Removes a callback that's invoked when map needs to release unused image resources.
     *
     * When all listeners are removed, platform will fallback to default behavior, which is to remove
     * unused images from the style.
     *
     * @param listener The callback that's invoked when map needs to release unused image resources
     */
    fun removeOnCanRemoveUnusedStyleImageListener(listener: OnCanRemoveUnusedStyleImageListener) {
        mapChangeReceiver.removeOnCanRemoveUnusedStyleImageListener(listener)
    }

    /**
     * Set a callback that's invoked before a shader is compiled.
     *
     * @param callback The callback that's invoked before a shader is compiled
     */
    fun addOnPreCompileShaderListener(callback: OnPreCompileShaderListener) {
        mapChangeReceiver.addOnPreCompileShaderListener(callback)
    }

    /**
     * Removes a callback that's invoked before a shader is compiled.
     *
     * @param callback The callback that's invoked before a shader is compiled
     */
    fun removeOnPreCompileShaderListener(callback: OnPreCompileShaderListener) {
        mapChangeReceiver.removeOnPreCompileShaderListener(callback)
    }

    /**
     * Set a callback that's invoked after a shader is compiled.
     *
     * @param callback The callback that's invoked after a shader is compiled
     */
    fun addOnPostCompileShaderListener(callback: OnPostCompileShaderListener) {
        mapChangeReceiver.addOnPostCompileShaderListener(callback)
    }

    /**
     * Removes a callback that's invoked after a shader is compiled.
     *
     * @param callback The callback that's invoked after a shader is compiled
     */
    fun removeOnPostCompileShaderListener(callback: OnPostCompileShaderListener) {
        mapChangeReceiver.removeOnPostCompileShaderListener(callback)
    }

    /**
     * Set a callback that's invoked after a shader failed to compile.
     *
     * @param callback The callback that's invoked after a shader failes to compile
     */
    fun addOnShaderCompileFailedListener(callback: OnShaderCompileFailedListener) {
        mapChangeReceiver.addOnShaderCompileFailedListener(callback)
    }

    /**
     * Removes a callback that's invoked after a shader failed to compile.
     *
     * @param callback The callback that's invoked after a shader failes to compile
     */
    fun removeOnShaderCompileFailedListener(callback: OnShaderCompileFailedListener) {
        mapChangeReceiver.removeOnShaderCompileFailedListener(callback)
    }

    /**
     * Set a callback that's invoked after a range of glyphs are loaded.
     *
     * @param callback The callback that's invoked after a range of glyphs are loaded
     */
    fun addOnGlyphsLoadedListener(callback: OnGlyphsLoadedListener) {
        mapChangeReceiver.addOnGlyphsLoadedListener(callback)
    }

    /**
     * Removes a callback that's invoked after a range of glyphs are loaded.
     *
     * @param callback The callback that's invoked after a range of glyphs are loaded
     */
    fun removeOnGlyphsLoadedListener(callback: OnGlyphsLoadedListener) {
        mapChangeReceiver.removeOnGlyphsLoadedListener(callback)
    }

    /**
     * Set a callback that's invoked after a range of glyphs fail to load.
     *
     * @param callback The callback that's invoked after a range of glyphs fail to load
     */
    fun addOnGlyphsErrorListener(callback: OnGlyphsErrorListener) {
        mapChangeReceiver.addOnGlyphsErrorListener(callback)
    }

    /**
     * Removes a callback that's invoked after a range of glyphs fail to load.
     *
     * @param callback The callback that's invoked after a range of glyphs fail to load
     */
    fun removeOnGlyphsErrorListener(callback: OnGlyphsErrorListener) {
        mapChangeReceiver.removeOnGlyphsErrorListener(callback)
    }

    /**
     * Set a callback that's invoked after a range of glyphs are requested.
     *
     * @param callback The callback that's invoked after a range of glyphs are requested
     */
    fun addOnGlyphsRequestedListener(callback: OnGlyphsRequestedListener) {
        mapChangeReceiver.addOnGlyphsRequestedListener(callback)
    }

    /**
     * Removes a callback that's invoked after a range of glyphs are requested.
     *
     * @param callback The callback that's invoked after a range of glyphs are requested
     */
    fun removeOnGlyphsRequestedListener(callback: OnGlyphsRequestedListener) {
        mapChangeReceiver.removeOnGlyphsRequestedListener(callback)
    }

    /**
     * Set a callback that's invoked after a tile action occurs.
     *
     * @param callback The callback that's invoked after a tile action occurs
     */
    fun addOnTileActionListener(callback: OnTileActionListener) {
        mapChangeReceiver.addOnTileActionListener(callback)
    }

    /**
     * Remove's a callback that's invoked after a tile action occurs.
     *
     * @param callback The callback that's invoked after a tile action occurs
     */
    fun removeOnTileActionListener(callback: OnTileActionListener) {
        mapChangeReceiver.removeOnTileActionListener(callback)
    }

    /**
     * Set a callback that's invoked after a sprite is loaded.
     *
     * @param callback The callback that's invoked after a sprite is loaded
     */
    fun addOnSpriteLoadedListener(callback: OnSpriteLoadedListener) {
        mapChangeReceiver.addOnSpriteLoadedListener(callback)
    }

    /**
     * Removes a callback that's invoked after a sprite is loaded.
     *
     * @param callback The callback that's invoked after a sprite is loaded
     */
    fun removeOnSpriteLoadedListener(callback: OnSpriteLoadedListener) {
        mapChangeReceiver.removeOnSpriteLoadedListener(callback)
    }

    /**
     * Set a callback that's invoked after a sprite fails to load.
     *
     * @param callback The callback that's invoked after a sprite fails to load
     */
    fun addOnSpriteErrorListener(callback: OnSpriteErrorListener) {
        mapChangeReceiver.addOnSpriteErrorListener(callback)
    }

    /**
     * Removes a callback that's invoked after a sprite fails to load.
     *
     * @param callback The callback that's invoked after a sprite fails to load
     */
    fun removeOnSpriteErrorListener(callback: OnSpriteErrorListener) {
        mapChangeReceiver.removeOnSpriteErrorListener(callback)
    }

    /**
     * Set a callback that's invoked after a sprite is requested.
     *
     * @param callback The callback that's invoked after a sprite is requested
     */
    fun addOnSpriteRequestedListener(callback: OnSpriteRequestedListener) {
        mapChangeReceiver.addOnSpriteRequestedListener(callback)
    }

    /**
     * Removes a callback that's invoked after a sprite is requested.
     *
     * @param callback The callback that's invoked after a sprite is requested
     */
    fun removeOnSpriteRequestedListener(callback: OnSpriteRequestedListener) {
        mapChangeReceiver.removeOnSpriteRequestedListener(callback)
    }

    /**
     * Set a callback that's invoked after an error occurs
     * while trying to render a layer or drawable.
     *
     * @param callback The callback that's invoked after an error occurs
     * while trying to render a layer or drawable.
     */
    fun addOnRenderErrorListener(callback: OnRenderErrorListener) {
        mapChangeReceiver.addOnRenderErrorListener(callback)
    }

    /**
     * Removes a callback that's invoked after an error occurs
     * while trying to render a layer or drawable.
     *
     * @param callback The callback that's invoked after an error occurs
     * while trying to render a layer or drawable.
     */
    fun removeOnRenderErrorListener(callback: OnRenderErrorListener) {
        mapChangeReceiver.removeOnRenderErrorListener(callback)
    }

    /**
     * Set a callback that's invoked after a corrupted symbol is detected
     *
     * @param callback The callback that's invoked
     */
    fun addOnSymbolErrorListener(callback: OnSymbolErrorListener) {
        mapChangeReceiver.addOnSymbolErrorListener(callback)
    }

    fun removeOnSymbolErrorListener(callback: OnSymbolErrorListener) {
        mapChangeReceiver.removeOnSymbolErrorListener(callback)
    }

    /**
     * Interface definition for a callback to be invoked when the camera will change.
     *
     * [MapView.addOnCameraWillChangeListener]
     */
    fun interface OnCameraWillChangeListener {
        /**
         * Called when the camera region will change.
         */
        fun onCameraWillChange(animated: Boolean)
    }

    /**
     * Interface definition for a callback to be invoked when the camera is changing.
     *
     * [MapView.addOnCameraIsChangingListener]
     */
    fun interface OnCameraIsChangingListener {
        /**
         * Called when the camera is changing.
         */
        fun onCameraIsChanging()
    }

    /**
     * Interface definition for a callback to be invoked when the map region did change.
     *
     * [MapView.addOnCameraDidChangeListener]
     */
    fun interface OnCameraDidChangeListener {
        /**
         * Called when the camera did change.
         */
        fun onCameraDidChange(animated: Boolean)
    }

    /**
     * Interface definition for a callback to be invoked when the map will start loading.
     *
     * [MapView.addOnWillStartLoadingMapListener]
     */
    fun interface OnWillStartLoadingMapListener {
        /**
         * Called when the map will start loading.
         */
        fun onWillStartLoadingMap()
    }

    /**
     * Interface definition for a callback to be invoked when the map finished loading.
     *
     * [MapView.addOnDidFinishLoadingMapListener]
     */
    fun interface OnDidFinishLoadingMapListener {
        /**
         * Called when the map has finished loading.
         */
        fun onDidFinishLoadingMap()
    }

    /**
     * Interface definition for a callback to be invoked when the map is changing.
     *
     * [MapView.addOnDidFailLoadingMapListener]
     */
    fun interface OnDidFailLoadingMapListener {
        /**
         * Called when the map failed to load.
         *
         * @param errorMessage The reason why the map failed to load
         */
        fun onDidFailLoadingMap(errorMessage: String)
    }

    /**
     * Interface definition for a callback to be invoked when the map will start rendering a frame.
     *
     * [MapView.addOnWillStartRenderingFrameListener]
     */
    fun interface OnWillStartRenderingFrameListener {
        /**
         * Called when the map will start rendering a frame.
         */
        fun onWillStartRenderingFrame()
    }

    /**
     * Interface definition for a callback to be invoked when the map finished rendering a frame.
     *
     * [MapView.addOnDidFinishRenderingFrameListener]
     */
    fun interface OnDidFinishRenderingFrameListener {
        /**
         * Called when the map has finished rendering a frame
         *
         * @param fully true if all frames have been rendered, false if partially rendered
         * @param frameEncodingTime CPU encoding time
         * @param frameRenderingTime CPU rendering time
         */
        fun onDidFinishRenderingFrame(
            fully: Boolean,
            frameEncodingTime: Double,
            frameRenderingTime: Double,
        )
    }

    /**
     * Interface definition for a callback to be invoked when the map finished rendering a frame.
     *
     * [MapView.addOnDidFinishRenderingFrameListener]
     */
    fun interface OnDidFinishRenderingFrameWithStatsListener {
        /**
         * Called when the map has finished rendering a frame
         *
         * @param fully true if all frames have been rendered, false if partially rendered
         * @param stats rendering statistics
         */
        fun onDidFinishRenderingFrame(
            fully: Boolean,
            stats: RenderingStats,
        )
    }

    /**
     * Interface definition for a callback to be invoked when the map will start rendering the map.
     *
     * [MapView.addOnWillStartRenderingMapListener]
     */
    fun interface OnWillStartRenderingMapListener {
        /**
         * Called when the map will start rendering.
         */
        fun onWillStartRenderingMap()
    }

    /**
     * Interface definition for a callback to be invoked when the map is changing.
     *
     * [MapView.addOnDidFinishRenderingMapListener]
     */
    fun interface OnDidFinishRenderingMapListener {
        /**
         * Called when the map has finished rendering.
         *
         * @param fully true if map is fully rendered, false if not fully rendered
         */
        fun onDidFinishRenderingMap(fully: Boolean)
    }

    /**
     * Interface definition for a callback to be invoked when the map has entered the idle state.
     *
     * Calling [MapLibreMap.snapshot] from this callback
     * will result in recursive execution. Use [OnDidFinishRenderingFrameListener] instead.
     *
     * [MapView.addOnDidBecomeIdleListener]
     */
    fun interface OnDidBecomeIdleListener {
        /**
         * Called when the map has entered the idle state.
         */
        fun onDidBecomeIdle()
    }

    /**
     * Interface definition for a callback to be invoked when the map has loaded the style.
     *
     * [MapView.addOnDidFinishLoadingStyleListener]
     */
    fun interface OnDidFinishLoadingStyleListener {
        /**
         * Called when a style has finished loading.
         */
        fun onDidFinishLoadingStyle()
    }

    /**
     * Interface definition for a callback to be invoked when a map source has changed.
     *
     * [MapView.addOnSourceChangedListener]
     */
    fun interface OnSourceChangedListener {
        /**
         * Called when a map source has changed.
         *
         * @param id the id of the source that has changed
         */
        fun onSourceChangedListener(id: String)
    }

    /**
     * Interface definition for a callback to be invoked with the id of a missing icon. The icon should be added
     * synchronously with [Style.addImage] to be rendered on the current zoom level. When loading
     * icons asynchronously, you can load a placeholder image and replace it when you icon has loaded.
     *
     * [MapView.addOnStyleImageMissingListener]
     */
    fun interface OnStyleImageMissingListener {
        /**
         * Called when the map is missing an icon. The icon should be added synchronously with
         * [Style.addImage] to be rendered on the current zoom level. When loading icons
         * asynchronously, you can load a placeholder image and replace it when you icon has loaded.
         *
         * @param id the id of the icon that is missing
         */
        fun onStyleImageMissing(id: String)
    }

    /**
     * Interface definition for a callback to be invoked with an unused image identifier.
     *
     * [MapView.addOnCanRemoveUnusedStyleImageListener]
     */
    fun interface OnCanRemoveUnusedStyleImageListener {
        /**
         * Called when the map needs to release unused image resources.
         *
         * @param id of an image that is not used by the map and can be removed from the style.
         * @return true if image can be removed, false otherwise.
         */
        fun onCanRemoveUnusedStyleImage(id: String): Boolean
    }

    /**
     * Interface definition for a callback to be invoked before a shader is compiled.
     *
     * [MapView.addOnPreCompileShaderListener]
     */
    fun interface OnPreCompileShaderListener {
        /**
         * Called before a shader is compiled.
         *
         * @param id of a shader type enumeration. See `mln::shaders::BuiltIn` for a list
         * of possible values.
         * @param type of graphics backend the shader is being compiled for. See
         * `mln::gfx::Backend::Type` for a list of possible values.
         * @param additionalDefines that specify the permutaion of the shader.
         */
        fun onPreCompileShader(
            id: Int,
            type: Int,
            additionalDefines: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a shader is compiled.
     *
     * [MapView.addOnPostCompileShaderListener]
     */
    fun interface OnPostCompileShaderListener {
        /**
         * Called after a shader is compiled.
         *
         * @param id of a shader type enumeration. See `mln::shaders::BuiltIn` for a list
         * of possible values.
         * @param type of graphics backend the shader is being compiled for. See
         * `mln::gfx::Backend::Type` for a list of possible values.
         * @param additionalDefines that specify the permutation of the shader.
         */
        fun onPostCompileShader(
            id: Int,
            type: Int,
            additionalDefines: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a shader failed to compile.
     *
     * [MapView.addOnShaderCompileFailedListener]
     */
    fun interface OnShaderCompileFailedListener {
        /**
         * Called when a shader fails to compile.
         *
         * @param id of a shader type enumeration. See `mln::shaders::BuiltIn` for a list
         * of possible values.
         * @param type of graphics backend the shader is being compiled for. See
         * `mln::gfx::Backend::Type` for a list of possible values.
         * @param additionalDefines that specify the permutation of the shader.
         */
        fun onShaderCompileFailed(
            id: Int,
            type: Int,
            additionalDefines: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a range of glyphs are loaded.
     *
     * [MapView.addOnGlyphsLoadedListener]
     */
    fun interface OnGlyphsLoadedListener {
        /**
         * Called when a range of glyphs for a font stack are loaded.
         *
         * @param stack of font names.
         * @param rangeStart of glyph indices being loaded.
         * @param rangeEnd of glyph indices being loaded.
         */
        fun onGlyphsLoaded(
            stack: Array<String>,
            rangeStart: Int,
            rangeEnd: Int,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a range of glyphs fail to load.
     *
     * [MapView.addOnGlyphsErrorListener]
     */
    fun interface OnGlyphsErrorListener {
        /**
         * Called when a range of glyphs for a font stack failed to load.
         *
         * @param stack of font names.
         * @param rangeStart of glyph indices that failed to load.
         * @param rangeEnd of glyph indices that failed to load.
         */
        fun onGlyphsError(
            stack: Array<String>,
            rangeStart: Int,
            rangeEnd: Int,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a range of glyphs are requested.
     *
     * [MapView.addOnGlyphsRequestedListener]
     */
    fun interface OnGlyphsRequestedListener {
        /**
         * Called when a range of glyphs for a font stack are requested.
         *
         * @param stack of font names.
         * @param rangeStart of glyph indices that are being requested.
         * @param rangeEnd of glyph indices that are being requested.
         */
        fun onGlyphsRequested(
            stack: Array<String>,
            rangeStart: Int,
            rangeEnd: Int,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a tile action occurs.
     *
     * [MapView.addOnTileActionListener]
     */
    @Suppress("LongParameterList")
    fun interface OnTileActionListener {
        /**
         * Called when a tile action occurs.
         *
         * @param op identifying the tile action that occurred.
         * @param x coordinate of the tile.
         * @param y coordinate of the tile.
         * @param z coordinate of the tile.
         * @param wrap coordinate of the tile.
         * @param overscaledZ coordinate of the tile.
         * @param sourceID of the tile.
         */
        fun onTileAction(
            op: TileOperation,
            x: Int,
            y: Int,
            z: Int,
            wrap: Int,
            overscaledZ: Int,
            sourceID: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a sprite is requested.
     *
     * [MapView.addOnSpriteLoadedListener]
     */
    fun interface OnSpriteLoadedListener {
        /**
         * Called when a sprite is loaded.
         *
         * @param id of the sprite.
         * @param url of the sprite.
         */
        fun onSpriteLoaded(
            id: String,
            url: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a sprite fails to load.
     *
     * [MapView.addOnSpriteErrorListener]
     */
    fun interface OnSpriteErrorListener {
        /**
         * Called when a sprite fails to load.
         *
         * @param id of the sprite.
         * @param url of the sprite.
         */
        fun onSpriteError(
            id: String,
            url: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after a sprite is requested.
     *
     * [MapView.addOnSpriteRequestedListener]
     */
    fun interface OnSpriteRequestedListener {
        /**
         * Called when a sprite is requested.
         *
         * @param id of the sprite.
         * @param url of the sprite.
         */
        fun onSpriteRequested(
            id: String,
            url: String,
        )
    }

    /**
     * Interface definition for a callback to be invoked after an error occurs
     * while trying to render a layer or drawable.
     *
     * [MapView.addOnRenderErrorListener]
     */
    fun interface OnRenderErrorListener {
        /**
         * Called when an error occurs while trying to render a layer or drawable.
         */
        fun onRenderError()
    }

    /**
     * Interface definition for a callback to be invoked after a corrupted symbol is detected
     *
     * [MapView.addOnSymbolErrorListener]
     */
    fun interface OnSymbolErrorListener {
        /**
         * Called when a corrupted symbol is detected.
         */
        fun onSymbolError(message: String)
    }

    /**
     * Sets a callback object which will be triggered when the [MapLibreMap] instance is ready to be used.
     *
     * @param callback The callback object that will be triggered when the map is ready to be used.
     */
    @UiThread
    fun getMapAsync(callback: OnMapReadyCallback) {
        val map = maplibreMap
        if (map == null) {
            // Add callback to the list only if the style hasn't loaded, or the drawing surface isn't ready
            mapCallback.addOnMapReadyCallback(callback)
        } else {
            callback.onMapReady(map)
        }
    }

    internal fun getMapLibreMap(): MapLibreMap? = maplibreMap

    internal fun setMapLibreMap(maplibreMap: MapLibreMap?) {
        this.maplibreMap = maplibreMap
    }

    private inner class FocalPointInvalidator : FocalPointChangeListener {
        private val focalPointChangeListeners = mutableListOf<FocalPointChangeListener>()

        fun addListener(focalPointChangeListener: FocalPointChangeListener) {
            focalPointChangeListeners.add(focalPointChangeListener)
        }

        override fun onFocalPointChanged(pointF: PointF?) {
            mapGestureDetector?.setFocalPoint(pointF)
            for (focalPointChangeListener in focalPointChangeListeners) {
                focalPointChangeListener.onFocalPointChanged(pointF)
            }
        }
    }

    /**
     * The initial render callback waits for rendering to happen before making the map visible for end-users.
     * We wait for the second DID_FINISH_RENDERING_FRAME map change event as the first will still show a black surface.
     */
    private inner class InitialRenderCallback : OnDidFinishRenderingFrameWithStatsListener {
        private var renderCount = 0

        init {
            addOnDidFinishRenderingFrameListener(this)
        }

        override fun onDidFinishRenderingFrame(
            fully: Boolean,
            stats: RenderingStats,
        ) {
            if (maplibreMap?.style != null) {
                renderCount++
                if (renderCount == RENDER_COUNT_THRESHOLD) {
                    this@MapView.foreground = null
                    removeOnDidFinishRenderingFrameListener(this)
                }
            }
        }

        fun onDestroy() {
            removeOnDidFinishRenderingFrameListener(this)
        }
    }

    @Suppress("TooManyFunctions")
    private inner class GesturesManagerInteractionListener : MapLibreMap.OnGesturesManagerInteractionListener {
        override fun onAddMapClickListener(listener: MapLibreMap.OnMapClickListener) {
            mapGestureDetector?.addOnMapClickListener(listener)
        }

        override fun onRemoveMapClickListener(listener: MapLibreMap.OnMapClickListener) {
            mapGestureDetector?.removeOnMapClickListener(listener)
        }

        override fun onAddMapLongClickListener(listener: MapLibreMap.OnMapLongClickListener) {
            mapGestureDetector?.addOnMapLongClickListener(listener)
        }

        override fun onRemoveMapLongClickListener(listener: MapLibreMap.OnMapLongClickListener) {
            mapGestureDetector?.removeOnMapLongClickListener(listener)
        }

        override fun onAddFlingListener(listener: MapLibreMap.OnFlingListener) {
            mapGestureDetector?.addOnFlingListener(listener)
        }

        override fun onRemoveFlingListener(listener: MapLibreMap.OnFlingListener) {
            mapGestureDetector?.removeOnFlingListener(listener)
        }

        override fun onAddMoveListener(listener: MapLibreMap.OnMoveListener) {
            mapGestureDetector?.addOnMoveListener(listener)
        }

        override fun onRemoveMoveListener(listener: MapLibreMap.OnMoveListener) {
            mapGestureDetector?.removeOnMoveListener(listener)
        }

        override fun onAddRotateListener(listener: MapLibreMap.OnRotateListener) {
            mapGestureDetector?.addOnRotateListener(listener)
        }

        override fun onRemoveRotateListener(listener: MapLibreMap.OnRotateListener) {
            mapGestureDetector?.removeOnRotateListener(listener)
        }

        override fun onAddScaleListener(listener: MapLibreMap.OnScaleListener) {
            mapGestureDetector?.addOnScaleListener(listener)
        }

        override fun onRemoveScaleListener(listener: MapLibreMap.OnScaleListener) {
            mapGestureDetector?.removeOnScaleListener(listener)
        }

        override fun onAddShoveListener(listener: MapLibreMap.OnShoveListener) {
            mapGestureDetector?.addShoveListener(listener)
        }

        override fun onRemoveShoveListener(listener: MapLibreMap.OnShoveListener) {
            mapGestureDetector?.removeShoveListener(listener)
        }

        override fun getGesturesManager(): AndroidGesturesManager = mapGestureDetector!!.getGesturesManager()

        override fun setGesturesManager(
            gesturesManager: AndroidGesturesManager,
            attachDefaultListeners: Boolean,
            setDefaultMutuallyExclusives: Boolean,
        ) {
            mapGestureDetector?.setGesturesManager(
                context,
                gesturesManager,
                attachDefaultListeners,
                setDefaultMutuallyExclusives,
            )
        }

        override fun cancelAllVelocityAnimations() {
            mapGestureDetector?.cancelAnimators()
        }
    }

    private inner class MapCallback :
        OnDidFinishLoadingStyleListener,
        OnDidFinishRenderingFrameWithStatsListener,
        OnDidFinishLoadingMapListener,
        OnCameraIsChangingListener,
        OnCameraDidChangeListener,
        OnDidFailLoadingMapListener {
        private val onMapReadyCallbackList = mutableListOf<OnMapReadyCallback>()

        init {
            addOnDidFinishLoadingStyleListener(this)
            addOnDidFinishRenderingFrameListener(this)
            addOnDidFinishLoadingMapListener(this)
            addOnCameraIsChangingListener(this)
            addOnCameraDidChangeListener(this)
            addOnDidFailLoadingMapListener(this)
        }

        fun initialised() {
            val map = maplibreMap ?: return
            map.onPreMapReady()
            onMapReady(map)
            map.onPostMapReady()
        }

        /**
         * Notify listeners, clear when done
         */
        private fun onMapReady(map: MapLibreMap) {
            val iterator = onMapReadyCallbackList.iterator()
            while (iterator.hasNext()) {
                iterator.next().onMapReady(map)
                iterator.remove()
            }
        }

        fun addOnMapReadyCallback(callback: OnMapReadyCallback) {
            onMapReadyCallbackList.add(callback)
        }

        fun onDestroy() {
            onMapReadyCallbackList.clear()
            removeOnDidFinishLoadingStyleListener(this)
            removeOnDidFinishRenderingFrameListener(this)
            removeOnDidFinishLoadingMapListener(this)
            removeOnCameraIsChangingListener(this)
            removeOnCameraDidChangeListener(this)
            removeOnDidFailLoadingMapListener(this)
        }

        override fun onDidFinishLoadingStyle() {
            maplibreMap?.onFinishLoadingStyle()
        }

        override fun onDidFailLoadingMap(errorMessage: String) {
            maplibreMap?.onFailLoadingStyle()
        }

        override fun onDidFinishRenderingFrame(
            fully: Boolean,
            stats: RenderingStats,
        ) {
            maplibreMap?.onUpdateFullyRendered()
        }

        override fun onDidFinishLoadingMap() {
            maplibreMap?.onUpdateRegionChange()
        }

        override fun onCameraIsChanging() {
            maplibreMap?.onUpdateRegionChange()
        }

        override fun onCameraDidChange(animated: Boolean) {
            maplibreMap?.onUpdateRegionChange()
        }
    }

    /**
     * Click event hook for providing a custom attribution dialog manager.
     */
    private class AttributionClickListener(
        context: Context,
        maplibreMap: MapLibreMap,
    ) : OnClickListener {
        private val defaultDialogManager = AttributionDialogManager(context, maplibreMap)
        private val uiSettings: UiSettings = maplibreMap.uiSettings

        override fun onClick(v: View) {
            getDialogManager().onClick(v)
        }

        fun onStop() {
            getDialogManager().onStop()
        }

        private fun getDialogManager(): AttributionDialogManager = uiSettings.getAttributionDialogManager() ?: defaultDialogManager
    }

    companion object {
        private const val RENDER_COUNT_THRESHOLD = 3

        /**
         * Sets the strict mode that will throw the [org.maplibre.android.MapStrictModeException]
         * whenever the map would fail silently otherwise.
         *
         * @param strictModeEnabled true to enable the strict mode, false otherwise
         */
        @JvmStatic
        fun setMapStrictModeEnabled(strictModeEnabled: Boolean) {
            MapStrictMode.setStrictModeEnabled(strictModeEnabled)
        }
    }
}
