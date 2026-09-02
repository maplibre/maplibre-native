package org.maplibre.android.maps.renderer.surfaceview

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.util.Log
import org.maplibre.android.maps.renderer.egl.EGLLogWrapper
import java.lang.ref.WeakReference
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGL11
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay
import javax.microedition.khronos.egl.EGLSurface
import javax.microedition.khronos.opengles.GL

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorNotifyAll() = (this as java.lang.Object).notifyAll()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait() = (this as java.lang.Object).wait()

class MapLibreGLSurfaceView : MapLibreSurfaceView {
    private val viewWeakReference = WeakReference(this)

    private var eglConfigChooser: GLSurfaceView.EGLConfigChooser? = null
    private var eglContextFactory: GLSurfaceView.EGLContextFactory? = null
    private var eglWindowSurfaceFactory: GLSurfaceView.EGLWindowSurfaceFactory? = null

    /**
     * Control whether the EGL context is preserved when the GLSurfaceView is paused and
     * resumed.
     *
     * If set to true, then the EGL context may be preserved when the GLSurfaceView is paused.
     *
     * Prior to API level 11, whether the EGL context is actually preserved or not
     * depends upon whether the Android device can support an arbitrary number of
     * EGL contexts or not. Devices that can only support a limited number of EGL
     * contexts must release the EGL context in order to allow multiple applications
     * to share the GPU.
     *
     * If set to false, the EGL context will be released when the GLSurfaceView is paused,
     * and recreated when the GLSurfaceView is resumed.
     *
     * The default is false.
     */
    var preserveEGLContextOnPause: Boolean = false

    constructor(context: Context) : super(context)

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)

    override fun setRenderer(renderer: SurfaceViewMapRenderer) {
        checkNotNull(eglConfigChooser) { "No eglConfigChooser provided" }
        checkNotNull(eglContextFactory) { "No eglContextFactory provided" }
        checkNotNull(eglWindowSurfaceFactory) { "No eglWindowSurfaceFactory provided" }

        super.setRenderer(renderer)
    }

    /**
     * Install a custom EGLContextFactory.
     *
     * If this method is called, it must be called before [setRenderer] is called.
     */
    fun setEGLContextFactory(factory: GLSurfaceView.EGLContextFactory) {
        checkRenderThreadState()
        eglContextFactory = factory
    }

    /**
     * Install a custom EGLWindowSurfaceFactory.
     *
     * If this method is called, it must be called before [setRenderer] is called.
     */
    fun setEGLWindowSurfaceFactory(factory: GLSurfaceView.EGLWindowSurfaceFactory) {
        checkRenderThreadState()
        eglWindowSurfaceFactory = factory
    }

    /**
     * Install a custom EGLConfigChooser.
     *
     * If this method is called, it must be called before [setRenderer] is called.
     */
    fun setEGLConfigChooser(configChooser: GLSurfaceView.EGLConfigChooser) {
        checkRenderThreadState()
        eglConfigChooser = configChooser
    }

    override fun createRenderThread() {
        renderThread = GLThread(viewWeakReference)
    }

    /**
     * An EGL helper class.
     */
    private class EglHelper(
        private val glSurfaceViewWeakRef: WeakReference<MapLibreGLSurfaceView>,
    ) {
        private var egl: EGL10? = null
        private var eglDisplay: EGLDisplay? = null
        private var eglSurface: EGLSurface? = null
        private var eglConfig: EGLConfig? = null
        private var eglContext: EGLContext? = null

        /**
         * Initialize EGL for a given configuration spec.
         */
        fun start() {
            try {
                // Get an EGL instance
                val egl = EGLContext.getEGL() as EGL10
                this.egl = egl

                // Get to the default display.
                val display = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY)
                eglDisplay = display

                if (display === EGL10.EGL_NO_DISPLAY) {
                    Log.e(MapLibreSurfaceView.TAG, "eglGetDisplay failed")
                    return
                }

                // We can now initialize EGL for that display
                val version = IntArray(2)
                if (!egl.eglInitialize(display, version)) {
                    Log.e(MapLibreSurfaceView.TAG, "eglInitialize failed")
                    return
                }
                val view = glSurfaceViewWeakRef.get()
                if (view == null) {
                    eglConfig = null
                    eglContext = null
                } else {
                    val config = view.eglConfigChooser!!.chooseConfig(egl, display)
                    eglConfig = config
                    if (config == null) {
                        Log.e(MapLibreSurfaceView.TAG, "failed to select an EGL configuration")
                        return
                    }

                    // Create an EGL context. We want to do this as rarely as we can, because an
                    // EGL context is a somewhat heavy object.
                    eglContext = view.eglContextFactory!!.createContext(egl, display, config)
                }
                val context = eglContext
                if (context == null || context === EGL10.EGL_NO_CONTEXT) {
                    eglContext = null
                    Log.e(MapLibreSurfaceView.TAG, "createContext failed")
                    return
                }
            } catch (exception: Exception) {
                Log.e(MapLibreSurfaceView.TAG, "createContext failed: ", exception)
            }
            eglSurface = null
        }

        /**
         * Create an egl surface for the current SurfaceHolder surface. If a surface
         * already exists, destroy it before creating the new surface.
         *
         * @return true if the surface was created successfully.
         */
        fun createSurface(): Boolean {
            // Check preconditions.
            val egl = this.egl
            if (egl == null) {
                Log.e(MapLibreSurfaceView.TAG, "egl not initialized")
                return false
            }
            val display = eglDisplay
            if (display == null) {
                Log.e(MapLibreSurfaceView.TAG, "eglDisplay not initialized")
                return false
            }
            val config = eglConfig
            if (config == null) {
                Log.e(MapLibreSurfaceView.TAG, "mEglConfig not initialized")
                return false
            }

            // The window size has changed, so we need to create a new surface.
            destroySurfaceImp()

            // Create an EGL surface we can render into.
            val view = glSurfaceViewWeakRef.get()
            val surface =
                view?.let {
                    it.eglWindowSurfaceFactory!!.createWindowSurface(egl, display, config, it.holder)
                }
            eglSurface = surface

            if (surface == null || surface === EGL10.EGL_NO_SURFACE) {
                val error = egl.eglGetError()
                if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                    Log.e(MapLibreSurfaceView.TAG, "createWindowSurface returned EGL_BAD_NATIVE_WINDOW.")
                }
                return false
            }

            // Before we can issue GL commands, we need to make sure
            // the context is current and bound to a surface.
            if (!egl.eglMakeCurrent(display, surface, surface, eglContext)) {
                // Could not make the context current, probably because the underlying
                // SurfaceView surface has been destroyed.
                logEglErrorAsWarning(MapLibreSurfaceView.TAG, "eglMakeCurrent", egl.eglGetError())
                return false
            }

            return true
        }

        /**
         * Create a GL object for the current EGL context.
         */
        fun createGL(): GL = eglContext!!.gl

        /**
         * Display the current render surface.
         *
         * @return the EGL error code from eglSwapBuffers.
         */
        fun swap(): Int {
            val egl = this.egl ?: return EGL10.EGL_SUCCESS
            if (!egl.eglSwapBuffers(eglDisplay, eglSurface)) {
                return egl.eglGetError()
            }
            return EGL10.EGL_SUCCESS
        }

        fun destroySurface() {
            destroySurfaceImp()
        }

        private fun destroySurfaceImp() {
            val surface = eglSurface
            val egl = this.egl
            if (surface != null && surface !== EGL10.EGL_NO_SURFACE && egl != null) {
                egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, eglContext)
                glSurfaceViewWeakRef.get()?.let { view ->
                    view.eglWindowSurfaceFactory!!.destroySurface(egl, eglDisplay, surface)
                }
                eglSurface = null
            }
        }

        fun finish() {
            val egl = this.egl
            if (eglDisplay != null) {
                egl?.eglMakeCurrent(
                    eglDisplay,
                    EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_CONTEXT,
                )
            }

            if (eglContext != null) {
                glSurfaceViewWeakRef.get()?.let { view ->
                    view.eglContextFactory!!.destroyContext(egl, eglDisplay, eglContext)
                }
                eglContext = null
            }
            if (eglDisplay != null) {
                egl?.eglTerminate(eglDisplay)
                eglDisplay = null
            }
        }

        companion object {
            fun logEglErrorAsWarning(
                tag: String,
                function: String,
                error: Int,
            ) {
                Log.w(tag, formatEglError(function, error))
            }

            fun formatEglError(
                function: String,
                error: Int,
            ): String = function + " failed: " + EGLLogWrapper.getErrorString(error)
        }
    }

    /**
     * A generic GL Thread. Takes care of initializing EGL and GL. Delegates
     * to a Renderer instance to do the actual drawing. Can be configured to
     * render continuously or on request.
     *
     * All potentially blocking synchronization is done through the
     * renderThreadManager object. This avoids multiple-lock ordering issues.
     *
     * @param surfaceViewWeakRef set once at thread construction time, nulled out when the parent view
     * is garbage collected. This weak reference allows the SurfaceView to be garbage collected while
     * the RenderThread is still alive.
     */
    internal class GLThread(
        private val surfaceViewWeakRef: WeakReference<MapLibreGLSurfaceView>,
    ) : MapLibreSurfaceView.RenderThread(surfaceViewWeakRef.get()!!.renderThreadManager) {
        private var surfaceIsBad = false
        private var haveEglContext = false
        private var haveEglSurface = false
        private var finishedCreatingEglSurface = false
        private var shouldReleaseEglContext = false

        private lateinit var eglHelper: EglHelper

        /**
         * This private method should only be called inside a
         * synchronized(renderThreadManager) block.
         */
        private fun stopEglSurfaceLocked() {
            if (haveEglSurface) {
                haveEglSurface = false
                eglHelper.destroySurface()
            }
        }

        /**
         * This private method should only be called inside a
         * synchronized(renderThreadManager) block.
         */
        private fun stopEglContextLocked() {
            if (haveEglContext) {
                eglHelper.finish()
                haveEglContext = false
                renderThreadManager.monitorNotifyAll()
            }
        }

        @Suppress(
            "CyclomaticComplexMethod",
            "ComplexMethod",
            "LongMethod",
            "NestedBlockDepth",
            "NAME_SHADOWING",
        )
        @Throws(InterruptedException::class)
        override fun guardedRun() {
            eglHelper = EglHelper(surfaceViewWeakRef)
            haveEglContext = false
            haveEglSurface = false
            wantRenderNotification = false

            try {
                var createEglContext = false
                var createEglSurface = false
                var createGlInterface = false
                var lostEglContext = false
                var sizeChanged = false
                var wantRenderNotification = false
                var doRenderNotification = false
                var askedToReleaseEglContext = false
                var w = 0
                var h = 0
                var event: Runnable? = null
                var finishDrawingRunnable: Runnable? = null

                while (true) {
                    synchronized(renderThreadManager) {
                        while (true) {
                            if (shouldExit) {
                                return
                            }

                            if (eventQueue.isNotEmpty()) {
                                event = eventQueue.removeAt(0)
                                break
                            }

                            // Update the pause state.
                            var pausing = false
                            if (paused != requestPaused) {
                                pausing = requestPaused
                                paused = requestPaused
                                renderThreadManager.monitorNotifyAll()
                            }

                            // Do we need to give up the EGL context?
                            if (shouldReleaseEglContext) {
                                stopEglSurfaceLocked()
                                stopEglContextLocked()
                                shouldReleaseEglContext = false
                                askedToReleaseEglContext = true
                            }

                            // Have we lost the EGL context?
                            if (lostEglContext) {
                                stopEglSurfaceLocked()
                                stopEglContextLocked()
                                lostEglContext = false
                            }

                            // When pausing, release the EGL surface:
                            if (pausing && haveEglSurface) {
                                stopEglSurfaceLocked()
                            }

                            // When pausing, optionally release the EGL Context:
                            if (pausing && haveEglContext) {
                                val view = surfaceViewWeakRef.get()
                                val preserveEglContextOnPause = view != null && view.preserveEGLContextOnPause
                                if (!preserveEglContextOnPause) {
                                    stopEglContextLocked()
                                }
                            }

                            // Have we lost the SurfaceView surface?
                            if (!hasSurface && !waitingForSurface) {
                                if (haveEglSurface) {
                                    stopEglSurfaceLocked()
                                }
                                waitingForSurface = true
                                surfaceIsBad = false
                                renderThreadManager.monitorNotifyAll()
                            }

                            // Have we acquired the surface view surface?
                            if (hasSurface && waitingForSurface) {
                                waitingForSurface = false
                                renderThreadManager.monitorNotifyAll()
                            }

                            if (doRenderNotification) {
                                this.wantRenderNotification = false
                                doRenderNotification = false
                                renderComplete = true
                                renderThreadManager.monitorNotifyAll()
                            }

                            if (this.finishDrawingRunnable != null) {
                                finishDrawingRunnable = this.finishDrawingRunnable
                                this.finishDrawingRunnable = null
                            }

                            // Ready to draw?
                            if (readyToDraw()) {
                                // If we don't have an EGL context, try to acquire one.
                                if (!haveEglContext) {
                                    if (askedToReleaseEglContext) {
                                        askedToReleaseEglContext = false
                                    } else {
                                        try {
                                            eglHelper.start()
                                        } catch (exception: RuntimeException) {
                                            renderThreadManager.monitorNotifyAll()
                                            return
                                        }
                                        haveEglContext = true
                                        createEglContext = true

                                        renderThreadManager.monitorNotifyAll()
                                    }
                                }

                                if (haveEglContext && !haveEglSurface) {
                                    haveEglSurface = true
                                    createEglSurface = true
                                    createGlInterface = true
                                    sizeChanged = true
                                }

                                if (haveEglSurface) {
                                    if (this.sizeChanged) {
                                        sizeChanged = true
                                        w = width
                                        h = height
                                        this.wantRenderNotification = true

                                        // Destroy and recreate the EGL surface.
                                        createEglSurface = true

                                        this.sizeChanged = false
                                    }
                                    requestRender = false
                                    renderThreadManager.monitorNotifyAll()
                                    if (this.wantRenderNotification) {
                                        wantRenderNotification = true
                                    }
                                    break
                                }
                            } else {
                                val pendingDraw = finishDrawingRunnable
                                if (pendingDraw != null) {
                                    Log.w(
                                        MapLibreSurfaceView.TAG,
                                        "Warning, !readyToDraw() but waiting for draw finished! " +
                                            "Early reporting draw finished.",
                                    )
                                    pendingDraw.run()
                                    finishDrawingRunnable = null
                                }
                            }
                            // By design, this is the only place in a GLThread thread where we wait().
                            renderThreadManager.monitorWait()
                        }
                    } // end of synchronized(renderThreadManager)

                    val pendingEvent = event
                    if (pendingEvent != null) {
                        pendingEvent.run()
                        event = null
                        continue
                    }

                    if (createEglSurface) {
                        var surfaceCreated = false
                        if (eglHelper.createSurface()) {
                            surfaceCreated = true
                            synchronized(renderThreadManager) {
                                finishedCreatingEglSurface = true
                                renderThreadManager.monitorNotifyAll()
                            }
                        } else {
                            synchronized(renderThreadManager) {
                                finishedCreatingEglSurface = true
                                surfaceIsBad = true
                                renderThreadManager.monitorNotifyAll()
                            }
                        }
                        if (!surfaceCreated) {
                            continue
                        }
                        createEglSurface = false
                    }

                    if (createGlInterface) {
                        eglHelper.createGL()

                        createGlInterface = false
                    }

                    if (createEglContext) {
                        surfaceViewWeakRef.get()?.renderer?.onSurfaceCreated(null)
                        createEglContext = false
                    }

                    if (sizeChanged) {
                        surfaceViewWeakRef.get()?.renderer?.onSurfaceChanged(w, h)
                        sizeChanged = false
                    }

                    surfaceViewWeakRef.get()?.let { view ->
                        view.renderer?.onDrawFrame()
                        val pendingDraw = finishDrawingRunnable
                        if (pendingDraw != null) {
                            pendingDraw.run()
                            finishDrawingRunnable = null
                        }
                    }

                    when (val swapError = eglHelper.swap()) {
                        EGL10.EGL_SUCCESS -> {
                            // Nothing to do
                        }

                        EGL11.EGL_CONTEXT_LOST -> {
                            lostEglContext = true
                        }

                        else -> {
                            // Other errors typically mean that the current surface is bad,
                            // probably because the SurfaceView surface has been destroyed,
                            // but we haven't been notified yet.
                            // Log the error to help developers understand why rendering stopped.
                            EglHelper.logEglErrorAsWarning(MapLibreSurfaceView.TAG, "eglSwapBuffers", swapError)

                            synchronized(renderThreadManager) {
                                surfaceIsBad = true
                                renderThreadManager.monitorNotifyAll()
                            }
                        }
                    }

                    if (wantRenderNotification) {
                        doRenderNotification = true
                        wantRenderNotification = false
                    }
                }
            } finally {
                // clean-up everything...
                synchronized(renderThreadManager) {
                    stopEglSurfaceLocked()
                    stopEglContextLocked()
                }
            }
        }

        override fun readyToDraw(): Boolean = super.readyToDraw() && !surfaceIsBad

        override fun ableToDraw(): Boolean = haveEglContext && haveEglSurface && readyToDraw()

        override fun surfaceCreated() {
            synchronized(renderThreadManager) {
                hasSurface = true
                finishedCreatingEglSurface = false
                renderThreadManager.monitorNotifyAll()
                while (waitingForSurface && !finishedCreatingEglSurface && !exited) {
                    try {
                        renderThreadManager.monitorWait()
                    } catch (exception: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        fun requestReleaseEglContextLocked() {
            shouldReleaseEglContext = true
            renderThreadManager.monitorNotifyAll()
        }
    }
}
