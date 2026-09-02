package org.maplibre.android.maps.renderer

import android.content.Context
import android.view.Surface
import android.view.TextureView
import android.view.View
import androidx.annotation.CallSuper
import androidx.annotation.Keep
import org.maplibre.android.LibraryLoader
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMapOptions

/**
 * The [MapRenderer] encapsulates the render thread.
 *
 * Performs actions on the render thread to manage the resources and
 * render on the one end and acts as a scheduler to request work to
 * be performed on the render thread on the other.
 */
@Keep
abstract class MapRenderer(
    context: Context,
    localIdeographFontFamily: String?,
) : MapRendererScheduler {
    /**
     * Rendering presentation refresh mode.
     */
    enum class RenderingRefreshMode {
        /**
         * The map is rendered only in response to an event that affects the rendering of the map.
         * This mode is preferred to improve battery life and overall system performance
         */
        WHEN_DIRTY,

        /**
         * The map is repeatedly re-rendered at the refresh rate of the display.
         * This mode is preferred when benchmarking the rendering
         */
        CONTINUOUS,
    }

    // Holds the pointer to the native peer after initialization
    @Suppress("unused")
    private var nativePtr: Long = 0

    private var expectedRenderTime = 0.0

    private var timeElapsed: Long = 0

    var onFpsChangedListener: MapLibreMap.OnFpsChangedListener? = null

    init {
        val pixelRatio = context.resources.displayMetrics.density

        // Initialize native peer
        nativeInitialize(this, pixelRatio, localIdeographFontFamily)
    }

    abstract val view: View

    open fun onStart() {
        // Implement if needed
    }

    open fun onPause() {
        // Implement if needed
    }

    open fun onResume() {
        // Implement if needed
    }

    open fun onStop() {
        // Implement if needed
    }

    open fun onDestroy() {
        // Implement if needed
    }

    abstract fun setRenderingRefreshMode(mode: RenderingRefreshMode)

    abstract fun getRenderingRefreshMode(): RenderingRefreshMode

    @CallSuper
    protected open fun onSurfaceCreated(surface: Surface?) {
        nativeOnSurfaceCreated(surface)
    }

    @CallSuper
    protected open fun onSurfaceChanged(
        width: Int,
        height: Int,
    ) {
        nativeOnSurfaceChanged(width, height)
    }

    @CallSuper
    protected open fun onSurfaceDestroyed() {
        nativeOnSurfaceDestroyed()
    }

    @CallSuper
    protected open fun onDrawFrame() {
        val startTime = System.nanoTime()
        try {
            nativeRender()
        } catch (error: Error) {
            Logger.e(TAG, error.message.toString())
        }
        val renderTime = System.nanoTime() - startTime
        if (renderTime < expectedRenderTime) {
            try {
                Thread.sleep(((expectedRenderTime - renderTime) / 1E6).toLong())
            } catch (ex: InterruptedException) {
                Logger.e(TAG, ex.message.toString())
            }
        }
        if (onFpsChangedListener != null) {
            updateFps()
        }
    }

    fun setSwapBehaviorFlush(flush: Boolean) {
        nativeSetSwapBehaviorFlush(flush)
    }

    /**
     * May be called from any thread.
     *
     * Called from the native peer to schedule work on the render
     * thread. Explicit override for easier to read jni code.
     *
     * @param runnable the runnable to execute
     * @see MapRendererRunnable
     */
    @CallSuper
    @JvmName("queueEvent")
    internal fun queueEvent(runnable: MapRendererRunnable) {
        this.queueEvent(runnable as Runnable)
    }

    private external fun nativeInitialize(
        self: MapRenderer,
        pixelRatio: Float,
        localIdeographFontFamily: String?,
    )

    @CallSuper
    @Suppress("unused")
    protected external fun finalize()

    private external fun nativeOnSurfaceCreated(surface: Surface?)

    private external fun nativeOnSurfaceChanged(
        width: Int,
        height: Int,
    )

    private external fun nativeOnSurfaceDestroyed()

    protected external fun nativeReset()

    private external fun nativeRender()

    private external fun nativeSetSwapBehaviorFlush(flush: Boolean)

    private fun updateFps() {
        val currentTime = System.nanoTime()
        if (timeElapsed > 0) {
            val fps = 1E9 / (currentTime - timeElapsed)
            onFpsChangedListener?.onFpsChanged(fps)
        }
        timeElapsed = currentTime
    }

    /**
     * The max frame rate at which this render is rendered,
     * but it can't excess the ability of device hardware.
     *
     * @param maximumFps Can be set to arbitrary integer values.
     */
    fun setMaximumFps(maximumFps: Int) {
        if (maximumFps <= 0) {
            // Not valid, just return
            return
        }
        expectedRenderTime = 1E9 / maximumFps
    }

    companion object {
        private const val TAG = "Mbgl-MapRenderer"

        init {
            LibraryLoader.load()
        }

        @JvmStatic
        fun create(
            options: MapLibreMapOptions,
            context: Context,
            initCallback: Runnable,
        ): MapRenderer {
            val localFontFamily = options.localIdeographFontFamily
            return if (options.textureMode) {
                val textureView = TextureView(context)
                val translucentSurface = options.translucentTextureSurface
                MapRendererFactory.newTextureViewMapRenderer(
                    context,
                    textureView,
                    localFontFamily,
                    translucentSurface,
                    initCallback,
                )
            } else {
                MapRendererFactory.newSurfaceViewMapRenderer(
                    context,
                    localFontFamily,
                    options.renderSurfaceOnTop,
                    initCallback,
                )
            }
        }
    }
}
