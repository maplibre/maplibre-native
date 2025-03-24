/*
 * Copyright (c) 2012-2025 Grab Taxi Holdings PTE LTD (GRAB), All Rights Reserved. NOTICE: All information contained herein is, and remains the property of GRAB. The intellectual and technical concepts contained herein are confidential, proprietary and controlled by GRAB and may be covered by patents, patents in process, and are protected by trade secret or copyright law.
 * You are strictly forbidden to copy, download, store (in any medium), transmit, disseminate, adapt or change this material in any way unless prior written permission is obtained from GRAB. Access to the source code contained herein is hereby forbidden to anyone except current GRAB employees or contractors with binding Confidentiality and Non-disclosure agreements explicitly covering such access.
 *
 * The copyright notice above does not evidence any actual or intended publication or disclosure of this source code, which includes information that is confidential and/or proprietary, and is a trade secret, of GRAB.
 * ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC PERFORMANCE, OR PUBLIC DISPLAY OF OR THROUGH USE OF THIS SOURCE CODE WITHOUT THE EXPRESS WRITTEN CONSENT OF GRAB IS STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE LAWS AND INTERNATIONAL TREATIES. THE RECEIPT OR POSSESSION OF THIS SOURCE CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE, OR SELL ANYTHING THAT IT MAY DESCRIBE, IN WHOLE OR IN PART.
 */

package org.maplibre.android.maps

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.Rect
import android.os.Handler
import android.os.Looper
import android.view.Surface
import org.maplibre.android.MapLibre
import org.maplibre.android.gestures.AndroidGesturesManager
import org.maplibre.android.location.LocationComponent
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap.OnDeveloperAnimationListener
import org.maplibre.android.maps.renderer.surface.SurfaceMapRenderer
import org.maplibre.android.net.ConnectivityReceiver
import org.maplibre.android.storage.FileSource

class MapSurface(
    private val context: Context,
    grabMapOptions: MapLibreMapOptions,
    private var surface: Surface?,
    private var width: Int,
    private var height: Int
) : NativeMapView.ViewCallback {

    private val mapChangeReceiver = MapChangeReceiver()

    private val mapCallback = MapCallback()

    private var mapGestureDetector: MapSurfaceGestureDetector? = null

    private var grabMap: MapLibreMap? = null

    private var mapRenderer: SurfaceMapRenderer? = null

    private var nativeMapView: NativeMapView? = null

    private var destroyed = false

    private val handler: Handler = Handler(Looper.getMainLooper())

    private var isStarted = false

    private val mapLibreMapOptions: MapLibreMapOptions = grabMapOptions

    init {
        initialiseDrawingSurface()
    }

    private fun initialiseDrawingSurface() {
        mapRenderer = SurfaceMapRenderer(context)
        val crossSourceCollisions = mapLibreMapOptions.crossSourceCollisions
        nativeMapView = NativeMapView(
            context,
            getPixelRatio(context),
            crossSourceCollisions,
            this,
            mapChangeReceiver,
            mapRenderer
        )

        mapRenderer!!.onSurfaceCreated(surface, width, height)
        mapRenderer!!.onSurfaceChanged(width, height)
        nativeMapView!!.resizeView(width, height)
        onSurfaceCreated()
    }

    private fun onSurfaceCreated() {
        handler.post {
            if (!destroyed && grabMap == null) {
                this@MapSurface.initialiseMap()
                grabMap!!.onStart()
            }
        }
    }

    private fun initialiseMap() {
        Logger.d(TAG, "initialiseMap")
        // callback for focal point invalidation
        val focalInvalidator = FocalPointInvalidator()
        focalInvalidator.addListener(createFocalPointChangeListener())

        // callback for registering touch listeners
        val registerTouchListener = GesturesManagerInteractionListener()

        // callback for camera change events
        val cameraDispatcher = CameraChangeDispatcher()


        // setup components for GrabMap creation
        val proj: Projection = ProjectionAuto(
            nativeMapView!!, width, height
        )
        val transform = TransformAuto(
            this, nativeMapView, cameraDispatcher
        )
        val uiSettings = UiSettings(
            proj, focalInvalidator, getPixelRatio(
                context
            ), null
        )

        // GrabMap
        val developerAnimationListeners: List<OnDeveloperAnimationListener> = ArrayList()
        grabMap = MapLibreMap(
            nativeMapView,
            transform,
            uiSettings,
            proj,
            registerTouchListener,
            cameraDispatcher,
            developerAnimationListeners
        )

        // user input
        mapGestureDetector =
            MapSurfaceGestureDetector(context, transform, proj, uiSettings, cameraDispatcher)

        // LocationComponent
        grabMap!!.injectLocationComponent(
            LocationComponent(
                grabMap!!,
                transform,
                developerAnimationListeners
            )
        )
        grabMap!!.injectAnnotationManager(AnnotationManagerAuto())

        // notify Map object about current connectivity state
        nativeMapView!!.setReachability(MapLibre.isConnected())

        // initialise GrabMap
        mapLibreMapOptions.compassEnabled(false).attributionEnabled(false).logoEnabled(false)
        grabMap!!.initialise(context, mapLibreMapOptions)

        mapCallback.initialised()
    }

    fun onSurfaceDestroyed() {
        mapRenderer!!.onSurfaceDestroyed()
    }

    fun isStarted(): Boolean {
        return isStarted
    }

    fun onStart() {
        if (!isStarted) {
            ConnectivityReceiver.instance(getContext()).activate()
            FileSource.getInstance(getContext()).activate()
            isStarted = true
        }

        mapRenderer!!.onStart()
    }

    fun onResume() {
    }


    fun onPause() {
    }

    fun onStop() {
        mapRenderer!!.onStop()

        if (isStarted) {
            ConnectivityReceiver.instance(getContext()).deactivate()
            FileSource.getInstance(getContext()).deactivate()
            isStarted = false
        }
    }

    fun onDestroy() {
        destroyed = true
        mapRenderer!!.onDestroy()
    }

    fun onLowMemory() {
        if (nativeMapView != null && !destroyed) {
            nativeMapView!!.onLowMemory()
        }
    }

    fun onVisibleAreaChanged(visibleArea: Rect) {
        mapGestureDetector?.onVisibleAreaChanged(visibleArea)
    }

    fun onScroll(distanceX: Float, distanceY: Float) {
        mapGestureDetector?.onScroll(distanceX, distanceY)
    }

    fun onFling(velocityX: Float, velocityY: Float) {
        mapGestureDetector?.onFling(velocityX, velocityY)
    }

    fun onScale(focusX: Float, focusY: Float, scaleFactor: Float) {
        mapGestureDetector?.onScale(focusX, focusY, scaleFactor)
    }

    fun onClick(x: Float, y: Float) {
        mapGestureDetector?.onClick(x, y)
    }

    fun isAndroidView(): Boolean {
        return false
    }

    private fun getPixelRatio(context: Context): Float {
        // check is user defined his own pixel ratio value
        var pixelRatio = mapLibreMapOptions.pixelRatio
        if (pixelRatio == 0f) {
            // if not, get the one defined by the system
            pixelRatio = context.resources.displayMetrics.density
        }
        if (pixelRatio == 0f) {
            pixelRatio = 1.0f
        }

        Logger.d(TAG, "pixelRatio:$pixelRatio")
        return pixelRatio
    }

    private fun getFontScale(context: Context): Float {
        return context.resources.configuration.fontScale
    }

    fun getMapAsync(callback: OnMapReadyCallback) {
        if (grabMap == null) {
            // Add callback to the list only if the style hasn't loaded, or the drawing surface isn't ready
            mapCallback.addOnMapReadyCallback(callback)
        } else {
            callback.onMapReady(grabMap!!)
        }
    }

    fun getWidth(): Int {
        return width
    }

    fun setMaximumFps(maxFps: Int) {
        if (mapRenderer != null) {
            mapRenderer!!.setMaximumFps(maxFps)
        } else {
            throw IllegalStateException("Calling MapView#setMaximumFps before mapRenderer is created.")
        }
    }

    fun getContext(): Context {
        return this.context
    }

    fun post(action: Runnable): Boolean {
        return handler.post(action)
    }

    fun getHeight(): Int {
        return height
    }

    fun isDestroyed(): Boolean {
        return destroyed
    }

    override fun getViewContent(): Bitmap? {
        return null
    }

    fun addOnCameraWillChangeListener(listener: MapView.OnCameraWillChangeListener) {
        mapChangeReceiver.addOnCameraWillChangeListener(listener)
    }

    fun removeOnCameraWillChangeListener(listener: MapView.OnCameraWillChangeListener) {
        mapChangeReceiver.removeOnCameraWillChangeListener(listener)
    }

    fun addOnCameraIsChangingListener(listener: MapView.OnCameraIsChangingListener) {
        mapChangeReceiver.addOnCameraIsChangingListener(listener)
    }

    fun removeOnCameraIsChangingListener(listener: MapView.OnCameraIsChangingListener) {
        mapChangeReceiver.removeOnCameraIsChangingListener(listener)
    }

    fun addOnCameraDidChangeListener(listener: MapView.OnCameraDidChangeListener) {
        mapChangeReceiver.addOnCameraDidChangeListener(listener)
    }

    fun removeOnCameraDidChangeListener(listener: MapView.OnCameraDidChangeListener) {
        mapChangeReceiver.removeOnCameraDidChangeListener(listener)
    }

    fun addOnWillStartLoadingMapListener(listener: MapView.OnWillStartLoadingMapListener) {
        mapChangeReceiver.addOnWillStartLoadingMapListener(listener)
    }

    fun removeOnWillStartLoadingMapListener(listener: MapView.OnWillStartLoadingMapListener) {
        mapChangeReceiver.removeOnWillStartLoadingMapListener(listener)
    }

    fun addOnDidFinishLoadingMapListener(listener: MapView.OnDidFinishLoadingMapListener) {
        mapChangeReceiver.addOnDidFinishLoadingMapListener(listener)
    }

    fun removeOnDidFinishLoadingMapListener(listener: MapView.OnDidFinishLoadingMapListener) {
        mapChangeReceiver.removeOnDidFinishLoadingMapListener(listener)
    }

    fun addOnDidFailLoadingMapListener(listener: MapView.OnDidFailLoadingMapListener) {
        mapChangeReceiver.addOnDidFailLoadingMapListener(listener)
    }

    fun removeOnDidFailLoadingMapListener(listener: MapView.OnDidFailLoadingMapListener) {
        mapChangeReceiver.removeOnDidFailLoadingMapListener(listener)
    }

    fun addOnWillStartRenderingFrameListener(listener: MapView.OnWillStartRenderingFrameListener) {
        mapChangeReceiver.addOnWillStartRenderingFrameListener(listener)
    }

    fun removeOnWillStartRenderingFrameListener(listener: MapView.OnWillStartRenderingFrameListener) {
        mapChangeReceiver.removeOnWillStartRenderingFrameListener(listener)
    }

    fun addOnDidFinishRenderingFrameListener(listener: MapView.OnDidFinishRenderingFrameListener) {
        mapChangeReceiver.addOnDidFinishRenderingFrameListener(listener)
    }

    fun removeOnDidFinishRenderingFrameListener(listener: MapView.OnDidFinishRenderingFrameListener) {
        mapChangeReceiver.removeOnDidFinishRenderingFrameListener(listener)
    }

    fun addOnWillStartRenderingMapListener(listener: MapView.OnWillStartRenderingMapListener) {
        mapChangeReceiver.addOnWillStartRenderingMapListener(listener)
    }

    fun removeOnWillStartRenderingMapListener(listener: MapView.OnWillStartRenderingMapListener) {
        mapChangeReceiver.removeOnWillStartRenderingMapListener(listener)
    }

    fun addOnDidFinishRenderingMapListener(listener: MapView.OnDidFinishRenderingMapListener) {
        mapChangeReceiver.addOnDidFinishRenderingMapListener(listener)
    }

    fun removeOnDidFinishRenderingMapListener(listener: MapView.OnDidFinishRenderingMapListener) {
        mapChangeReceiver.removeOnDidFinishRenderingMapListener(listener)
    }

    fun addOnDidBecomeIdleListener(listener: MapView.OnDidBecomeIdleListener) {
        mapChangeReceiver.addOnDidBecomeIdleListener(listener)
    }

    fun removeOnDidBecomeIdleListener(listener: MapView.OnDidBecomeIdleListener) {
        mapChangeReceiver.removeOnDidBecomeIdleListener(listener)
    }

    fun addOnDidFinishLoadingStyleListener(listener: MapView.OnDidFinishLoadingStyleListener) {
        mapChangeReceiver.addOnDidFinishLoadingStyleListener(listener)
    }

    fun removeOnDidFinishLoadingStyleListener(listener: MapView.OnDidFinishLoadingStyleListener) {
        mapChangeReceiver.removeOnDidFinishLoadingStyleListener(listener)
    }

    fun addOnSourceChangedListener(listener: MapView.OnSourceChangedListener) {
        mapChangeReceiver.addOnSourceChangedListener(listener)
    }

    fun removeOnSourceChangedListener(listener: MapView.OnSourceChangedListener?) {
        mapChangeReceiver.removeOnSourceChangedListener(listener)
    }

    private inner class FocalPointInvalidator : FocalPointChangeListener {
        private val focalPointChangeListeners: MutableList<FocalPointChangeListener> = ArrayList()

        fun addListener(focalPointChangeListener: FocalPointChangeListener) {
            focalPointChangeListeners.add(focalPointChangeListener)
        }

        override fun onFocalPointChanged(pointF: PointF) {
            mapGestureDetector?.setFocalPoint(pointF)
            for (focalPointChangeListener in focalPointChangeListeners) {
                focalPointChangeListener.onFocalPointChanged(pointF)
            }
        }
    }

    private inner class GesturesManagerInteractionListener :
        MapLibreMap.OnGesturesManagerInteractionListener {
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
        }

        override fun onRemoveRotateListener(listener: MapLibreMap.OnRotateListener) {
        }

        override fun onAddScaleListener(listener: MapLibreMap.OnScaleListener) {
            mapGestureDetector?.addOnScaleListener(listener)
        }

        override fun onRemoveScaleListener(listener: MapLibreMap.OnScaleListener) {
            mapGestureDetector?.removeOnScaleListener(listener)
        }

        override fun onAddShoveListener(listener: MapLibreMap.OnShoveListener) {
        }

        override fun onRemoveShoveListener(listener: MapLibreMap.OnShoveListener) {
        }

        override fun getGesturesManager(): AndroidGesturesManager? {
            return null
        }

        override fun setGesturesManager(
            gesturesManager: AndroidGesturesManager, attachDefaultListeners: Boolean,
            setDefaultMutuallyExclusives: Boolean
        ) {
        }

        override fun cancelAllVelocityAnimations() {
            mapGestureDetector?.cancelAnimators()
        }
    }

    private fun createFocalPointChangeListener(): FocalPointChangeListener {
        return FocalPointChangeListener { _: PointF? -> }
    }

    private inner class MapCallback : MapView.OnDidFinishLoadingStyleListener,
        MapView.OnDidFinishRenderingFrameListener, MapView.OnDidFinishLoadingMapListener,
        MapView.OnCameraIsChangingListener, MapView.OnCameraDidChangeListener,
        MapView.OnDidFailLoadingMapListener {
        private val onMapReadyCallbackList: MutableList<OnMapReadyCallback> = ArrayList()

        init {
            addOnDidFinishLoadingStyleListener(this)
            addOnDidFinishRenderingFrameListener(this)
            addOnDidFinishLoadingMapListener(this)
            addOnCameraIsChangingListener(this)
            addOnCameraDidChangeListener(this)
            addOnDidFailLoadingMapListener(this)
        }

        fun initialised() {
            grabMap!!.onPreMapReady()
            onMapReady()
            grabMap!!.onPostMapReady()
        }

        /**
         * Notify listeners, clear when done
         */
        fun onMapReady() {
            if (onMapReadyCallbackList.size > 0) {
                val iterator = onMapReadyCallbackList.iterator()
                while (iterator.hasNext()) {
                    val callback = iterator.next()
                    callback.onMapReady(grabMap!!)
                    iterator.remove()
                }
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
            grabMap?.onFinishLoadingStyle()
        }

        override fun onDidFailLoadingMap(errorMessage: String) {
            grabMap?.onFailLoadingStyle()
        }

        override fun onDidFinishLoadingMap() {
            grabMap?.onUpdateRegionChange()
        }

        override fun onCameraIsChanging() {
            grabMap?.onUpdateRegionChange()
        }

        override fun onCameraDidChange(animated: Boolean) {
            grabMap?.onUpdateRegionChange()
        }

        override fun onDidFinishRenderingFrame(
            fully: Boolean,
            frameEncodingTime: Double,
            frameRenderingTime: Double
        ) {
            if (fully) {
                grabMap?.onUpdateFullyRendered()
            }
        }
    }

    companion object {
        private const val TAG: String = "GrabMap-MapSurface"
    }
}
