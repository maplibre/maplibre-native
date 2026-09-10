package org.maplibre.android.maps.renderer.surfaceview

import android.content.Context
import android.util.AttributeSet
import android.util.Log
import java.lang.ref.WeakReference

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorNotifyAll() = (this as java.lang.Object).notifyAll()

@Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN")
private fun Any.monitorWait() = (this as java.lang.Object).wait()

class MapLibreVulkanSurfaceView : MapLibreSurfaceView {
    private val viewWeakReference = WeakReference(this)

    constructor(context: Context) : super(context)

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)

    override fun createRenderThread() {
        renderThread = VulkanThread(viewWeakReference)
    }

    internal class VulkanThread(
        private val surfaceViewWeakRef: WeakReference<MapLibreVulkanSurfaceView>,
    ) : MapLibreSurfaceView.RenderThread(surfaceViewWeakRef.get()!!.renderThreadManager) {
        private var graphicsSurfaceCreated = false

        @Suppress(
            "CyclomaticComplexMethod",
            "ComplexMethod",
            "LongMethod",
            "NestedBlockDepth",
            "NAME_SHADOWING",
        )
        @Throws(InterruptedException::class)
        override fun guardedRun() {
            wantRenderNotification = false

            var sizeChanged = false
            var initSurface = false
            var wantRenderNotification = false
            var doRenderNotification = false
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
                        if (paused != requestPaused) {
                            paused = requestPaused
                            renderThreadManager.monitorNotifyAll()
                        }

                        // lost surface
                        if (!hasSurface && !waitingForSurface) {
                            val view = surfaceViewWeakRef.get()
                            if (view != null && graphicsSurfaceCreated) {
                                view.renderer?.onSurfaceDestroyed()
                            }
                            graphicsSurfaceCreated = false
                            waitingForSurface = true
                            renderThreadManager.monitorNotifyAll()
                        }

                        // acquired surface
                        if (hasSurface && waitingForSurface) {
                            if (surfaceViewWeakRef.get() != null) {
                                initSurface = true
                                graphicsSurfaceCreated = true
                            }
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
                        if (readyToDraw() && graphicsSurfaceCreated) {
                            if (this.sizeChanged) {
                                sizeChanged = true
                                w = width
                                h = height
                                this.wantRenderNotification = true
                                this.sizeChanged = false
                            }
                            requestRender = false
                            renderThreadManager.monitorNotifyAll()
                            if (this.wantRenderNotification) {
                                wantRenderNotification = true
                            }
                            break
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
                        // By design, this is the only place in a RenderThread thread where we wait().
                        renderThreadManager.monitorWait()
                    }
                } // end of synchronized(renderThreadManager)

                val pendingEvent = event
                if (pendingEvent != null) {
                    pendingEvent.run()
                    event = null
                    continue
                }

                if (initSurface) {
                    surfaceViewWeakRef.get()?.let { view ->
                        view.renderer?.onSurfaceCreated(view.holder.surface)
                        initSurface = false
                    }
                }

                if (sizeChanged) {
                    surfaceViewWeakRef.get()?.let { view ->
                        view.renderer?.onSurfaceChanged(w, h)
                        sizeChanged = false
                    }
                }

                surfaceViewWeakRef.get()?.let { view ->
                    view.renderer?.onDrawFrame()
                    val pendingDraw = finishDrawingRunnable
                    if (pendingDraw != null) {
                        pendingDraw.run()
                        finishDrawingRunnable = null
                    }
                }

                if (wantRenderNotification) {
                    doRenderNotification = true
                    wantRenderNotification = false
                }
            }
        }

        override fun ableToDraw(): Boolean = graphicsSurfaceCreated && readyToDraw()
    }
}
