package org.maplibre.android.maps.renderer.textureview

import android.view.TextureView
import androidx.annotation.UiThread
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.renderer.egl.EGLConfigChooser
import java.lang.ref.WeakReference
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGL11
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay
import javax.microedition.khronos.egl.EGLSurface
import javax.microedition.khronos.opengles.GL10

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorNotifyAll() = (this as java.lang.Object).notifyAll()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait() = (this as java.lang.Object).wait()

/**
 * The render thread is responsible for managing the communication between the
 * ui thread and the render thread it creates. Also, the EGL and GL contexts
 * are managed from here.
 *
 * @param textureView the TextureView
 * @param mapRenderer the MapRenderer
 */
class GLTextureViewRenderThread
    @UiThread
    constructor(
        textureView: TextureView,
        mapRenderer: TextureViewMapRenderer,
    ) : TextureViewRenderThread(textureView, mapRenderer) {
        private val eglHolder = EGLHolder(WeakReference(textureView), mapRenderer.isTranslucentSurface)

        private var destroyContext = false

        // Thread implementation

        @Suppress("CyclomaticComplexMethod", "ComplexMethod", "NestedBlockDepth", "LongMethod")
        override fun run() {
            try {
                while (true) {
                    var event: Runnable? = null
                    var initializeEGL = false
                    var recreateSurface = false
                    var w = -1
                    var h = -1

                    // Guarded block
                    synchronized(lock) {
                        while (true) {
                            if (shouldExit) {
                                return
                            }

                            // If any events are scheduled, pop one for processing
                            if (eventQueue.isNotEmpty()) {
                                event = eventQueue.removeAt(0)
                                break
                            }

                            if (destroySurface) {
                                eglHolder.destroySurface()
                                destroySurface = false
                                this.hasNativeSurface = false
                                lock.monitorNotifyAll()
                                break
                            }

                            if (destroyContext) {
                                eglHolder.destroyContext()
                                destroyContext = false
                                break
                            }

                            if (surfaceTexture != null && !paused && requestRender) {
                                w = width
                                h = height

                                // Initialize EGL if needed
                                if (eglHolder.eglContext === EGL10.EGL_NO_CONTEXT) {
                                    this.hasNativeSurface = true
                                    initializeEGL = true
                                    break
                                }

                                // (re-)Initialize EGL Surface if needed
                                if (eglHolder.eglSurface === EGL10.EGL_NO_SURFACE) {
                                    this.hasNativeSurface = true
                                    recreateSurface = true
                                    break
                                }

                                // Reset the request render flag now, so we can catch new requests
                                // while rendering
                                requestRender = false

                                // Break the guarded loop and continue to process
                                break
                            }

                            // Wait until needed
                            lock.monitorWait()
                        } // end guarded while loop
                    } // end guarded block

                    // Run event, if any
                    val pendingEvent = event
                    if (pendingEvent != null) {
                        pendingEvent.run()
                        continue
                    }

                    eglHolder.createGL()

                    // Initialize EGL
                    if (initializeEGL) {
                        eglHolder.prepare()
                        var surfaceCreated = false
                        synchronized(lock) {
                            surfaceCreated = eglHolder.createSurface()
                            if (!surfaceCreated) {
                                // Cleanup the surface if one could not be created
                                // and wait for another to be ready.
                                destroySurface = true
                            }
                        }
                        if (!surfaceCreated) {
                            continue
                        }
                        mapRenderer.onSurfaceCreated(null)
                        mapRenderer.onSurfaceChanged(w, h)
                        continue
                    }

                    // If the surface size has changed inform the map renderer.
                    if (recreateSurface) {
                        synchronized(lock) {
                            eglHolder.createSurface()
                        }
                        mapRenderer.onSurfaceChanged(w, h)
                        continue
                    }

                    if (sizeChanged) {
                        mapRenderer.onSurfaceChanged(w, h)
                        sizeChanged = false
                        continue
                    }

                    // Don't continue without a surface
                    if (eglHolder.eglSurface === EGL10.EGL_NO_SURFACE) {
                        continue
                    }

                    // Time to render a frame
                    mapRenderer.onDrawFrame()

                    // Swap and check the result
                    when (val swapError = eglHolder.swap()) {
                        EGL10.EGL_SUCCESS -> {
                            // Nothing to do
                        }

                        EGL11.EGL_CONTEXT_LOST -> {
                            Logger.w(TAG, "Context lost. Waiting for re-aquire")
                            synchronized(lock) {
                                surfaceTexture = null
                                destroySurface = true
                                destroyContext = true
                            }
                        }

                        else -> {
                            Logger.w(TAG, "eglSwapBuffer error: $swapError. Waiting or new surface")
                            // Probably lost the surface. Clear the current one and
                            // wait for a new one to be set
                            synchronized(lock) {
                                surfaceTexture = null
                                destroySurface = true
                            }
                        }
                    }
                }
            } catch (err: InterruptedException) {
                // To be expected
            } finally {
                // Cleanup
                eglHolder.cleanup()

                // Signal we're done
                synchronized(lock) {
                    this.hasNativeSurface = false
                    this.exited = true
                    lock.monitorNotifyAll()
                }
            }
        }

        /**
         * Holds the EGL state and offers methods to mutate it.
         */
        private class EGLHolder(
            private val textureViewWeakRef: WeakReference<TextureView>,
            private val translucentSurface: Boolean,
        ) {
            private lateinit var egl: EGL10
            private var eglConfig: EGLConfig? = null
            private var eglDisplay: EGLDisplay = EGL10.EGL_NO_DISPLAY
            var eglContext: EGLContext = EGL10.EGL_NO_CONTEXT
                private set
            var eglSurface: EGLSurface? = EGL10.EGL_NO_SURFACE
                private set

            fun prepare() {
                this.egl = EGLContext.getEGL() as EGL10

                // Only re-initialize display when needed
                if (eglDisplay === EGL10.EGL_NO_DISPLAY) {
                    this.eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY)

                    if (eglDisplay === EGL10.EGL_NO_DISPLAY) {
                        throw RuntimeException("eglGetDisplay failed")
                    }

                    val version = IntArray(2)
                    if (!egl.eglInitialize(eglDisplay, version)) {
                        throw RuntimeException("eglInitialize failed")
                    }
                }

                if (eglContext === EGL10.EGL_NO_CONTEXT) {
                    eglConfig = EGLConfigChooser(translucentSurface).chooseConfig(egl, eglDisplay)
                    val attribList = intArrayOf(EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE)
                    eglContext = egl.eglCreateContext(eglDisplay, eglConfig, EGL10.EGL_NO_CONTEXT, attribList)
                }

                if (eglContext === EGL10.EGL_NO_CONTEXT) {
                    throw RuntimeException("createContext")
                }
            }

            fun createGL(): GL10 = eglContext.gl as GL10

            fun createSurface(): Boolean {
                // The window size has changed, so we need to create a new surface.
                destroySurface()

                // Create an EGL surface we can render into.
                val view = textureViewWeakRef.get()
                val surfaceTexture = view?.surfaceTexture
                eglSurface =
                    if (surfaceTexture != null) {
                        val surfaceAttribs = intArrayOf(EGL10.EGL_NONE)
                        egl.eglCreateWindowSurface(eglDisplay, eglConfig, surfaceTexture, surfaceAttribs)
                    } else {
                        EGL10.EGL_NO_SURFACE
                    }

                if (eglSurface == null || eglSurface === EGL10.EGL_NO_SURFACE) {
                    val error = egl.eglGetError()
                    if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                        Logger.e(TextureViewRenderThread.TAG, "createWindowSurface returned EGL_BAD_NATIVE_WINDOW.")
                    }
                    return false
                }

                return makeCurrent()
            }

            fun makeCurrent(): Boolean {
                if (!egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
                    // Could not make the context current, probably because the underlying
                    // SurfaceView surface has been destroyed.
                    Logger.w(TextureViewRenderThread.TAG, "eglMakeCurrent: ${egl.eglGetError()}")
                    return false
                }

                return true
            }

            fun swap(): Int {
                if (!egl.eglSwapBuffers(eglDisplay, eglSurface)) {
                    return egl.eglGetError()
                }
                return EGL10.EGL_SUCCESS
            }

            fun destroySurface() {
                if (eglSurface === EGL10.EGL_NO_SURFACE) {
                    return
                }

                if (!egl.eglDestroySurface(eglDisplay, eglSurface)) {
                    Logger.w(
                        TextureViewRenderThread.TAG,
                        "Could not destroy egl surface. Display $eglDisplay, Surface $eglSurface",
                    )
                }

                eglSurface = EGL10.EGL_NO_SURFACE
            }

            fun destroyContext() {
                if (eglContext === EGL10.EGL_NO_CONTEXT) {
                    return
                }

                if (!egl.eglDestroyContext(eglDisplay, eglContext)) {
                    Logger.w(
                        TextureViewRenderThread.TAG,
                        "Could not destroy egl context. Display $eglDisplay, Context $eglContext",
                    )
                }

                eglContext = EGL10.EGL_NO_CONTEXT
            }

            private fun terminate() {
                if (eglDisplay === EGL10.EGL_NO_DISPLAY) {
                    return
                }

                if (!egl.eglTerminate(eglDisplay)) {
                    Logger.w(TextureViewRenderThread.TAG, "Could not terminate egl. Display $eglDisplay")
                }
                eglDisplay = EGL10.EGL_NO_DISPLAY
            }

            fun cleanup() {
                destroySurface()
                destroyContext()
                terminate()
            }

            private companion object {
                private const val EGL_CONTEXT_CLIENT_VERSION = 0x3098
            }
        }
    }
