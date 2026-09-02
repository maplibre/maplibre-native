package org.maplibre.android.maps

import org.maplibre.android.log.Logger
import org.maplibre.android.tile.TileOperation
import java.util.concurrent.CopyOnWriteArrayList

@Suppress("TooManyFunctions", "LargeClass")
internal class MapChangeReceiver : NativeMapView.StateCallback {
    private val onCameraWillChangeListenerList = CopyOnWriteArrayList<MapView.OnCameraWillChangeListener>()
    private val onCameraIsChangingListenerList = CopyOnWriteArrayList<MapView.OnCameraIsChangingListener>()
    private val onCameraDidChangeListenerList = CopyOnWriteArrayList<MapView.OnCameraDidChangeListener>()
    private val onWillStartLoadingMapListenerList = CopyOnWriteArrayList<MapView.OnWillStartLoadingMapListener>()
    private val onDidFinishLoadingMapListenerList = CopyOnWriteArrayList<MapView.OnDidFinishLoadingMapListener>()
    private val onDidFailLoadingMapListenerList = CopyOnWriteArrayList<MapView.OnDidFailLoadingMapListener>()
    private val onWillStartRenderingFrameList = CopyOnWriteArrayList<MapView.OnWillStartRenderingFrameListener>()
    private val onDidFinishRenderingFrameList = CopyOnWriteArrayList<MapView.OnDidFinishRenderingFrameListener>()
    private val onDidFinishRenderingFrameWithStatsList =
        CopyOnWriteArrayList<MapView.OnDidFinishRenderingFrameWithStatsListener>()
    private val onWillStartRenderingMapListenerList = CopyOnWriteArrayList<MapView.OnWillStartRenderingMapListener>()
    private val onDidFinishRenderingMapListenerList = CopyOnWriteArrayList<MapView.OnDidFinishRenderingMapListener>()
    private val onDidBecomeIdleListenerList = CopyOnWriteArrayList<MapView.OnDidBecomeIdleListener>()
    private val onDidFinishLoadingStyleListenerList = CopyOnWriteArrayList<MapView.OnDidFinishLoadingStyleListener>()
    private val onSourceChangedListenerList = CopyOnWriteArrayList<MapView.OnSourceChangedListener>()
    private val onStyleImageMissingListenerList = CopyOnWriteArrayList<MapView.OnStyleImageMissingListener>()
    private val onCanRemoveUnusedStyleImageListenerList =
        CopyOnWriteArrayList<MapView.OnCanRemoveUnusedStyleImageListener>()
    private val onPreCompileShaderList = CopyOnWriteArrayList<MapView.OnPreCompileShaderListener>()
    private val onPostCompileShaderList = CopyOnWriteArrayList<MapView.OnPostCompileShaderListener>()
    private val onShaderCompileFailedList = CopyOnWriteArrayList<MapView.OnShaderCompileFailedListener>()
    private val onGlyphsLoadedList = CopyOnWriteArrayList<MapView.OnGlyphsLoadedListener>()
    private val onGlyphsErrorList = CopyOnWriteArrayList<MapView.OnGlyphsErrorListener>()
    private val onGlyphsRequestedList = CopyOnWriteArrayList<MapView.OnGlyphsRequestedListener>()
    private val onTileActionList = CopyOnWriteArrayList<MapView.OnTileActionListener>()
    private val onSpriteLoadedList = CopyOnWriteArrayList<MapView.OnSpriteLoadedListener>()
    private val onSpriteErrorList = CopyOnWriteArrayList<MapView.OnSpriteErrorListener>()
    private val onSpriteRequestedList = CopyOnWriteArrayList<MapView.OnSpriteRequestedListener>()
    private val onRenderErrorList = CopyOnWriteArrayList<MapView.OnRenderErrorListener>()
    private val onSymbolErrorList = CopyOnWriteArrayList<MapView.OnSymbolErrorListener>()

    override fun onCameraWillChange(animated: Boolean) {
        try {
            for (listener in onCameraWillChangeListenerList) {
                listener.onCameraWillChange(animated)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onCameraWillChange", err)
            throw err
        }
    }

    override fun onCameraIsChanging() {
        try {
            for (listener in onCameraIsChangingListenerList) {
                listener.onCameraIsChanging()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onCameraIsChanging", err)
            throw err
        }
    }

    override fun onCameraDidChange(animated: Boolean) {
        try {
            for (listener in onCameraDidChangeListenerList) {
                listener.onCameraDidChange(animated)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onCameraDidChange", err)
            throw err
        }
    }

    override fun onWillStartLoadingMap() {
        try {
            for (listener in onWillStartLoadingMapListenerList) {
                listener.onWillStartLoadingMap()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onWillStartLoadingMap", err)
            throw err
        }
    }

    override fun onDidFinishLoadingMap() {
        try {
            for (listener in onDidFinishLoadingMapListenerList) {
                listener.onDidFinishLoadingMap()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onDidFinishLoadingMap", err)
            throw err
        }
    }

    override fun onDidFailLoadingMap(error: String) {
        try {
            for (listener in onDidFailLoadingMapListenerList) {
                listener.onDidFailLoadingMap(error)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onDidFailLoadingMap", err)
            throw err
        }
    }

    override fun onWillStartRenderingFrame() {
        try {
            for (listener in onWillStartRenderingFrameList) {
                listener.onWillStartRenderingFrame()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onWillStartRenderingFrame", err)
            throw err
        }
    }

    override fun onDidFinishRenderingFrame(
        fully: Boolean,
        stats: RenderingStats,
    ) {
        try {
            for (listener in onDidFinishRenderingFrameList) {
                listener.onDidFinishRenderingFrame(fully, stats.encodingTime, stats.renderingTime)
            }

            for (listener in onDidFinishRenderingFrameWithStatsList) {
                listener.onDidFinishRenderingFrame(fully, stats)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onDidFinishRenderingFrame", err)
            throw err
        }
    }

    override fun onWillStartRenderingMap() {
        try {
            for (listener in onWillStartRenderingMapListenerList) {
                listener.onWillStartRenderingMap()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onWillStartRenderingMap", err)
            throw err
        }
    }

    override fun onDidFinishRenderingMap(fully: Boolean) {
        try {
            for (listener in onDidFinishRenderingMapListenerList) {
                listener.onDidFinishRenderingMap(fully)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onDidFinishRenderingMap", err)
            throw err
        }
    }

    override fun onDidBecomeIdle() {
        try {
            for (listener in onDidBecomeIdleListenerList) {
                listener.onDidBecomeIdle()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onDidBecomeIdle", err)
            throw err
        }
    }

    override fun onDidFinishLoadingStyle() {
        try {
            for (listener in onDidFinishLoadingStyleListenerList) {
                listener.onDidFinishLoadingStyle()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onDidFinishLoadingStyle", err)
            throw err
        }
    }

    override fun onSourceChanged(sourceId: String) {
        try {
            for (listener in onSourceChangedListenerList) {
                listener.onSourceChangedListener(sourceId)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onSourceChanged", err)
            throw err
        }
    }

    override fun onStyleImageMissing(imageId: String) {
        try {
            for (listener in onStyleImageMissingListenerList) {
                listener.onStyleImageMissing(imageId)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onStyleImageMissing", err)
            throw err
        }
    }

    override fun onCanRemoveUnusedStyleImage(imageId: String): Boolean {
        if (onCanRemoveUnusedStyleImageListenerList.isEmpty()) {
            return true
        }

        try {
            var canRemove = true
            for (listener in onCanRemoveUnusedStyleImageListenerList) {
                canRemove = canRemove and listener.onCanRemoveUnusedStyleImage(imageId)
            }
            return canRemove
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onCanRemoveUnusedStyleImage", err)
            throw err
        }
    }

    override fun onPreCompileShader(
        id: Int,
        type: Int,
        additionalDefines: String,
    ) {
        try {
            for (listener in onPreCompileShaderList) {
                listener.onPreCompileShader(id, type, additionalDefines)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onPreCompileShader", err)
            throw err
        }
    }

    override fun onPostCompileShader(
        id: Int,
        type: Int,
        additionalDefines: String,
    ) {
        try {
            for (listener in onPostCompileShaderList) {
                listener.onPostCompileShader(id, type, additionalDefines)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onPostCompileShader", err)
            throw err
        }
    }

    override fun onShaderCompileFailed(
        id: Int,
        type: Int,
        additionalDefines: String,
    ) {
        try {
            for (listener in onShaderCompileFailedList) {
                listener.onShaderCompileFailed(id, type, additionalDefines)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onShaderCompileFailed", err)
            throw err
        }
    }

    override fun onGlyphsLoaded(
        stack: Array<String>,
        rangeStart: Int,
        rangeEnd: Int,
    ) {
        try {
            for (listener in onGlyphsLoadedList) {
                listener.onGlyphsLoaded(stack, rangeStart, rangeEnd)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onGlyphsLoaded", err)
            throw err
        }
    }

    override fun onGlyphsError(
        stack: Array<String>,
        rangeStart: Int,
        rangeEnd: Int,
    ) {
        try {
            for (listener in onGlyphsErrorList) {
                listener.onGlyphsError(stack, rangeStart, rangeEnd)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onGlyphsError", err)
            throw err
        }
    }

    override fun onGlyphsRequested(
        stack: Array<String>,
        rangeStart: Int,
        rangeEnd: Int,
    ) {
        try {
            for (listener in onGlyphsRequestedList) {
                listener.onGlyphsRequested(stack, rangeStart, rangeEnd)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onGlyphsRequested", err)
            throw err
        }
    }

    @Suppress("LongParameterList")
    override fun onTileAction(
        op: TileOperation,
        x: Int,
        y: Int,
        z: Int,
        wrap: Int,
        overscaledZ: Int,
        sourceID: String,
    ) {
        try {
            for (listener in onTileActionList) {
                listener.onTileAction(op, x, y, z, wrap, overscaledZ, sourceID)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onTileAction", err)
            throw err
        }
    }

    override fun onSpriteLoaded(
        id: String,
        url: String,
    ) {
        try {
            for (listener in onSpriteLoadedList) {
                listener.onSpriteLoaded(id, url)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onSpriteLoaded", err)
            throw err
        }
    }

    override fun onSpriteError(
        id: String,
        url: String,
    ) {
        try {
            for (listener in onSpriteErrorList) {
                listener.onSpriteError(id, url)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onSpriteError", err)
            throw err
        }
    }

    override fun onSpriteRequested(
        id: String,
        url: String,
    ) {
        try {
            for (listener in onSpriteRequestedList) {
                listener.onSpriteRequested(id, url)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onSpriteRequested", err)
            throw err
        }
    }

    override fun onRenderError() {
        try {
            for (listener in onRenderErrorList) {
                listener.onRenderError()
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onRenderError", err)
            throw err
        }
    }

    override fun onSymbolError(message: String) {
        try {
            for (listener in onSymbolErrorList) {
                listener.onSymbolError(message)
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onSymbolError", err)
        }
    }

    fun addOnCameraWillChangeListener(listener: MapView.OnCameraWillChangeListener) {
        onCameraWillChangeListenerList.add(listener)
    }

    fun removeOnCameraWillChangeListener(listener: MapView.OnCameraWillChangeListener) {
        onCameraWillChangeListenerList.remove(listener)
    }

    fun addOnCameraIsChangingListener(listener: MapView.OnCameraIsChangingListener) {
        onCameraIsChangingListenerList.add(listener)
    }

    fun removeOnCameraIsChangingListener(listener: MapView.OnCameraIsChangingListener) {
        onCameraIsChangingListenerList.remove(listener)
    }

    fun addOnCameraDidChangeListener(listener: MapView.OnCameraDidChangeListener) {
        onCameraDidChangeListenerList.add(listener)
    }

    fun removeOnCameraDidChangeListener(listener: MapView.OnCameraDidChangeListener) {
        onCameraDidChangeListenerList.remove(listener)
    }

    fun addOnWillStartLoadingMapListener(listener: MapView.OnWillStartLoadingMapListener) {
        onWillStartLoadingMapListenerList.add(listener)
    }

    fun removeOnWillStartLoadingMapListener(listener: MapView.OnWillStartLoadingMapListener) {
        onWillStartLoadingMapListenerList.remove(listener)
    }

    fun addOnDidFinishLoadingMapListener(listener: MapView.OnDidFinishLoadingMapListener) {
        onDidFinishLoadingMapListenerList.add(listener)
    }

    fun removeOnDidFinishLoadingMapListener(listener: MapView.OnDidFinishLoadingMapListener) {
        onDidFinishLoadingMapListenerList.remove(listener)
    }

    fun addOnDidFailLoadingMapListener(listener: MapView.OnDidFailLoadingMapListener) {
        onDidFailLoadingMapListenerList.add(listener)
    }

    fun removeOnDidFailLoadingMapListener(listener: MapView.OnDidFailLoadingMapListener) {
        onDidFailLoadingMapListenerList.remove(listener)
    }

    fun addOnWillStartRenderingFrameListener(listener: MapView.OnWillStartRenderingFrameListener) {
        onWillStartRenderingFrameList.add(listener)
    }

    fun removeOnWillStartRenderingFrameListener(listener: MapView.OnWillStartRenderingFrameListener) {
        onWillStartRenderingFrameList.remove(listener)
    }

    fun addOnDidFinishRenderingFrameListener(listener: MapView.OnDidFinishRenderingFrameListener) {
        onDidFinishRenderingFrameList.add(listener)
    }

    fun removeOnDidFinishRenderingFrameListener(listener: MapView.OnDidFinishRenderingFrameListener) {
        onDidFinishRenderingFrameList.remove(listener)
    }

    fun addOnDidFinishRenderingFrameListener(listener: MapView.OnDidFinishRenderingFrameWithStatsListener) {
        onDidFinishRenderingFrameWithStatsList.add(listener)
    }

    fun removeOnDidFinishRenderingFrameListener(listener: MapView.OnDidFinishRenderingFrameWithStatsListener) {
        onDidFinishRenderingFrameWithStatsList.remove(listener)
    }

    fun addOnWillStartRenderingMapListener(listener: MapView.OnWillStartRenderingMapListener) {
        onWillStartRenderingMapListenerList.add(listener)
    }

    fun removeOnWillStartRenderingMapListener(listener: MapView.OnWillStartRenderingMapListener) {
        onWillStartRenderingMapListenerList.remove(listener)
    }

    fun addOnDidFinishRenderingMapListener(listener: MapView.OnDidFinishRenderingMapListener) {
        onDidFinishRenderingMapListenerList.add(listener)
    }

    fun removeOnDidFinishRenderingMapListener(listener: MapView.OnDidFinishRenderingMapListener) {
        onDidFinishRenderingMapListenerList.remove(listener)
    }

    fun addOnDidBecomeIdleListener(listener: MapView.OnDidBecomeIdleListener) {
        onDidBecomeIdleListenerList.add(listener)
    }

    fun removeOnDidBecomeIdleListener(listener: MapView.OnDidBecomeIdleListener) {
        onDidBecomeIdleListenerList.remove(listener)
    }

    fun addOnDidFinishLoadingStyleListener(listener: MapView.OnDidFinishLoadingStyleListener) {
        onDidFinishLoadingStyleListenerList.add(listener)
    }

    fun removeOnDidFinishLoadingStyleListener(listener: MapView.OnDidFinishLoadingStyleListener) {
        onDidFinishLoadingStyleListenerList.remove(listener)
    }

    fun addOnSourceChangedListener(listener: MapView.OnSourceChangedListener) {
        onSourceChangedListenerList.add(listener)
    }

    fun removeOnSourceChangedListener(listener: MapView.OnSourceChangedListener) {
        onSourceChangedListenerList.remove(listener)
    }

    fun addOnStyleImageMissingListener(listener: MapView.OnStyleImageMissingListener) {
        onStyleImageMissingListenerList.add(listener)
    }

    fun removeOnStyleImageMissingListener(listener: MapView.OnStyleImageMissingListener) {
        onStyleImageMissingListenerList.remove(listener)
    }

    fun addOnCanRemoveUnusedStyleImageListener(listener: MapView.OnCanRemoveUnusedStyleImageListener) {
        onCanRemoveUnusedStyleImageListenerList.add(listener)
    }

    fun removeOnCanRemoveUnusedStyleImageListener(listener: MapView.OnCanRemoveUnusedStyleImageListener) {
        onCanRemoveUnusedStyleImageListenerList.remove(listener)
    }

    fun addOnPreCompileShaderListener(callback: MapView.OnPreCompileShaderListener) {
        onPreCompileShaderList.add(callback)
    }

    fun addOnPostCompileShaderListener(callback: MapView.OnPostCompileShaderListener) {
        onPostCompileShaderList.add(callback)
    }

    fun addOnShaderCompileFailedListener(callback: MapView.OnShaderCompileFailedListener) {
        onShaderCompileFailedList.add(callback)
    }

    fun addOnGlyphsLoadedListener(callback: MapView.OnGlyphsLoadedListener) {
        onGlyphsLoadedList.add(callback)
    }

    fun addOnGlyphsErrorListener(callback: MapView.OnGlyphsErrorListener) {
        onGlyphsErrorList.add(callback)
    }

    fun addOnGlyphsRequestedListener(callback: MapView.OnGlyphsRequestedListener) {
        onGlyphsRequestedList.add(callback)
    }

    fun addOnTileActionListener(callback: MapView.OnTileActionListener) {
        onTileActionList.add(callback)
    }

    fun addOnSpriteLoadedListener(callback: MapView.OnSpriteLoadedListener) {
        onSpriteLoadedList.add(callback)
    }

    fun addOnSpriteErrorListener(callback: MapView.OnSpriteErrorListener) {
        onSpriteErrorList.add(callback)
    }

    fun addOnSpriteRequestedListener(callback: MapView.OnSpriteRequestedListener) {
        onSpriteRequestedList.add(callback)
    }

    fun addOnRenderErrorListener(callback: MapView.OnRenderErrorListener) {
        onRenderErrorList.add(callback)
    }

    fun addOnSymbolErrorListener(callback: MapView.OnSymbolErrorListener) {
        onSymbolErrorList.add(callback)
    }

    fun removeOnPreCompileShaderListener(callback: MapView.OnPreCompileShaderListener) {
        onPreCompileShaderList.remove(callback)
    }

    fun removeOnPostCompileShaderListener(callback: MapView.OnPostCompileShaderListener) {
        onPostCompileShaderList.remove(callback)
    }

    fun removeOnShaderCompileFailedListener(callback: MapView.OnShaderCompileFailedListener) {
        onShaderCompileFailedList.remove(callback)
    }

    fun removeOnGlyphsLoadedListener(callback: MapView.OnGlyphsLoadedListener) {
        onGlyphsLoadedList.remove(callback)
    }

    fun removeOnGlyphsErrorListener(callback: MapView.OnGlyphsErrorListener) {
        onGlyphsErrorList.remove(callback)
    }

    fun removeOnGlyphsRequestedListener(callback: MapView.OnGlyphsRequestedListener) {
        onGlyphsRequestedList.remove(callback)
    }

    fun removeOnTileActionListener(callback: MapView.OnTileActionListener) {
        onTileActionList.remove(callback)
    }

    fun removeOnSpriteLoadedListener(callback: MapView.OnSpriteLoadedListener) {
        onSpriteLoadedList.remove(callback)
    }

    fun removeOnSpriteErrorListener(callback: MapView.OnSpriteErrorListener) {
        onSpriteErrorList.remove(callback)
    }

    fun removeOnSpriteRequestedListener(callback: MapView.OnSpriteRequestedListener) {
        onSpriteRequestedList.remove(callback)
    }

    fun removeOnRenderErrorListener(callback: MapView.OnRenderErrorListener) {
        onRenderErrorList.remove(callback)
    }

    fun removeOnSymbolErrorListener(callback: MapView.OnSymbolErrorListener) {
        onSymbolErrorList.remove(callback)
    }

    fun clear() {
        onCameraWillChangeListenerList.clear()
        onCameraIsChangingListenerList.clear()
        onCameraDidChangeListenerList.clear()
        onWillStartLoadingMapListenerList.clear()
        onDidFinishLoadingMapListenerList.clear()
        onDidFailLoadingMapListenerList.clear()
        onWillStartRenderingFrameList.clear()
        onDidFinishRenderingFrameList.clear()
        onWillStartRenderingMapListenerList.clear()
        onDidFinishRenderingMapListenerList.clear()
        onDidBecomeIdleListenerList.clear()
        onDidFinishLoadingStyleListenerList.clear()
        onSourceChangedListenerList.clear()
        onStyleImageMissingListenerList.clear()
        onCanRemoveUnusedStyleImageListenerList.clear()
        onPreCompileShaderList.clear()
        onPostCompileShaderList.clear()
        onShaderCompileFailedList.clear()
        onGlyphsLoadedList.clear()
        onGlyphsErrorList.clear()
        onGlyphsRequestedList.clear()
        onTileActionList.clear()
        onSpriteLoadedList.clear()
        onSpriteErrorList.clear()
        onSpriteRequestedList.clear()
        onRenderErrorList.clear()
        onSymbolErrorList.clear()
    }

    private companion object {
        const val TAG = "Mbgl-MapChangeReceiver"
    }
}
