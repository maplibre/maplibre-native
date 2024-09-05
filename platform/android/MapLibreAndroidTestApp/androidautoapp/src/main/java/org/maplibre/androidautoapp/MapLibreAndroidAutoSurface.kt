package org.maplibre.androidautoapp

import android.animation.Animator
import android.animation.ValueAnimator
import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.PixelFormat
import android.graphics.PointF
import android.graphics.Rect
import android.os.Handler
import android.os.Looper
import android.view.Surface
import android.view.TextureView
import android.view.View
import android.view.WindowManager
import android.view.animation.DecelerateInterpolator
import androidx.annotation.MainThread
import androidx.car.app.AppManager
import androidx.car.app.CarContext
import androidx.car.app.SurfaceCallback
import androidx.car.app.SurfaceContainer
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleOwner
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import kotlin.math.ln

class MapLibreAndroidAutoSurface(private val mCarContext: CarContext, lifecycle: Lifecycle) :
    DefaultLifecycleObserver {

    var mSurface: Surface? = null
    private var mapView: MapView? = null
    private var surfaceWidth: Int? = null
    private var surfaceHeight: Int? = null
    private var isMoving = false
    private var scaleAnimator: Animator? = null
    private var mapLibreMap: MapLibreMap? = null
    private var listSurfaceCallback = mutableListOf<SurfaceCallback>()
    private val mSurfaceCallback: SurfaceCallback = object : SurfaceCallback {
        override fun onSurfaceAvailable(surfaceContainer: SurfaceContainer) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onSurfaceAvailable(surfaceContainer)
            }

            if (mSurface != null) {
                mSurface!!.release()
            }
            mSurface = surfaceContainer.surface
            renderLayout()
        }

        override fun onVisibleAreaChanged(visibleArea: Rect) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onVisibleAreaChanged(visibleArea)
            }
            renderLayout()

        }

        override fun onStableAreaChanged(stableArea: Rect) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onStableAreaChanged(stableArea)
            }
            renderLayout()

        }

        override fun onSurfaceDestroyed(surfaceContainer: SurfaceContainer) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onSurfaceDestroyed(surfaceContainer)
            }
            if (mSurface != null) {
                mSurface!!.release()
                mSurface = null
            }
        }

        override fun onClick(x: Float, y: Float) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onClick(x, y)
            }
            super.onClick(x, y)
        }

        override fun onFling(velocityX: Float, velocityY: Float) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onFling(velocityX, velocityY)
            }
            renderLayout()
            isMoving = false
            super.onFling(velocityX, velocityY)
        }


        override fun onScroll(distanceX: Float, distanceY: Float) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onScroll(distanceX, distanceY)
            }

            if (!java.lang.Float.isNaN(distanceX) && !java.lang.Float.isNaN(distanceY) && (distanceX != 0f || distanceY != 0f)) {
                mapLibreMap?.scrollBy(-distanceX, -distanceY, 0 /*no duration*/)
            }
        }

        override fun onScale(focusX: Float, focusY: Float, scaleFactor: Float) {
            for (surfaceCallback in listSurfaceCallback) {
                surfaceCallback.onScale(focusX, focusY, scaleFactor)
            }
            if (scaleFactor == 2.0f) {
                zoomAnimated(true, PointF(focusX, focusY))
            } else {
                // Calculate the zoom level change from the scale factor for smooth zooming
                val zoomBy =
                    (ln(
                        scaleFactor.toDouble()
                    ) / ln(Math.PI / 2)) * MapLibreConstants.ZOOM_RATE
                mapLibreMap?.zoomBy(zoomBy, PointF(focusX, focusY))
            }

            renderLayout()
        }
    }

    init {
        lifecycle.addObserver(this)
    }

    fun getMapView(): MapView? {
        return mapView
    }

    fun addOnSurfaceCallbackListener(surfaceCallback: SurfaceCallback) {
        listSurfaceCallback.add(surfaceCallback)
    }

    private fun createScaleAnimator(
        currentZoom: Double, zoomAddition: Double,
        animationFocalPoint: PointF?,
    ): Animator {
        val animator =
            ValueAnimator.ofFloat(currentZoom.toFloat(), (currentZoom + zoomAddition).toFloat())
        animator.apply {
            setDuration(
                MapLibreConstants.ANIMATION_DURATION.toLong()
            )
            interpolator = DecelerateInterpolator()
            addUpdateListener { animation ->
                animationFocalPoint?.let {
                    mapLibreMap?.setZoom(
                        (animation.animatedValue as Float).toDouble(),
                        it
                    )
                }
            }
        }
        return animator
    }

    private fun zoomAnimated(zoomIn: Boolean, zoomFocalPoint: PointF?) {
        cancelAnimator(scaleAnimator)

        val currentZoom = mapLibreMap?.zoom
        currentZoom?.let {
            scaleAnimator = createScaleAnimator(
                it,
                (if (zoomIn) 1 else -1).toDouble(),
                zoomFocalPoint
            )
            scaleAnimator!!.start()
        }
    }

    private fun cancelAnimator(animator: Animator?) {
        if (animator != null && animator.isStarted) {
            animator.cancel()
        }
    }

    override fun onDestroy(owner: LifecycleOwner) {
        Handler(Looper.getMainLooper()).post {
            mapView?.onDestroy()
            mapView?.run {
                onStop()
                onDestroy()
                mCarContext.windowManager.removeView(this)
            }
            mapView = null
        }
        super.onDestroy(owner)
    }

    override fun onStart(owner: LifecycleOwner) {
        mapView?.onStart()
    }

    override fun onResume(owner: LifecycleOwner) {
        mapView?.onResume()
    }

    override fun onPause(owner: LifecycleOwner) {
        mapView?.onPause()
    }

    override fun onStop(owner: LifecycleOwner) {
        mapView?.onStop()
    }

    override fun onCreate(owner: LifecycleOwner) {
        Handler(Looper.getMainLooper()).post {
            mCarContext.getCarService(AppManager::class.java).setSurfaceCallback(mSurfaceCallback)
            mapView = createMapViewInstance().apply {
                mCarContext.windowManager.addView(
                    this,
                    getWindowManagerLayoutParams()
                )
                onStart()
                getMapAsync {
                    mapLibreMap = it
                }
                addOnDidBecomeIdleListener { renderLayout() }
                addOnWillStartRenderingFrameListener {
                    renderLayout()
                }
            }
        }
    }

    private fun getWindowManagerLayoutParams() = WindowManager.LayoutParams(
        surfaceWidth ?: WindowManager.LayoutParams.MATCH_PARENT,
        surfaceHeight ?: WindowManager.LayoutParams.MATCH_PARENT,
        WindowManager.LayoutParams.TYPE_PRIVATE_PRESENTATION,
        WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
        PixelFormat.RGBX_8888
    )

    private fun createMapViewInstance() =
        MapView(mCarContext, MapLibreMapOptions.createFromAttributes(mCarContext).apply {
            // Set the textureMode to true, so a TextureView is created
            // We can extract this TextureView to draw on the Android Auto surface
            textureMode(true)
        }).apply {
            setLayerType(View.LAYER_TYPE_HARDWARE, Paint())
        }

    fun init(
        styleBuilder: Style.Builder,
        onStyleLoadedCallback: Style.OnStyleLoaded,
        onMapReadyCallback: OnMapReadyCallback,
    ) {
        Handler(Looper.getMainLooper()).post {
            mapView?.getMapAsync {
                onMapReadyCallback.onMapReady(mapLibreMap!!)
                mapLibreMap = it
                mapLibreMap?.setStyle(
                    styleBuilder
                ) { style: Style? ->
                    if (style != null) {
                        onStyleLoadedCallback.onStyleLoaded(style)
                    }
                }
            }
        }
    }

    fun init(styleBuilder: Style.Builder, onMapReadyCallback: OnMapReadyCallback) {
        Handler(Looper.getMainLooper()).post {
            mapView?.getMapAsync {
                onMapReadyCallback.onMapReady(mapLibreMap!!)
                mapLibreMap = it
                mapLibreMap?.setStyle(
                    styleBuilder
                )
            }
        }
    }

    fun init(styleBuilder: Style.Builder, onStyleLoadedCallback: Style.OnStyleLoaded) {
        Handler(Looper.getMainLooper()).post {
            mapView?.getMapAsync {
                mapLibreMap = it
                mapLibreMap?.setStyle(
                    styleBuilder
                ) { style: Style? ->
                    if (style != null) {
                        onStyleLoadedCallback.onStyleLoaded(style)
                    }
                }
            }
        }
    }

    fun init(styleBuilder: Style.Builder) {
        Handler(Looper.getMainLooper()).post {
            mapView?.getMapAsync {
                mapLibreMap = it
                mapLibreMap?.setStyle(
                    styleBuilder
                )
            }
        }
    }

    private val Context.windowManager: WindowManager
        get() = getSystemService(Context.WINDOW_SERVICE) as WindowManager


    private fun renderLayout() {
        val canvas: Canvas? = mSurface?.lockCanvas(null)
        canvas?.let {
            mapView?.let { map -> drawMapOnCanvas(map, it) }
            mSurface?.unlockCanvasAndPost(it)
        }
    }

    private fun drawMapOnCanvas(mapView: MapView, canvas: Canvas) {
        val mapViewTextureView = mapView.takeIf { it.childCount > 0 }?.getChildAt(0) as? TextureView
        mapViewTextureView?.bitmap?.let {
            canvas.drawBitmap(it, 0f, 0f, Paint())
        }
    }
}