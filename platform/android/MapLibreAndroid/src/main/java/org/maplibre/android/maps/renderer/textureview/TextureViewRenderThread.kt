package org.maplibre.android.maps.renderer.textureview

import android.graphics.SurfaceTexture
import android.view.TextureView
import androidx.annotation.UiThread
import java.util.LinkedList

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorNotifyAll() = (this as java.lang.Object).notifyAll()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait(timeoutMillis: Long = 0) = (this as java.lang.Object).wait(timeoutMillis)

/**
 * The render thread is responsible for managing the communication between the
 * ui thread and the render thread it creates
 *
 * @param textureView the TextureView
 * @param mapRenderer the MapRenderer
 */
@Suppress("TooManyFunctions")
abstract class TextureViewRenderThread
    @UiThread
    constructor(
        textureView: TextureView,
        mapRenderer: TextureViewMapRenderer,
    ) : Thread(),
        TextureView.SurfaceTextureListener {
        @JvmField
        protected val mapRenderer: TextureViewMapRenderer = mapRenderer

        // Lock used for synchronization
        @JvmField
        protected val lock = Any()

        // Guarded by lock
        @JvmField
        protected val eventQueue = LinkedList<Runnable>()

        @JvmField
        protected var surfaceTexture: SurfaceTexture? = null

        @JvmField
        protected var hasNativeSurface = false

        @JvmField
        protected var width = 0

        @JvmField
        protected var height = 0

        @JvmField
        protected var requestRender = false

        @JvmField
        protected var sizeChanged = false

        @JvmField
        protected var paused = false

        @JvmField
        protected var destroySurface = false

        @JvmField
        protected var shouldExit = false

        @JvmField
        protected var exited = false

        init {
            textureView.isOpaque = !mapRenderer.isTranslucentSurface
            textureView.surfaceTextureListener = this
        }

        // SurfaceTextureListener methods

        @UiThread
        override fun onSurfaceTextureAvailable(
            surfaceTexture: SurfaceTexture,
            width: Int,
            height: Int,
        ) {
            synchronized(lock) {
                this.surfaceTexture = surfaceTexture
                this.width = width
                this.height = height
                this.requestRender = true
                lock.monitorNotifyAll()
            }
        }

        @UiThread
        override fun onSurfaceTextureSizeChanged(
            surfaceTexture: SurfaceTexture,
            width: Int,
            height: Int,
        ) {
            synchronized(lock) {
                this.width = width
                this.height = height
                this.sizeChanged = true
                this.requestRender = true
                lock.monitorNotifyAll()
            }
        }

        @UiThread
        override fun onSurfaceTextureDestroyed(surfaceTexture: SurfaceTexture): Boolean {
            synchronized(lock) {
                this.surfaceTexture = null
                this.destroySurface = true
                this.requestRender = false
                lock.monitorNotifyAll()

                while (this.hasNativeSurface && !this.exited) {
                    try {
                        lock.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
            return true
        }

        @UiThread
        override fun onSurfaceTextureUpdated(surfaceTexture: SurfaceTexture) {
            // Ignored
        }

        // MapRenderer delegate methods

        /**
         * May be called from any thread
         */
        fun requestRender() {
            synchronized(lock) {
                requestRender = true
                lock.monitorNotifyAll()
            }
        }

        /**
         * May be called from any thread
         */
        fun queueEvent(runnable: Runnable) {
            synchronized(lock) {
                eventQueue.add(runnable)
                lock.monitorNotifyAll()
            }
        }

        /**
         * Wait for the queue to be empty.
         */
        @UiThread
        fun waitForEmpty() {
            synchronized(lock) {
                // Wait for the queue to be empty
                while (eventQueue.isNotEmpty()) {
                    try {
                        lock.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        @UiThread
        fun onPause() {
            synchronized(lock) {
                this.paused = true
                lock.monitorNotifyAll()
            }
        }

        @UiThread
        fun onResume() {
            synchronized(lock) {
                this.paused = false
                lock.monitorNotifyAll()
            }
        }

        @UiThread
        fun onDestroy() {
            synchronized(lock) {
                this.shouldExit = true
                lock.monitorNotifyAll()

                // Wait for the thread to exit
                while (!this.exited) {
                    try {
                        lock.monitorWait()
                    } catch (ex: InterruptedException) {
                        Thread.currentThread().interrupt()
                    }
                }
            }
        }

        companion object {
            const val TAG = "Mbgl-TextureViewRenderThread"
        }
    }
