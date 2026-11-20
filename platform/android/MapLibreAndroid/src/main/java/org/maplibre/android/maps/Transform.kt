package org.maplibre.android.maps

import android.graphics.PointF
import android.os.Handler
import android.os.Looper
import androidx.annotation.UiThread
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdate
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap.OnCameraMoveStartedListener

/**
 * Internal use.
 *
 * Resembles the current Map transformation.
 *
 * Responsible for synchronising [CameraPosition] state and notifying camera change listeners.
 */
@Suppress("TooManyFunctions")
class Transform internal constructor(
    private val mapView: MapView,
    private val nativeMap: NativeMap,
    private val cameraChangeDispatcher: CameraChangeDispatcher,
) : MapView.OnCameraDidChangeListener {
    private val handler = Handler(Looper.getMainLooper())

    private var currentCameraPosition: CameraPosition? = null
    private var cameraCancelableCallback: MapLibreMap.CancelableCallback? = null

    private val moveByChangeListener =
        object : MapView.OnCameraDidChangeListener {
            override fun onCameraDidChange(animated: Boolean) {
                if (animated) {
                    cameraChangeDispatcher.onCameraIdle()
                    mapView.removeOnCameraDidChangeListener(this)
                }
            }
        }

    internal fun initialise(
        maplibreMap: MapLibreMap,
        options: MapLibreMapOptions,
    ) {
        val position = options.camera
        if (position != null && position != CameraPosition.DEFAULT) {
            moveCamera(maplibreMap, CameraUpdateFactory.newCameraPosition(position), null)
        }
        minZoom = options.minZoomPreference
        maxZoom = options.maxZoomPreference
        minPitch = options.minPitchPreference
        maxPitch = options.maxPitchPreference
    }

    //
    // Camera API
    //

    @get:UiThread
    val cameraPosition: CameraPosition?
        get() {
            if (currentCameraPosition == null) {
                currentCameraPosition = invalidateCameraPosition()
            }
            return currentCameraPosition
        }

    override fun onCameraDidChange(animated: Boolean) {
        if (animated) {
            invalidateCameraPosition()
            cameraCancelableCallback?.let { callback ->
                // nullification has to happen before Handler#post,
                // see https://github.com/robolectric/robolectric/issues/1306
                cameraCancelableCallback = null

                handler.post { callback.onFinish() }
            }
            cameraChangeDispatcher.onCameraIdle()
            mapView.removeOnCameraDidChangeListener(this)
        }
    }

    /**
     * Internal use.
     */
    @UiThread
    fun moveCamera(
        maplibreMap: MapLibreMap,
        update: CameraUpdate,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        val cameraPosition = update.getCameraPosition(maplibreMap)
        if (isValidCameraPosition(cameraPosition)) {
            cancelTransitions()
            cameraChangeDispatcher.onCameraMoveStarted(OnCameraMoveStartedListener.REASON_API_ANIMATION)
            nativeMap.jumpTo(
                cameraPosition!!.target!!,
                cameraPosition.zoom,
                cameraPosition.tilt,
                cameraPosition.bearing,
                cameraPosition.padding,
            )
            invalidateCameraPosition()
            cameraChangeDispatcher.onCameraIdle()
            handler.post { callback?.onFinish() }
        } else {
            callback?.onFinish()
        }
    }

    @UiThread
    @Suppress("LongParameterList")
    internal fun easeCamera(
        maplibreMap: MapLibreMap,
        update: CameraUpdate,
        durationMs: Int,
        easingInterpolator: Boolean,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        val cameraPosition = update.getCameraPosition(maplibreMap)
        if (isValidCameraPosition(cameraPosition)) {
            cancelTransitions()
            cameraChangeDispatcher.onCameraMoveStarted(OnCameraMoveStartedListener.REASON_API_ANIMATION)

            if (callback != null) {
                cameraCancelableCallback = callback
            }
            mapView.addOnCameraDidChangeListener(this)
            nativeMap.easeTo(
                cameraPosition!!.target!!,
                cameraPosition.zoom,
                cameraPosition.bearing,
                cameraPosition.tilt,
                cameraPosition.padding,
                durationMs.toLong(),
                easingInterpolator,
            )
        } else {
            callback?.onFinish()
        }
    }

    /**
     * Internal use.
     */
    @UiThread
    fun animateCamera(
        maplibreMap: MapLibreMap,
        update: CameraUpdate,
        durationMs: Int,
        callback: MapLibreMap.CancelableCallback?,
    ) {
        val cameraPosition = update.getCameraPosition(maplibreMap)
        if (isValidCameraPosition(cameraPosition)) {
            cancelTransitions()
            cameraChangeDispatcher.onCameraMoveStarted(OnCameraMoveStartedListener.REASON_API_ANIMATION)

            if (callback != null) {
                cameraCancelableCallback = callback
            }
            mapView.addOnCameraDidChangeListener(this)
            nativeMap.flyTo(
                cameraPosition!!.target!!,
                cameraPosition.zoom,
                cameraPosition.bearing,
                cameraPosition.tilt,
                cameraPosition.padding,
                durationMs.toLong(),
            )
        } else {
            callback?.onFinish()
        }
    }

    private fun isValidCameraPosition(cameraPosition: CameraPosition?): Boolean =
        cameraPosition != null && cameraPosition != currentCameraPosition

    @UiThread
    internal fun invalidateCameraPosition(): CameraPosition? {
        val updatedCameraPosition = nativeMap.cameraPosition
        val previousCameraPosition = currentCameraPosition
        if (previousCameraPosition != null && previousCameraPosition != updatedCameraPosition) {
            cameraChangeDispatcher.onCameraMove()
        }

        currentCameraPosition = updatedCameraPosition
        return currentCameraPosition
    }

    internal fun cancelTransitions() {
        // notify user about cancel
        cameraChangeDispatcher.onCameraMoveCanceled()

        // notify animateCamera and easeCamera about cancelling
        cameraCancelableCallback?.let { callback ->
            cameraChangeDispatcher.onCameraIdle()

            // nullification has to happen before Handler#post,
            // see https://github.com/robolectric/robolectric/issues/1306
            cameraCancelableCallback = null

            handler.post { callback.onCancel() }
        }

        // cancel ongoing transitions
        nativeMap.cancelTransitions()

        cameraChangeDispatcher.onCameraIdle()
    }

    @UiThread
    internal fun resetNorth() {
        cancelTransitions()
        nativeMap.resetNorth()
    }

    //
    // non Camera API
    //

    // Zoom in or out

    internal fun getRawZoom(): Double = nativeMap.zoom

    internal fun zoomBy(
        zoomAddition: Double,
        focalPoint: PointF,
    ) {
        setZoom(nativeMap.zoom + zoomAddition, focalPoint)
    }

    internal fun setZoom(
        zoom: Double,
        focalPoint: PointF,
    ) {
        nativeMap.setZoom(zoom, focalPoint, 0)
    }

    // Direction
    internal fun getBearing(): Double {
        var direction = -nativeMap.bearing

        while (direction > 360) {
            direction -= 360.0
        }
        while (direction < 0) {
            direction += 360.0
        }

        return direction
    }

    internal fun getRawBearing(): Double = nativeMap.bearing

    internal fun setBearing(bearing: Double) {
        nativeMap.setBearing(bearing, 0)
    }

    internal fun setBearing(
        bearing: Double,
        focalX: Float,
        focalY: Float,
    ) {
        nativeMap.setBearing(bearing, focalX.toDouble(), focalY.toDouble(), 0)
    }

    internal fun setBearing(
        bearing: Double,
        focalX: Float,
        focalY: Float,
        duration: Long,
    ) {
        nativeMap.setBearing(bearing, focalX.toDouble(), focalY.toDouble(), duration)
    }

    //
    // LatLng / CenterCoordinate
    //

    internal fun getLatLng(): LatLng = nativeMap.latLng

    //
    // Pitch / Tilt
    //

    internal fun getTilt(): Double = nativeMap.pitch

    internal fun setTilt(pitch: Double) {
        nativeMap.setPitch(pitch, 0)
    }

    //
    // Center coordinate
    //

    internal fun getCenterCoordinate(): LatLng = nativeMap.latLng

    internal fun setCenterCoordinate(centerCoordinate: LatLng) {
        nativeMap.setLatLng(centerCoordinate, 0)
    }

    internal fun setGestureInProgress(gestureInProgress: Boolean) {
        nativeMap.setGestureInProgress(gestureInProgress)
        if (!gestureInProgress) {
            invalidateCameraPosition()
        }
    }

    internal fun moveBy(
        offsetX: Double,
        offsetY: Double,
        duration: Long,
    ) {
        if (duration > 0) {
            mapView.addOnCameraDidChangeListener(moveByChangeListener)
        }
        nativeMap.moveBy(offsetX, offsetY, duration)
    }

    //
    // Min & Max ZoomLevel
    //

    internal var minZoom: Double
        get() = nativeMap.minZoom
        set(value) {
            if (value < MapLibreConstants.MINIMUM_ZOOM || value > MapLibreConstants.MAXIMUM_ZOOM) {
                Logger.e(TAG, "Not setting minZoomPreference, value is in unsupported range: $value")
                return
            }
            nativeMap.minZoom = value
        }

    internal var maxZoom: Double
        get() = nativeMap.maxZoom
        set(value) {
            if (value < MapLibreConstants.MINIMUM_ZOOM || value > MapLibreConstants.MAXIMUM_ZOOM) {
                Logger.e(TAG, "Not setting maxZoomPreference, value is in unsupported range: $value")
                return
            }
            nativeMap.maxZoom = value
        }

    internal var minPitch: Double
        get() = nativeMap.minPitch
        set(value) {
            if (value < MapLibreConstants.MINIMUM_PITCH || value > MapLibreConstants.MAXIMUM_PITCH) {
                Logger.e(TAG, "Not setting minPitchPreference, value is in unsupported range: $value")
                return
            }
            nativeMap.minPitch = value
        }

    internal var maxPitch: Double
        get() = nativeMap.maxPitch
        set(value) {
            if (value < MapLibreConstants.MINIMUM_PITCH || value > MapLibreConstants.MAXIMUM_PITCH) {
                Logger.e(TAG, "Not setting maxPitchPreference, value is in unsupported range: $value")
                return
            }
            nativeMap.maxPitch = value
        }

    private companion object {
        const val TAG = "Mbgl-Transform"
    }
}
