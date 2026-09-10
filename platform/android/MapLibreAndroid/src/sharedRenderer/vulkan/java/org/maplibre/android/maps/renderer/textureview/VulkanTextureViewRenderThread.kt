package org.maplibre.android.maps.renderer.textureview

import android.view.Surface
import android.view.TextureView
import androidx.annotation.UiThread

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorNotifyAll() = (this as java.lang.Object).notifyAll()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait() = (this as java.lang.Object).wait()

class VulkanTextureViewRenderThread
    @UiThread
    constructor(
        textureView: TextureView,
        mapRenderer: TextureViewMapRenderer,
    ) : TextureViewRenderThread(textureView, mapRenderer) {
        private var surface: Surface? = null

        internal fun cleanup() {
            surface?.let {
                mapRenderer.onSurfaceDestroyed()
                it.release()
                surface = null
            }

            hasNativeSurface = false
        }

        // Thread implementation

        @Suppress("CyclomaticComplexMethod", "ComplexMethod", "NestedBlockDepth")
        override fun run() {
            try {
                while (true) {
                    var event: Runnable? = null
                    var createSurface = false
                    var sizeChanged = false
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

                            if (this.destroySurface) {
                                surface?.let {
                                    mapRenderer.onSurfaceDestroyed()
                                    it.release()
                                    surface = null
                                }

                                this.hasNativeSurface = false
                                this.destroySurface = false
                                lock.monitorNotifyAll()
                                break
                            }

                            if (surfaceTexture != null && !paused && requestRender) {
                                w = width
                                h = height

                                if (surface == null) {
                                    surface = Surface(surfaceTexture)
                                    this.hasNativeSurface = true
                                    createSurface = true
                                }

                                if (this.sizeChanged) {
                                    sizeChanged = true
                                    this.sizeChanged = false
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

                    if (createSurface) {
                        mapRenderer.onSurfaceCreated(surface)
                        mapRenderer.onSurfaceChanged(w, h)
                        createSurface = false
                        continue
                    }

                    // If the surface size has changed inform the map renderer.
                    if (sizeChanged) {
                        mapRenderer.onSurfaceChanged(w, h)
                        sizeChanged = false
                        continue
                    }

                    if (surface == null) {
                        continue
                    }

                    // Time to render a frame
                    mapRenderer.onDrawFrame()
                }
            } catch (err: InterruptedException) {
                // To be expected
            } finally {
                // Signal we're done
                synchronized(lock) {
                    cleanup()
                    this.exited = true
                    lock.monitorNotifyAll()
                }
            }
        }
    }
