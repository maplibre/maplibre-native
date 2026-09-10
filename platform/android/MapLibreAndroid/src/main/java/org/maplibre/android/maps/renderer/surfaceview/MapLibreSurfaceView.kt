package org.maplibre.android.maps.renderer.surfaceview

import android.content.Context
import android.graphics.PixelFormat
import android.util.AttributeSet
import android.view.SurfaceHolder
import android.view.SurfaceView
import org.maplibre.android.maps.renderer.MapRenderer

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorNotifyAll() = (this as java.lang.Object).notifyAll()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait() = (this as java.lang.Object).wait()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait(timeoutMillis: Long) = (this as java.lang.Object).wait(timeoutMillis)

@Suppress("TooManyFunctions")
abstract class MapLibreSurfaceView :
    SurfaceView,
    SurfaceHolder.Callback2 {
    // internal rather than protected: the flavor-specific render threads are nested classes
    // that read these through a MapLibreSurfaceView instance, which Kotlin only allows for
    // module-visible members.
    @JvmField
    internal val renderThreadManager = RenderThreadManager()

    @JvmField
    internal var renderer: SurfaceViewMapRenderer? = null

    @JvmField
    protected var renderThread: RenderThread? = null

    @JvmField
    protected var detachedListener: OnSurfaceViewDetachedListener? = null

    @JvmField
    protected var detached = false

    /**
     * Standard View constructor. In order to render something, you
     * must call [setRenderer] to register a renderer.
     */
    constructor(context: Context) : super(context) {
        init()
    }

    /**
     * Standard View constructor. In order to render something, you
     * must call [setRenderer] to register a renderer.
     */
    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        init()
    }

    private fun init() {
        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed
        holder.setFormat(PixelFormat.TRANSPARENT)
        holder.addCallback(this)
    }

    @Suppress("unused")
    protected fun finalize() {
        val thread = renderThread
        if (thread != null && thread.isAlive) {
            // thread may still be running if this view was never
            // attached to a window.
            thread.requestExitAndWait()
        }
    }

    /**
     * Set a listener that gets notified when the view is detached from window.
     *
     * @param detachedListener listener
     */
    open fun setDetachedListener(detachedListener: OnSurfaceViewDetachedListener) {
        require(this.detachedListener == null) { "Detached from window listener has been already set." }
        this.detachedListener = detachedListener
    }

    /**
     * Set the renderer associated with this view. Also starts the thread that
     * will call the renderer, which in turn causes the rendering to start.
     *
     * This method should be called once and only once in the life-cycle of a SurfaceView.
     *
     * The following SurfaceView methods can only be called *after* setRenderer is called:
     *
     *  - [getRenderingRefreshMode]
     *  - [onPause]
     *  - [onResume]
     *  - [queueEvent]
     *  - [requestRender]
     *  - [setRenderingRefreshMode]
     *
     * @param renderer the renderer to use to perform drawing.
     */
    open fun setRenderer(renderer: SurfaceViewMapRenderer) {
        checkRenderThreadState()
        this.renderer = renderer

        createRenderThread()
        renderThread?.start()
    }

    /**
     * Set the rendering refresh mode to CONTINUOUS or WHEN_DIRTY.
     * Defaults to MapRenderer.RenderingRefreshMode.WHEN_DIRTY.
     * The renderer is called repeatedly to re-render the scene in continuous mode otherwise
     * the renderer is called when the surface is created, or when [requestRender] is called.
     *
     * Using WHEN_DIRTY can improve battery life and overall system performance
     * by allowing the GPU and CPU to idle when the view does not need to be updated.
     *
     * This method can only be called after [setRenderer]
     *
     * @param mode one of the MapRenderer.RenderingRefreshMode constants
     */
    open fun setRenderingRefreshMode(mode: MapRenderer.RenderingRefreshMode) {
        renderThread?.setRenderingRefreshMode(mode)
    }

    /**
     * Get the current rendering mode. May be called
     * from any thread. Must not be called before a renderer has been set.
     *
     * @return the current rendering mode.
     */
    open fun getRenderingRefreshMode(): MapRenderer.RenderingRefreshMode =
        renderThread?.getRenderingRefreshMode() ?: MapRenderer.RenderingRefreshMode.WHEN_DIRTY

    /**
     * Request that the renderer render a frame.
     * This method is typically used when the render mode has been set to
     * MapRenderer.RenderingRefreshMode.WHEN_DIRTY, so that frames are only rendered on demand.
     * May be called from any thread. Must not be called before a renderer has been set.
     */
    open fun requestRender() {
        renderThread?.requestRender()
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of MapLibreSurfaceView.
     */
    override fun surfaceCreated(holder: SurfaceHolder) {
        renderThread?.surfaceCreated()
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of MapLibreSurfaceView.
     */
    override fun surfaceDestroyed(holder: SurfaceHolder) {
        // Surface will be destroyed when we return
        renderThread?.surfaceDestroyed()
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of MapLibreSurfaceView.
     */
    override fun surfaceChanged(
        holder: SurfaceHolder,
        format: Int,
        w: Int,
        h: Int,
    ) {
        renderThread?.onWindowResize(w, h)
    }

    /**
     * This method is part of the SurfaceHolder.Callback2 interface, and is
     * not normally called or subclassed by clients of MapLibreSurfaceView.
     */
    override fun surfaceRedrawNeededAsync(
        holder: SurfaceHolder,
        drawingFinished: Runnable,
    ) {
        renderThread?.requestRenderAndNotify(drawingFinished)
    }

    /**
     * This method is part of the SurfaceHolder.Callback2 interface, and is
     * not normally called or subclassed by clients of MapLibreSurfaceView.
     */
    @Deprecated("Since we are part of the framework we know only surfaceRedrawNeededAsync will be called.")
    override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
        // Since we are part of the framework we know only surfaceRedrawNeededAsync
        // will be called.
    }

    /**
     * Pause the rendering thread.
     *
     * Must not be called before a renderer has been set.
     */
    open fun onPause() {
        renderThread?.onPause()
    }

    /**
     * Resumes the rendering thread. It is the counterpart to [onPause].
     *
     * Must not be called before a renderer has been set.
     */
    open fun onResume() {
        renderThread?.onResume()
    }

    /**
     * Queue a runnable to be run on the rendering thread. This can be used
     * to communicate with the Renderer on the rendering thread.
     * Must not be called before a renderer has been set.
     *
     * @param r the runnable to be run on the rendering thread.
     */
    open fun queueEvent(r: Runnable) {
        renderThread?.queueEvent(r)
    }

    /**
     * Wait for the queue to become empty
     */
    open fun waitForEmpty() {
        renderThread?.waitForEmpty()
    }

    /**
     * This method is used as part of the View class and is not normally
     * called or subclassed by clients of MapLibreSurfaceView.
     */
    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        if (detached && renderer != null) {
            var renderMode = MapRenderer.RenderingRefreshMode.WHEN_DIRTY
            renderThread?.let { renderMode = it.getRenderingRefreshMode() }
            createRenderThread()
            if (renderMode != MapRenderer.RenderingRefreshMode.WHEN_DIRTY) {
                renderThread?.setRenderingRefreshMode(renderMode)
            }
            renderThread?.start()
        }
        detached = false
    }

    override fun onDetachedFromWindow() {
        detachedListener?.onSurfaceViewDetached()
        val thread = renderThread
        if (thread != null && thread.isAlive) {
            thread.requestExitAndWait()
        }
        detached = true
        super.onDetachedFromWindow()
    }

    protected abstract fun createRenderThread()

    protected fun checkRenderThreadState() {
        check(renderThread == null) { "setRenderer has already been called for this instance." }
    }

    /**
     * A generic render Thread. Delegates
     * to a Renderer instance to do the actual drawing. Can be configured to
     * render continuously or on request.
     *
     * All potentially blocking synchronization is done through the
     * renderThreadManager object. This avoids multiple-lock ordering issues.
     */
    @Suppress("TooManyFunctions")
    abstract class RenderThread(
        aRenderThreadManager: RenderThreadManager,
    ) : Thread() {
        // Once the thread is started, all accesses to the following member
        // variables are protected by the renderThreadManager monitor
        @JvmField
        protected var shouldExit = false

        @JvmField
        var exited = false

        @JvmField
        protected var requestPaused = false

        @JvmField
        protected var paused = false

        @JvmField
        protected var hasSurface = false

        @JvmField
        protected var waitingForSurface = true

        @JvmField
        protected var width = 0

        @JvmField
        protected var height = 0

        @JvmField
        protected var renderMode = MapRenderer.RenderingRefreshMode.WHEN_DIRTY

        @JvmField
        protected var requestRender = true

        @JvmField
        protected var wantRenderNotification = false

        @JvmField
        protected var renderComplete = false

        @JvmField
        protected var eventQueue = ArrayList<Runnable>()

        @JvmField
        protected var sizeChanged = true

        @JvmField
        protected var finishDrawingRunnable: Runnable? = null

        @JvmField
        protected val renderThreadManager: RenderThreadManager = aRenderThreadManager
        // End of member variables protected by the renderThreadManager monitor.

        override fun run() {
            name = "RenderThread $id"

            try {
                guardedRun()
            } catch (exception: InterruptedException) {
                // fall thru and exit normally
            } finally {
                renderThreadManager.threadExiting(this)
            }
        }

        @Throws(InterruptedException::class)
        protected abstract fun guardedRun()

        protected open fun readyToDraw(): Boolean =
            !paused && hasSurface && width > 0 && height > 0 &&
                (requestRender || renderMode == MapRenderer.RenderingRefreshMode.CONTINUOUS)

        open fun ableToDraw(): Boolean = readyToDraw()

        open fun setRenderingRefreshMode(mode: MapRenderer.RenderingRefreshMode) {
            synchronized(renderThreadManager) {
                renderMode = mode
                renderThreadManager.monitorNotifyAll()
            }
        }

        open fun getRenderingRefreshMode(): MapRenderer.RenderingRefreshMode = synchronized(renderThreadManager) { renderMode }

        open fun requestRender() {
            synchronized(renderThreadManager) {
                requestRender = true
                renderThreadManager.monitorNotifyAll()
            }
        }

        open fun requestRenderAndNotify(finishDrawing: Runnable?) {
            synchronized(renderThreadManager) {
                // If we are already on the render thread, this means a client callback
                // has caused reentrancy, for example via updating the SurfaceView parameters.
                // We will return to the client rendering code, so here we don't need to
                // do anything.
                if (Thread.currentThread() === this) {
                    return
                }

                wantRenderNotification = true
                requestRender = true
                renderComplete = false
                val oldCallback = finishDrawingRunnable
                finishDrawingRunnable =
                    Runnable {
                        oldCallback?.run()
                        finishDrawing?.run()
                    }

                renderThreadManager.monitorNotifyAll()
            }
        }

        open fun surfaceCreated() {
            synchronized(renderThreadManager) {
                hasSurface = true
                renderThreadManager.monitorNotifyAll()
                while (!exited && waitingForSurface) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (exception: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        open fun surfaceDestroyed() {
            synchronized(renderThreadManager) {
                hasSurface = false
                renderThreadManager.monitorNotifyAll()
                while (!exited && !waitingForSurface) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (exception: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        open fun onPause() {
            synchronized(renderThreadManager) {
                requestPaused = true
                renderThreadManager.monitorNotifyAll()
                while (!exited && !paused) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        open fun onResume() {
            synchronized(renderThreadManager) {
                requestPaused = false
                requestRender = true
                renderComplete = false
                renderThreadManager.monitorNotifyAll()
                while (!exited && paused && !renderComplete) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        open fun onWindowResize(
            w: Int,
            h: Int,
        ) {
            synchronized(renderThreadManager) {
                width = w
                height = h
                sizeChanged = true
                requestRender = true
                renderComplete = false

                // If we are already on the render thread, this means a client callback
                // has caused reentrancy, for example via updating the SurfaceView parameters.
                // We need to process the size change eventually though and update our surface.
                // So we set the parameters and return so they can be processed on our
                // next iteration.
                if (Thread.currentThread() === this) {
                    return
                }

                renderThreadManager.monitorNotifyAll()

                // Wait for thread to react to resize and render a frame
                while (!exited && !paused && !renderComplete && ableToDraw()) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        open fun requestExitAndWait() {
            // don't call this from renderThread thread or it is a guaranteed
            // deadlock!
            synchronized(renderThreadManager) {
                shouldExit = true
                renderThreadManager.monitorNotifyAll()
                while (!exited && isAlive) {
                    try {
                        renderThreadManager.monitorWait(WAIT_FOR_EXIT_MILLIS)
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        /**
         * Queue an "event" to be run on the rendering thread.
         *
         * @param r the runnable to be run on the rendering thread.
         */
        open fun queueEvent(r: Runnable) {
            synchronized(renderThreadManager) {
                eventQueue.add(r)
                renderThreadManager.monitorNotifyAll()
            }
        }

        /**
         * Wait for the queue to become empty
         */
        open fun waitForEmpty() {
            synchronized(renderThreadManager) {
                // Wait for the queue to be empty
                while (eventQueue.isNotEmpty()) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        private companion object {
            const val WAIT_FOR_EXIT_MILLIS = 100L
        }
    }

    class RenderThreadManager {
        @Synchronized
        internal fun threadExiting(thread: RenderThread) {
            thread.exited = true
            monitorNotifyAll()
        }
    }

    /**
     * Listener interface that notifies when a [MapLibreSurfaceView] is detached from window.
     */
    fun interface OnSurfaceViewDetachedListener {
        /**
         * Called when a [MapLibreSurfaceView] is detached from window.
         */
        fun onSurfaceViewDetached()
    }

    companion object {
        const val TAG = "MapLibreSurfaceView"
    }
}
